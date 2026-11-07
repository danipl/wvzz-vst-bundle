#include "BPFilterView.h"

namespace wvzz::modules {

void BPFilterView::PassbandVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const float trackTop = bounds.getY() + 4.0f;
    const float trackBottom = bounds.getBottom() - 4.0f;
    const float trackHeight = std::max(1.0f, trackBottom - trackTop);

    auto bgRect = juce::Rectangle<float>(bounds.getX(), trackTop, bounds.getWidth(), trackHeight);

    g.setColour(ui::WvZzLookAndFeel::trackBg);
    g.fillRoundedRectangle(bgRect, 3.0f);
    g.setColour(ui::WvZzLookAndFeel::cardBorder);
    g.drawRoundedRectangle(bgRect, 3.0f, 1.0f);

    const float lowProp = static_cast<float>(lowSlider.valueToProportionOfLength(lowSlider.getValue()));
    const float highProp = static_cast<float>(highSlider.valueToProportionOfLength(highSlider.getValue()));

    // y position matching track
    const float yLow = trackBottom - lowProp * trackHeight;
    const float yHigh = trackBottom - highProp * trackHeight;

    const float passTop = std::min(yLow, yHigh);
    const float passBottom = std::max(yLow, yHigh);

    if (passBottom - passTop > 1.0f)
    {
        auto passRect = juce::Rectangle<float>(bgRect.getX() + 1.0f, passTop,
                                               bgRect.getWidth() - 2.0f, passBottom - passTop);
        g.setColour(ui::WvZzLookAndFeel::warmAmber.withAlpha(0.28f));
        g.fillRoundedRectangle(passRect, 2.0f);
    }
}

BPFilterView::BPFilterView(juce::AudioProcessorValueTreeState& apvts,
                           const juce::String& prefix,
                           ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("BP FILTER & SPLIT", ui::WvZzLookAndFeel::warmAmber)
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
        s.getProperties().set("accentColor", static_cast<int>(ui::WvZzLookAndFeel::warmAmber.getARGB()));
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
    linkToggle.button.getProperties().set("accentColor", static_cast<int>(ui::WvZzLookAndFeel::warmAmber.getARGB()));
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
    initRotary(lowResKnob, paramId("low_res"), "Low Res", ui::WvZzLookAndFeel::warmAmber);
    initRotary(highResKnob, paramId("high_res"), "High Res", ui::WvZzLookAndFeel::warmAmber);
    initRotary(brMixKnob, paramId("br_mix"), "BR Mix", ui::WvZzLookAndFeel::warmAmber);
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

    // Link pill button at top of BP card
    auto bpTopRow = content.removeFromTop(20);
    linkToggle.button.setBounds(bpTopRow.removeFromRight(55));

    // Vertical Cutoff Sliders Area (Height 150)
    auto bpSliderArea = content.removeFromTop(150);
    auto s1 = bpSliderArea.removeFromLeft(bpSliderArea.getWidth() / 2 - 12);
    lowCutLabel.setBounds(s1.removeFromBottom(16));
    lowCutSlider.setBounds(s1.reduced(10, 0));

    auto s2 = bpSliderArea.removeFromRight(bpSliderArea.getWidth() - 24);
    highCutLabel.setBounds(s2.removeFromBottom(16));
    highCutSlider.setBounds(s2.reduced(10, 0));

    if (passbandVisualizer != nullptr)
    {
        // Align passband visualizer top and bottom precisely with the sliders' bounds
        auto sBounds = lowCutSlider.getBounds();
        passbandVisualizer->setBounds(bpSliderArea.getX() + 2, sBounds.getY(),
                                      bpSliderArea.getWidth() - 4, sBounds.getHeight());
    }

    content.removeFromTop(6);

    auto layoutKnob = [](RotaryControl& rc, juce::Rectangle<int> r) {
        int labelH = 14;
        auto knobArea = r.reduced(1);
        rc.label.setBounds(knobArea.removeFromBottom(labelH));
        rc.slider.setBounds(knobArea);
    };

    // Bottom Knobs Row
    int bpKw = content.getWidth() / 3;
    layoutKnob(lowResKnob, content.removeFromLeft(bpKw));
    layoutKnob(highResKnob, content.removeFromLeft(bpKw));
    layoutKnob(brMixKnob, content);
}

} // namespace wvzz::modules
