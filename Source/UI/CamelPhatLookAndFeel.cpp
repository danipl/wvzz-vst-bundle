#include "CamelPhatLookAndFeel.h"

namespace camelphat::ui {

const juce::Colour CamelPhatLookAndFeel::bgBase        = juce::Colour(0xff12161a);
const juce::Colour CamelPhatLookAndFeel::cardBg        = juce::Colour(0xff1a2026);
const juce::Colour CamelPhatLookAndFeel::cardBorder    = juce::Colour(0xff2a323c);
const juce::Colour CamelPhatLookAndFeel::neonGreen     = juce::Colour(0xff00e676);
const juce::Colour CamelPhatLookAndFeel::neonAmber     = juce::Colour(0xffff9100);
const juce::Colour CamelPhatLookAndFeel::neonCyan      = juce::Colour(0xff00e5ff);
const juce::Colour CamelPhatLookAndFeel::trackBg       = juce::Colour(0xff242c35);
const juce::Colour CamelPhatLookAndFeel::textPrimary   = juce::Colour(0xfff0f4f8);
const juce::Colour CamelPhatLookAndFeel::textSecondary = juce::Colour(0xff8a99a8);

CamelPhatLookAndFeel::CamelPhatLookAndFeel()
{
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxTextColourId, textPrimary);
    setColour(juce::ComboBox::backgroundColourId, cardBg);
    setColour(juce::ComboBox::outlineColourId, cardBorder);
    setColour(juce::ComboBox::textColourId, textPrimary);
    setColour(juce::PopupMenu::backgroundColourId, cardBg);
    setColour(juce::PopupMenu::textColourId, textPrimary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, cardBorder);
    setColour(juce::PopupMenu::highlightedTextColourId, neonGreen);
}

void CamelPhatLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                            int x, int y, int width, int height,
                                            float sliderPosProportional,
                                            float rotaryStartAngle,
                                            float rotaryEndAngle,
                                            juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    // Reserve bottom space for text box if visible
    const float textBoxHeight = slider.getTextBoxPosition() != juce::Slider::NoTextBox ? 14.0f : 0.0f;
    auto dialArea = bounds.removeFromTop(bounds.getHeight() - textBoxHeight);

    // Keep dial compact to prevent overlapping with adjacent labels or boundaries
    const float maxRadius = juce::jmin(dialArea.getWidth(), dialArea.getHeight()) * 0.5f - 2.0f;
    const float radius = juce::jmax(8.0f, maxRadius);
    const float centreX = dialArea.getCentreX();
    const float centreY = dialArea.getCentreY();
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;

    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    const float trackWidth = juce::jlimit(2.0f, 4.0f, radius * 0.15f);

    // 1. Outer Inactive Track Arc
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, radius - trackWidth * 0.5f, radius - trackWidth * 0.5f,
                                0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(trackBg);
    g.strokePath(backgroundArc, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 2. Active Glow Progress Arc
    if (sliderPosProportional > 0.001f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, radius - trackWidth * 0.5f, radius - trackWidth * 0.5f,
                               0.0f, rotaryStartAngle, angle, true);

        // Pick color based on custom property or default to neon green
        juce::Colour arcColor = neonGreen;
        if (slider.getProperties().contains("accentColor"))
            arcColor = juce::Colour(static_cast<juce::uint32>(static_cast<int>(slider.getProperties()["accentColor"])));

        g.setColour(arcColor);
        g.strokePath(valueArc, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 3. Inner Dark Metallic Dial Body
    const float innerRadius = radius - trackWidth * 1.5f;
    if (innerRadius > 4.0f)
    {
        auto innerArea = juce::Rectangle<float>(centreX - innerRadius, centreY - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);
        juce::ColourGradient dialGrad(cardBg.brighter(0.08f), centreX, ry + trackWidth,
                                      cardBg.darker(0.15f), centreX, ry + rw - trackWidth, false);
        g.setGradientFill(dialGrad);
        g.fillEllipse(innerArea);

        g.setColour(cardBorder);
        g.drawEllipse(innerArea, 1.0f);

        // 4. Pointer Needle Line
        juce::Path p;
        const float pointerLength = innerRadius * 0.8f;
        p.startNewSubPath(centreX, centreY);
        p.lineTo(centreX + pointerLength * std::sin(angle), centreY - pointerLength * std::cos(angle));

        g.setColour(textPrimary);
        g.strokePath(p, juce::PathStrokeType(2.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
    }
}

void CamelPhatLookAndFeel::drawToggleButton(juce::Graphics& g,
                                            juce::ToggleButton& button,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);

    const bool isOn = button.getToggleState();
    const float toggleWidth = 32.0f;
    const float toggleHeight = 16.0f;
    auto toggleArea = bounds.removeFromLeft(toggleWidth).withHeight(toggleHeight).withY(bounds.getCentreY() - toggleHeight * 0.5f);

    // Track
    g.setColour(isOn ? neonGreen.darker(0.3f) : trackBg);
    g.fillRoundedRectangle(toggleArea, toggleHeight * 0.5f);
    g.setColour(cardBorder);
    g.drawRoundedRectangle(toggleArea, toggleHeight * 0.5f, 1.0f);

    // Thumb puck
    const float puckSize = toggleHeight - 4.0f;
    const float puckX = isOn ? (toggleArea.getRight() - puckSize - 2.0f) : (toggleArea.getX() + 2.0f);
    const float puckY = toggleArea.getY() + 2.0f;

    g.setColour(isOn ? neonGreen : textSecondary);
    g.fillEllipse(puckX, puckY, puckSize, puckSize);

    // Label text
    g.setColour(textPrimary);
    g.setFont(12.0f);
    bounds.removeFromLeft(6.0f);
    g.drawText(button.getButtonText(), bounds, juce::Justification::centredLeft, true);
}

void CamelPhatLookAndFeel::drawComboBox(juce::Graphics& g,
                                        int width, int height,
                                        bool isButtonDown,
                                        int buttonX, int buttonY, int buttonW, int buttonH,
                                        juce::ComboBox& box)
{
    juce::ignoreUnused(isButtonDown, buttonX, buttonY, buttonW, buttonH);
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1.0f);

    g.setColour(cardBg);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(cardBorder);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    // Down chevron
    juce::Path arrow;
    const float cx = static_cast<float>(width) - 14.0f;
    const float cy = static_cast<float>(height) * 0.5f;
    arrow.startNewSubPath(cx - 4.0f, cy - 2.0f);
    arrow.lineTo(cx, cy + 3.0f);
    arrow.lineTo(cx + 4.0f, cy - 2.0f);

    g.setColour(neonGreen);
    g.strokePath(arrow, juce::PathStrokeType(1.5f));

    juce::ignoreUnused(box);
}

void CamelPhatLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.fillAll(cardBg);
    g.setColour(cardBorder);
    g.drawRect(0, 0, width, height, 1);
}

} // namespace camelphat::ui
