#include "CamelPhatLookAndFeel.h"

namespace camelphat::ui {

const juce::Colour CamelPhatLookAndFeel::bgBase        = juce::Colour(0xff0d1015);
const juce::Colour CamelPhatLookAndFeel::cardBg        = juce::Colour(0xff151a22);
const juce::Colour CamelPhatLookAndFeel::cardBorder    = juce::Colour(0xff222a36);
const juce::Colour CamelPhatLookAndFeel::cardHighlight = juce::Colour(0xff323e4f);
const juce::Colour CamelPhatLookAndFeel::trackBg       = juce::Colour(0xff181f28);
const juce::Colour CamelPhatLookAndFeel::textPrimary   = juce::Colour(0xfff0f4f8);
const juce::Colour CamelPhatLookAndFeel::textSecondary = juce::Colour(0xff8a99a8);

// Curated Luminous Module Palette
const juce::Colour CamelPhatLookAndFeel::coralRed       = juce::Colour(0xffff4545);
const juce::Colour CamelPhatLookAndFeel::skyCyan        = juce::Colour(0xff00e5ff);
const juce::Colour CamelPhatLookAndFeel::honeyGold      = juce::Colour(0xffffb300);
const juce::Colour CamelPhatLookAndFeel::emeraldGreen   = juce::Colour(0xff00e676);
const juce::Colour CamelPhatLookAndFeel::electricViolet = juce::Colour(0xffb388ff);
const juce::Colour CamelPhatLookAndFeel::warmAmber      = juce::Colour(0xffff9100);
const juce::Colour CamelPhatLookAndFeel::arcticIce      = juce::Colour(0xffe0f7fa);

// Legacy Aliases
const juce::Colour CamelPhatLookAndFeel::neonGreen     = CamelPhatLookAndFeel::emeraldGreen;
const juce::Colour CamelPhatLookAndFeel::neonAmber     = CamelPhatLookAndFeel::warmAmber;
const juce::Colour CamelPhatLookAndFeel::neonCyan      = CamelPhatLookAndFeel::skyCyan;

CamelPhatLookAndFeel::CamelPhatLookAndFeel()
{
    setColour(juce::Slider::textBoxOutlineColourId, cardHighlight.withAlpha(0.5f));
    setColour(juce::Slider::textBoxBackgroundColourId, cardBg.darker(0.25f));
    setColour(juce::Slider::textBoxTextColourId, textPrimary);
    setColour(juce::Slider::textBoxHighlightColourId, skyCyan.withAlpha(0.35f));
    setColour(juce::ComboBox::backgroundColourId, cardBg);
    setColour(juce::ComboBox::outlineColourId, cardBorder);
    setColour(juce::ComboBox::textColourId, textPrimary);
    setColour(juce::PopupMenu::backgroundColourId, cardBg.darker(0.2f));
    setColour(juce::PopupMenu::textColourId, textPrimary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, cardBorder);
    setColour(juce::PopupMenu::highlightedTextColourId, skyCyan);
}

void CamelPhatLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                       int x, int y, int width, int height,
                                       float sliderPosProportional,
                                       float rotaryStartAngle,
                                       float rotaryEndAngle,
                                       juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    const bool isHovered = slider.isMouseOverOrDragging();

    // Reserve bottom space for value readout if slider is tall enough
    const bool showValue = height >= 50;
    const float valueHeight = showValue ? 13.0f : 0.0f;
    auto dialArea = bounds.removeFromTop(bounds.getHeight() - valueHeight);

    // Keep dial compact, balanced, and responsive to scale
    const float maxRadius = juce::jmin(dialArea.getWidth(), dialArea.getHeight()) * 0.5f - 2.0f;
    const float radius = juce::jmax(8.0f, maxRadius);
    const float centreX = dialArea.getCentreX();
    const float centreY = dialArea.getCentreY();

    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    // Slender precision track
    const float trackWidth = juce::jlimit(2.0f, 3.2f, radius * 0.13f);

    // Check if slider is configured as bipolar
    const bool isBipolar = slider.getProperties().contains("isBipolar") &&
                           static_cast<bool>(slider.getProperties()["isBipolar"]);

    // Arc accent color
    juce::Colour arcColor = emeraldGreen;
    if (slider.getProperties().contains("accentColor"))
        arcColor = juce::Colour(static_cast<juce::uint32>(static_cast<int>(slider.getProperties()["accentColor"])));

    // 1. Inactive Track Background Arc (subtle, recessed)
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, radius - trackWidth * 0.5f, radius - trackWidth * 0.5f,
                                0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(trackBg.withAlpha(0.85f));
    g.strokePath(backgroundArc, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 2. Active Glowing Progress Arc (Two-pass vector stroke glow)
    auto drawGlowArc = [&](const juce::Path& path)
    {
        // Pass 1: Diffused outer glow stroke
        const float glowWidth = trackWidth * (isHovered ? 2.4f : 1.9f);
        const float glowAlpha = isHovered ? 0.35f : 0.18f;
        g.setColour(arcColor.withAlpha(glowAlpha));
        g.strokePath(path, juce::PathStrokeType(glowWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Pass 2: Crisp inner luminous stroke
        const auto coreColor = isHovered ? arcColor.brighter(0.12f) : arcColor;
        g.setColour(coreColor);
        g.strokePath(path, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    };

    if (isBipolar)
    {
        const float midAngle = rotaryStartAngle + 0.5f * (rotaryEndAngle - rotaryStartAngle);

        // 12 o'clock center tick notch
        g.setColour(cardHighlight.withAlpha(0.7f));
        const float tickLen = trackWidth * 1.4f;
        g.drawLine(centreX, centreY - radius, centreX, centreY - radius + tickLen, 1.2f);

        if (std::abs(sliderPosProportional - 0.5f) > 0.005f)
        {
            juce::Path valueArc;
            const float startA = std::min(midAngle, angle);
            const float endA   = std::max(midAngle, angle);
            valueArc.addCentredArc(centreX, centreY, radius - trackWidth * 0.5f, radius - trackWidth * 0.5f,
                                   0.0f, startA, endA, true);
            drawGlowArc(valueArc);
        }
    }
    else if (sliderPosProportional > 0.001f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, radius - trackWidth * 0.5f, radius - trackWidth * 0.5f,
                                0.0f, rotaryStartAngle, angle, true);
        drawGlowArc(valueArc);
    }

    // 3. Inner Flat Matte Cap (minimalist disc)
    const float innerRadius = radius - trackWidth * 1.6f;
    if (innerRadius > 4.0f)
    {
        auto innerArea = juce::Rectangle<float>(centreX - innerRadius, centreY - innerRadius,
                                                innerRadius * 2.0f, innerRadius * 2.0f);

        // Subtle dark slate matte gradient fill
        const auto topCap = isHovered ? cardBg.brighter(0.04f) : cardBg.darker(0.05f);
        const auto botCap = isHovered ? cardBg.darker(0.12f) : cardBg.darker(0.22f);
        juce::ColourGradient capGrad(topCap, centreX, innerArea.getY(),
                                     botCap, centreX, innerArea.getBottom(), false);
        g.setGradientFill(capGrad);
        g.fillEllipse(innerArea);

        // Fine hairline perimeter edge
        g.setColour(isHovered ? cardHighlight.withAlpha(0.6f) : cardBorder.withAlpha(0.4f));
        g.drawEllipse(innerArea, 0.8f);

        // 4. Perimeter Rim Tick Indicator (perimeter notch at outer edge of cap)
        const float tickOuter = innerRadius - 1.2f;
        const float tickInner = innerRadius - juce::jlimit(3.0f, 5.0f, innerRadius * 0.35f);
        const float sinA = std::sin(angle);
        const float cosA = std::cos(angle);

        // Pointer notch tick
        const auto tickColor = isHovered ? juce::Colours::white : textPrimary.withAlpha(0.9f);
        g.setColour(tickColor);
        g.drawLine(centreX + tickInner * sinA, centreY - tickInner * cosA,
                   centreX + tickOuter * sinA, centreY - tickOuter * cosA,
                   1.8f);
    }

    // 5. Interactive Value Text Readout
    if (showValue)
    {
        juce::String valStr = slider.getTextFromValue(slider.getValue());
        g.setFont(juce::FontOptions(isHovered ? 10.0f : 9.5f, isHovered ? juce::Font::bold : juce::Font::plain));

        // High-contrast illumination on hover/drag
        if (isHovered)
            g.setColour(juce::Colours::white);
        else
            g.setColour(textSecondary.withAlpha(0.85f));

        g.drawText(valStr, bounds, juce::Justification::centred, true);
    }
}

void CamelPhatLookAndFeel::drawLinearSlider(juce::Graphics& g,
                                       int x, int y, int width, int height,
                                       float sliderPos,
                                       float minSliderPos,
                                       float maxSliderPos,
                                       juce::Slider::SliderStyle style,
                                       juce::Slider& slider)
{
    juce::ignoreUnused(minSliderPos, maxSliderPos, style);

    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    const bool isHovered = slider.isMouseOverOrDragging();

    juce::Colour accent = skyCyan;
    if (slider.getProperties().contains("accentColor"))
        accent = juce::Colour(static_cast<juce::uint32>(static_cast<int>(slider.getProperties()["accentColor"])));

    const float trackW = 4.0f;
    const float cx = bounds.getCentreX();

    const float trackTop = bounds.getY() + 4.0f;
    const float trackBottom = bounds.getBottom() - 4.0f;
    const float trackH = std::max(1.0f, trackBottom - trackTop);

    auto trackRect = juce::Rectangle<float>(cx - trackW * 0.5f, trackTop, trackW, trackH);

    // 1. Recessed Track Background
    g.setColour(trackBg.withAlpha(0.85f));
    g.fillRoundedRectangle(trackRect, 2.0f);
    g.setColour(cardBorder.withAlpha(0.4f));
    g.drawRoundedRectangle(trackRect, 2.0f, 0.8f);

    // 2. Active Glowing Track Fill
    const float clampedPos = juce::jlimit(trackTop, trackBottom, sliderPos);
    auto activeRect = juce::Rectangle<float>(trackRect.getX(), clampedPos, trackRect.getWidth(), trackBottom - clampedPos);

    if (activeRect.getHeight() > 0.5f)
    {
        // Outer soft glow
        g.setColour(accent.withAlpha(isHovered ? 0.35f : 0.18f));
        g.fillRoundedRectangle(activeRect.expanded(1.5f, 0.0f), 2.5f);

        // Core fill
        g.setColour(isHovered ? accent.brighter(0.1f) : accent);
        g.fillRoundedRectangle(activeRect, 2.0f);
    }

    // 3. Minimalist Capsule Fader Thumb
    const float thumbH = 12.0f;
    const float halfThumbH = thumbH * 0.5f;
    const float thumbW = juce::jmin(bounds.getWidth() - 2.0f, 22.0f);
    auto thumbRect = juce::Rectangle<float>(cx - thumbW * 0.5f, clampedPos - halfThumbH, thumbW, thumbH);

    // Dark glass body
    juce::ColourGradient thumbGrad(cardBg.brighter(0.1f), cx, thumbRect.getY(),
                                   cardBg.darker(0.2f), cx, thumbRect.getBottom(), false);
    g.setGradientFill(thumbGrad);
    g.fillRoundedRectangle(thumbRect, halfThumbH);

    // Glowing border highlight
    g.setColour(isHovered ? accent : accent.withAlpha(0.7f));
    g.drawRoundedRectangle(thumbRect, halfThumbH, 1.0f);

    // Precision center notch line
    g.setColour(isHovered ? juce::Colours::white : textPrimary.withAlpha(0.9f));
    g.drawHorizontalLine(static_cast<int>(clampedPos), thumbRect.getX() + 4.0f, thumbRect.getRight() - 4.0f);
}

void CamelPhatLookAndFeel::drawToggleButton(juce::Graphics& g,
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
        // Sleek translucent glass pill in card header
        g.setColour(isOn ? cardBg.darker(0.2f) : cardBg.brighter(0.05f));
        g.fillRoundedRectangle(bounds, bounds.getHeight() * 0.5f);

        const auto borderColor = isOn ? warmAmber.withAlpha(0.85f) : emeraldGreen.withAlpha(0.85f);
        g.setColour(borderColor);
        g.drawRoundedRectangle(bounds, bounds.getHeight() * 0.5f, 1.0f);

        // Luminous LED dot
        const float dotSize = 4.5f;
        const float dotX = bounds.getX() + 6.0f;
        const float dotY = bounds.getCentreY() - dotSize * 0.5f;

        g.setColour((isOn ? warmAmber : emeraldGreen).withAlpha(0.3f));
        g.fillEllipse(dotX - 1.5f, dotY - 1.5f, dotSize + 3.0f, dotSize + 3.0f);

        g.setColour(isOn ? warmAmber : emeraldGreen);
        g.fillEllipse(dotX, dotY, dotSize, dotSize);

        g.setColour(isOn ? textSecondary : textPrimary);
        g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
        auto textArea = bounds.withTrimmedLeft(14.0f);
        g.drawText(isOn ? "BYP" : "ON", textArea, juce::Justification::centred, false);
        return;
    }

    if (isPill)
    {
        // Compact illuminated pill toggle (PHAT, LINK, SYNC)
        const auto accent = button.getProperties().contains("accentColor")
            ? juce::Colour(static_cast<juce::uint32>(static_cast<int>(button.getProperties()["accentColor"])))
            : emeraldGreen;

        const float corner = 4.0f;
        g.setColour(isOn ? accent.withAlpha(0.22f) : cardBg.darker(0.15f));
        g.fillRoundedRectangle(bounds, corner);

        g.setColour(isOn ? accent : cardBorder.withAlpha(0.5f));
        g.drawRoundedRectangle(bounds, corner, 1.0f);

        g.setColour(isOn ? accent.brighter(0.2f) : textSecondary);
        g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
        g.drawText(button.getButtonText(), bounds, juce::Justification::centred, false);
        return;
    }

    // Default rocker switch
    const float toggleWidth = 28.0f;
    const float toggleHeight = 14.0f;
    auto toggleArea = bounds.removeFromLeft(toggleWidth).withHeight(toggleHeight).withY(bounds.getCentreY() - toggleHeight * 0.5f);

    g.setColour(isOn ? emeraldGreen.darker(0.3f) : trackBg);
    g.fillRoundedRectangle(toggleArea, toggleHeight * 0.5f);
    g.setColour(cardBorder);
    g.drawRoundedRectangle(toggleArea, toggleHeight * 0.5f, 1.0f);

    const float puckSize = toggleHeight - 4.0f;
    const float puckX = isOn ? (toggleArea.getRight() - puckSize - 2.0f) : (toggleArea.getX() + 2.0f);
    const float puckY = toggleArea.getY() + 2.0f;

    g.setColour(isOn ? emeraldGreen : textSecondary);
    g.fillEllipse(puckX, puckY, puckSize, puckSize);

    g.setColour(textPrimary);
    g.setFont(11.0f);
    bounds.removeFromLeft(6.0f);
    g.drawText(button.getButtonText(), bounds, juce::Justification::centredLeft, true);
}

void CamelPhatLookAndFeel::drawButtonBackground(juce::Graphics& g,
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
        border = emeraldGreen;

    g.setColour(border);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

void CamelPhatLookAndFeel::drawComboBox(juce::Graphics& g,
                                   int width, int height,
                                   bool isButtonDown,
                                   int buttonX, int buttonY, int buttonW, int buttonH,
                                   juce::ComboBox& box)
{
    juce::ignoreUnused(isButtonDown, buttonX, buttonY, buttonW, buttonH);
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(0.5f);

    juce::ColourGradient cbGrad(cardBg.brighter(0.02f), bounds.getCentreX(), bounds.getY(),
                                cardBg.darker(0.18f), bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(cbGrad);
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(cardHighlight.withAlpha(0.4f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    // Elegant minimal down-chevron
    juce::Path arrow;
    const float cx = static_cast<float>(width) - 13.0f;
    const float cy = static_cast<float>(height) * 0.5f;
    arrow.startNewSubPath(cx - 3.5f, cy - 1.5f);
    arrow.lineTo(cx, cy + 2.5f);
    arrow.lineTo(cx + 3.5f, cy - 1.5f);

    g.setColour(skyCyan);
    g.strokePath(arrow, juce::PathStrokeType(1.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::ignoreUnused(box);
}

void CamelPhatLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.fillAll(cardBg.darker(0.2f));
    g.setColour(cardBorder);
    g.drawRect(0, 0, width, height, 1);
}

} // namespace camelphat::ui
