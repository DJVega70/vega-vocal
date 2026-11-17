#pragma once
#include "PluginProcessor.h"
#include "VegaUI.h"

class VegaVocalEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit VegaVocalEditor (VegaVocalProcessor&);
    ~VegaVocalEditor() override;
    void paint (juce::Graphics&) override {}
    void resized() override;
    static constexpr int W = 1120, H = 470;
    class Main;
private:
    void timerCallback() override;
    VegaVocalProcessor& proc;
    VegaLookAndFeel lnf;
    std::unique_ptr<Main> main;
    juce::ComponentBoundsConstrainer constrainer;
};
