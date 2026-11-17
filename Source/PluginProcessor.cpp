#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
enum { kLowCut, kGateOn, kGateThr, kGateRange, kGateRel, kDsOn, kDsFreq, kDsThr, kDsRange, kDsMode, kDsListen, kCompOn, kComp, kCompSpeed,
       kPresence, kAir, kDblOn, kDbl, kDblWidth, kMix, kOut };
const char* ids[] = { "lowCut", "gateOn", "gateThreshold", "gateRange", "gateRelease", "deessOn", "deessFreq", "deessThreshold", "deessRange", "deessMode", "deessListen",
                      "compOn", "compAmount", "compSpeed", "presence", "air", "doublerOn", "doubler", "doublerWidth", "mix", "output" };
constexpr int numP = 21;
inline float coef (double sr, float ms) { return 1.0f - std::exp (-1.0f / (float) (sr * ms * 0.001)); }
inline float toDb (float x) { return 20.0f * std::log10 (std::max (x, 1.0e-6f)); }
}

juce::AudioProcessorValueTreeState::ParameterLayout VegaVocalProcessor::layout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> v;
    auto pct = [] (float x, int) { return String (roundToInt (x)) + "%"; };
    auto db = [] (float x, int) { return String (x, 1) + " dB"; };
    auto sdb = [] (float x, int) { return (x > 0 ? "+" : "") + String (x, 1) + " dB"; };
    auto hz = [] (float x, int) { return x >= 1000 ? String (x / 1000, 1) + " kHz" : String (roundToInt (x)) + " Hz"; };
    auto ms = [] (float x, int) { return String (roundToInt (x)) + " ms"; };
    auto F = [&] (int id, const char* name, NormalisableRange<float> r, float def, std::function<String (float, int)> f)
    { v.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ids[id], 1 }, name, r, def, AudioParameterFloatAttributes().withStringFromValueFunction (f))); };
    auto B = [&] (int id, const char* name, bool def) { v.push_back (std::make_unique<AudioParameterBool> (ParameterID { ids[id], 1 }, name, def)); };
    F (kLowCut, "Low Cut", NormalisableRange<float> (20, 300, 0, 0.5f), 80, [hz] (float x, int k) { return x <= 21 ? String ("Off") : hz (x, k); });
    B (kGateOn, "Gate", true);
    F (kGateThr, "Gate Threshold", NormalisableRange<float> (-80, -10), -50, db);
    F (kGateRange, "Gate Range", NormalisableRange<float> (0, 40), 12, db);
    F (kGateRel, "Gate Release", NormalisableRange<float> (20, 500, 0, 0.5f), 120, ms);
    B (kDsOn, "De-Esser", true);
    F (kDsFreq, "De-Ess Freq", NormalisableRange<float> (3000, 12000, 0, 0.5f), 6500, hz);
    F (kDsThr, "De-Ess Threshold", NormalisableRange<float> (-60, 0), -28, db);
    F (kDsRange, "De-Ess Range", NormalisableRange<float> (0, 20), 10, db);
    v.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ids[kDsMode], 1 }, "De-Ess Mode", StringArray { "Split", "Wideband" }, 0));
    B (kDsListen, "Listen", false);
    B (kCompOn, "Leveler", true);
    F (kComp, "Leveler Amount", NormalisableRange<float> (0, 100), 40, pct);
    F (kCompSpeed, "Leveler Speed", NormalisableRange<float> (0, 100), 50, [] (float x, int) { return x < 33 ? String ("Slow") : x < 66 ? String ("Medium") : String ("Fast"); });
    F (kPresence, "Presence", NormalisableRange<float> (-6, 9), 2, sdb);
    F (kAir, "Air", NormalisableRange<float> (-6, 9), 3, sdb);
    B (kDblOn, "Doubler", false);
    F (kDbl, "Doubler Amount", NormalisableRange<float> (0, 100), 40, pct);
    F (kDblWidth, "Doubler Width", NormalisableRange<float> (0, 100), 80, pct);
    F (kMix, "Mix", NormalisableRange<float> (0, 100), 100, pct);
    F (kOut, "Output", NormalisableRange<float> (-24, 12), 0, db);
    return { v.begin(), v.end() };
}

VegaVocalProcessor::VegaVocalProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true).withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", layout())
{
    for (int i = 0; i < numP; ++i) p[i] = apvts.getRawParameterValue (ids[i]);
}

bool VegaVocalProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto o = l.getMainOutputChannelSet(), i = l.getMainInputChannelSet();
    return (o == juce::AudioChannelSet::stereo() || o == juce::AudioChannelSet::mono()) && (i == o || (i == juce::AudioChannelSet::mono() && o == juce::AudioChannelSet::stereo()));
}

void VegaVocalProcessor::prepareToPlay (double s, int block)
{
    sr = s;
    juce::dsp::ProcessSpec spec { s, (juce::uint32) block, 2 };
    dsLo.prepare (spec); dsHi.prepare (spec);
    dsLo.setType (juce::dsp::LinkwitzRileyFilterType::lowpass); dsHi.setType (juce::dsp::LinkwitzRileyFilterType::highpass);
    for (int c = 0; c < 2; ++c) { lowCut[c].reset(); presence[c].reset(); air[c].reset(); }
    sibDetect.reset();
    lastLowCut = lastPres = lastAir = lastDsFreq = -1000;
    for (auto& d : dbl) d.assign ((size_t) (s * 0.06) + 4, 0.0f);
    dpos = 0; gateEnv = dsEnv = compEnv = 0; gateGain = dsGain = compGain = 1;
}

void VegaVocalProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals nd;
    const int n = buffer.getNumSamples(), inCh = getTotalNumInputChannels(), outCh = std::min (2, getTotalNumOutputChannels());
    if (n == 0 || outCh == 0) return;
    if (inCh == 1 && outCh == 2) buffer.copyFrom (1, 0, buffer, 0, 0, n);
    const int nch = outCh;

    // ---- coefficient updates
    const float lc = p[kLowCut]->load(), pres = p[kPresence]->load(), airDb = p[kAir]->load(), dsF = p[kDsFreq]->load();
    if (lc != lastLowCut) { lastLowCut = lc; auto c = juce::dsp::IIR::Coefficients<float>::makeHighPass (sr, lc, 0.707f); for (auto& f : lowCut) f.coefficients = c; }
    if (pres != lastPres) { lastPres = pres; auto c = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sr, 3200.0, 0.8f, juce::Decibels::decibelsToGain (pres)); for (auto& f : presence) f.coefficients = c; }
    if (airDb != lastAir) { lastAir = airDb; auto c = juce::dsp::IIR::Coefficients<float>::makeHighShelf (sr, std::min (12000.0, sr * 0.4), 0.7f, juce::Decibels::decibelsToGain (airDb)); for (auto& f : air) f.coefficients = c; }
    if (dsF != lastDsFreq) { lastDsFreq = dsF; sibDetect.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (sr, dsF, 0.9f); dsLo.setCutoffFrequency (dsF * 0.7f); dsHi.setCutoffFrequency (dsF * 0.7f); }

    const bool gateOn = p[kGateOn]->load() > 0.5f, dsOn = p[kDsOn]->load() > 0.5f, compOn = p[kCompOn]->load() > 0.5f, dblOn = p[kDblOn]->load() > 0.5f;
    const bool listen = p[kDsListen]->load() > 0.5f, split = p[kDsMode]->load() < 0.5f;
    const float gThr = p[kGateThr]->load(), gRange = p[kGateRange]->load();
    const float gRel = coef (sr, p[kGateRel]->load()), gAtk = coef (sr, 0.5f), gDet = coef (sr, 2.0f);
    const float dThr = p[kDsThr]->load(), dRange = p[kDsRange]->load();
    const float dAtk = coef (sr, 0.5f), dRel = coef (sr, 60.0f), dDetA = coef (sr, 0.2f), dDetR = coef (sr, 20.0f);
    const float amt = p[kComp]->load() * 0.01f, speed = p[kCompSpeed]->load() * 0.01f;
    const float cThr = -8.0f - amt * 30.0f, cRatio = 2.0f + amt * 3.0f, cKnee = 8.0f;
    const float cAtk = coef (sr, 25.0f - speed * 22.0f), cRel = coef (sr, 300.0f - speed * 240.0f), cDet = coef (sr, 8.0f);
    const float makeup = compOn ? juce::Decibels::decibelsToGain (-cThr * (1.0f - 1.0f / cRatio) * 0.45f) : 1.0f;
    const float dblAmt = p[kDbl]->load() * 0.01f, dblW = p[kDblWidth]->load() * 0.01f;
    const float mix = p[kMix]->load() * 0.01f, outG = juce::Decibels::decibelsToGain (p[kOut]->load());
    const int dn = (int) dbl[0].size();

    float* ch[2] { buffer.getWritePointer (0), buffer.getWritePointer (nch > 1 ? 1 : 0) };
    float maxG = 0, maxD = 0, maxC = 0, sibPk = 0, inPk = 0, pk[2] {};
    for (int i = 0; i < n; ++i)
    {
        float x[2] { ch[0][i], ch[1][i] };
        const float dry[2] { x[0], x[1] };
        if (lc > 21) for (int c = 0; c < 2; ++c) x[c] = lowCut[c].processSample (x[c]);

        // ---- gate / breath control (downward expander)
        const float lvl = std::max (std::abs (x[0]), std::abs (x[1]));
        gateEnv += gDet * (lvl - gateEnv);
        const float envDb = toDb (gateEnv);
        inPk = std::max (inPk, lvl);
        if (gateOn)
        {
            const float under = juce::jlimit (0.0f, 1.0f, (gThr - envDb) / 6.0f);
            const float target = juce::Decibels::decibelsToGain (-gRange * under);
            gateGain += (target > gateGain ? gAtk : gRel) * (target - gateGain);
            for (auto& v : x) v *= gateGain;
            maxG = std::max (maxG, -toDb (gateGain));
        }

        // ---- de-esser
        const float sib = std::abs (sibDetect.processSample (0.5f * (x[0] + x[1])));
        dsEnv += (sib > dsEnv ? dDetA : dDetR) * (sib - dsEnv);
        const float sibDb = toDb (dsEnv);
        sibPk = std::max (sibPk, dsEnv);
        float target = 1.0f;
        if (dsOn) target = juce::Decibels::decibelsToGain (-std::min (dRange, std::max (0.0f, (sibDb - dThr) * 0.75f)));
        dsGain += (target < dsGain ? dAtk : dRel) * (target - dsGain);
        maxD = std::max (maxD, -toDb (dsGain));
        for (int c = 0; c < 2; ++c)
        {
            const float lo = dsLo.processSample (c, x[c]), hi = dsHi.processSample (c, x[c]);
            if (listen) x[c] = hi;
            else if (dsOn) x[c] = split ? lo + hi * dsGain : x[c] * dsGain;
        }

        // ---- leveler (soft-knee compressor with auto makeup)
        if (compOn && ! listen)
        {
            const float sq = std::max (x[0] * x[0], x[1] * x[1]);
            compEnv += cDet * (sq - compEnv);
            const float l = 10.0f * std::log10 (std::max (compEnv, 1.0e-12f));
            const float over = l - cThr;
            float red = 0;
            if (over > cKnee * 0.5f) red = over * (1.0f - 1.0f / cRatio);
            else if (over > -cKnee * 0.5f) { const float t = over + cKnee * 0.5f; red = (1.0f - 1.0f / cRatio) * t * t / (2.0f * cKnee); }
            const float tgt = juce::Decibels::decibelsToGain (-red);
            compGain += (tgt < compGain ? cAtk : cRel) * (tgt - compGain);
            maxC = std::max (maxC, -toDb (compGain));
            for (auto& v : x) v *= compGain * makeup;
        }

        // ---- presence / air
        if (! listen)
            for (int c = 0; c < 2; ++c)
            {
                if (std::abs (pres) > 0.05f) x[c] = presence[c].processSample (x[c]);
                if (std::abs (airDb) > 0.05f) x[c] = air[c].processSample (x[c]);
            }

        // ---- doubler: two slightly detuned, delayed copies panned apart
        const float mono = 0.5f * (x[0] + x[1]);
        dbl[0][(size_t) dpos] = mono;
        if (dblOn && ! listen)
        {
            float tap[2];
            for (int c = 0; c < 2; ++c)
            {
                modPh[c] += (c == 0 ? 0.83 : 1.07) / sr; if (modPh[c] >= 1) modPh[c] -= 1;
                const double dMs = (c == 0 ? 17.0 : 26.0) + 0.35 * std::sin (juce::MathConstants<double>::twoPi * modPh[c]);
                double rp = dpos - dMs * 0.001 * sr; while (rp < 0) rp += dn;
                const int i0 = (int) rp; const float f = (float) (rp - i0);
                tap[c] = dbl[0][(size_t) i0] + (dbl[0][(size_t) ((i0 + 1) % dn)] - dbl[0][(size_t) i0]) * f;
            }
            const float side = dblW;
            x[0] += dblAmt * (tap[0] * (0.5f + 0.5f * side) + tap[1] * (0.5f - 0.5f * side));
            x[1] += dblAmt * (tap[1] * (0.5f + 0.5f * side) + tap[0] * (0.5f - 0.5f * side));
        }
        if (++dpos >= dn) dpos = 0;

        for (int c = 0; c < nch; ++c)
        {
            const float y = (listen ? x[c] : dry[c] + (x[c] - dry[c]) * mix) * outG;
            ch[c][i] = y; pk[c] = std::max (pk[c], std::abs (y));
        }
    }
    grGate.store (maxG); grDess.store (maxD); grComp.store (maxC);
    sibLevel.store (toDb (sibPk)); inLevel.store (toDb (inPk));
    outL.store (std::max (outL.load(), pk[0])); outR.store (std::max (outR.load(), nch > 1 ? pk[1] : pk[0]));
}

void VegaVocalProcessor::getStateInformation (juce::MemoryBlock& d)
{
    auto s = apvts.copyState(); s.setProperty ("uiScale", uiScale.load(), nullptr);
    if (auto x = s.createXml()) copyXmlToBinary (*x, d);
}
void VegaVocalProcessor::setStateInformation (const void* data, int size)
{
    if (auto x = getXmlFromBinary (data, size))
    {
        auto s = juce::ValueTree::fromXml (*x);
        if (! s.hasType (apvts.state.getType())) return;
        uiScale.store ((float) s.getProperty ("uiScale", 1.0f));
        apvts.replaceState (s);
    }
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new VegaVocalProcessor(); }
