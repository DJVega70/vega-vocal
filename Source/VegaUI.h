#pragma once
// =====================================================================
//  Vega Productions — shared UI kit (suite look & feel + widgets)
//  Used by: Mastering, Reverb, Bus Compressor, Channel Strip, Workstation
// =====================================================================
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

// =====================================================================
//  Vega Productions — hardware‑style look & feel
//  (charcoal panel, machined knobs, cyan illuminated switches, OLED)
// =====================================================================
namespace VegaColours
{
    const juce::Colour background { 0xff1e1e1e };
    const juce::Colour panel      { 0xff2a2a2a };
    const juce::Colour panelLight { 0xff303030 };
    const juce::Colour bar        { 0xff232323 };
    const juce::Colour line       { 0xff3e3e3e };
    const juce::Colour text       { 0xffd8d8d8 };
    const juce::Colour textDim    { 0xff8e8e8e };
    const juce::Colour accent     { 0xff7ff7f2 };
    const juce::Colour accentDim  { 0xff2f6f6c };
    const juce::Colour oled       { 0xff050505 };
    const juce::Colour gold       { 0xffd9b45a };
}

namespace VegaFonts
{
    inline juce::Font ui (float h, bool bold = false)
    {
        return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
    }
    inline juce::Font mono (float h)
    {
        return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), h, juce::Font::plain));
    }
}

class VegaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    VegaLookAndFeel()
    {
        using namespace VegaColours;
        setColour (juce::Slider::textBoxTextColourId, textDim);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxHighlightColourId, accentDim);
        setColour (juce::Slider::backgroundColourId, juce::Colour (0xff151515));
        setColour (juce::Slider::trackColourId, accentDim);
        setColour (juce::Slider::thumbColourId, accent);
        setColour (juce::ProgressBar::foregroundColourId, accent);
        setColour (juce::ProgressBar::backgroundColourId, juce::Colour (0xff151515));
        setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, accent.withAlpha (0.7f));
        setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, accent.withAlpha (0.25f));
        setColour (juce::Label::textColourId, text);
        setColour (juce::Label::textWhenEditingColourId, text);
        setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff151515));
        setColour (juce::TextEditor::textColourId, text);
        setColour (juce::TextEditor::highlightColourId, accentDim);
        setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1b1b1b));
        setColour (juce::ComboBox::textColourId, textDim);
        setColour (juce::ComboBox::outlineColourId, line);
        setColour (juce::ComboBox::arrowColourId, textDim);
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1c1c1c));
        setColour (juce::PopupMenu::textColourId, text);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, accentDim);
        setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
        setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3a3a3a));
        setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffd0d0d0));
        setColour (juce::TextButton::textColourOffId, text);
        setColour (juce::TextButton::textColourOnId, juce::Colour (0xff1a1a1a));
        setColour (juce::AlertWindow::backgroundColourId, panel);
        setColour (juce::AlertWindow::textColourId, text);
    }

    // ------------------------------------------------------------------ knobs
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                           float startA, float endA, juce::Slider& s) override
    {
        using namespace VegaColours;
        const bool big = s.getProperties()["big"];
        const auto stepLabels = s.getProperties()["stepLabels"].toString();

        auto area = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
        const float tickSpace = big ? 16.0f : 9.0f;
        const float labelSpace = stepLabels.isNotEmpty() ? 18.0f : 0.0f;
        const float size = std::min (area.getWidth() - 2 * (tickSpace + labelSpace),
                                     area.getHeight() - 2 * tickSpace - labelSpace * 1.4f);
        auto knob = juce::Rectangle<float> (size, size).withCentre (area.getCentre().translated (0.0f, labelSpace * 0.5f));
        const auto c = knob.getCentre();
        const float r = size * 0.5f;
        const float angle = startA + pos * (endA - startA);

        // tick marks
        {
            const int numTicks = stepLabels.isNotEmpty() ? juce::StringArray::fromTokens (stepLabels, "|", "").size()
                                                         : (big ? 11 : 11);
            for (int i = 0; i < numTicks; ++i)
            {
                const float t = numTicks > 1 ? (float) i / (float) (numTicks - 1) : 0.0f;
                const float a = startA + t * (endA - startA);
                const bool major = (i == 0 || i == numTicks - 1 || (big && i == numTicks / 2));
                const float r1 = r + (big ? 7.0f : 3.5f), r2 = r1 + (major ? (big ? 7.0f : 5.0f) : (big ? 4.5f : 3.0f));
                g.setColour (textDim.withAlpha (major ? 0.9f : 0.55f));
                g.drawLine (c.x + r1 * std::sin (a), c.y - r1 * std::cos (a),
                            c.x + r2 * std::sin (a), c.y - r2 * std::cos (a), big ? 1.6f : 1.1f);
            }
            if (stepLabels.isNotEmpty())
            {
                auto labels = juce::StringArray::fromTokens (stepLabels, "|", "");
                g.setFont (VegaFonts::ui (9.5f));
                for (int i = 0; i < labels.size(); ++i)
                {
                    const float t = labels.size() > 1 ? (float) i / (float) (labels.size() - 1) : 0.0f;
                    const float a = startA + t * (endA - startA);
                    const float rl = r + 18.0f;
                    auto lc = juce::Point<float> (c.x + rl * std::sin (a), c.y - rl * std::cos (a));
                    const bool sel = juce::roundToInt (pos * (labels.size() - 1)) == i;
                    g.setColour (sel ? accent : textDim);
                    g.drawFittedText (labels[i], juce::Rectangle<int> (40, 12).withCentre (lc.toInt()),
                                      juce::Justification::centred, 1);
                }
            }
        }

        // drop shadow
        {
            juce::Path sh; sh.addEllipse (knob.translated (0, big ? 5.0f : 3.0f).expanded (big ? 3.0f : 1.5f));
            juce::DropShadow (juce::Colours::black.withAlpha (0.75f), big ? 18 : 9, {}).drawForPath (g, sh);
        }

        if (big)
        {
            // outer machined collar
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3b3b3b), c.x, knob.getY(),
                                                     juce::Colour (0xff121212), c.x, knob.getBottom(), false));
            g.fillEllipse (knob);
            g.setColour (juce::Colour (0xff4a4a4a));
            g.drawEllipse (knob.reduced (0.5f), 1.0f);
            knob = knob.reduced (r * 0.13f);
        }

        // knob body
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff353535), c.x - r * 0.4f, c.y - r * 0.6f,
                                                 juce::Colour (0xff141414), c.x + r * 0.5f, c.y + r * 0.8f, true));
        g.fillEllipse (knob);
        // face
        auto face = knob.reduced (knob.getWidth() * 0.06f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2c2c2c), c.x, face.getY(),
                                                 juce::Colour (0xff1a1a1a), c.x, face.getBottom(), false));
        g.fillEllipse (face);
        g.setColour (juce::Colours::white.withAlpha (0.06f));
        g.drawEllipse (face, 1.0f);
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawEllipse (knob, 1.0f);

        // pointer
        const float kr = knob.getWidth() * 0.5f;
        const float p1 = big ? kr * 0.68f : kr * 0.35f, p2 = kr * 0.94f;
        g.setColour (juce::Colours::white.withAlpha (0.95f));
        g.drawLine (c.x + p1 * std::sin (angle), c.y - p1 * std::cos (angle),
                    c.x + p2 * std::sin (angle), c.y - p2 * std::cos (angle), big ? 2.6f : 2.0f);

        if (s.isMouseOverOrDragging())
        {
            g.setColour (accent.withAlpha (0.12f));
            g.drawEllipse (knob.expanded (2.0f), 1.2f);
        }
    }

    // ------------------------------------------------------------------ vertical fader
    void drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float sliderPos, float minPos, float maxPos,
                           juce::Slider::SliderStyle style, juce::Slider& s) override
    {
        using namespace VegaColours;
        if (style != juce::Slider::LinearVertical)
        {
            if (style == juce::Slider::LinearBar || style == juce::Slider::LinearBarVertical)
            {
                auto r = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
                g.setColour (juce::Colour (0xff151515)); g.fillRoundedRectangle (r, 2.0f);
                g.setColour (accentDim.withAlpha (s.isEnabled() ? 0.9f : 0.4f));
                if (s.getMinimum() < 0.0 && s.getMaximum() > 0.0)   // bipolar: fill from zero
                {
                    const float zx = (float) x + (float) w * (float) s.valueToProportionOfLength (0.0);
                    g.fillRect (juce::Rectangle<float>::leftTopRightBottom (std::min (zx, sliderPos), r.getY() + 1, std::max (zx, sliderPos), r.getBottom() - 1));
                    g.setColour (textDim.withAlpha (0.5f)); g.drawVerticalLine ((int) zx, r.getY() + 2, r.getBottom() - 2);
                }
                else
                    g.fillRoundedRectangle (r.withRight (sliderPos), 2.0f);
                g.setColour (line); g.drawRoundedRectangle (r.reduced (0.5f), 2.0f, 1.0f);
                return;
            }
            LookAndFeel_V4::drawLinearSlider (g, x, y, w, h, sliderPos, minPos, maxPos, style, s);
            return;
        }
        auto area = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
        const float cx = area.getCentreX();
        const float top = area.getY() + 8.0f, bottom = area.getBottom() - 8.0f;

        // ticks
        for (int i = 0; i <= 8; ++i)
        {
            const float ty = top + (bottom - top) * (float) i / 8.0f;
            const bool major = (i % 4 == 0);
            g.setColour (textDim.withAlpha (major ? 0.8f : 0.4f));
            g.drawLine (cx - 20, ty, cx - (major ? 11.0f : 13.0f), ty, 1.0f);
            g.drawLine (cx + (major ? 11.0f : 13.0f), ty, cx + 20, ty, 1.0f);
        }
        // slot
        g.setColour (juce::Colour (0xff121212));
        g.fillRoundedRectangle (cx - 2.0f, top - 4, 4.0f, bottom - top + 8, 2.0f);
        g.setColour (juce::Colours::white.withAlpha (0.05f));
        g.drawRoundedRectangle (cx - 2.0f, top - 4, 4.0f, bottom - top + 8, 2.0f, 0.8f);

        // cap
        auto cap = juce::Rectangle<float> (28.0f, 16.0f).withCentre ({ cx, sliderPos });
        juce::Path sh; sh.addRoundedRectangle (cap.translated (0, 3), 2.0f);
        juce::DropShadow (juce::Colours::black.withAlpha (0.8f), 8, {}).drawForPath (g, sh);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff5a5a5a), cx, cap.getY(),
                                                 juce::Colour (0xff222222), cx, cap.getBottom(), false));
        g.fillRoundedRectangle (cap, 2.0f);
        g.setColour (juce::Colour (0xff1a1a1a));
        g.drawRoundedRectangle (cap, 2.0f, 1.0f);
        g.setColour (s.isMouseOverOrDragging() ? accent : juce::Colours::white);
        g.drawLine (cap.getX() + 3, cap.getCentreY(), cap.getRight() - 3, cap.getCentreY(), 1.6f);
    }

    juce::Slider::SliderLayout getSliderLayout (juce::Slider& s) override
    {
        auto layout = LookAndFeel_V4::getSliderLayout (s);
        if (s.getTextBoxPosition() == juce::Slider::TextBoxBelow)
        {
            auto b = s.getLocalBounds();
            layout.textBoxBounds = b.removeFromBottom (16);
            layout.sliderBounds = b;
        }
        return layout;
    }

    juce::Font getLabelFont (juce::Label& l) override
    {
        if (dynamic_cast<juce::Slider*> (l.getParentComponent()) != nullptr)
            return VegaFonts::ui (11.0f);
        return l.getFont();
    }

    void drawLabel (juce::Graphics& g, juce::Label& l) override
    {
        if (dynamic_cast<juce::Slider*> (l.getParentComponent()) != nullptr && ! l.isBeingEdited())
        {
            g.setColour (VegaColours::textDim);
            g.setFont (VegaFonts::ui (11.0f));
            g.drawFittedText (l.getText(), l.getLocalBounds(), juce::Justification::centred, 1);
            return;
        }
        LookAndFeel_V4::drawLabel (g, l);
    }

    juce::Label* createSliderTextBox (juce::Slider& s) override
    {
        auto* l = LookAndFeel_V4::createSliderTextBox (s);
        l->setColour (juce::Label::textColourId, VegaColours::textDim);
        l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
        l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        l->setJustificationType (juce::Justification::centred);
        return l;
    }

    // ------------------------------------------------------------------ toggles
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool) override
    {
        using namespace VegaColours;
        const bool on = b.getToggleState();
        auto area = b.getLocalBounds().toFloat();

        if (b.getProperties()["switch"])
        {
            // small vertical toggle switch with two labels (e.g. Stereo / M‑S)
            const auto labels = juce::StringArray::fromTokens (b.getButtonText(), "|", "");
            auto sw = juce::Rectangle<float> (10.0f, 22.0f).withCentre ({ area.getX() + 12.0f, area.getCentreY() });
            g.setColour (juce::Colour (0xff111111));
            g.fillRoundedRectangle (sw, 5.0f);
            g.setColour (line);
            g.drawRoundedRectangle (sw, 5.0f, 1.0f);
            auto lever = juce::Rectangle<float> (8.0f, 8.0f).withCentre ({ sw.getCentreX(), on ? sw.getBottom() - 5.5f : sw.getY() + 5.5f });
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe8e8e8), lever.getX(), lever.getY(),
                                                     juce::Colour (0xff7a7a7a), lever.getRight(), lever.getBottom(), false));
            g.fillEllipse (lever);
            g.setFont (VegaFonts::ui (10.0f));
            if (labels.size() >= 2)
            {
                g.setColour (on ? textDim : text);
                g.drawText (labels[0], (int) sw.getRight() + 5, (int) sw.getY() - 5, 70, 12, juce::Justification::left);
                g.setColour (on ? text : textDim);
                g.drawText (labels[1], (int) sw.getRight() + 5, (int) sw.getBottom() - 7, 70, 12, juce::Justification::left);
            }
            return;
        }

        if (b.getButtonText().isNotEmpty())
        {
            // labelled check box (e.g. option toggles inside pages)
            auto box = juce::Rectangle<float> (16.0f, 16.0f).withCentre ({ area.getX() + 10.0f, area.getCentreY() });
            g.setColour (juce::Colour (0xff161616)); g.fillRoundedRectangle (box, 2.0f);
            g.setColour (on ? accent : juce::Colour (0xff5c5c5c)); g.drawRoundedRectangle (box, 2.0f, 1.0f);
            if (on) { g.setColour (accent); g.fillRoundedRectangle (box.reduced (4.0f), 1.0f); }
            g.setColour (over ? juce::Colours::white : text);
            g.setFont (VegaFonts::ui (12.0f));
            g.drawText (b.getButtonText(), area.withTrimmedLeft (26.0f).toNearestInt(), juce::Justification::centredLeft);
            return;
        }

        // illuminated square push‑button
        const float sz = std::min (area.getWidth(), area.getHeight() - 16.0f);
        auto sq = juce::Rectangle<float> (sz, sz).withCentre ({ area.getCentreX(), area.getY() + sz * 0.5f + 1.0f });
        juce::Path sh; sh.addRoundedRectangle (sq.translated (0, 2), 2.0f);
        juce::DropShadow (juce::Colours::black.withAlpha (0.7f), 6, {}).drawForPath (g, sh);

        g.setColour (juce::Colour (0xff161616));
        g.fillRoundedRectangle (sq, 2.0f);
        auto inner = sq.reduced (sz * 0.14f);
        if (on)
        {
            g.setColour (accent.withAlpha (0.35f));
            g.fillRoundedRectangle (sq.expanded (2.0f), 3.0f);
            g.setColour (accent);
            g.fillRoundedRectangle (sq.reduced (1.0f), 2.0f);
            g.setColour (juce::Colours::white.withAlpha (0.6f));
            g.fillRoundedRectangle (inner.reduced (sz * 0.12f), 1.5f);
            g.setColour (accent.brighter (0.4f));
            g.drawRoundedRectangle (inner, 1.5f, 1.2f);
        }
        else
        {
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a4a4a), inner.getX(), inner.getY(),
                                                     juce::Colour (0xff262626), inner.getRight(), inner.getBottom(), false));
            g.fillRoundedRectangle (inner, 1.5f);
            g.setColour (juce::Colour (0xff5c5c5c));
            g.drawRoundedRectangle (inner, 1.5f, 1.0f);
        }
        if (over)
        {
            g.setColour (juce::Colours::white.withAlpha (0.08f));
            g.fillRoundedRectangle (sq, 2.0f);
        }
        g.setColour (textDim);
        g.setFont (VegaFonts::ui (11.0f));
        g.drawText (on ? "On" : "Off", area.withTop (sq.getBottom() + 3.0f).toNearestInt(), juce::Justification::centredTop);
    }

    // ------------------------------------------------------------------ text buttons (top bar)
    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) override
    {
        auto r = b.getLocalBounds().toFloat().reduced (0.5f);
        auto col = b.getToggleState() ? findColour (juce::TextButton::buttonOnColourId)
                                      : findColour (juce::TextButton::buttonColourId);
        if (over) col = col.brighter (0.08f);
        if (down) col = col.darker (0.15f);
        g.setColour (col);
        g.fillRoundedRectangle (r, 2.0f);
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.drawRoundedRectangle (r, 2.0f, 1.0f);
    }
    juce::Font getTextButtonFont (juce::TextButton&, int) override { return VegaFonts::ui (10.5f, true); }

    // ------------------------------------------------------------------ combo boxes
    void drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box) override
    {
        auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (0.5f);
        g.setColour (findColour (juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle (r, 2.0f);
        g.setColour (findColour (juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle (r, 2.0f, 1.0f);
        // chevron
        juce::Path p;
        const float cx = (float) w - 10.0f, cy = (float) h * 0.5f;
        p.startNewSubPath (cx - 4, cy + 2); p.lineTo (cx, cy - 2); p.lineTo (cx + 4, cy + 2);
        g.setColour (box.isMouseOver() ? VegaColours::text : VegaColours::textDim);
        g.strokePath (p, juce::PathStrokeType (1.2f));
    }
    juce::Font getComboBoxFont (juce::ComboBox&) override { return VegaFonts::ui (10.5f); }
    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (4, 0, box.getWidth() - 20, box.getHeight());
        label.setFont (getComboBoxFont (box));
        label.setJustificationType (juce::Justification::centred);
    }
    juce::Font getPopupMenuFont() override { return VegaFonts::ui (13.0f); }
};


namespace vegaui
{
using APVTS = juce::AudioProcessorValueTreeState;

inline float gainToDb (float g) { return 20.0f * std::log10 (std::max (g, 1.0e-9f)); }

// ---------------------------------------------------------------- knob
class Knob : public juce::Component
{
public:
    Knob (APVTS& state, const juce::String& paramId, const juce::String& title, bool big = false,
          std::function<juce::String (double)> formatter = {}, const juce::String& stepLabels = {})
    {
        label.setText (title, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (VegaFonts::ui (big ? 12.5f : 11.5f));
        label.setColour (juce::Label::textColourId, VegaColours::text);
        label.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (label);

        slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
        slider.setTextBoxStyle (stepLabels.isNotEmpty() ? juce::Slider::NoTextBox : juce::Slider::TextBoxBelow, false, 90, 16);
        slider.setMouseDragSensitivity (big ? 320 : 220);
        slider.getProperties().set ("big", big);
        if (stepLabels.isNotEmpty()) slider.getProperties().set ("stepLabels", stepLabels);
        addAndMakeVisible (slider);

        if (auto* p = state.getParameter (paramId))
        {
            attachment = std::make_unique<APVTS::SliderAttachment> (state, paramId, slider);
            slider.setDoubleClickReturnValue (true, (double) p->convertFrom0to1 (p->getDefaultValue()));
        }
        else
            jassertfalse; // unknown parameter id

        if (formatter)
        {
            slider.textFromValueFunction = [formatter] (double v) { return formatter (v); };
            slider.updateText();
        }
    }
    void resized() override
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromTop (16));
        slider.setBounds (b);
    }
    juce::Slider slider;
    juce::Label label;
private:
    std::unique_ptr<APVTS::SliderAttachment> attachment;
};

// ---------------------------------------------------------------- toggle / switch
class Toggle : public juce::Component
{
public:
    /** switchLabels: "Off|On" style text turns it into a small lever switch */
    Toggle (APVTS& state, const juce::String& paramId, const juce::String& title, const juce::String& switchLabels = {})
    {
        label.setText (title, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (VegaFonts::ui (11.5f));
        label.setColour (juce::Label::textColourId, VegaColours::text);
        label.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (label);
        isSwitch = switchLabels.isNotEmpty();
        if (isSwitch) { button.getProperties().set ("switch", true); button.setButtonText (switchLabels); }
        addAndMakeVisible (button);
        if (state.getParameter (paramId) != nullptr)
            attachment = std::make_unique<APVTS::ButtonAttachment> (state, paramId, button);
        else
            jassertfalse;
    }
    void resized() override
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromTop (16));
        if (isSwitch) button.setBounds (b.withSizeKeepingCentre (std::min (b.getWidth(), 84), 34).translated (8, 0));
        else          button.setBounds (b.withSizeKeepingCentre (34, 52).withY (b.getY() + 6));
    }
    juce::ToggleButton button;
    juce::Label label;
private:
    bool isSwitch = false;
    std::unique_ptr<APVTS::ButtonAttachment> attachment;
};

// ---------------------------------------------------------------- choice selector (combo)
class Choice : public juce::Component
{
public:
    Choice (APVTS& state, const juce::String& paramId, const juce::String& title)
    {
        label.setText (title, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (VegaFonts::ui (11.5f));
        label.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (label);
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (state.getParameter (paramId)))
            box.addItemList (p->choices, 1);
        addAndMakeVisible (box);
        if (state.getParameter (paramId) != nullptr)
            attachment = std::make_unique<APVTS::ComboBoxAttachment> (state, paramId, box);
    }
    void resized() override
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromTop (16));
        box.setBounds (b.withSizeKeepingCentre (b.getWidth() - 6, 22).withY (b.getY() + 6));
    }
    juce::ComboBox box;
    juce::Label label;
private:
    std::unique_ptr<APVTS::ComboBoxAttachment> attachment;
};

// ---------------------------------------------------------------- vertical LED level meter
class LedMeter : public juce::Component
{
public:
    void setLevel (float linearPeak)
    {
        const float db = gainToDb (linearPeak);
        level = db > level ? db : level - 1.2f;
        if (db > hold) { hold = db; holdTicks = 45; } else if (--holdTicks <= 0) hold -= 0.8f;
        repaint();
    }
    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        const int n = (int) std::size (segDb);
        const float segH = b.getHeight() / (float) n;
        for (int i = 0; i < n; ++i)
        {
            auto seg = juce::Rectangle<float> (b.getX(), b.getY() + i * segH, b.getWidth(), segH).reduced (0, segH * 0.18f);
            const float th = segDb[i];
            const juce::Colour c = th >= -0.1f ? juce::Colour (0xffff5a5a) : (th > -18.0f ? juce::Colours::white : VegaColours::accent);
            const bool lit = level >= th;
            g.setColour (lit ? c : juce::Colour (0xff3a3a3a));
            g.fillRect (seg);
            if (! lit && i < n - 1 && hold <= th && hold > segDb[i + 1]) { g.setColour (c.withAlpha (0.7f)); g.fillRect (seg); }
        }
    }
    static constexpr float segDb[] { 0, -1, -2, -3, -4, -5, -6, -8, -10, -12, -15, -18, -21, -24, -30, -36, -42, -48, -54, -60 };
private:
    float level = -100, hold = -100; int holdTicks = 0;
};

class LedScale : public juce::Component
{
public:
    void paint (juce::Graphics& g) override
    {
        const int n = (int) std::size (LedMeter::segDb);
        const float segH = (float) getHeight() / (float) n;
        g.setFont (VegaFonts::ui (9.5f));
        g.setColour (VegaColours::textDim);
        for (int i = 0; i < n; ++i)
        {
            const int db = (int) LedMeter::segDb[i];
            if (db == 0 || db == -3 || db == -6 || db == -12 || db == -18 || db == -24 || db == -36 || db == -48 || db == -60)
                g.drawText (juce::String (std::abs (db)), 0, (int) (i * segH + segH * 0.5f) - 6, getWidth(), 12, juce::Justification::centred);
        }
    }
};

/** Stereo output meter block: scale + L + R with captions */
class StereoMeter : public juce::Component
{
public:
    StereoMeter() { addAndMakeVisible (scale); addAndMakeVisible (l); addAndMakeVisible (r); }
    void setLevels (float left, float right) { l.setLevel (left); r.setLevel (right); }
    void resized() override
    {
        auto b = getLocalBounds().withTrimmedTop (16).withTrimmedBottom (16);
        scale.setBounds (b.removeFromLeft (22));
        b.removeFromLeft (4);  l.setBounds (b.removeFromLeft (14));
        b.removeFromLeft (6);  r.setBounds (b.removeFromLeft (14));
    }
    void paint (juce::Graphics& g) override
    {
        g.setFont (VegaFonts::ui (9.5f));
        g.setColour (VegaColours::textDim);
        g.drawText ("dB", l.getX(), 0, 34, 12, juce::Justification::centred);
        g.drawText (capA, l.getX() - 6, getHeight() - 13, 26, 12, juce::Justification::centred);
        g.drawText (capB, r.getX() - 6, getHeight() - 13, 26, 12, juce::Justification::centred);
    }
    juce::String capA { "L" }, capB { "R" };
private:
    LedScale scale; LedMeter l, r;
};

// ---------------------------------------------------------------- gain‑reduction LED column (0..12 dB)
class GrColumn : public juce::Component
{
public:
    void setGr (float db) { value = db > value ? db : std::max (0.0f, value - 0.25f); repaint(); }
    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setFont (VegaFonts::ui (10.5f));
        g.setColour (VegaColours::text);
        g.drawText (title, b.removeFromTop (16.0f).toNearestInt(), juce::Justification::centredLeft);
        auto meter = b.removeFromLeft (14.0f);
        constexpr int n = 16;
        const float segH = meter.getHeight() / n;
        for (int i = 0; i < n; ++i)
        {
            const float th = (i + 0.5f) * maxDb / n;
            auto seg = juce::Rectangle<float> (meter.getX(), meter.getY() + i * segH, meter.getWidth(), segH).reduced (0, segH * 0.18f);
            g.setColour (value >= th ? VegaColours::accent : juce::Colour (0xff3a3a3a));
            g.fillRect (seg);
        }
        g.setFont (VegaFonts::ui (9.0f));
        g.setColour (VegaColours::textDim);
        for (int i = 0; i <= 4; ++i)
        {
            const float db = maxDb * i / 4.0f;
            g.drawText (juce::String ((int) db), (int) b.getX() + 3,
                        juce::jlimit ((int) meter.getY(), (int) meter.getBottom() - 12, (int) (meter.getY() + db / maxDb * meter.getHeight()) - 6),
                        22, 12, juce::Justification::left);
        }
    }
    float maxDb = 12.0f;
    juce::String title { "GR" };
private:
    float value = 0;
};

// ---------------------------------------------------------------- OLED read‑out with up to 3 lines
class Oled : public juce::Component
{
public:
    void setLines (const juce::String& title, const juce::String& big,
                   const juce::String& l1 = {}, const juce::String& v1 = {},
                   const juce::String& l2 = {}, const juce::String& v2 = {})
    {
        t = title; b = big; a1 = l1; b1 = v1; a2 = l2; b2 = v2; repaint();
    }
    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff101010)); g.fillRoundedRectangle (r, 3.0f);
        auto s = r.reduced (4.0f);
        g.setColour (VegaColours::oled); g.fillRect (s);
        g.setColour (juce::Colours::white.withAlpha (0.92f));
        const bool hasBottom = a1.isNotEmpty() || a2.isNotEmpty();
        auto top = s.removeFromTop (hasBottom ? s.getHeight() * 0.5f : s.getHeight());
        g.setFont (VegaFonts::mono (11.0f));
        g.drawText (t, top.removeFromTop (top.getHeight() * 0.5f), juce::Justification::centredBottom);
        g.setFont (VegaFonts::mono (12.5f));
        g.drawText (b, top, juce::Justification::centred);
        if (hasBottom)
        {
            g.setFont (VegaFonts::mono (10.5f));
            auto left = s.removeFromLeft (s.getWidth() * 0.5f);
            g.drawText (a1, left.removeFromTop (left.getHeight() * 0.5f), juce::Justification::centredBottom);
            g.drawText (b1, left, juce::Justification::centredTop);
            g.drawText (a2, s.removeFromTop (s.getHeight() * 0.5f), juce::Justification::centredBottom);
            g.drawText (b2, s, juce::Justification::centredTop);
        }
        g.setColour (juce::Colours::black.withAlpha (0.25f));
        for (float y = r.getY() + 4; y < r.getBottom() - 4; y += 2.0f) g.drawHorizontalLine ((int) y, r.getX() + 4, r.getRight() - 4);
    }
private:
    juce::String t, b, a1, b1, a2, b2;
};

// ---------------------------------------------------------------- dark screen panel (for scopes/analysers)
inline void drawScreen (juce::Graphics& g, juce::Rectangle<float> r)
{
    g.setColour (juce::Colour (0xff141414));
    g.fillRoundedRectangle (r, 3.0f);
}

// ---------------------------------------------------------------- chrome: top bar, body, sections, bottom bar
/** Paints the standard Vega Productions top bar (logo + product line). Returns the bar height. */
inline int drawTopBar (juce::Graphics& g, int width, const juce::String& productLine)
{
    using namespace VegaColours;
    constexpr int h = 34;
    g.setColour (bar); g.fillRect (0, 0, width, h);
    g.setColour (line); g.drawHorizontalLine (h, 0, (float) width);
    const juce::Point<float> c (22.0f, 17.0f);
    g.setColour (juce::Colours::white.withAlpha (0.9f));
    g.drawEllipse (c.x - 9, c.y - 9, 18, 18, 1.4f);
    juce::Path v;
    v.startNewSubPath (c.x - 4.5f, c.y - 4); v.lineTo (c.x, c.y + 4.5f); v.lineTo (c.x + 4.5f, c.y - 4);
    g.strokePath (v, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setFont (VegaFonts::ui (14.5f, true));
    g.drawText ("Vega Productions", 38, 0, 160, h, juce::Justification::centredLeft);
    g.setFont (VegaFonts::ui (10.0f));
    g.setColour (textDim);
    g.drawText (productLine, 182, 0, 260, h, juce::Justification::centredLeft);
    return h;
}

/** Brushed charcoal body for the control area. */
inline void drawBody (juce::Graphics& g, juce::Rectangle<int> r)
{
    using namespace VegaColours;
    g.setGradientFill (juce::ColourGradient (panelLight, 0, (float) r.getY(), panel, 0, (float) r.getBottom(), false));
    g.fillRect (r);
}

/** Section divider on the left edge + small caps title. */
inline void drawSection (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title, bool leftDivider = true)
{
    using namespace VegaColours;
    if (leftDivider)
    {
        g.setColour (line);
        g.drawVerticalLine (r.getX(), (float) r.getY() + 4.0f, (float) r.getBottom() - 4.0f);
    }
    g.setFont (VegaFonts::ui (9.5f, true));
    g.setColour (textDim.withAlpha (0.7f));
    g.drawText (title.toUpperCase(), r.getX(), r.getY(), r.getWidth(), 14, juce::Justification::centred);
}

inline void drawBottomBar (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& caption = {})
{
    using namespace VegaColours;
    g.setColour (bar); g.fillRect (r);
    g.setColour (line); g.drawHorizontalLine (r.getY(), (float) r.getX(), (float) r.getRight());
    if (caption.isNotEmpty())
    {
        g.setFont (VegaFonts::ui (10.0f, true));
        g.setColour (textDim.withAlpha (0.45f));
        g.drawText (caption, r, juce::Justification::centred);
    }
}

/** Resets every parameter to its default (for a RESET / INIT button). */
inline void resetAllParameters (juce::AudioProcessor& p)
{
    for (auto* prm : p.getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (prm))
        {
            rp->beginChangeGesture();
            rp->setValueNotifyingHost (rp->getDefaultValue());
            rp->endChangeGesture();
        }
}
} // namespace vegaui
