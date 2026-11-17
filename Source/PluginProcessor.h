#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class VegaVocalProcessor : public juce::AudioProcessor
{
public:
    VegaVocalProcessor();
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void prepareToPlay (double, int) override;
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Vega Productions Vocal"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.1; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float> grGate { 0 }, grDess { 0 }, grComp { 0 }, sibLevel { -100 }, inLevel { -100 }, outL { 0 }, outR { 0 };
    std::atomic<float> uiScale { 1.0f };

private:
    double sr = 48000;
    juce::dsp::IIR::Filter<float> lowCut[2], presence[2], air[2], sibDetect;
    juce::dsp::LinkwitzRileyFilter<float> dsLo, dsHi;
    float lastLowCut = -1, lastPres = -100, lastAir = -100, lastDsFreq = -1;
    float gateEnv = 0, gateGain = 1, dsEnv = 0, dsGain = 1, compEnv = 0, compGain = 1;
    std::vector<float> dbl[2]; int dpos = 0; double modPh[2] {};
    std::atomic<float>* p[24] {};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VegaVocalProcessor)
};
