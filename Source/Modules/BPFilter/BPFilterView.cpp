#include "BPFilterView.h"

namespace wvzz::modules {

void BPFilterView::PassbandVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(ui::WvZzLookAndFeel::trackBg);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(ui::WvZzLookAndFeel::cardBorder);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    const float lowProp = static_cast<float>(lowSlider.valueToProportionOfLength(lowSlider.getValue()));
    const float highProp = static_cast<float>(highSlider.valueToProportionOfLength(highSlider.getValue()));

    const float yLow = bounds.getBottom() - lowProp * bounds.getHeight();
    const float yHigh = bounds.getBottom() - highProp * bounds.getHeight();

    const float passTop = std::min(yLow, yHigh);
    const float passBottom = std::max(yLow, yHigh);

    if (passBottom - passTop > 1.0f)
    {
        auto passRect = juce::Rectangle<float>(bounds.getX() + 2.0f, passTop,
                                               bounds.getWidth() - 4.0f, passBottom - passTop);
        g.setColour(ui::WvZzLookAndFeel::neonCyan.withAlpha(0.28f));
        g.fillRoundedRectangle(passRect, 2.0f);
    }
}

BPFilterView::BPFilterView(juce::AudioProcessorValueTreeState& apvts,
                           const juce::String& prefix,
                           ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("BP Filter", ui::WvZzLookAndFeel::neonCyan)
{
    addAndMakeVisible(card);

    auto paramId = [this](const juce::String& name) {
        return instancePrefix.isEmpty() ? name : instancePrefix + "_" + name;
    };

    // Header Bypass
    bypassToggle.button.getProperties().set("isHeaderBypass", true);
    bypassToggle.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsState, paramId("bypass"), bypassToggle.button);
    bypassToggle.button.onClick = [this] {
        card.setBypassed(bypassToggle.button.getToggleState());
    };
    addAndMakeVisible(bypassToggle.button);
    card.setBypassed(bypassToggle.button.getToggleState());

    // Header Help Button
    if (helpHost != nullptr)
    {
        helpButton = std::make_unique<ui::HelpButtonComponent>("bp_filter", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    // Sliders
    auto setupSlider = [this](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::LinearVertical);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(s);
    };

    setupSlider(lowCutSlider);
    setupSlider(highCutSlider);

    lowCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsState, paramId("low_cut"), lowCutSlider);
    highCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsState, paramId("high_cut"), highCutSlider);

    passbandVisualizer = std::make_unique<PassbandVisualizer>(lowCutSlider, highCutSlider);
    addAndMakeVisible(*passbandVisualizer);

    // Link Toggle
    linkToggle.button.setButtonText("LINK");
    linkToggle.button.getProperties().set("isPill", true);
    linkToggle.button.getProperties().set("accentColor", static_cast<int>(ui::WvZzLookAndFeel::neonCyan.getARGB()));
    linkToggle.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsState, paramId("link"), linkToggle.button);
    addAndMakeVisible(linkToggle.button);

    // Dynamic Linking & Repaint callbacks
    lowCutSlider.onValueChange = [this] {
        if (isUpdatingCutoffs) return;
        const double curLow = lowCutSlider.getValue();
        const double curHigh = highCutSlider.getValue();
        const bool isLinked = linkToggle.button.getToggleState();

        isUpdatingCutoffs = true;
        if (isLinked)
        {
            const double ratio = lastHighCutVal / std::max(1.0, lastLowCutVal);
            const double newHigh = juce::jlimit(curLow, 20000.0, curLow * ratio);
            highCutSlider.setValue(newHigh, juce::sendNotificationSync);
            lastLowCutVal = curLow;
            lastHighCutVal = newHigh;
        }
        else
        {
            if (curLow > curHigh)
                highCutSlider.setValue(curLow, juce::sendNotificationSync);
            lastLowCutVal = curLow;
            lastHighCutVal = highCutSlider.getValue();
        }
        isUpdatingCutoffs = false;
        if (passbandVisualizer != nullptr) passbandVisualizer->repaint();
    };

    highCutSlider.onValueChange = [this] {
        if (isUpdatingCutoffs) return;
        const double curLow = lowCutSlider.getValue();
        const double curHigh = highCutSlider.getValue();
        const bool isLinked = linkToggle.button.getToggleState();

        isUpdatingCutoffs = true;
        if (isLinked)
        {
            const double ratio = lastHighCutVal / std::max(1.0, lastLowCutVal);
            const double newLow = juce::jlimit(20.0, curHigh, curHigh / std::max(0.01, ratio));
            lowCutSlider.setValue(newLow, juce::sendNotificationSync);
            lastLowCutVal = newLow;
            lastHighCutVal = curHigh;
        }
        else
        {
            if (curHigh < curLow)
                lowCutSlider.setValue(curHigh, juce::sendNotificationSync);
            lastLowCutVal = lowCutSlider.getValue();
            lastHighCutVal = curHigh;
        }
        isUpdatingCutoffs = false;
        if (passbandVisualizer != nullptr) passbandVisualizer->repaint();
    };

    linkToggle.button.onClick = [this] {
        lastLowCutVal = lowCutSlider.getValue();
        lastHighCutVal = highCutSlider.getValue();
    };

    lowCutLabel.setText("Low Cut", juce::dontSendNotification);
    lowCutLabel.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    lowCutLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lowCutLabel);

    highCutLabel.setText("High Cut", juce::dontSendNotification);
    highCutLabel.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    highCutLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(highCutLabel);

    // Rotary Knobs
    initRotary(lowResKnob, paramId("low_res"), "Low Res", ui::WvZzLookAndFeel::neonCyan);
    initRotary(highResKnob, paramId("high_res"), "High Res", ui::WvZzLookAndFeel::neonCyan);
    initRotary(brMixKnob, paramId("br_mix"), "BR Mix", ui::WvZzLookAndFeel::neonCyan);
}

void BPFilterView::initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText, juce::Colour accent)
{
    rc.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rc.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    rc.slider.getProperties().set("accentColor", static_cast<int>(accent.getARGB()));
    rc.slider.setColour(juce::Slider::rotarySliderFillColourId, accent);
    addAndMakeVisible(rc.slider);

    rc.label.setText(labelText, juce::dontSendNotification);
    rc.label.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    rc.label.setJustificationType(juce::Justification::centred);
    rc.label.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::textSecondary);
    addAndMakeVisible(rc.label);

    rc.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsState, paramId, rc.slider);
}

void BPFilterView::paint(juce::Graphics&)
{
}

void BPFilterView::updateVisuals()
{
    if (passbandVisualizer != nullptr)
        passbandVisualizer->repaint();
}

void BPFilterView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();

    // Upper area for faders, passband visualizer, and labels (approx height 155px)
    auto topArea = content.removeFromTop(155);

    const int sliderWidth = 32;
    const int passbandWidth = 38;
    const int sliderX = content.getX() + 18;
    const int faderHeight = 110;
    const int faderY = topArea.getY() + 10;

    lowCutSlider.setBounds(sliderX, faderY, sliderWidth, faderHeight);
    passbandVisualizer->setBounds(sliderX + sliderWidth + 4, faderY, passbandWidth, faderHeight);
    highCutSlider.setBounds(sliderX + sliderWidth + passbandWidth + 8, faderY, sliderWidth, faderHeight);

    lowCutLabel.setBounds(sliderX - 6, faderY + faderHeight + 4, sliderWidth + 12, 16);
    highCutLabel.setBounds(sliderX + sliderWidth + passbandWidth + 2, faderY + faderHeight + 4, sliderWidth + 12, 16);

    linkToggle.button.setBounds(sliderX + sliderWidth + 4, faderY + faderHeight + 2, passbandWidth, 18);

    // Lower area for 3 rotary knobs: Low Res, High Res, BR Mix
    auto bottomArea = content.removeFromTop(75);
    const int knobW = bottomArea.getWidth() / 3;
    const int knobSize = 48;
    const int knobY = bottomArea.getY() + 4;

    auto placeKnob = [&](RotaryControl& rc, int col) {
        int cx = bottomArea.getX() + col * knobW + (knobW - knobSize) / 2;
        rc.slider.setBounds(cx, knobY, knobSize, knobSize);
        rc.label.setBounds(bottomArea.getX() + col * knobW, knobY + knobSize + 2, knobW, 14);
    };

    placeKnob(lowResKnob, 0);
    placeKnob(highResKnob, 1);
    placeKnob(brMixKnob, 2);
}

} // namespace wvzz::modules
