#include "../Source/PluginProcessor.h"
#include <iostream>
static int fails = 0;
static void check (bool ok, const juce::String& m) { std::cout << (ok ? "PASS " : "FAIL ") << m << "\n"; if (! ok) ++fails; }
static void set (VegaVocalProcessor& p, const char* id, float v) { auto* q = p.apvts.getParameter (id); q->setValueNotifyingHost (q->convertTo0to1 (v)); }
const double fs = 48000; const int blk = 480;
using Gen = std::function<float (int)>;
static juce::AudioBuffer<float> run (VegaVocalProcessor& p, int n, Gen gen)
{
    juce::AudioBuffer<float> out (2, n), b (2, blk); juce::MidiBuffer m;
    for (int pos = 0; pos < n; pos += blk)
    {
        for (int i = 0; i < blk; ++i) { const float x = gen (pos + i); b.setSample (0, i, x); b.setSample (1, i, x); }
        p.processBlock (b, m);
        for (int c = 0; c < 2; ++c) out.copyFrom (c, pos, b, c, 0, blk);
    }
    return out;
}
static double rms (const juce::AudioBuffer<float>& b, int ch, int a, int e) { double s = 0; for (int i = a; i < e; ++i) s += b.getSample (ch, i) * b.getSample (ch, i); return std::sqrt (s / (e - a)); }
static float sine (double f, int i, float amp) { return amp * (float) std::sin (2 * juce::MathConstants<double>::pi * f * i / fs); }
static void allOff (VegaVocalProcessor& p) { for (auto* id : { "gateOn", "deessOn", "compOn", "doublerOn" }) set (p, id, 0); set (p, "lowCut", 20); set (p, "presence", 0); set (p, "air", 0); }

int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    auto make = [&] { auto* p = new VegaVocalProcessor(); p->setPlayConfigDetails (2, 2, fs, blk); p->prepareToPlay (fs, blk); return std::unique_ptr<VegaVocalProcessor> (p); };
    {
        auto p = make(); allOff (*p); p->prepareToPlay (fs, blk);
        auto o = run (*p, 24000, [] (int i) { return sine (440, i, 0.3f); });
        double err = 0; for (int i = 0; i < 24000; ++i) err = std::max (err, (double) std::abs (o.getSample (0, i) - sine (440, i, 0.3f)));
        check (err < 1e-4, "all modules off = transparent (" + juce::String (err, 7) + ")");
    }
    {   // de-esser: loud 7 kHz "ess" is cut, 300 Hz vowel is not
        auto p = make(); allOff (*p); set (*p, "deessOn", 1); set (*p, "deessThreshold", -30); set (*p, "deessRange", 12);
        auto s = run (*p, 24000, [] (int i) { return sine (7500, i, 0.3f); });
        auto v = run (*p, 24000, [] (int i) { return sine (300, i, 0.3f); });
        const double sDb = 20 * std::log10 (rms (s, 0, 12000, 24000) / (0.3 / std::sqrt (2.0))), vDb = 20 * std::log10 (rms (v, 0, 12000, 24000) / (0.3 / std::sqrt (2.0)));
        check (sDb < -6 && std::abs (vDb) < 0.5, "de-esser: 7.5 kHz cut " + juce::String (sDb, 1) + " dB, 300 Hz " + juce::String (vDb, 2) + " dB");
    }
    {   // gate: quiet noise between phrases is reduced, singing is not
        auto p = make(); allOff (*p); set (*p, "gateOn", 1); set (*p, "gateThreshold", -45); set (*p, "gateRange", 20);
        auto o = run (*p, 48000, [] (int i) { return i < 24000 ? sine (220, i, 0.3f) : sine (220, i, 0.002f); });
        const double loud = 20 * std::log10 (rms (o, 0, 4800, 24000) / (0.3 / std::sqrt (2.0))), quiet = 20 * std::log10 (rms (o, 0, 40000, 48000) / (0.002 / std::sqrt (2.0)));
        check (std::abs (loud) < 0.5 && quiet < -15, "gate: phrase untouched (" + juce::String (loud, 2) + " dB), gap reduced (" + juce::String (quiet, 1) + " dB)");
    }
    {   // leveler narrows the gap between a loud and a quiet phrase
        auto p = make(); allOff (*p); set (*p, "compOn", 1); set (*p, "compAmount", 80);
        auto o = run (*p, 96000, [] (int i) { return i < 48000 ? sine (220, i, 0.5f) : sine (220, i, 0.05f); });
        const double gapIn = 20.0, gapOut = 20 * std::log10 (rms (o, 0, 24000, 48000) / rms (o, 0, 72000, 96000));
        check (gapOut < gapIn - 6, "leveler: 20 dB phrase difference becomes " + juce::String (gapOut, 1) + " dB");
    }
    {   // doubler makes a mono vocal stereo
        auto p = make(); allOff (*p); set (*p, "doublerOn", 1); set (*p, "doubler", 60); set (*p, "doublerWidth", 100);
        auto o = run (*p, 48000, [] (int i) { return sine (330, i, 0.3f) * (0.6f + 0.4f * (float) std::sin (i * 0.0007)); });
        double diff = 0; for (int i = 12000; i < 48000; ++i) diff += std::pow (o.getSample (0, i) - o.getSample (1, i), 2);
        check (std::sqrt (diff / 36000) > 0.03, "doubler widens mono input (L-R rms " + juce::String (std::sqrt (diff / 36000), 3) + ")");
    }
    {   // stability with everything on
        auto p = make(); set (*p, "doublerOn", 1); set (*p, "presence", 9); set (*p, "air", 9); set (*p, "compAmount", 100);
        juce::Random r (5); auto o = run (*p, 96000, [&r] (int) { return (r.nextFloat() - 0.5f) * 1.5f; });
        bool fin = true; for (int c = 0; c < 2; ++c) for (int i = 0; i < 96000; ++i) fin &= std::isfinite (o.getSample (c, i));
        check (fin, "everything on with loud noise stays finite");
    }
    {
        auto p = make(); set (*p, "deessFreq", 8000); juce::MemoryBlock mb; p->getStateInformation (mb);
        auto q = make(); q->setStateInformation (mb.getData(), (int) mb.getSize());
        check (std::abs (q->apvts.getRawParameterValue ("deessFreq")->load() - 8000) < 5, "state round trip");
    }
    std::cout << (fails == 0 ? "ALL PASSED" : "FAILURES: " + std::to_string (fails)) << "\n";
    return fails == 0 ? 0 : 1;
}
