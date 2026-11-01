#include "PluginEditor.h"

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
            "X/Y PAD", "X Axis", "Y Axis",
            camelphat::ui::CamelPhatLookAndFeel::neonGreen),
      vuMeter(proc.getMasterSection())
{
    setLookAndFeel(&lookAndFeel);

    // Root content container for 900x560 canvas scaling
    addAndMakeVisible(contentContainer);
    contentContainer.setBounds(0, 0, 900, 560);

    // 1. Module Cards
    contentContainer.addAndMakeVisible(bpFilterCard);
    contentContainer.addAndMakeVisible(distCard);
    contentContainer.addAndMakeVisible(centerModCard);
    contentContainer.addAndMakeVisible(mmFilterCard);
    contentContainer.addAndMakeVisible(flangerCard);
    contentContainer.addAndMakeVisible(magicEqCard);
    contentContainer.addAndMakeVisible(compCard);
    contentContainer.addAndMakeVisible(masterCard);

    // 2. Help Buttons
    contentContainer.addAndMakeVisible(bpHelp);
    contentContainer.addAndMakeVisible(distHelp);
    contentContainer.addAndMakeVisible(centerModHelp);
    contentContainer.addAndMakeVisible(mmHelp);
    contentContainer.addAndMakeVisible(flangerHelp);
    contentContainer.addAndMakeVisible(eqHelp);
    contentContainer.addAndMakeVisible(compHelp);
    contentContainer.addAndMakeVisible(masterHelp);

    // 3. Preset Selector
    const auto& presets = camelphat::getFactoryPresets();
    for (size_t i = 0; i < presets.size(); ++i)
        presetSelector.addItem(presets[i].name, static_cast<int>(i + 1));
    presetSelector.setSelectedId(proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetSelector.onChange = [this] {
        audioProcessor.setCurrentProgram(presetSelector.getSelectedId() - 1);
    };
    contentContainer.addAndMakeVisible(presetSelector);

    // 4. Left Column: BP Filter
    initToggle(bpBypassToggle, camelphat::ParameterIDs::bpBypass, "BYPASS");
    initToggle(bpLinkToggle, camelphat::ParameterIDs::bpLink, "LINK");

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

    bpLowCutLabel.setText("Low Cut", juce::dontSendNotification);
    bpLowCutLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    bpLowCutLabel.setJustificationType(juce::Justification::centred);
    contentContainer.addAndMakeVisible(bpLowCutLabel);

    bpHighCutLabel.setText("High Cut", juce::dontSendNotification);
    bpHighCutLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    bpHighCutLabel.setJustificationType(juce::Justification::centred);
    contentContainer.addAndMakeVisible(bpHighCutLabel);

    initRotary(bpLowResKnob, camelphat::ParameterIDs::bpLowRes, "Low Res");
    initRotary(bpHighResKnob, camelphat::ParameterIDs::bpHighRes, "High Res");
    initRotary(bpBrMixKnob, camelphat::ParameterIDs::bpBrMix, "BR Mix");

    // Left Column: Distortion
    initToggle(distBypassToggle, camelphat::ParameterIDs::distBypass, "BYPASS");
    initRotary(distMechKnob, camelphat::ParameterIDs::distMech, "Mech");
    initRotary(distTubeKnob, camelphat::ParameterIDs::distTube, "Tube");
    initRotary(distBitKnob, camelphat::ParameterIDs::distBit, "Bitcrush");
    initRotary(distXcitaKnob, camelphat::ParameterIDs::distXcita, "Xcita");

    // Center Column: Assignable X/Y Pad & LFOs
    contentContainer.addAndMakeVisible(xyPad);
    const auto& modTargets = camelphat::getModTargetNames();
    initCombo(xyXTargetBox, camelphat::ParameterIDs::xyXTarget, "X Target", modTargets);
    initCombo(xyYTargetBox, camelphat::ParameterIDs::xyYTarget, "Y Target", modTargets);
    initRotary(xyXDepthKnob, camelphat::ParameterIDs::xyXDepth, "X Depth", true);
    initRotary(xyYDepthKnob, camelphat::ParameterIDs::xyYDepth, "Y Depth", true);

    const auto& lfoWaves = camelphat::getLFOWaveformNames();
    const auto& syncDivs = camelphat::getLFOSyncDivNames();

    initCombo(lfo1WaveBox, camelphat::ParameterIDs::lfo1Wave, "LFO1 Wave", lfoWaves);
    initCombo(lfo1TargetBox, camelphat::ParameterIDs::lfo1Target, "Target", modTargets);
    initCombo(lfo1SyncDivBox, camelphat::ParameterIDs::lfo1SyncDiv, "Sync Div", syncDivs);
    initToggle(lfo1SyncToggle, camelphat::ParameterIDs::lfo1Sync, "SYNC");
    initRotary(lfo1RateKnob, camelphat::ParameterIDs::lfo1Rate, "Rate");
    initRotary(lfo1DepthKnob, camelphat::ParameterIDs::lfo1Depth, "Depth", true);

    initCombo(lfo2WaveBox, camelphat::ParameterIDs::lfo2Wave, "LFO2 Wave", lfoWaves);
    initCombo(lfo2TargetBox, camelphat::ParameterIDs::lfo2Target, "Target", modTargets);
    initCombo(lfo2SyncDivBox, camelphat::ParameterIDs::lfo2SyncDiv, "Sync Div", syncDivs);
    initToggle(lfo2SyncToggle, camelphat::ParameterIDs::lfo2Sync, "SYNC");
    initRotary(lfo2RateKnob, camelphat::ParameterIDs::lfo2Rate, "Rate");
    initRotary(lfo2DepthKnob, camelphat::ParameterIDs::lfo2Depth, "Depth", true);

    // Right Column: MM Filter
    initToggle(mmBypassToggle, camelphat::ParameterIDs::mmBypass, "BYPASS");
    initCombo(mmTypeBox, camelphat::ParameterIDs::mmType, "Type", camelphat::getMMFilterTypeNames());
    initRotary(mmCutoffKnob, camelphat::ParameterIDs::mmCutoff, "Cutoff");
    initRotary(mmResKnob, camelphat::ParameterIDs::mmRes, "Res");
    initRotary(mmMixKnob, camelphat::ParameterIDs::mmMix, "Mix");
    initRotary(mmEnvAttKnob, camelphat::ParameterIDs::mmEnvAtt, "Attack");
    initRotary(mmEnvRelKnob, camelphat::ParameterIDs::mmEnvRel, "Release");
    initRotary(mmEnvDepthKnob, camelphat::ParameterIDs::mmEnvDepth, "Env Mod", true);

    // Right Column: Flanger
    initToggle(flgBypassToggle, camelphat::ParameterIDs::flgBypass, "BYPASS");
    initRotary(flgRateKnob, camelphat::ParameterIDs::flgRate, "Rate");
    initRotary(flgAmountKnob, camelphat::ParameterIDs::flgAmount, "Amount");

    // Right Column: Magic EQ
    initToggle(meqBypassToggle, camelphat::ParameterIDs::meqBypass, "BYPASS");
    initToggle(meqPhatToggle, camelphat::ParameterIDs::meqPhat, "PHAT");
    initRotary(meqAmountKnob, camelphat::ParameterIDs::meqAmount, "Amount");
    initRotary(meqTuneKnob, camelphat::ParameterIDs::meqTune, "Tune");

    // Right Column: Compressor
    initToggle(compBypassToggle, camelphat::ParameterIDs::compBypass, "BYPASS");
    initToggle(compPhatToggle, camelphat::ParameterIDs::compPhat, "PHAT");
    initRotary(compAmountKnob, camelphat::ParameterIDs::compAmount, "Amount");
    initRotary(compReleaseKnob, camelphat::ParameterIDs::compRelease, "Release");

    // Right Column: Master
    initToggle(masterBypassToggle, camelphat::ParameterIDs::masterBypass, "BYPASS");
    initRotary(inputGainKnob, camelphat::ParameterIDs::inputGain, "In Gain");
    initRotary(masterMixKnob, camelphat::ParameterIDs::masterMix, "Dry/Wet");
    initRotary(masterVolKnob, camelphat::ParameterIDs::masterVol, "Volume");

    diceButton.setButtonText("DICE");
    diceButton.onClick = [this] { audioProcessor.randomizeParameters(); };
    contentContainer.addAndMakeVisible(diceButton);

    contentContainer.addAndMakeVisible(vuMeter);

    // Help Overlay (top layer)
    contentContainer.addAndMakeVisible(helpOverlay);

    // Setup Fixed-Aspect-Ratio resizable window (900x560, 75% to 200%)
    setResizable(true, true);
    setResizeLimits(675, 420, 1800, 1120);
    getConstrainer()->setFixedAspectRatio(900.0 / 560.0);
    setSize(900, 560);

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
}

void CamelPhatAudioProcessorEditor::initRotary(RotaryControl& ctrl, const juce::String& paramId, const juce::String& labelText, bool isBipolar)
{
    ctrl.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ctrl.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    ctrl.slider.setPopupDisplayEnabled(true, false, this);
    contentContainer.addAndMakeVisible(ctrl.slider);

    ctrl.label.setText(labelText, juce::dontSendNotification);
    ctrl.label.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    ctrl.label.setJustificationType(juce::Justification::centred);
    ctrl.label.setColour(juce::Label::textColourId, camelphat::ui::CamelPhatLookAndFeel::textSecondary);
    contentContainer.addAndMakeVisible(ctrl.label);

    ctrl.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), paramId, ctrl.slider);
}

void CamelPhatAudioProcessorEditor::initToggle(ToggleControl& ctrl, const juce::String& paramId, const juce::String& text)
{
    ctrl.button.setButtonText(text);
    contentContainer.addAndMakeVisible(ctrl.button);
    ctrl.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), paramId, ctrl.button);
}

void CamelPhatAudioProcessorEditor::initCombo(ComboControl& ctrl, const juce::String& paramId, const juce::String& labelText, const juce::StringArray& items)
{
    ctrl.box.addItemList(items, 1);
    ctrl.box.setJustificationType(juce::Justification::centred);
    contentContainer.addAndMakeVisible(ctrl.box);

    ctrl.label.setText(labelText, juce::dontSendNotification);
    ctrl.label.setFont(juce::FontOptions(9.5f, juce::Font::plain));
    ctrl.label.setJustificationType(juce::Justification::centredLeft);
    ctrl.label.setColour(juce::Label::textColourId, camelphat::ui::CamelPhatLookAndFeel::textSecondary);
    contentContainer.addAndMakeVisible(ctrl.label);

    ctrl.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), paramId, ctrl.box);
}

void CamelPhatAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Content container painted inside its own paint
}

void CamelPhatAudioProcessorEditor::resized()
{
    // Scale 900x560 contentContainer to fit current window size preserving aspect ratio
    const float scale = static_cast<float>(getWidth()) / 900.0f;
    contentContainer.setTransform(juce::AffineTransform::scale(scale));

    // Internal layout of 900 x 560 canvas
    auto area = juce::Rectangle<int>(0, 0, 900, 560);

    // Title bar (height 40)
    auto titleArea = area.removeFromTop(40).reduced(10, 4);
    presetSelector.setBounds(titleArea.removeFromRight(200));

    // 3 Columns:
    // Left: 260 px (BP Filter & Distortion)
    // Center: 300 px (X/Y Pad & LFO 1/2)
    // Right: 320 px (MM Filter, Flanger, Magic EQ, Compressor, Master)
    const int gap = 8;
    auto leftCol   = area.removeFromLeft(260).reduced(4);
    area.removeFromLeft(gap);
    auto centerCol = area.removeFromLeft(310).reduced(4);
    area.removeFromLeft(gap);
    auto rightCol  = area.reduced(4);

    auto layoutKnob = [](RotaryControl& ctrl, juce::Rectangle<int> r) {
        int labelH = 12;
        auto knobArea = r.reduced(1);
        ctrl.label.setBounds(knobArea.removeFromBottom(labelH));
        ctrl.slider.setBounds(knobArea);
    };

    // --- LEFT COLUMN ---
    // BP Filter Card (Top 260)
    auto bpBounds = leftCol.removeFromTop(270);
    bpFilterCard.setBounds(bpBounds);
    bpHelp.setBounds(bpBounds.getRight() - 24, bpBounds.getY() + 4, 16, 16);
    auto bpInner = bpFilterCard.getContentBounds();

    auto bpTopToggles = bpInner.removeFromTop(22);
    bpBypassToggle.button.setBounds(bpTopToggles.removeFromLeft(70));
    bpLinkToggle.button.setBounds(bpTopToggles.removeFromLeft(60));

    auto bpSliderArea = bpInner.removeFromTop(130);
    auto s1 = bpSliderArea.removeFromLeft(bpSliderArea.getWidth() / 2);
    bpLowCutLabel.setBounds(s1.removeFromBottom(14));
    bpLowCutSlider.setBounds(s1.reduced(10, 0));

    bpHighCutLabel.setBounds(bpSliderArea.removeFromBottom(14));
    bpHighCutSlider.setBounds(bpSliderArea.reduced(10, 0));

    auto bpKnobRow = bpInner;
    int bpKw = bpKnobRow.getWidth() / 3;
    layoutKnob(bpLowResKnob, bpKnobRow.removeFromLeft(bpKw));
    layoutKnob(bpHighResKnob, bpKnobRow.removeFromLeft(bpKw));
    layoutKnob(bpBrMixKnob, bpKnobRow);

    leftCol.removeFromTop(gap);

    // Distortion Card (Bottom)
    auto distBounds = leftCol;
    distCard.setBounds(distBounds);
    distHelp.setBounds(distBounds.getRight() - 24, distBounds.getY() + 4, 16, 16);
    auto distInner = distCard.getContentBounds();
    distBypassToggle.button.setBounds(distInner.removeFromTop(22).removeFromLeft(70));

    int dKw = distInner.getWidth() / 4;
    layoutKnob(distMechKnob, distInner.removeFromLeft(dKw));
    layoutKnob(distTubeKnob, distInner.removeFromLeft(dKw));
    layoutKnob(distBitKnob, distInner.removeFromLeft(dKw));
    layoutKnob(distXcitaKnob, distInner);

    // --- CENTER COLUMN ---
    // X/Y Pad Card (Top 250)
    centerModCard.setBounds(centerCol);
    centerModHelp.setBounds(centerCol.getRight() - 24, centerCol.getY() + 4, 16, 16);
    auto centerInner = centerModCard.getContentBounds();

    // XY Pad & Controls
    auto xyArea = centerInner.removeFromTop(230);
    xyPad.setBounds(xyArea.removeFromTop(150));

    auto xyControls = xyArea;
    auto xyRow1 = xyControls.removeFromTop(38);
    xyXTargetBox.label.setBounds(xyRow1.removeFromLeft(50));
    xyXTargetBox.box.setBounds(xyRow1.removeFromLeft(110).reduced(2, 6));
    layoutKnob(xyXDepthKnob, xyRow1.reduced(2, 0));

    auto xyRow2 = xyControls;
    xyYTargetBox.label.setBounds(xyRow2.removeFromLeft(50));
    xyYTargetBox.box.setBounds(xyRow2.removeFromLeft(110).reduced(2, 6));
    layoutKnob(xyYDepthKnob, xyRow2.reduced(2, 0));

    centerInner.removeFromTop(6);

    // LFO 1 Panel
    auto lfo1Area = centerInner.removeFromTop(110);
    auto lfo1Row1 = lfo1Area.removeFromTop(24);
    lfo1WaveBox.box.setBounds(lfo1Row1.removeFromLeft(80));
    lfo1SyncToggle.button.setBounds(lfo1Row1.removeFromLeft(55));
    lfo1SyncDivBox.box.setBounds(lfo1Row1.removeFromLeft(60));
    lfo1TargetBox.box.setBounds(lfo1Row1);

    auto lfo1KnobRow = lfo1Area;
    layoutKnob(lfo1RateKnob, lfo1KnobRow.removeFromLeft(lfo1KnobRow.getWidth() / 2));
    layoutKnob(lfo1DepthKnob, lfo1KnobRow);

    centerInner.removeFromTop(6);

    // LFO 2 Panel
    auto lfo2Area = centerInner;
    auto lfo2Row1 = lfo2Area.removeFromTop(24);
    lfo2WaveBox.box.setBounds(lfo2Row1.removeFromLeft(80));
    lfo2SyncToggle.button.setBounds(lfo2Row1.removeFromLeft(55));
    lfo2SyncDivBox.box.setBounds(lfo2Row1.removeFromLeft(60));
    lfo2TargetBox.box.setBounds(lfo2Row1);

    auto lfo2KnobRow = lfo2Area;
    layoutKnob(lfo2RateKnob, lfo2KnobRow.removeFromLeft(lfo2KnobRow.getWidth() / 2));
    layoutKnob(lfo2DepthKnob, lfo2KnobRow);

    // --- RIGHT COLUMN ---
    // MM Filter (Top 150)
    auto mmBounds = rightCol.removeFromTop(150);
    mmFilterCard.setBounds(mmBounds);
    mmHelp.setBounds(mmBounds.getRight() - 24, mmBounds.getY() + 4, 16, 16);
    auto mmInner = mmFilterCard.getContentBounds();

    auto mmTop = mmInner.removeFromTop(24);
    mmBypassToggle.button.setBounds(mmTop.removeFromLeft(65));
    mmTypeBox.box.setBounds(mmTop.removeFromLeft(130));

    auto mmRow1 = mmInner.removeFromTop(48);
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

    // Flanger & Magic EQ Row (Height 90)
    auto row2Bounds = rightCol.removeFromTop(95);
    auto flgBounds = row2Bounds.removeFromLeft(row2Bounds.getWidth() / 2 - 2);
    flangerCard.setBounds(flgBounds);
    flangerHelp.setBounds(flgBounds.getRight() - 20, flgBounds.getY() + 3, 14, 14);
    auto flgInner = flangerCard.getContentBounds();
    flgBypassToggle.button.setBounds(flgInner.removeFromTop(18).removeFromLeft(60));
    layoutKnob(flgRateKnob, flgInner.removeFromLeft(flgInner.getWidth() / 2));
    layoutKnob(flgAmountKnob, flgInner);

    auto meqBounds = row2Bounds.removeFromRight(flgBounds.getWidth());
    magicEqCard.setBounds(meqBounds);
    eqHelp.setBounds(meqBounds.getRight() - 20, meqBounds.getY() + 3, 14, 14);
    auto meqInner = magicEqCard.getContentBounds();
    auto meqTop = meqInner.removeFromTop(18);
    meqBypassToggle.button.setBounds(meqTop.removeFromLeft(55));
    meqPhatToggle.button.setBounds(meqTop.removeFromLeft(50));
    layoutKnob(meqAmountKnob, meqInner.removeFromLeft(meqInner.getWidth() / 2));
    layoutKnob(meqTuneKnob, meqInner);

    rightCol.removeFromTop(4);

    // Compressor Card (Height 100)
    auto compBounds = rightCol.removeFromTop(105);
    compCard.setBounds(compBounds);
    compHelp.setBounds(compBounds.getRight() - 24, compBounds.getY() + 4, 16, 16);
    auto compInner = compCard.getContentBounds();
    auto compTop = compInner.removeFromTop(20);
    compBypassToggle.button.setBounds(compTop.removeFromLeft(65));
    compPhatToggle.button.setBounds(compTop.removeFromLeft(60));
    int compKw = compInner.getWidth() / 2;
    layoutKnob(compAmountKnob, compInner.removeFromLeft(compKw));
    layoutKnob(compReleaseKnob, compInner);

    rightCol.removeFromTop(4);

    // Master Card (Remaining height)
    auto masterBounds = rightCol;
    masterCard.setBounds(masterBounds);
    masterHelp.setBounds(masterBounds.getRight() - 24, masterBounds.getY() + 4, 16, 16);
    auto masterInner = masterCard.getContentBounds();

    auto mTop = masterInner.removeFromTop(22);
    masterBypassToggle.button.setBounds(mTop.removeFromLeft(65));
    diceButton.setBounds(mTop.removeFromRight(55).reduced(2, 2));

    auto vuArea = masterInner.removeFromRight(32).reduced(0, 2);
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
    helpOverlay.displayTopic(topic, targetScreenBounds, this);
}

void CamelPhatAudioProcessorEditor::hideHelp()
{
    helpOverlay.dismiss();
}

bool CamelPhatAudioProcessorEditor::isHelpVisible() const noexcept
{
    return helpOverlay.isVisible();
}
