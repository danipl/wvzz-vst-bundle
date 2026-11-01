#include "XYPadComponent.h"

namespace camelphat::ui {

XYPadComponent::XYPadComponent(juce::AudioProcessorValueTreeState& state,
                               const juce::String& paramXId,
                               const juce::String& paramYId,
                               const juce::String& title,
                               const juce::String& xLabel,
                               const juce::String& yLabel,
                               juce::Colour padAccent)
    : apvts(state),
      xId(paramXId),
      yId(paramYId),
      padTitle(title),
      labelX(xLabel),
      labelY(yLabel),
      accentColor(padAccent)
{
    paramX = apvts.getParameter(xId);
    paramY = apvts.getParameter(yId);

    if (paramX != nullptr)
        currentNormX.store(paramX->getValue(), std::memory_order_relaxed);
    if (paramY != nullptr)
        currentNormY.store(paramY->getValue(), std::memory_order_relaxed);

    apvts.addParameterListener(xId, this);
    apvts.addParameterListener(yId, this);

    setMouseCursor(juce::MouseCursor::CrosshairCursor);
}

XYPadComponent::~XYPadComponent()
{
    apvts.removeParameterListener(xId, this);
    apvts.removeParameterListener(yId, this);
}

void XYPadComponent::resized()
{
}

void XYPadComponent::parameterChanged(const juce::String& parameterID, float newValue)
{
    juce::ignoreUnused(newValue);

    if (paramX != nullptr && parameterID == xId)
        currentNormX.store(paramX->getValue(), std::memory_order_relaxed);
    else if (paramY != nullptr && parameterID == yId)
        currentNormY.store(paramY->getValue(), std::memory_order_relaxed);

    juce::MessageManager::callAsync([this] { repaint(); });
}

void XYPadComponent::updateFromMouse(const juce::MouseEvent& e)
{
    auto padArea = getLocalBounds().toFloat().reduced(4.0f);
    if (padArea.getWidth() <= 0.0f || padArea.getHeight() <= 0.0f)
        return;

    const float normX = juce::jlimit(0.0f, 1.0f, (e.position.x - padArea.getX()) / padArea.getWidth());
    const float normY = juce::jlimit(0.0f, 1.0f, 1.0f - (e.position.y - padArea.getY()) / padArea.getHeight());

    currentNormX.store(normX, std::memory_order_relaxed);
    currentNormY.store(normY, std::memory_order_relaxed);

    if (paramX != nullptr)
        paramX->setValueNotifyingHost(normX);
    if (paramY != nullptr)
        paramY->setValueNotifyingHost(normY);

    repaint();
}

void XYPadComponent::mouseDown(const juce::MouseEvent& e)
{
    isDragging = true;
    updateFromMouse(e);
}

void XYPadComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (isDragging)
        updateFromMouse(e);
}

void XYPadComponent::mouseUp(const juce::MouseEvent&)
{
    isDragging = false;
    repaint();
}

juce::Point<float> XYPadComponent::getPuckScreenPosition() const noexcept
{
    auto padArea = getLocalBounds().toFloat().reduced(4.0f);
    const float px = padArea.getX() + currentNormX.load(std::memory_order_relaxed) * padArea.getWidth();
    const float py = padArea.getY() + (1.0f - currentNormY.load(std::memory_order_relaxed)) * padArea.getHeight();
    return { px, py };
}

void XYPadComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    // 1. Pad Background
    g.setColour(CamelPhatLookAndFeel::cardBg.darker(0.2f));
    g.fillRoundedRectangle(bounds, 6.0f);

    g.setColour(CamelPhatLookAndFeel::cardBorder);
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

    // 2. Subtle coordinate grid lines
    g.setColour(CamelPhatLookAndFeel::cardBorder.withAlpha(0.35f));
    const float midX = bounds.getCentreX();
    const float midY = bounds.getCentreY();
    const float dashLengths[] = { 4.0f, 4.0f };
    g.drawDashedLine(juce::Line<float>(bounds.getX(), midY, bounds.getRight(), midY), dashLengths, 2, 1.0f);
    g.drawDashedLine(juce::Line<float>(midX, bounds.getY(), midX, bounds.getBottom()), dashLengths, 2, 1.0f);

    // 3. Dynamic Crosshairs at current puck position
    auto puck = getPuckScreenPosition();
    g.setColour(accentColor.withAlpha(isDragging ? 0.45f : 0.25f));
    g.drawLine(bounds.getX(), puck.y, bounds.getRight(), puck.y, 1.0f);
    g.drawLine(puck.x, bounds.getY(), puck.x, bounds.getBottom(), 1.0f);

    // 4. Glowing Position Puck
    const float puckRadius = isDragging ? 8.0f : 6.0f;
    g.setColour(accentColor.withAlpha(0.25f));
    g.fillEllipse(puck.x - puckRadius * 1.6f, puck.y - puckRadius * 1.6f, puckRadius * 3.2f, puckRadius * 3.2f);

    g.setColour(accentColor);
    g.fillEllipse(puck.x - puckRadius, puck.y - puckRadius, puckRadius * 2.0f, puckRadius * 2.0f);

    g.setColour(CamelPhatLookAndFeel::textPrimary);
    g.fillEllipse(puck.x - 2.0f, puck.y - 2.0f, 4.0f, 4.0f);

    // 5. Title & Axis Labels
    g.setColour(CamelPhatLookAndFeel::textPrimary.withAlpha(0.85f));
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText(padTitle, bounds.reduced(6.0f), juce::Justification::topLeft, false);

    g.setColour(CamelPhatLookAndFeel::textSecondary.withAlpha(0.7f));
    g.setFont(juce::FontOptions(9.5f, juce::Font::plain));
    g.drawText("X: " + labelX, bounds.reduced(6.0f), juce::Justification::bottomLeft, false);
    g.drawText("Y: " + labelY, bounds.reduced(6.0f), juce::Justification::topRight, false);
}

} // namespace camelphat::ui
