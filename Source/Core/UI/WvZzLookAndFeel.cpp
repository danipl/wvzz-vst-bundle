#include "WvZzLookAndFeel.h"

namespace wvzz::ui {

const juce::Colour WvZzLookAndFeel::bgBase        = juce::Colour(0xff12161a);
const juce::Colour WvZzLookAndFeel::cardBg        = juce::Colour(0xff1a2026);
const juce::Colour WvZzLookAndFeel::cardBorder    = juce::Colour(0xff2a323c);
const juce::Colour WvZzLookAndFeel::neonGreen     = juce::Colour(0xff00e676);
const juce::Colour WvZzLookAndFeel::neonAmber     = juce::Colour(0xffff9100);
const juce::Colour WvZzLookAndFeel::neonCyan      = juce::Colour(0xff00e5ff);
const juce::Colour WvZzLookAndFeel::trackBg       = juce::Colour(0xff242c35);
const juce::Colour WvZzLookAndFeel::textPrimary   = juce::Colour(0xfff0f4f8);
const juce::Colour WvZzLookAndFeel::textSecondary = juce::Colour(0xff8a99a8);

WvZzLookAndFeel::WvZzLookAndFeel()
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

void WvZzLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                       int x, int y, int width, int height,
                                       float sliderPosProportional,
                                       float rotaryStartAngle,
                                       float rotaryEndAngle,
                                       juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    // Reserve bottom space for value readout if slider is tall enough
    const bool showValue = height >= 52;
    const float valueHeight = showValue ? 12.0f : 0.0f;
    auto dialArea = bounds.removeFromTop(bounds.getHeight() - valueHeight);

    // Keep dial compact and balanced
    const float maxRadius = juce::jmin(dialArea.getWidth(), dialArea.getHeight()) * 0.5f - 2.0f;
    const float radius = juce::jmax(8.0f, maxRadius);
    const float centreX = dialArea.getCentreX();
    const float centreY = dialArea.getCentreY();
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;

    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    const float trackWidth = juce::jlimit(2.0f, 4.0f, radius * 0.16f);

    // Check if slider is configured as bipolar
    const bool isBipolar = slider.getProperties().contains("isBipolar") &&
                           static_cast<bool>(slider.getProperties()["isBipolar"]);

    // 1. Outer Inactive Track Arc
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, radius - trackWidth * 0.5f, radius - trackWidth * 0.5f,
                                0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(trackBg);
    g.strokePath(backgroundArc, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Pick arc color based on custom property or default to neon green
    juce::Colour arcColor = neonGreen;
    if (slider.getProperties().contains("accentColor"))
        arcColor = juce::Colour(static_cast<juce::uint32>(static_cast<int>(slider.getProperties()["accentColor"])));

    // 2. Active Glow Progress Arc
    if (isBipolar)
    {
        // 12 o'clock center tick and arc from center (0.5 proportional)
        const float midAngle = rotaryStartAngle + 0.5f * (rotaryEndAngle - rotaryStartAngle);

        // Center notch
        g.setColour(cardBorder.brighter(0.2f));
        const float tickLen = trackWidth * 1.5f;
        g.drawLine(centreX, centreY - radius, centreX, centreY - radius + tickLen, 1.2f);

        if (std::abs(sliderPosProportional - 0.5f) > 0.005f)
        {
            juce::Path valueArc;
            const float startA = std::min(midAngle, angle);
            const float endA   = std::max(midAngle, angle);
            valueArc.addCentredArc(centreX, centreY, radius - trackWidth * 0.5f, radius - trackWidth * 0.5f,
                                   0.0f, startA, endA, true);

            g.setColour(arcColor);
            g.strokePath(valueArc, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }
    else if (sliderPosProportional > 0.001f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, radius - trackWidth * 0.5f, radius - trackWidth * 0.5f,
                                0.0f, rotaryStartAngle, angle, true);

        g.setColour(arcColor);
        g.strokePath(valueArc, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 3. Inner Dark Metallic Dial Body
    const float innerRadius = radius - trackWidth * 1.5f;
    if (innerRadius > 4.0f)
    {
        auto innerArea = juce::Rectangle<float>(centreX - innerRadius, centreY - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);
        juce::ColourGradient dialGrad(cardBg.brighter(0.08f), centreX, ry + trackWidth,
                                      cardBg.darker(0.18f), centreX, ry + rw - trackWidth, false);
        g.setGradientFill(dialGrad);
        g.fillEllipse(innerArea);

        g.setColour(cardBorder);
        g.drawEllipse(innerArea, 1.0f);

        // 4. Pointer Needle Line
        juce::Path p;
        const float pointerLength = innerRadius * 0.82f;
        p.startNewSubPath(centreX, centreY);
        p.lineTo(centreX + pointerLength * std::sin(angle), centreY - pointerLength * std::cos(angle));

        g.setColour(textPrimary);
        g.strokePath(p, juce::PathStrokeType(2.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
    }

    // 5. Value Text Readout
    if (showValue)
    {
        g.setFont(juce::FontOptions(9.5f, juce::Font::plain));
        g.setColour(textSecondary);
        juce::String valStr = slider.getTextFromValue(slider.getValue());
        g.drawText(valStr, bounds, juce::Justification::centred, true);
    }
}

void WvZzLookAndFeel::drawLinearSlider(juce::Graphics& g,
                                       int x, int y, int width, int height,
                                       float sliderPos,
                                       float minSliderPos,
                                       float maxSliderPos,
                                       juce::Slider::SliderStyle style,
                                       juce::Slider& slider)
{
    juce::ignoreUnused(minSliderPos, maxSliderPos, style);

    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    const float trackW = 6.0f;
    const float cx = bounds.getCentreX();
    auto trackRect = juce::Rectangle<float>(cx - trackW * 0.5f, bounds.getY() + 4.0f, trackW, bounds.getHeight() - 8.0f);

    // Track background
    g.setColour(trackBg);
    g.fillRoundedRectangle(trackRect, 3.0f);
    g.setColour(cardBorder);
    g.drawRoundedRectangle(trackRect, 3.0f, 1.0f);

    // Slider active track to thumb
    auto activeRect = trackRect.withTop(sliderPos);
    g.setColour(neonCyan.withAlpha(0.65f));
    g.fillRoundedRectangle(activeRect, 3.0f);

    // Thumb fader cap
    const float thumbH = 14.0f;
    const float thumbW = juce::jmin(bounds.getWidth() - 2.0f, 26.0f);
    auto thumbRect = juce::Rectangle<float>(cx - thumbW * 0.5f, sliderPos - thumbH * 0.5f, thumbW, thumbH);

    g.setColour(cardBg.brighter(0.12f));
    g.fillRoundedRectangle(thumbRect, 3.0f);
    g.setColour(neonCyan);
    g.drawRoundedRectangle(thumbRect, 3.0f, 1.2f);

    // Thumb center indicator line
    g.setColour(textPrimary);
    g.drawHorizontalLine(static_cast<int>(sliderPos), thumbRect.getX() + 3.0f, thumbRect.getRight() - 3.0f);

    juce::ignoreUnused(slider);
}

void WvZzLookAndFeel::drawToggleButton(juce::Graphics& g,
                                       juce::ToggleButton& button,
                                       bool shouldDrawButtonAsHighlighted,
                                       bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    const bool isOn = button.getToggleState();

    // Check if styled as a compact header bypass pill or module pill button
    const bool isHeaderBypass = button.getProperties().contains("isHeaderBypass") &&
                                static_cast<bool>(button.getProperties()["isHeaderBypass"]);
    const bool isPill = button.getProperties().contains("isPill") &&
                        static_cast<bool>(button.getProperties()["isPill"]);

    if (isHeaderBypass)
    {
        // Compact ON / BYP pill button in card header
        g.setColour(isOn ? cardBg.brighter(0.1f) : cardBg);
        g.fillRoundedRectangle(bounds, bounds.getHeight() * 0.5f);

        const auto borderColor = isOn ? neonAmber : neonGreen;
        g.setColour(borderColor);
        g.drawRoundedRectangle(bounds, bounds.getHeight() * 0.5f, 1.0f);

        // LED dot + Text
        const float dotSize = 5.0f;
        const float dotX = bounds.getX() + 5.0f;
        const float dotY = bounds.getCentreY() - dotSize * 0.5f;
        g.setColour(isOn ? neonAmber : neonGreen);
        g.fillEllipse(dotX, dotY, dotSize, dotSize);

        g.setColour(isOn ? textSecondary : textPrimary);
        g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
        auto textArea = bounds.withTrimmedLeft(12.0f);
        g.drawText(isOn ? "BYP" : "ON", textArea, juce::Justification::centred, false);
        return;
    }

    if (isPill)
    {
        // Compact illuminated pill toggle (PHAT, LINK, SYNC)
        const auto accent = button.getProperties().contains("accentColor")
            ? juce::Colour(static_cast<juce::uint32>(static_cast<int>(button.getProperties()["accentColor"])))
            : neonGreen;

        g.setColour(isOn ? accent.withAlpha(0.2f) : cardBg);
        g.fillRoundedRectangle(bounds, 4.0f);

        g.setColour(isOn ? accent : cardBorder);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        g.setColour(isOn ? accent : textSecondary);
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.drawText(button.getButtonText(), bounds, juce::Justification::centred, false);
        return;
    }

    // Default rocker switch
    const float toggleWidth = 28.0f;
    const float toggleHeight = 14.0f;
    auto toggleArea = bounds.removeFromLeft(toggleWidth).withHeight(toggleHeight).withY(bounds.getCentreY() - toggleHeight * 0.5f);

    g.setColour(isOn ? neonGreen.darker(0.3f) : trackBg);
    g.fillRoundedRectangle(toggleArea, toggleHeight * 0.5f);
    g.setColour(cardBorder);
    g.drawRoundedRectangle(toggleArea, toggleHeight * 0.5f, 1.0f);

    const float puckSize = toggleHeight - 4.0f;
    const float puckX = isOn ? (toggleArea.getRight() - puckSize - 2.0f) : (toggleArea.getX() + 2.0f);
    const float puckY = toggleArea.getY() + 2.0f;

    g.setColour(isOn ? neonGreen : textSecondary);
    g.fillEllipse(puckX, puckY, puckSize, puckSize);

    g.setColour(textPrimary);
    g.setFont(11.0f);
    bounds.removeFromLeft(6.0f);
    g.drawText(button.getButtonText(), bounds, juce::Justification::centredLeft, true);
}

void WvZzLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                           juce::Button& button,
                                           const juce::Colour& backgroundColour,
                                           bool shouldDrawButtonAsHighlighted,
                                           bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(backgroundColour);
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);

    juce::Colour bg = cardBg;
    if (shouldDrawButtonAsDown)
        bg = cardBg.darker(0.2f);
    else if (shouldDrawButtonAsHighlighted)
        bg = cardBg.brighter(0.1f);

    g.setColour(bg);
    g.fillRoundedRectangle(bounds, 4.0f);

    juce::Colour border = cardBorder;
    if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
        border = neonGreen;

    g.setColour(border);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

void WvZzLookAndFeel::drawComboBox(juce::Graphics& g,
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

void WvZzLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.fillAll(cardBg);
    g.setColour(cardBorder);
    g.drawRect(0, 0, width, height, 1);
}

} // namespace wvzz::ui
