#include "ModulationView.h"

namespace wvzz::modules {

void ModulationView::LFOScopeComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(ui::WvZzLookAndFeel::cardBg.darker(0.25f));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(ui::WvZzLookAndFeel::cardBorder);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    const float midY = bounds.getCentreY();
    g.setColour(ui::WvZzLookAndFeel::cardBorder.withAlpha(0.4f));
    g.drawHorizontalLine(static_cast<int>(midY), bounds.getX() + 2.0f, bounds.getRight() - 2.0f);

    const int wave = waveFn ? waveFn() : 0;
    const float phase = phaseFn ? phaseFn() : 0.0f;

    juce::Path wavePath;
    const int numPoints = static_cast<int>(bounds.getWidth() - 4.0f);
    for (int i = 0; i <= numPoints; ++i)
    {
        const float x = bounds.getX() + 2.0f + static_cast<float>(i);
        const float p = static_cast<float>(i) / static_cast<float>(numPoints);

        float yVal = 0.0f;
        switch (wave)
        {
            case 0: yVal = std::sin(p * juce::MathConstants<float>::twoPi); break;
            case 1: yVal = 4.0f * std::abs(p - 0.5f) - 1.0f; break;
            case 2: yVal = 1.0f - 2.0f * p; break;
            case 3: yVal = 2.0f * p - 1.0f; break;
            case 4: yVal = (p < 0.5f) ? 0.85f : -0.85f; break;
            default: yVal = std::sin(p * juce::MathConstants<float>::twoPi); break;
        }

        const float y = midY - yVal * (bounds.getHeight() * 0.38f);
        if (i == 0) wavePath.startNewSubPath(x, y);
        else wavePath.lineTo(x, y);
    }

    g.setColour(colour.withAlpha(0.75f));
    g.strokePath(wavePath, juce::PathStrokeType(1.5f));

    const float dotX = bounds.getX() + 2.0f + phase * (bounds.getWidth() - 4.0f);
    const float snap = snapFn ? snapFn() : 0.0f;
    const float dotY = midY - snap * (bounds.getHeight() * 0.38f);
    g.setColour(colour);
    g.fillEllipse(dotX - 3.0f, dotY - 3.0f, 6.0f, 6.0f);
}

ModulationView::ModulationView(juce::AudioProcessorValueTreeState& apvts,
                               ModulationDSP& dsp,
                               dsp::AudioVisualizerSource* visualizerSource,
                               ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      modDSP(dsp),
      card("MODULATION & X/Y", ui::WvZzLookAndFeel::neonGreen)
{
    addAndMakeVisible(card);

    if (helpHost != nullptr)
    {
        helpButton = std::make_unique<ui::HelpButtonComponent>("xy_pad", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    // Setup Center Tab Buttons
    auto setupCenterTabBtn = [this](juce::TextButton& btn) {
        btn.setColour(juce::TextButton::buttonColourId, ui::WvZzLookAndFeel::cardBg.darker(0.2f));
        btn.setColour(juce::TextButton::textColourOffId, ui::WvZzLookAndFeel::textSecondary);
        addAndMakeVisible(btn);
    };
    setupCenterTabBtn(xyTabButton);
    setupCenterTabBtn(monitorTabButton);

    xyTabButton.onClick = [this] { setCenterTab(false); };
    monitorTabButton.onClick = [this] { setCenterTab(true); };

    // X/Y Pad & Visualizer
    xyPad = std::make_unique<ui::XYPadComponent>(apvtsState, "xy_x", "xy_y", "X/Y PAD", "X Target", "Y Target", ui::WvZzLookAndFeel::neonGreen);
    addAndMakeVisible(*xyPad);

    if (visualizerSource != nullptr)
    {
        signalVisualizer = std::make_unique<ui::SignalComparisonComponent>(*visualizerSource);
        addChildComponent(*signalVisualizer);
    }

    const auto& modTargets = getModTargetNames();
    initCombo(xyXTargetBox, "xy_x_target", "X Target", modTargets);
    initCombo(xyYTargetBox, "xy_y_target", "Y Target", modTargets);
    initRotary(xyXDepthKnob, "xy_x_depth", "X Depth", true, ui::WvZzLookAndFeel::neonGreen);
    initRotary(xyYDepthKnob, "xy_y_depth", "Y Depth", true, ui::WvZzLookAndFeel::neonGreen);

    xyXTargetBox.box.onChange = [this, &modTargets] {
        int idx = xyXTargetBox.box.getSelectedItemIndex();
        if (idx >= 0 && idx < modTargets.size())
        {
            int yIdx = xyYTargetBox.box.getSelectedItemIndex();
            xyPad->setAxisLabels(modTargets[idx], yIdx >= 0 ? modTargets[yIdx] : "Y");
        }
    };
    xyYTargetBox.box.onChange = [this, &modTargets] {
        int idx = xyYTargetBox.box.getSelectedItemIndex();
        if (idx >= 0 && idx < modTargets.size())
        {
            int xIdx = xyXTargetBox.box.getSelectedItemIndex();
            xyPad->setAxisLabels(xIdx >= 0 ? modTargets[xIdx] : "X", modTargets[idx]);
        }
    };

    // LFO 1 Panel
    const auto& lfoWaves = getLFOWaveformNames();
    const auto& lfoSyncDivs = getLFOSyncDivNames();

    lfo1Scope = std::make_unique<LFOScopeComponent>(
        [this] { return lfo1WaveBox.box.getSelectedItemIndex(); },
        [this] { return modDSP.getLfo1Phase(); },
        [this] { return modDSP.getLfo1Snapshot(); },
        ui::WvZzLookAndFeel::neonGreen);
    addAndMakeVisible(*lfo1Scope);

    initCombo(lfo1WaveBox, "lfo1_wave", "Wave", lfoWaves);
    initToggle(lfo1SyncToggle, "lfo1_sync", "SYNC", true, ui::WvZzLookAndFeel::neonGreen);
    initCombo(lfo1SyncDivBox, "lfo1_sync_div", "Sync Div", lfoSyncDivs);
    initCombo(lfo1TargetBox, "lfo1_target", "Target", modTargets);
    initRotary(lfo1RateKnob, "lfo1_rate", "Rate", false, ui::WvZzLookAndFeel::neonGreen);
    initRotary(lfo1DepthKnob, "lfo1_depth", "Depth", true, ui::WvZzLookAndFeel::neonGreen);

    // LFO 2 Panel
    lfo2Scope = std::make_unique<LFOScopeComponent>(
        [this] { return lfo2WaveBox.box.getSelectedItemIndex(); },
        [this] { return modDSP.getLfo2Phase(); },
        [this] { return modDSP.getLfo2Snapshot(); },
        ui::WvZzLookAndFeel::neonGreen);
    addAndMakeVisible(*lfo2Scope);

    initCombo(lfo2WaveBox, "lfo2_wave", "Wave", lfoWaves);
    initToggle(lfo2SyncToggle, "lfo2_sync", "SYNC", true, ui::WvZzLookAndFeel::neonGreen);
    initCombo(lfo2SyncDivBox, "lfo2_sync_div", "Sync Div", lfoSyncDivs);
    initCombo(lfo2TargetBox, "lfo2_target", "Target", modTargets);
    initRotary(lfo2RateKnob, "lfo2_rate", "Rate", false, ui::WvZzLookAndFeel::neonGreen);
    initRotary(lfo2DepthKnob, "lfo2_depth", "Depth", true, ui::WvZzLookAndFeel::neonGreen);

    setCenterTab(false);
}

void ModulationView::initToggle(ToggleControl& tc, const juce::String& paramId, const juce::String& text,
                                bool isPill, juce::Colour accent)
{
    tc.button.setButtonText(text);
    tc.button.getProperties().set("isPill", isPill);
    tc.button.getProperties().set("accentColor", static_cast<int>(accent.getARGB()));
    tc.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsState, paramId, tc.button);
    addAndMakeVisible(tc.button);
}

void ModulationView::initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText,
                                bool isBipolar, juce::Colour accent)
{
    rc.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rc.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    rc.slider.getProperties().set("accentColor", static_cast<int>(accent.getARGB()));
    if (isBipolar)
        rc.slider.getProperties().set("isBipolar", true);
    rc.slider.setColour(juce::Slider::rotarySliderFillColourId, accent);
    addAndMakeVisible(rc.slider);

    rc.label.setText(labelText, juce::dontSendNotification);
    rc.label.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    rc.label.setJustificationType(juce::Justification::centred);
    rc.label.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::textSecondary);
    addAndMakeVisible(rc.label);

    rc.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsState, paramId, rc.slider);
}

void ModulationView::initCombo(ComboControl& cc, const juce::String& paramId, const juce::String& labelText,
                               const juce::StringArray& items)
{
    for (int i = 0; i < items.size(); ++i)
        cc.box.addItem(items[i], i + 1);

    cc.label.setText(labelText, juce::dontSendNotification);
    cc.label.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    cc.label.setJustificationType(juce::Justification::centredLeft);
    cc.label.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::textSecondary);
    addAndMakeVisible(cc.label);
    addAndMakeVisible(cc.box);

    cc.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvtsState, paramId, cc.box);
}

void ModulationView::setCenterTab(bool showMonitor)
{
    isMonitorActive = showMonitor;

    xyTabButton.setColour(juce::TextButton::buttonColourId,
        !showMonitor ? ui::WvZzLookAndFeel::neonGreen.withAlpha(0.25f) : ui::WvZzLookAndFeel::cardBg.darker(0.2f));
    xyTabButton.setColour(juce::TextButton::textColourOffId,
        !showMonitor ? ui::WvZzLookAndFeel::neonGreen : ui::WvZzLookAndFeel::textSecondary);

    monitorTabButton.setColour(juce::TextButton::buttonColourId,
        showMonitor ? ui::WvZzLookAndFeel::neonCyan.withAlpha(0.25f) : ui::WvZzLookAndFeel::cardBg.darker(0.2f));
    monitorTabButton.setColour(juce::TextButton::textColourOffId,
        showMonitor ? ui::WvZzLookAndFeel::neonCyan : ui::WvZzLookAndFeel::textSecondary);

    if (xyPad != nullptr) xyPad->setVisible(!showMonitor);
    if (signalVisualizer != nullptr) signalVisualizer->setVisible(showMonitor);

    xyXTargetBox.box.setVisible(!showMonitor);
    xyXTargetBox.label.setVisible(!showMonitor);
    xyYTargetBox.box.setVisible(!showMonitor);
    xyYTargetBox.label.setVisible(!showMonitor);
    xyXDepthKnob.slider.setVisible(!showMonitor);
    xyXDepthKnob.label.setVisible(!showMonitor);
    xyYDepthKnob.slider.setVisible(!showMonitor);
    xyYDepthKnob.label.setVisible(!showMonitor);

    card.setTitle(showMonitor ? "ANALYSIS & MONITOR" : "MODULATION & X/Y");
    card.setAccentColor(showMonitor ? ui::WvZzLookAndFeel::neonCyan : ui::WvZzLookAndFeel::neonGreen);

    resized();
}

void ModulationView::paint(juce::Graphics&)
{
}

void ModulationView::updateVisuals()
{
    if (lfo1Scope != nullptr) lfo1Scope->repaint();
    if (lfo2Scope != nullptr) lfo2Scope->repaint();
}

void ModulationView::resized()
{
    card.setBounds(getLocalBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto tabArea = juce::Rectangle<int>(getWidth() - 170, 5, 140, 18);
    xyTabButton.setBounds(tabArea.removeFromLeft(68));
    tabArea.removeFromLeft(4);
    monitorTabButton.setBounds(tabArea.removeFromLeft(68));

    auto content = card.getContentBounds();

    auto layoutKnob = [](RotaryControl& rc, juce::Rectangle<int> r) {
        int labelH = 14;
        auto knobArea = r.reduced(1);
        rc.label.setBounds(knobArea.removeFromBottom(labelH));
        rc.slider.setBounds(knobArea);
    };

    // XY Pad & Controls or Signal Visualizer
    auto xyArea = content.removeFromTop(270);
    if (signalVisualizer != nullptr)
        signalVisualizer->setBounds(xyArea);
    if (xyPad != nullptr)
        xyPad->setBounds(xyArea.removeFromTop(175));

    xyArea.removeFromTop(4);
    auto xyRow1 = xyArea.removeFromTop(42);
    xyXTargetBox.label.setBounds(xyRow1.removeFromLeft(50));
    xyXTargetBox.box.setBounds(xyRow1.removeFromLeft(145).reduced(2, 8));
    layoutKnob(xyXDepthKnob, xyRow1.reduced(2, 0));

    auto xyRow2 = xyArea;
    xyYTargetBox.label.setBounds(xyRow2.removeFromLeft(50));
    xyYTargetBox.box.setBounds(xyRow2.removeFromLeft(145).reduced(2, 8));
    layoutKnob(xyYDepthKnob, xyRow2.reduced(2, 0));

    content.removeFromTop(6);

    // LFO 1 Panel (Height: 140)
    auto lfo1Area = content.removeFromTop(140);
    auto lfo1Top = lfo1Area.removeFromTop(26);
    lfo1WaveBox.box.setBounds(lfo1Top.removeFromLeft(85));
    lfo1Top.removeFromLeft(4);
    lfo1SyncToggle.button.setBounds(lfo1Top.removeFromLeft(50).reduced(0, 2));
    lfo1Top.removeFromLeft(4);
    lfo1SyncDivBox.box.setBounds(lfo1Top.removeFromLeft(80));
    lfo1Top.removeFromLeft(4);
    lfo1TargetBox.box.setBounds(lfo1Top);

    auto lfo1Mid = lfo1Area;
    if (lfo1Scope != nullptr)
        lfo1Scope->setBounds(lfo1Mid.removeFromLeft(110).reduced(2, 4));

    int l1Kw = lfo1Mid.getWidth() / 2;
    layoutKnob(lfo1RateKnob, lfo1Mid.removeFromLeft(l1Kw));
    layoutKnob(lfo1DepthKnob, lfo1Mid);

    content.removeFromTop(6);

    // LFO 2 Panel (Remaining height)
    auto lfo2Area = content;
    auto lfo2Top = lfo2Area.removeFromTop(26);
    lfo2WaveBox.box.setBounds(lfo2Top.removeFromLeft(85));
    lfo2Top.removeFromLeft(4);
    lfo2SyncToggle.button.setBounds(lfo2Top.removeFromLeft(50).reduced(0, 2));
    lfo2Top.removeFromLeft(4);
    lfo2SyncDivBox.box.setBounds(lfo2Top.removeFromLeft(80));
    lfo2Top.removeFromLeft(4);
    lfo2TargetBox.box.setBounds(lfo2Top);

    auto lfo2Mid = lfo2Area;
    if (lfo2Scope != nullptr)
        lfo2Scope->setBounds(lfo2Mid.removeFromLeft(110).reduced(2, 4));

    int l2Kw = lfo2Mid.getWidth() / 2;
    layoutKnob(lfo2RateKnob, lfo2Mid.removeFromLeft(l2Kw));
    layoutKnob(lfo2DepthKnob, lfo2Mid);
}

} // namespace wvzz::modules
