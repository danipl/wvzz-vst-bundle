#include "PluginEditor.h"

// ==============================================================================
// Sub-component Implementations
// ==============================================================================

void CamelPhatAudioProcessorEditor::ContentContainer::paint(juce::Graphics& g)
{
    // 1. Base dark background
    g.fillAll(camelphat::ui::CamelPhatLookAndFeel::bgBase);

    // 2. Top Header Bar
    auto headerArea = juce::Rectangle<int>(0, 0, 1080, 48).toFloat();
    juce::ColourGradient headerGrad(camelphat::ui::CamelPhatLookAndFeel::cardBg.brighter(0.05f),
                                    0, 0,
                                    camelphat::ui::CamelPhatLookAndFeel::cardBg.darker(0.1f),
                                    0, 48, false);
    g.setGradientFill(headerGrad);
    g.fillRect(headerArea);

    g.setColour(camelphat::ui::CamelPhatLookAndFeel::cardBorder);
    g.drawHorizontalLine(48, 0, 1080);
}

void CamelPhatAudioProcessorEditor::PassbandVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(camelphat::ui::CamelPhatLookAndFeel::trackBg);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(camelphat::ui::CamelPhatLookAndFeel::cardBorder);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    // Compute slider relative thumb positions
    const float lowNorm = static_cast<float>(lowSlider.getValue() / lowSlider.getMaximum());
    const float highNorm = static_cast<float>(highSlider.getValue() / highSlider.getMaximum());

    const float yLow = bounds.getBottom() - lowNorm * bounds.getHeight();
    const float yHigh = bounds.getBottom() - highNorm * bounds.getHeight();

    const float passTop = std::min(yLow, yHigh);
    const float passBottom = std::max(yLow, yHigh);

    if (passBottom - passTop > 1.0f)
    {
        auto passRect = juce::Rectangle<float>(bounds.getX() + 2.0f, passTop,
                                               bounds.getWidth() - 4.0f, passBottom - passTop);
        g.setColour(camelphat::ui::CamelPhatLookAndFeel::neonCyan.withAlpha(0.28f));
        g.fillRoundedRectangle(passRect, 2.0f);
    }
}

void CamelPhatAudioProcessorEditor::LFOScopeComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(camelphat::ui::CamelPhatLookAndFeel::cardBg.darker(0.25f));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(camelphat::ui::CamelPhatLookAndFeel::cardBorder);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    // Center zero line
    const float midY = bounds.getCentreY();
    g.setColour(camelphat::ui::CamelPhatLookAndFeel::cardBorder.withAlpha(0.4f));
    g.drawHorizontalLine(static_cast<int>(midY), bounds.getX() + 2.0f, bounds.getRight() - 2.0f);

    const int wave = waveFn ? waveFn() : 0;
    const float phase = phaseFn ? phaseFn() : 0.0f; // [0, 1)

    // Render waveform path
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

    // Current phase marker dot
    const float dotX = bounds.getX() + 2.0f + phase * (bounds.getWidth() - 4.0f);
    const float snap = snapFn ? snapFn() : 0.0f;
    const float dotY = midY - snap * (bounds.getHeight() * 0.38f);
    g.setColour(colour);
    g.fillEllipse(dotX - 3.0f, dotY - 3.0f, 6.0f, 6.0f);
}

void CamelPhatAudioProcessorEditor::CompressorGRMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    g.setColour(camelphat::ui::CamelPhatLookAndFeel::cardBg.darker(0.3f));
    g.fillRoundedRectangle(bounds, 3.0f);
    g.setColour(camelphat::ui::CamelPhatLookAndFeel::cardBorder);
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

    const float grDb = compressor.getGainReductionDb(); // Negative or 0.0f
    // Scale: 0 dB at top to -24 dB at bottom
    const float norm = juce::jlimit(0.0f, 1.0f, -grDb / 24.0f);

    if (norm > 0.005f)
    {
        const float meterH = bounds.getHeight() * norm;
        auto meterRect = juce::Rectangle<float>(bounds.getX() + 1.0f, bounds.getY() + 1.0f,
                                               bounds.getWidth() - 2.0f, meterH);
        juce::ColourGradient grGrad(camelphat::ui::CamelPhatLookAndFeel::neonAmber,
                                    bounds.getX(), bounds.getY(),
                                    juce::Colour(0xffff1744),
                                    bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(grGrad);
        g.fillRoundedRectangle(meterRect, 2.0f);
    }

    // Label: GR
    g.setColour(camelphat::ui::CamelPhatLookAndFeel::textSecondary);
    g.setFont(juce::FontOptions(8.5f, juce::Font::bold));
    g.drawText("GR", bounds.removeFromBottom(12.0f), juce::Justification::centred, false);
}

// ==============================================================================
// CamelPhatAudioProcessorEditor Main
// ==============================================================================

CamelPhatAudioProcessorEditor::CamelPhatAudioProcessorEditor(CamelPhatAudioProcessor& proc)
    : AudioProcessorEditor(proc),
      audioProcessor(proc),
      bpFilterCard("BP FILTER & SPLIT", camelphat::ui::CamelPhatLookAndFeel::neonCyan),
      distCard("DISTORTION", camelphat::ui::CamelPhatLookAndFeel::neonAmber),
      centerModCard("MODULATION & X/Y", camelphat::ui::CamelPhatLookAndFeel::neonGreen),
      mmFilterCard("MM FILTER", camelphat::ui::CamelPhatLookAndFeel::neonGreen),
      flangerCard("FLANGER", camelphat::ui::CamelPhatLookAndFeel::neonCyan),
      magicEqCard("MAGIC EQ", camelphat::ui::CamelPhatLookAndFeel::neonAmber),
      compCard("COMPRESSOR", camelphat::ui::CamelPhatLookAndFeel::neonGreen),
      masterCard("MASTER", camelphat::ui::CamelPhatLookAndFeel::neonGreen),
      bpHelp("bp_filter", *this),
      distHelp("distortion", *this),
      centerModHelp("xy_pad", *this),
      mmHelp("mm_filter", *this),
      flangerHelp("flanger", *this),
      eqHelp("magic_eq", *this),
      compHelp("compressor", *this),
      masterHelp("master", *this),
      xyPad(proc.getAPVTS(),
            camelphat::ParameterIDs::xyX,
            camelphat::ParameterIDs::xyY,
            "X/Y PAD", "X Target", "Y Target",
            camelphat::ui::CamelPhatLookAndFeel::neonGreen),
      vuMeter(proc.getMasterSection())
{
    setLookAndFeel(&lookAndFeel);

    // Root content container for 1080x680 canvas scaling
    addAndMakeVisible(contentContainer);
    contentContainer.setBounds(0, 0, 1080, 680);

    // 1. Top Header Bar
    titleLabel.setText("CAMELPHAT", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    contentContainer.addAndMakeVisible(titleLabel);

    subTitleLabel.setText("MULTI-EFFECT SOUND SHAPER  //  VST3 & AU", juce::dontSendNotification);
    subTitleLabel.setFont(juce::FontOptions(11.0f, juce::Font::plain));
    subTitleLabel.setColour(juce::Label::textColourId, camelphat::ui::CamelPhatLookAndFeel::textSecondary);
    contentContainer.addAndMakeVisible(subTitleLabel);

    prevPresetButton.setButtonText("<");
    prevPresetButton.onClick = [this] {
        int current = audioProcessor.getCurrentProgram();
        int total = audioProcessor.getNumPrograms();
        audioProcessor.setCurrentProgram((current - 1 + total) % total);
    };
    contentContainer.addAndMakeVisible(prevPresetButton);

    const auto& presets = camelphat::getFactoryPresets();
    for (size_t i = 0; i < presets.size(); ++i)
        presetSelector.addItem(presets[i].name, static_cast<int>(i + 1));
    presetSelector.setSelectedId(proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetSelector.onChange = [this] {
        audioProcessor.setCurrentProgram(presetSelector.getSelectedId() - 1);
    };
    contentContainer.addAndMakeVisible(presetSelector);

    nextPresetButton.setButtonText(">");
    nextPresetButton.onClick = [this] {
        int current = audioProcessor.getCurrentProgram();
        int total = audioProcessor.getNumPrograms();
        audioProcessor.setCurrentProgram((current + 1) % total);
    };
    contentContainer.addAndMakeVisible(nextPresetButton);

    diceButton.setButtonText("DICE");
    diceButton.onClick = [this] { audioProcessor.randomizeParameters(); };
    contentContainer.addAndMakeVisible(diceButton);

    initToggle(masterBypassToggle, camelphat::ParameterIDs::masterBypass, "BYPASS", false, true,
               camelphat::ui::CamelPhatLookAndFeel::neonAmber);

    // 2. Module Cards
    contentContainer.addAndMakeVisible(bpFilterCard);
    contentContainer.addAndMakeVisible(distCard);
    contentContainer.addAndMakeVisible(centerModCard);
    contentContainer.addAndMakeVisible(mmFilterCard);
    contentContainer.addAndMakeVisible(flangerCard);
    contentContainer.addAndMakeVisible(magicEqCard);
    contentContainer.addAndMakeVisible(compCard);
    contentContainer.addAndMakeVisible(masterCard);

    // 3. Help Buttons
    contentContainer.addAndMakeVisible(bpHelp);
    contentContainer.addAndMakeVisible(distHelp);
    contentContainer.addAndMakeVisible(centerModHelp);
    contentContainer.addAndMakeVisible(mmHelp);
    contentContainer.addAndMakeVisible(flangerHelp);
    contentContainer.addAndMakeVisible(eqHelp);
    contentContainer.addAndMakeVisible(compHelp);
    contentContainer.addAndMakeVisible(masterHelp);

    // 4. Header-Integrated Bypass Toggles
    initToggle(bpBypassToggle, camelphat::ParameterIDs::bpBypass, "BP BYP", true);
    initToggle(distBypassToggle, camelphat::ParameterIDs::distBypass, "DIST BYP", true);
    initToggle(mmBypassToggle, camelphat::ParameterIDs::mmBypass, "MM BYP", true);
    initToggle(flgBypassToggle, camelphat::ParameterIDs::flgBypass, "FLG BYP", true);
    initToggle(meqBypassToggle, camelphat::ParameterIDs::meqBypass, "MEQ BYP", true);
    initToggle(compBypassToggle, camelphat::ParameterIDs::compBypass, "COMP BYP", true);

    // 5. Left Column: BP Filter
    initToggle(bpLinkToggle, camelphat::ParameterIDs::bpLink, "LINK", false, true,
               camelphat::ui::CamelPhatLookAndFeel::neonCyan);

    bpLowCutSlider.setSliderStyle(juce::Slider::LinearVertical);
    bpLowCutSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    bpLowCutSlider.setPopupDisplayEnabled(true, false, this);
    contentContainer.addAndMakeVisible(bpLowCutSlider);
    bpLowCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        proc.getAPVTS(), camelphat::ParameterIDs::bpLowCut, bpLowCutSlider);

    bpHighCutSlider.setSliderStyle(juce::Slider::LinearVertical);
    bpHighCutSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    bpHighCutSlider.setPopupDisplayEnabled(true, false, this);
    contentContainer.addAndMakeVisible(bpHighCutSlider);
    bpHighCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        proc.getAPVTS(), camelphat::ParameterIDs::bpHighCut, bpHighCutSlider);

    bpPassband = std::make_unique<PassbandVisualizer>(bpLowCutSlider, bpHighCutSlider);
    contentContainer.addAndMakeVisible(*bpPassband);

    bpLowCutLabel.setText("Low Cut", juce::dontSendNotification);
    bpLowCutLabel.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    bpLowCutLabel.setJustificationType(juce::Justification::centred);
    contentContainer.addAndMakeVisible(bpLowCutLabel);

    bpHighCutLabel.setText("High Cut", juce::dontSendNotification);
    bpHighCutLabel.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    bpHighCutLabel.setJustificationType(juce::Justification::centred);
    contentContainer.addAndMakeVisible(bpHighCutLabel);

    initRotary(bpLowResKnob, camelphat::ParameterIDs::bpLowRes, "Low Res", false,
               camelphat::ui::CamelPhatLookAndFeel::neonCyan);
    initRotary(bpHighResKnob, camelphat::ParameterIDs::bpHighRes, "High Res", false,
               camelphat::ui::CamelPhatLookAndFeel::neonCyan);
    initRotary(bpBrMixKnob, camelphat::ParameterIDs::bpBrMix, "BR Mix", false,
               camelphat::ui::CamelPhatLookAndFeel::neonCyan);

    // Left Column: Distortion
    initRotary(distMechKnob, camelphat::ParameterIDs::distMech, "Mech", false,
               camelphat::ui::CamelPhatLookAndFeel::neonAmber);
    initRotary(distTubeKnob, camelphat::ParameterIDs::distTube, "Tube", false,
               camelphat::ui::CamelPhatLookAndFeel::neonAmber);
    initRotary(distBitKnob, camelphat::ParameterIDs::distBit, "Bitcrush", false,
               camelphat::ui::CamelPhatLookAndFeel::neonAmber);
    initRotary(distXcitaKnob, camelphat::ParameterIDs::distXcita, "Xcita", false,
               camelphat::ui::CamelPhatLookAndFeel::neonAmber);

    // 6. Center Column: Assignable X/Y Pad & LFOs
    contentContainer.addAndMakeVisible(xyPad);
    const auto& modTargets = camelphat::getModTargetNames();
    initCombo(xyXTargetBox, camelphat::ParameterIDs::xyXTarget, "X Target", modTargets);
    initCombo(xyYTargetBox, camelphat::ParameterIDs::xyYTarget, "Y Target", modTargets);
    initRotary(xyXDepthKnob, camelphat::ParameterIDs::xyXDepth, "X Depth", true,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    initRotary(xyYDepthKnob, camelphat::ParameterIDs::xyYDepth, "Y Depth", true,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);

    // Update XY pad labels on target change
    xyXTargetBox.box.onChange = [this, &modTargets] {
        int idx = xyXTargetBox.box.getSelectedItemIndex();
        if (idx >= 0 && idx < modTargets.size())
        {
            int yIdx = xyYTargetBox.box.getSelectedItemIndex();
            xyPad.setAxisLabels(modTargets[idx], yIdx >= 0 ? modTargets[yIdx] : "Y");
        }
    };
    xyYTargetBox.box.onChange = [this, &modTargets] {
        int idx = xyYTargetBox.box.getSelectedItemIndex();
        if (idx >= 0 && idx < modTargets.size())
        {
            int xIdx = xyXTargetBox.box.getSelectedItemIndex();
            xyPad.setAxisLabels(xIdx >= 0 ? modTargets[xIdx] : "X", modTargets[idx]);
        }
    };

    const auto& lfoWaves = camelphat::getLFOWaveformNames();
    const auto& syncDivs = camelphat::getLFOSyncDivNames();

    auto& modEng = audioProcessor.getModulationEngine();
    lfo1Scope = std::make_unique<LFOScopeComponent>(
        [this] { return lfo1WaveBox.box.getSelectedItemIndex(); },
        [&modEng] { return modEng.getLfo1Phase(); },
        [&modEng] { return modEng.getLfo1Snapshot(); },
        camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    contentContainer.addAndMakeVisible(*lfo1Scope);

    initCombo(lfo1WaveBox, camelphat::ParameterIDs::lfo1Wave, "Wave", lfoWaves);
    initCombo(lfo1TargetBox, camelphat::ParameterIDs::lfo1Target, "Target", modTargets);
    initCombo(lfo1SyncDivBox, camelphat::ParameterIDs::lfo1SyncDiv, "Sync Div", syncDivs);
    initToggle(lfo1SyncToggle, camelphat::ParameterIDs::lfo1Sync, "SYNC", false, true,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    initRotary(lfo1RateKnob, camelphat::ParameterIDs::lfo1Rate, "Rate", false,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    initRotary(lfo1DepthKnob, camelphat::ParameterIDs::lfo1Depth, "Depth", true,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);

    lfo2Scope = std::make_unique<LFOScopeComponent>(
        [this] { return lfo2WaveBox.box.getSelectedItemIndex(); },
        [&modEng] { return modEng.getLfo2Phase(); },
        [&modEng] { return modEng.getLfo2Snapshot(); },
        camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    contentContainer.addAndMakeVisible(*lfo2Scope);

    initCombo(lfo2WaveBox, camelphat::ParameterIDs::lfo2Wave, "Wave", lfoWaves);
    initCombo(lfo2TargetBox, camelphat::ParameterIDs::lfo2Target, "Target", modTargets);
    initCombo(lfo2SyncDivBox, camelphat::ParameterIDs::lfo2SyncDiv, "Sync Div", syncDivs);
    initToggle(lfo2SyncToggle, camelphat::ParameterIDs::lfo2Sync, "SYNC", false, true,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    initRotary(lfo2RateKnob, camelphat::ParameterIDs::lfo2Rate, "Rate", false,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    initRotary(lfo2DepthKnob, camelphat::ParameterIDs::lfo2Depth, "Depth", true,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);

    // 7. Right Column: MM Filter
    initCombo(mmTypeBox, camelphat::ParameterIDs::mmType, "Filter Type", camelphat::getMMFilterTypeNames());
    initRotary(mmCutoffKnob, camelphat::ParameterIDs::mmCutoff, "Cutoff", false,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    initRotary(mmResKnob, camelphat::ParameterIDs::mmRes, "Res", false,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    initRotary(mmMixKnob, camelphat::ParameterIDs::mmMix, "Mix", false,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    initRotary(mmEnvAttKnob, camelphat::ParameterIDs::mmEnvAtt, "Attack", false,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    initRotary(mmEnvRelKnob, camelphat::ParameterIDs::mmEnvRel, "Release", false,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    initRotary(mmEnvDepthKnob, camelphat::ParameterIDs::mmEnvDepth, "Env Mod", true,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);

    // Right Column: Flanger
    initRotary(flgRateKnob, camelphat::ParameterIDs::flgRate, "Rate", false,
               camelphat::ui::CamelPhatLookAndFeel::neonCyan);
    initRotary(flgAmountKnob, camelphat::ParameterIDs::flgAmount, "Amount", false,
               camelphat::ui::CamelPhatLookAndFeel::neonCyan);

    // Right Column: Magic EQ
    initToggle(meqPhatToggle, camelphat::ParameterIDs::meqPhat, "PHAT", false, true,
               camelphat::ui::CamelPhatLookAndFeel::neonAmber);
    initRotary(meqAmountKnob, camelphat::ParameterIDs::meqAmount, "Amount", true,
               camelphat::ui::CamelPhatLookAndFeel::neonAmber);
    initRotary(meqTuneKnob, camelphat::ParameterIDs::meqTune, "Tune", false,
               camelphat::ui::CamelPhatLookAndFeel::neonAmber);

    // Right Column: Compressor
    initToggle(compPhatToggle, camelphat::ParameterIDs::compPhat, "PHAT", false, true,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    initRotary(compAmountKnob, camelphat::ParameterIDs::compAmount, "Amount", false,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    initRotary(compReleaseKnob, camelphat::ParameterIDs::compRelease, "Release", false,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    compMeter = std::make_unique<CompressorGRMeter>(audioProcessor.getCompressorModule());
    contentContainer.addAndMakeVisible(*compMeter);

    // Right Column: Master
    initRotary(inputGainKnob, camelphat::ParameterIDs::inputGain, "In Gain", false,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    initRotary(masterMixKnob, camelphat::ParameterIDs::masterMix, "Dry/Wet", false,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    initRotary(masterVolKnob, camelphat::ParameterIDs::masterVol, "Volume", false,
               camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    contentContainer.addAndMakeVisible(vuMeter);

    // Help Overlay (top layer)
    contentContainer.addAndMakeVisible(helpOverlay);

    // Setup Resizable window (1080x680, 75% to 200%)
    setResizable(true, true);
    setResizeLimits(810, 510, 1920, 1200);
    getConstrainer()->setFixedAspectRatio(1080.0 / 680.0);
    setSize(1080, 680);

    startTimerHz(30);
}

CamelPhatAudioProcessorEditor::~CamelPhatAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void CamelPhatAudioProcessorEditor::timerCallback()
{
    // Update preset selector if changed externally
    const int currentProg = audioProcessor.getCurrentProgram();
    if (presetSelector.getSelectedId() != currentProg + 1)
        presetSelector.setSelectedId(currentProg + 1, juce::dontSendNotification);

    // Repaint visualizers
    if (bpPassband != nullptr) bpPassband->repaint();
    if (lfo1Scope != nullptr) lfo1Scope->repaint();
    if (lfo2Scope != nullptr) lfo2Scope->repaint();
    if (compMeter != nullptr) compMeter->repaint();

    updateModuleBypassVisuals();
}

void CamelPhatAudioProcessorEditor::updateModuleBypassVisuals()
{
    bpFilterCard.setBypassed(bpBypassToggle.button.getToggleState());
    distCard.setBypassed(distBypassToggle.button.getToggleState());
    mmFilterCard.setBypassed(mmBypassToggle.button.getToggleState());
    flangerCard.setBypassed(flgBypassToggle.button.getToggleState());
    magicEqCard.setBypassed(meqBypassToggle.button.getToggleState());
    compCard.setBypassed(compBypassToggle.button.getToggleState());

    // Dim child controls of bypassed modules
    const float bpAlpha = bpBypassToggle.button.getToggleState() ? 0.45f : 1.0f;
    bpLowCutSlider.setAlpha(bpAlpha);
    bpHighCutSlider.setAlpha(bpAlpha);
    bpLowCutLabel.setAlpha(bpAlpha);
    bpHighCutLabel.setAlpha(bpAlpha);
    bpLinkToggle.button.setAlpha(bpAlpha);
    bpLowResKnob.slider.setAlpha(bpAlpha);
    bpHighResKnob.slider.setAlpha(bpAlpha);
    bpBrMixKnob.slider.setAlpha(bpAlpha);

    const float distAlpha = distBypassToggle.button.getToggleState() ? 0.45f : 1.0f;
    distMechKnob.slider.setAlpha(distAlpha);
    distTubeKnob.slider.setAlpha(distAlpha);
    distBitKnob.slider.setAlpha(distAlpha);
    distXcitaKnob.slider.setAlpha(distAlpha);

    const float mmAlpha = mmBypassToggle.button.getToggleState() ? 0.45f : 1.0f;
    mmTypeBox.box.setAlpha(mmAlpha);
    mmCutoffKnob.slider.setAlpha(mmAlpha);
    mmResKnob.slider.setAlpha(mmAlpha);
    mmMixKnob.slider.setAlpha(mmAlpha);
    mmEnvAttKnob.slider.setAlpha(mmAlpha);
    mmEnvRelKnob.slider.setAlpha(mmAlpha);
    mmEnvDepthKnob.slider.setAlpha(mmAlpha);

    const float flgAlpha = flgBypassToggle.button.getToggleState() ? 0.45f : 1.0f;
    flgRateKnob.slider.setAlpha(flgAlpha);
    flgAmountKnob.slider.setAlpha(flgAlpha);

    const float meqAlpha = meqBypassToggle.button.getToggleState() ? 0.45f : 1.0f;
    meqPhatToggle.button.setAlpha(meqAlpha);
    meqAmountKnob.slider.setAlpha(meqAlpha);
    meqTuneKnob.slider.setAlpha(meqAlpha);

    const float compAlpha = compBypassToggle.button.getToggleState() ? 0.45f : 1.0f;
    compPhatToggle.button.setAlpha(compAlpha);
    compAmountKnob.slider.setAlpha(compAlpha);
    compReleaseKnob.slider.setAlpha(compAlpha);
}

void CamelPhatAudioProcessorEditor::initRotary(RotaryControl& ctrl, const juce::String& paramId,
                                                const juce::String& labelText, bool isBipolar, juce::Colour accent)
{
    ctrl.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ctrl.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    ctrl.slider.setPopupDisplayEnabled(true, false, this);
    ctrl.slider.getProperties().set("isBipolar", isBipolar);
    ctrl.slider.getProperties().set("accentColor", static_cast<int>(accent.getARGB()));
    contentContainer.addAndMakeVisible(ctrl.slider);

    ctrl.label.setText(labelText, juce::dontSendNotification);
    ctrl.label.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    ctrl.label.setJustificationType(juce::Justification::centred);
    ctrl.label.setColour(juce::Label::textColourId, camelphat::ui::CamelPhatLookAndFeel::textSecondary);
    contentContainer.addAndMakeVisible(ctrl.label);

    ctrl.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), paramId, ctrl.slider);
}

void CamelPhatAudioProcessorEditor::initToggle(ToggleControl& ctrl, const juce::String& paramId,
                                                const juce::String& text, bool isHeaderBypass,
                                                bool isPill, juce::Colour accent)
{
    ctrl.button.setButtonText(text);
    ctrl.button.getProperties().set("isHeaderBypass", isHeaderBypass);
    ctrl.button.getProperties().set("isPill", isPill);
    ctrl.button.getProperties().set("accentColor", static_cast<int>(accent.getARGB()));
    contentContainer.addAndMakeVisible(ctrl.button);
    ctrl.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), paramId, ctrl.button);
}

void CamelPhatAudioProcessorEditor::initCombo(ComboControl& ctrl, const juce::String& paramId,
                                               const juce::String& labelText, const juce::StringArray& items)
{
    ctrl.box.addItemList(items, 1);
    ctrl.box.setJustificationType(juce::Justification::centred);
    contentContainer.addAndMakeVisible(ctrl.box);

    ctrl.label.setText(labelText, juce::dontSendNotification);
    ctrl.label.setFont(juce::FontOptions(10.0f, juce::Font::plain));
    ctrl.label.setJustificationType(juce::Justification::centredLeft);
    ctrl.label.setColour(juce::Label::textColourId, camelphat::ui::CamelPhatLookAndFeel::textSecondary);
    contentContainer.addAndMakeVisible(ctrl.label);

    ctrl.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), paramId, ctrl.box);
}

void CamelPhatAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Letterbox margin fill if resized to a non-16:10 aspect ratio
    g.fillAll(camelphat::ui::CamelPhatLookAndFeel::bgBase);
}

void CamelPhatAudioProcessorEditor::resized()
{
    // Aspect-fit 1080x680 canvas inside current window bounds
    const float scale = std::min(static_cast<float>(getWidth()) / 1080.0f,
                                 static_cast<float>(getHeight()) / 680.0f);
    const int scaledW = juce::roundToInt(1080.0f * scale);
    const int scaledH = juce::roundToInt(680.0f * scale);
    const int offsetX = (getWidth() - scaledW) / 2;
    const int offsetY = (getHeight() - scaledH) / 2;

    contentContainer.setTransform(juce::AffineTransform::scale(scale).translated(static_cast<float>(offsetX), static_cast<float>(offsetY)));

    // Internal layout of 1080 x 680 canvas
    auto area = juce::Rectangle<int>(0, 0, 1080, 680);

    // 1. Top Header Bar (Height: 48)
    auto headerArea = area.removeFromTop(48).reduced(12, 6);
    titleLabel.setBounds(headerArea.removeFromLeft(140));
    subTitleLabel.setBounds(headerArea.removeFromLeft(330));

    // Right side of header: Global bypass, Dice, and Preset Toolbar
    masterBypassToggle.button.setBounds(headerArea.removeFromRight(70).reduced(0, 4));
    headerArea.removeFromRight(8);
    diceButton.setBounds(headerArea.removeFromRight(50).reduced(0, 4));
    headerArea.removeFromRight(8);
    nextPresetButton.setBounds(headerArea.removeFromRight(26).reduced(0, 4));
    presetSelector.setBounds(headerArea.removeFromRight(200).reduced(0, 4));
    prevPresetButton.setBounds(headerArea.removeFromRight(26).reduced(0, 4));

    // 2. 3 Columns:
    // Left: 300 px (BP Filter & Distortion)
    // Center: 370 px (X/Y Pad & LFO 1/2)
    // Right: 410 px (MM Filter, Flanger, Magic EQ, Compressor, Master)
    const int gap = 8;
    area.reduce(6, 6);
    auto leftCol   = area.removeFromLeft(305).reduced(4);
    area.removeFromLeft(gap);
    auto centerCol = area.removeFromLeft(370).reduced(4);
    area.removeFromLeft(gap);
    auto rightCol  = area.reduced(4);

    auto layoutKnob = [](RotaryControl& ctrl, juce::Rectangle<int> r) {
        int labelH = 14;
        auto knobArea = r.reduced(1);
        ctrl.label.setBounds(knobArea.removeFromBottom(labelH));
        ctrl.slider.setBounds(knobArea);
    };

    // --- LEFT COLUMN ---
    // BP Filter Card (Top: 330 px)
    auto bpBounds = leftCol.removeFromTop(330);
    bpFilterCard.setBounds(bpBounds);
    bpHelp.setBounds(bpFilterCard.getHeaderHelpBounds());
    bpBypassToggle.button.setBounds(bpFilterCard.getHeaderBypassBounds());
    auto bpInner = bpFilterCard.getContentBounds();

    // Link pill button at top of BP card
    auto bpTopRow = bpInner.removeFromTop(20);
    bpLinkToggle.button.setBounds(bpTopRow.removeFromRight(55));

    // Vertical Cutoff Sliders Area
    auto bpSliderArea = bpInner.removeFromTop(150);
    auto s1 = bpSliderArea.removeFromLeft(bpSliderArea.getWidth() / 2 - 12);
    bpLowCutLabel.setBounds(s1.removeFromBottom(16));
    bpLowCutSlider.setBounds(s1.reduced(10, 0));

    auto s2 = bpSliderArea.removeFromRight(bpSliderArea.getWidth() - 24);
    bpHighCutLabel.setBounds(s2.removeFromBottom(16));
    bpHighCutSlider.setBounds(s2.reduced(10, 0));

    if (bpPassband != nullptr)
        bpPassband->setBounds(bpSliderArea.reduced(2, 4));

    bpInner.removeFromTop(6);

    // Bottom Knobs Row
    auto bpKnobRow = bpInner;
    int bpKw = bpKnobRow.getWidth() / 3;
    layoutKnob(bpLowResKnob, bpKnobRow.removeFromLeft(bpKw));
    layoutKnob(bpHighResKnob, bpKnobRow.removeFromLeft(bpKw));
    layoutKnob(bpBrMixKnob, bpKnobRow);

    leftCol.removeFromTop(gap);

    // Distortion Card (Bottom)
    auto distBounds = leftCol;
    distCard.setBounds(distBounds);
    distHelp.setBounds(distCard.getHeaderHelpBounds());
    distBypassToggle.button.setBounds(distCard.getHeaderBypassBounds());
    auto distInner = distCard.getContentBounds();

    int dKw = distInner.getWidth() / 4;
    layoutKnob(distMechKnob, distInner.removeFromLeft(dKw));
    layoutKnob(distTubeKnob, distInner.removeFromLeft(dKw));
    layoutKnob(distBitKnob, distInner.removeFromLeft(dKw));
    layoutKnob(distXcitaKnob, distInner);

    // --- CENTER COLUMN ---
    centerModCard.setBounds(centerCol);
    centerModHelp.setBounds(centerModCard.getHeaderHelpBounds());
    auto centerInner = centerModCard.getContentBounds();

    // XY Pad & Controls
    auto xyArea = centerInner.removeFromTop(270);
    xyPad.setBounds(xyArea.removeFromTop(175));

    xyArea.removeFromTop(4);
    auto xyRow1 = xyArea.removeFromTop(42);
    xyXTargetBox.label.setBounds(xyRow1.removeFromLeft(50));
    xyXTargetBox.box.setBounds(xyRow1.removeFromLeft(145).reduced(2, 8));
    layoutKnob(xyXDepthKnob, xyRow1.reduced(2, 0));

    auto xyRow2 = xyArea;
    xyYTargetBox.label.setBounds(xyRow2.removeFromLeft(50));
    xyYTargetBox.box.setBounds(xyRow2.removeFromLeft(145).reduced(2, 8));
    layoutKnob(xyYDepthKnob, xyRow2.reduced(2, 0));

    centerInner.removeFromTop(6);

    // LFO 1 Panel (Height: 140)
    auto lfo1Area = centerInner.removeFromTop(140);
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

    centerInner.removeFromTop(6);

    // LFO 2 Panel (Remaining height)
    auto lfo2Area = centerInner;
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

    // --- RIGHT COLUMN ---
    // MM Filter (Height: 180)
    auto mmBounds = rightCol.removeFromTop(180);
    mmFilterCard.setBounds(mmBounds);
    mmHelp.setBounds(mmFilterCard.getHeaderHelpBounds());
    mmBypassToggle.button.setBounds(mmFilterCard.getHeaderBypassBounds());
    auto mmInner = mmFilterCard.getContentBounds();

    auto mmTop = mmInner.removeFromTop(26);
    mmTypeBox.label.setBounds(mmTop.removeFromLeft(70));
    mmTypeBox.box.setBounds(mmTop.removeFromLeft(170).reduced(2, 2));

    auto mmRow1 = mmInner.removeFromTop(62);
    int mmKw1 = mmRow1.getWidth() / 4;
    layoutKnob(mmCutoffKnob, mmRow1.removeFromLeft(mmKw1));
    layoutKnob(mmResKnob, mmRow1.removeFromLeft(mmKw1));
    layoutKnob(mmMixKnob, mmRow1.removeFromLeft(mmKw1));
    layoutKnob(mmEnvDepthKnob, mmRow1);

    auto mmRow2 = mmInner;
    int mmKw2 = mmRow2.getWidth() / 2;
    layoutKnob(mmEnvAttKnob, mmRow2.removeFromLeft(mmKw2));
    layoutKnob(mmEnvRelKnob, mmRow2);

    rightCol.removeFromTop(4);

    // Flanger & Magic EQ Row (Height: 115)
    auto row2Bounds = rightCol.removeFromTop(115);
    auto flgBounds = row2Bounds.removeFromLeft(row2Bounds.getWidth() / 2 - 2);
    flangerCard.setBounds(flgBounds);
    flangerHelp.setBounds(flangerCard.getHeaderHelpBounds());
    flgBypassToggle.button.setBounds(flangerCard.getHeaderBypassBounds());
    auto flgInner = flangerCard.getContentBounds();
    layoutKnob(flgRateKnob, flgInner.removeFromLeft(flgInner.getWidth() / 2));
    layoutKnob(flgAmountKnob, flgInner);

    auto meqBounds = row2Bounds.removeFromRight(flgBounds.getWidth());
    magicEqCard.setBounds(meqBounds);
    eqHelp.setBounds(magicEqCard.getHeaderHelpBounds());
    meqBypassToggle.button.setBounds(magicEqCard.getHeaderBypassBounds());
    auto meqInner = magicEqCard.getContentBounds();
    auto meqTop = meqInner.removeFromTop(20);
    meqPhatToggle.button.setBounds(meqTop.removeFromRight(50).reduced(0, 1));
    layoutKnob(meqAmountKnob, meqInner.removeFromLeft(meqInner.getWidth() / 2));
    layoutKnob(meqTuneKnob, meqInner);

    rightCol.removeFromTop(4);

    // Compressor Card (Height: 130)
    auto compBounds = rightCol.removeFromTop(130);
    compCard.setBounds(compBounds);
    compHelp.setBounds(compCard.getHeaderHelpBounds());
    compBypassToggle.button.setBounds(compCard.getHeaderBypassBounds());
    auto compInner = compCard.getContentBounds();

    auto compTop = compInner.removeFromTop(20);
    compPhatToggle.button.setBounds(compTop.removeFromRight(50).reduced(0, 1));

    if (compMeter != nullptr)
        compMeter->setBounds(compInner.removeFromRight(32).reduced(2, 4));

    int compKw = compInner.getWidth() / 2;
    layoutKnob(compAmountKnob, compInner.removeFromLeft(compKw));
    layoutKnob(compReleaseKnob, compInner);

    rightCol.removeFromTop(4);

    // Master Card (Remaining height)
    auto masterBounds = rightCol;
    masterCard.setBounds(masterBounds);
    masterHelp.setBounds(masterCard.getHeaderHelpBounds());
    auto masterInner = masterCard.getContentBounds();

    auto vuArea = masterInner.removeFromRight(40).reduced(2, 4);
    vuMeter.setBounds(vuArea);

    int mKw = masterInner.getWidth() / 3;
    layoutKnob(inputGainKnob, masterInner.removeFromLeft(mKw));
    layoutKnob(masterMixKnob, masterInner.removeFromLeft(mKw));
    layoutKnob(masterVolKnob, masterInner);
}

void CamelPhatAudioProcessorEditor::mouseDown(const juce::MouseEvent&)
{
    if (helpOverlay.isVisible())
        helpOverlay.dismiss();
}

void CamelPhatAudioProcessorEditor::showHelp(const juce::String& topicId, juce::Rectangle<int> targetScreenBounds)
{
    auto topic = helpRepo.getTopic(topicId);
    helpOverlay.displayTopic(topic, targetScreenBounds, &contentContainer);
}

void CamelPhatAudioProcessorEditor::hideHelp()
{
    helpOverlay.dismiss();
}

bool CamelPhatAudioProcessorEditor::isHelpVisible() const noexcept
{
    return helpOverlay.isVisible();
}
