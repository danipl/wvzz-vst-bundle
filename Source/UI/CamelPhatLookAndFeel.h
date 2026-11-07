#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace camelphat::ui {

/**
 * @brief Modern vector LookAndFeel for CamelPhat.
 * Renders sleek neon/dark themed rotary dials, illuminated buttons, and combo boxes.
 */
class CamelPhatLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CamelPhatLookAndFeel();
    ~CamelPhatLookAndFeel() override = default;

    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider& slider) override;

    void drawLinearSlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos,
                          float minSliderPos,
                          float maxSliderPos,
                          juce::Slider::SliderStyle style,
                          juce::Slider& slider) override;

    void drawToggleButton(juce::Graphics& g,
                          juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics& g,
                      int width, int height,
                      bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

    // Palette Colors
    static const juce::Colour bgBase;
    static const juce::Colour cardBg;
    static const juce::Colour cardBorder;
    static const juce::Colour cardHighlight;
    static const juce::Colour trackBg;
    static const juce::Colour textPrimary;
    static const juce::Colour textSecondary;

    // Curated Luminous Module Palette
    static const juce::Colour coralRed;       // Distortion (Ember / Coral)
    static const juce::Colour skyCyan;        // Magic EQ (Electric Sky)
    static const juce::Colour honeyGold;      // Compressor (Warm Honey / Ochre)
    static const juce::Colour emeraldGreen;   // MM Filter (Vivid Emerald)
    static const juce::Colour electricViolet; // Flanger (Violet / Purple)
    static const juce::Colour warmAmber;      // BP Filter (Warm Brass / Amber)
    static const juce::Colour arcticIce;      // Master (Arctic Ice White)

    // Legacy Aliases
    static const juce::Colour neonGreen;
    static const juce::Colour neonAmber;
    static const juce::Colour neonCyan;
};

} // namespace camelphat::ui
