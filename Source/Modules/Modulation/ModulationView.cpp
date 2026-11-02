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
      modDSP(dsp)
{
    juce::ignoreUnused(helpHost);

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
    xyPad = std::make_unique<ui::XYPadComponent>(apvtsState, "xy_x", "xy_y", "X/Y Pad", "Target X", "Target Y", ui::WvZzLookAndFeel::neonGreen);
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

    lfo1SyncToggle.button.setButtonText("SYNC");
    lfo1SyncToggle.button.getProperties().set("isPill", true);
    lfo1SyncToggle.button.getProperties().set("accentColor", static_cast<int>(ui::WvZzLookAndFeel::neonGreen.getARGB()));
    lfo1SyncToggle.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsState, "lfo1_sync", lfo1SyncToggle.button);
    addAndMakeVisible(lfo1SyncToggle.button);

    initRotary(lfo1RateKnob, "lfo1_rate", "Rate", false, ui::WvZzLookAndFeel::neonGreen);
    initCombo(lfo1SyncDivBox, "lfo1_sync_div", "Div", lfoSyncDivs);
    initCombo(lfo1TargetBox, "lfo1_target", "Target", modTargets);
    initRotary(lfo1DepthKnob, "lfo1_depth", "Depth", true, ui::WvZzLookAndFeel::neonGreen);

    lfo1SyncToggle.button.onStateChange = [this] {
        bool sync = lfo1SyncToggle.button.getToggleState();
        lfo1RateKnob.slider.setVisible(!sync);
        lfo1RateKnob.label.setVisible(!sync);
        lfo1SyncDivBox.box.setVisible(sync);
        lfo1SyncDivBox.label.setVisible(sync);
    };
    lfo1SyncToggle.button.onStateChange();

    // LFO 2 Panel
    lfo2Scope = std::make_unique<LFOScopeComponent>(
        [this] { return lfo2WaveBox.box.getSelectedItemIndex(); },
        [this] { return modDSP.getLfo2Phase(); },
        [this] { return modDSP.getLfo2Snapshot(); },
        ui::WvZzLookAndFeel::neonCyan);
    addAndMakeVisible(*lfo2Scope);

    initCombo(lfo2WaveBox, "lfo2_wave", "Wave", lfoWaves);

    lfo2SyncToggle.button.setButtonText("SYNC");
    lfo2SyncToggle.button.getProperties().set("isPill", true);
    lfo2SyncToggle.button.getProperties().set("accentColor", static_cast<int>(ui::WvZzLookAndFeel::neonCyan.getARGB()));
    lfo2SyncToggle.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsState, "lfo2_sync", lfo2SyncToggle.button);
    addAndMakeVisible(lfo2SyncToggle.button);

    initRotary(lfo2RateKnob, "lfo2_rate", "Rate", false, ui::WvZzLookAndFeel::neonCyan);
    initCombo(lfo2SyncDivBox, "lfo2_sync_div", "Div", lfoSyncDivs);
    initCombo(lfo2TargetBox, "lfo2_target", "Target", modTargets);
    initRotary(lfo2DepthKnob, "lfo2_depth", "Depth", true, ui::WvZzLookAndFeel::neonCyan);

    lfo2SyncToggle.button.onStateChange = [this] {
        bool sync = lfo2SyncToggle.button.getToggleState();
        lfo2RateKnob.slider.setVisible(!sync);
        lfo2RateKnob.label.setVisible(!sync);
        lfo2SyncDivBox.box.setVisible(sync);
        lfo2SyncDivBox.label.setVisible(sync);
    };
    lfo2SyncToggle.button.onStateChange();

    setCenterTab(false);
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
        !showMonitor ? ui::WvZzLookAndFeel::textPrimary : ui::WvZzLookAndFeel::textSecondary);

    monitorTabButton.setColour(juce::TextButton::buttonColourId,
        showMonitor ? ui::WvZzLookAndFeel::neonCyan.withAlpha(0.25f) : ui::WvZzLookAndFeel::cardBg.darker(0.2f));
    monitorTabButton.setColour(juce::TextButton::textColourOffId,
        showMonitor ? ui::WvZzLookAndFeel::textPrimary : ui::WvZzLookAndFeel::textSecondary);

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

    resized();
}

void ModulationView::paint(juce::Graphics& g)
{
    // Draw outer background and borders for LFO cards
    auto bounds = getLocalBounds().toFloat();

    // LFO 1 card container (mid section)
    auto lfo1Area = juce::Rectangle<float>(bounds.getX(), 270.0f, bounds.getWidth(), 155.0f);
    g.setColour(ui::WvZzLookAndFeel::cardBg);
    g.fillRoundedRectangle(lfo1Area, 6.0f);
    g.setColour(ui::WvZzLookAndFeel::cardBorder);
    g.drawRoundedRectangle(lfo1Area, 6.0f, 1.0f);

    // LFO 1 Header Title
    g.setColour(ui::WvZzLookAndFeel::neonGreen);
    g.fillEllipse(lfo1Area.getX() + 8.0f, lfo1Area.getY() + 10.0f, 6.0f, 6.0f);
    g.setColour(ui::WvZzLookAndFeel::textPrimary);
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText("LFO 1", lfo1Area.getX() + 20.0f, lfo1Area.getY() + 5.0f, 80.0f, 16.0f, juce::Justification::centredLeft);

    // LFO 2 card container (bottom section)
    auto lfo2Area = juce::Rectangle<float>(bounds.getX(), 435.0f, bounds.getWidth(), 155.0f);
    g.setColour(ui::WvZzLookAndFeel::cardBg);
    g.fillRoundedRectangle(lfo2Area, 6.0f);
    g.setColour(ui::WvZzLookAndFeel::cardBorder);
    g.drawRoundedRectangle(lfo2Area, 6.0f, 1.0f);

    // LFO 2 Header Title
    g.setColour(ui::WvZzLookAndFeel::neonCyan);
    g.fillEllipse(lfo2Area.getX() + 8.0f, lfo2Area.getY() + 10.0f, 6.0f, 6.0f);
    g.setColour(ui::WvZzLookAndFeel::textPrimary);
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText("LFO 2", static_cast<int>(lfo2Area.getX() + 20.0f), static_cast<int>(lfo2Area.getY() + 5.0f), 80, 16, juce::Justification::centredLeft);
}

void ModulationView::updateVisuals()
{
    if (lfo1Scope != nullptr) lfo1Scope->repaint();
    if (lfo2Scope != nullptr) lfo2Scope->repaint();
}

void ModulationView::resized()
{
    auto bounds = getLocalBounds();

    // 1. Tab Bar at top
    auto tabArea = bounds.removeFromTop(24);
    const int tabW = 90;
    xyTabButton.setBounds(tabArea.getX(), tabArea.getY(), tabW, 22);
    monitorTabButton.setBounds(tabArea.getX() + tabW + 6, tabArea.getY(), tabW, 22);

    bounds.removeFromTop(6);

    // 2. Upper Region: XY Pad OR Signal Visualizer
    auto upperArea = bounds.removeFromTop(226);
    if (xyPad != nullptr)
        xyPad->setBounds(upperArea.removeFromTop(170));
    if (signalVisualizer != nullptr)
        signalVisualizer->setBounds(upperArea.withY(tabArea.getBottom() + 6).withHeight(226));

    if (!isMonitorActive)
    {
        // 4 sub-controls below XY Pad: X Target, Y Target, X Depth, Y Depth
        auto ctrlArea = upperArea;
        const int colW = ctrlArea.getWidth() / 4;
        xyXTargetBox.label.setBounds(ctrlArea.getX(), ctrlArea.getY(), colW, 14);
        xyXTargetBox.box.setBounds(ctrlArea.getX(), ctrlArea.getY() + 16, colW - 4, 22);

        xyYTargetBox.label.setBounds(ctrlArea.getX() + colW, ctrlArea.getY(), colW, 14);
        xyYTargetBox.box.setBounds(ctrlArea.getX() + colW, ctrlArea.getY() + 16, colW - 4, 22);

        xyXDepthKnob.slider.setBounds(ctrlArea.getX() + 2 * colW + (colW - 36) / 2, ctrlArea.getY(), 36, 36);
        xyXDepthKnob.label.setBounds(ctrlArea.getX() + 2 * colW, ctrlArea.getY() + 38, colW, 12);

        xyYDepthKnob.slider.setBounds(ctrlArea.getX() + 3 * colW + (colW - 36) / 2, ctrlArea.getY(), 36, 36);
        xyYDepthKnob.label.setBounds(ctrlArea.getX() + 3 * colW, ctrlArea.getY() + 38, colW, 12);
    }

    bounds.removeFromTop(14);

    // 3. LFO 1 Panel
    auto lfo1Area = bounds.removeFromTop(155).reduced(8, 4);
    lfo1Area.removeFromTop(22); // Header label space
    const int scopeW = 100;
    lfo1Scope->setBounds(lfo1Area.removeFromLeft(scopeW));

    lfo1Area.removeFromLeft(12);
    auto lfo1Controls = lfo1Area;

    // Controls: Wave, Sync, Rate/Div, Target, Depth
    const int cW = lfo1Controls.getWidth() / 3;
    lfo1WaveBox.label.setBounds(lfo1Controls.getX(), lfo1Controls.getY(), cW, 14);
    lfo1WaveBox.box.setBounds(lfo1Controls.getX(), lfo1Controls.getY() + 16, cW - 4, 22);

    lfo1TargetBox.label.setBounds(lfo1Controls.getX(), lfo1Controls.getY() + 48, cW, 14);
    lfo1TargetBox.box.setBounds(lfo1Controls.getX(), lfo1Controls.getY() + 64, cW - 4, 22);

    lfo1SyncToggle.button.setBounds(lfo1Controls.getX() + cW, lfo1Controls.getY() + 16, 48, 22);
    lfo1RateKnob.slider.setBounds(lfo1Controls.getX() + cW + 4, lfo1Controls.getY() + 48, 44, 44);
    lfo1RateKnob.label.setBounds(lfo1Controls.getX() + cW, lfo1Controls.getY() + 94, cW, 12);

    lfo1SyncDivBox.label.setBounds(lfo1Controls.getX() + cW, lfo1Controls.getY() + 48, cW, 14);
    lfo1SyncDivBox.box.setBounds(lfo1Controls.getX() + cW, lfo1Controls.getY() + 64, cW - 4, 22);

    lfo1DepthKnob.slider.setBounds(lfo1Controls.getX() + 2 * cW + (cW - 44) / 2, lfo1Controls.getY() + 16, 44, 44);
    lfo1DepthKnob.label.setBounds(lfo1Controls.getX() + 2 * cW, lfo1Controls.getY() + 62, cW, 12);

    bounds.removeFromTop(10);

    // 4. LFO 2 Panel
    auto lfo2Area = bounds.removeFromTop(155).reduced(8, 4);
    lfo2Area.removeFromTop(22);
    lfo2Scope->setBounds(lfo2Area.removeFromLeft(scopeW));

    lfo2Area.removeFromLeft(12);
    auto lfo2Controls = lfo2Area;

    lfo2WaveBox.label.setBounds(lfo2Controls.getX(), lfo2Controls.getY(), cW, 14);
    lfo2WaveBox.box.setBounds(lfo2Controls.getX(), lfo2Controls.getY() + 16, cW - 4, 22);

    lfo2TargetBox.label.setBounds(lfo2Controls.getX(), lfo2Controls.getY() + 48, cW, 14);
    lfo2TargetBox.box.setBounds(lfo2Controls.getX(), lfo2Controls.getY() + 64, cW - 4, 22);

    lfo2SyncToggle.button.setBounds(lfo2Controls.getX() + cW, lfo2Controls.getY() + 16, 48, 22);
    lfo2RateKnob.slider.setBounds(lfo2Controls.getX() + cW + 4, lfo2Controls.getY() + 48, 44, 44);
    lfo2RateKnob.label.setBounds(lfo2Controls.getX() + cW, lfo2Controls.getY() + 94, cW, 12);

    lfo2SyncDivBox.label.setBounds(lfo2Controls.getX() + cW, lfo2Controls.getY() + 48, cW, 14);
    lfo2SyncDivBox.box.setBounds(lfo2Controls.getX() + cW, lfo2Controls.getY() + 64, cW - 4, 22);

    lfo2DepthKnob.slider.setBounds(lfo2Controls.getX() + 2 * cW + (cW - 44) / 2, lfo2Controls.getY() + 16, 44, 44);
    lfo2DepthKnob.label.setBounds(lfo2Controls.getX() + 2 * cW, lfo2Controls.getY() + 62, cW, 12);
}

} // namespace wvzz::modules
