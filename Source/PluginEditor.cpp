#include "PluginEditor.h"

class VegaVocalEditor::Main : public juce::Component
{
public:
    explicit Main (VegaVocalProcessor& p) : proc (p), a (p.apvts),
        lowCut (a, "lowCut", "Low Cut"), gateOn (a, "gateOn", "Gate"), gateThr (a, "gateThreshold", "Threshold"), gateRange (a, "gateRange", "Range"), gateRel (a, "gateRelease", "Release"),
        dsOn (a, "deessOn", "De-Ess"), dsFreq (a, "deessFreq", "Frequency"), dsThr (a, "deessThreshold", "Threshold"), dsRange (a, "deessRange", "Range"), dsMode (a, "deessMode", "Mode"), dsListen (a, "deessListen", "Listen"),
        compOn (a, "compOn", "Leveler"), comp (a, "compAmount", "AMOUNT", true), compSpeed (a, "compSpeed", "Speed"),
        presence (a, "presence", "Presence"), air (a, "air", "Air"),
        dblOn (a, "doublerOn", "Doubler"), dbl (a, "doubler", "Amount"), dblW (a, "doublerWidth", "Width"),
        mix (a, "mix", "Mix"), output (a, "output", "Output")
    {
        for (auto* c : std::initializer_list<juce::Component*> { &lowCut, &gateOn, &gateThr, &gateRange, &gateRel, &dsOn, &dsFreq, &dsThr, &dsRange, &dsMode, &dsListen,
                                                                 &compOn, &comp, &compSpeed, &presence, &air, &dblOn, &dbl, &dblW, &mix, &output, &meter, &gGate, &gDs, &gComp, &sib })
            addAndMakeVisible (c);
        gGate.title = "GATE"; gDs.title = "S"; gComp.title = "GR"; gGate.maxDb = 24; gDs.maxDb = 12; gComp.maxDb = 12;
    }
    void refresh()
    {
        meter.setLevels (proc.outL.exchange (0.0f), proc.outR.exchange (0.0f));
        gGate.setGr (proc.grGate.load()); gDs.setGr (proc.grDess.load()); gComp.setGr (proc.grComp.load());
        sib.setLines ("SIBILANCE", juce::String (std::max (-80.0f, proc.sibLevel.load()), 1) + " dB", "THRESH", juce::String (a.getRawParameterValue ("deessThreshold")->load(), 1), "CUT", "-" + juce::String (proc.grDess.load(), 1));
        auto en = [this] (const char* id, std::initializer_list<juce::Component*> cs) { const bool on = a.getRawParameterValue (id)->load() > 0.5f; for (auto* c : cs) c->setEnabled (on); };
        en ("gateOn", { &gateThr, &gateRange, &gateRel }); en ("deessOn", { &dsFreq, &dsThr, &dsRange, &dsMode }); en ("compOn", { &comp, &compSpeed }); en ("doublerOn", { &dbl, &dblW });
    }
    void paint (juce::Graphics& g) override
    {
        g.fillAll (VegaColours::background);
        vegaui::drawTopBar (g, W, "VOCAL  VP-15");
        vegaui::drawBody (g, { 0, 35, W, H - 65 });
        vegaui::drawSection (g, { 0, 40, 250, 396 }, "1  CLEAN", false);
        vegaui::drawSection (g, { 250, 40, 300, 396 }, "2  DE-ESS");
        vegaui::drawSection (g, { 550, 40, 170, 396 }, "3  LEVEL");
        vegaui::drawSection (g, { 720, 40, 140, 396 }, "4  TONE");
        vegaui::drawSection (g, { 860, 40, 130, 396 }, "5  DOUBLE");
        vegaui::drawSection (g, { 990, 40, W - 990, 396 }, "OUTPUT");
        vegaui::drawBottomBar (g, { 0, H - 30, W, 30 }, "CLEAN  >  DE-ESS  >  LEVEL  >  TONE  >  DOUBLE   -   LISTEN SOLOS WHAT THE DE-ESSER HEARS");
    }
    void resized() override
    {
        lowCut.setBounds (14, 60, 90, 108); gateOn.setBounds (120, 60, 70, 80); gGate.setBounds (200, 66, 40, 150);
        gateThr.setBounds (14, 180, 90, 108); gateRange.setBounds (110, 180, 84, 108); gateRel.setBounds (14, 300, 90, 108);
        dsOn.setBounds (262, 60, 70, 80); dsListen.setBounds (336, 60, 70, 80); gDs.setBounds (506, 66, 36, 150);
        dsFreq.setBounds (262, 180, 90, 108); dsThr.setBounds (356, 180, 90, 108); dsRange.setBounds (262, 300, 90, 108);
        dsMode.setBounds (360, 310, 136, 46); sib.setBounds (412, 70, 90, 96);
        compOn.setBounds (566, 60, 70, 80); gComp.setBounds (670, 66, 40, 150); comp.setBounds (560, 180, 150, 150); compSpeed.setBounds (590, 330, 90, 100);
        presence.setBounds (735, 70, 110, 120); air.setBounds (735, 220, 110, 120);
        dblOn.setBounds (890, 60, 70, 80); dbl.setBounds (875, 180, 100, 108); dblW.setBounds (875, 300, 100, 108);
        meter.setBounds (994, 56, 62, 220); mix.setBounds (1054, 60, 64, 108); output.setBounds (1054, 180, 64, 108);
    }
private:
    VegaVocalProcessor& proc; juce::AudioProcessorValueTreeState& a;
    vegaui::Knob lowCut; vegaui::Toggle gateOn; vegaui::Knob gateThr, gateRange, gateRel;
    vegaui::Toggle dsOn; vegaui::Knob dsFreq, dsThr, dsRange; vegaui::Choice dsMode; vegaui::Toggle dsListen;
    vegaui::Toggle compOn; vegaui::Knob comp, compSpeed, presence, air; vegaui::Toggle dblOn; vegaui::Knob dbl, dblW, mix, output;
    vegaui::StereoMeter meter; vegaui::GrColumn gGate, gDs, gComp; vegaui::Oled sib;
};

VegaVocalEditor::VegaVocalEditor (VegaVocalProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&lnf);
    main = std::make_unique<Main> (p);
    addAndMakeVisible (*main);
    constrainer.setFixedAspectRatio ((double) W / H);
    constrainer.setSizeLimits (W / 2, H / 2, W * 2, H * 2);
    setConstrainer (&constrainer);
    setResizable (true, true);
    const float s = juce::jlimit (0.5f, 2.0f, proc.uiScale.load());
    setSize (juce::roundToInt (W * s), juce::roundToInt (H * s));
    startTimerHz (30);
}
VegaVocalEditor::~VegaVocalEditor() { stopTimer(); setConstrainer (nullptr); main.reset(); setLookAndFeel (nullptr); }
void VegaVocalEditor::timerCallback() { main->refresh(); }
void VegaVocalEditor::resized()
{
    main->setBounds (0, 0, W, H);
    const float s = (float) getWidth() / (float) W;
    main->setTransform (juce::AffineTransform::scale (s));
    proc.uiScale.store (s);
}
juce::AudioProcessorEditor* VegaVocalProcessor::createEditor() { return new VegaVocalEditor (*this); }
