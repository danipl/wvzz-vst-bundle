#include "PluginEditor.h"

namespace wvzz {

void WvZzDistortionAudioProcessorEditor::ContentContainer::paint(juce::Graphics& g)
{
    // 1. Base dark background
    g.fillAll(ui::WvZzLookAndFeel::bgBase);

    // 2. Top Header Bar
    auto headerArea = juce::Rectangle<int>(0, 0, 1080, 48).toFloat();
    juce::ColourGradient headerGrad(ui::WvZzLookAndFeel::cardBg.brighter(0.04f),
                                    0, 0,
                                    ui::WvZzLookAndFeel::cardBg.darker(0.15f),
                                    0, 48, false);
    g.setGradientFill(headerGrad);
    g.fillRect(headerArea);

    g.setColour(ui::WvZzLookAndFeel::cardBorder.withAlpha(0.45f));
    g.drawHorizontalLine(48, 0, 1080);
}

WvZzDistortionAudioProcessorEditor::WvZzDistortionAudioProcessorEditor(WvZzDistortionAudioProcessor& proc)
    : AudioProcessorEditor(proc),
      audioProcessor(proc)
{
    setLookAndFeel(&lookAndFeel);

    // Root content container for 1080x680 canvas scaling
    addAndMakeVisible(contentContainer);
    contentContainer.setBounds(0, 0, 1080, 680);

    // Header: Vector Logo
    contentContainer.addAndMakeVisible(logoComponent);

    subTitleLabel.setText("DISTORTION  //  MODULAR SOUND SHAPER", juce::dontSendNotification);
    subTitleLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    subTitleLabel.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::textSecondary);
    contentContainer.addAndMakeVisible(subTitleLabel);

    signalFlowLabel.setText("BP \u2192 DIST \u2192 MM+BR \u2192 FLG \u2192 EQ \u2192 COMP \u2192 MASTER", juce::dontSendNotification);
    signalFlowLabel.setFont(juce::FontOptions(9.5f, juce::Font::plain));
    signalFlowLabel.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::skyCyan.withAlpha(0.75f));
    signalFlowLabel.setJustificationType(juce::Justification::centred);
    contentContainer.addAndMakeVisible(signalFlowLabel);

    prevPresetButton.setButtonText("<");
    prevPresetButton.onClick = [this] {
        int current = audioProcessor.getCurrentProgram();
        int total = audioProcessor.getNumPrograms();
        audioProcessor.setCurrentProgram((current - 1 + total) % total);
    };
    contentContainer.addAndMakeVisible(prevPresetButton);

    const auto& presets = getFactoryPresets();
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

    initToggle(masterBypassToggle, "master_bypass", "BYPASS", false, true,
               ui::WvZzLookAndFeel::warmAmber);

    // Create 8 polymorphic module views
    bpView = audioProcessor.getBPFilterModule().createView(audioProcessor.getAPVTS(), this);
    distView = audioProcessor.getDistortionModule().createView(audioProcessor.getAPVTS(), this);
    modView = audioProcessor.getModulationModule().createView(audioProcessor.getAPVTS(), this);
    mmView = audioProcessor.getMMFilterModule().createView(audioProcessor.getAPVTS(), this);
    flgView = audioProcessor.getFlangerModule().createView(audioProcessor.getAPVTS(), this);
    meqView = audioProcessor.getMagicEQModule().createView(audioProcessor.getAPVTS(), this);
    compView = audioProcessor.getCompressorModule().createView(audioProcessor.getAPVTS(), this);
    masterView = audioProcessor.getMasterModule().createView(audioProcessor.getAPVTS(), this);

    contentContainer.addAndMakeVisible(*bpView);
    contentContainer.addAndMakeVisible(*distView);
    contentContainer.addAndMakeVisible(*modView);
    contentContainer.addAndMakeVisible(*mmView);
    contentContainer.addAndMakeVisible(*flgView);
    contentContainer.addAndMakeVisible(*meqView);
    contentContainer.addAndMakeVisible(*compView);
    contentContainer.addAndMakeVisible(*masterView);

    // Help Overlay
    contentContainer.addAndMakeVisible(helpOverlay);

    // Setup Resizable window (1080x680, 75% to 200%)
    setResizable(true, true);
    setResizeLimits(810, 510, 1920, 1200);
    getConstrainer()->setFixedAspectRatio(1080.0 / 680.0);
    setSize(1080, 680);

    startTimerHz(60);
}

WvZzDistortionAudioProcessorEditor::~WvZzDistortionAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void WvZzDistortionAudioProcessorEditor::initToggle(ToggleControl& ctrl, const juce::String& paramId,
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

void WvZzDistortionAudioProcessorEditor::timerCallback()
{
    // Update preset selector if changed externally
    const int currentProg = audioProcessor.getCurrentProgram();
    if (presetSelector.getSelectedId() != currentProg + 1)
        presetSelector.setSelectedId(currentProg + 1, juce::dontSendNotification);

    // Dynamic signal flow indicator based on Magic EQ pre-distortion routing
    const bool meqIsPre = audioProcessor.getMagicEQModule().getMagicEQDSP().isPreDistortion();
    const juce::String flowText = meqIsPre
        ? "BP \u2192 EQ \u2192 DIST \u2192 MM+BR \u2192 FLG \u2192 COMP \u2192 MASTER"
        : "BP \u2192 DIST \u2192 MM+BR \u2192 FLG \u2192 EQ \u2192 COMP \u2192 MASTER";
    if (signalFlowLabel.getText() != flowText)
        signalFlowLabel.setText(flowText, juce::dontSendNotification);

    // Call updateVisuals() on all 8 views
    if (bpView)     bpView->updateVisuals();
    if (distView)   distView->updateVisuals();
    if (modView)    modView->updateVisuals();
    if (mmView)     mmView->updateVisuals();
    if (flgView)    flgView->updateVisuals();
    if (meqView)    meqView->updateVisuals();
    if (compView)   compView->updateVisuals();
    if (masterView) masterView->updateVisuals();
}

void WvZzDistortionAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(ui::WvZzLookAndFeel::bgBase);
}

void WvZzDistortionAudioProcessorEditor::resized()
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
    logoComponent.setBounds(headerArea.removeFromLeft(150).reduced(0, 4));
    headerArea.removeFromLeft(4);
    subTitleLabel.setBounds(headerArea.removeFromLeft(225));
    signalFlowLabel.setBounds(headerArea.removeFromLeft(200));

    // Right side of header: Global bypass, Dice, and Preset Toolbar
    masterBypassToggle.button.setBounds(headerArea.removeFromRight(70).reduced(0, 4));
    headerArea.removeFromRight(8);
    diceButton.setBounds(headerArea.removeFromRight(50).reduced(0, 4));
    headerArea.removeFromRight(8);
    nextPresetButton.setBounds(headerArea.removeFromRight(26).reduced(0, 4));
    presetSelector.setBounds(headerArea.removeFromRight(200).reduced(0, 4));
    prevPresetButton.setBounds(headerArea.removeFromRight(26).reduced(0, 4));

    // 2. 3 Columns:
    // Left: 305 px (BP Filter & Distortion)
    // Center: 370 px (Modulation, XY Pad & Dual LFOs)
    // Right: 385 px (MM Filter, Flanger, Magic EQ, Compressor, Master)
    const int gap = 8;
    area.reduce(6, 6);
    auto leftCol   = area.removeFromLeft(305).reduced(4);
    area.removeFromLeft(gap);
    auto centerCol = area.removeFromLeft(370).reduced(4);
    area.removeFromLeft(gap);
    auto rightCol  = area.reduced(4);

    // --- LEFT COLUMN ---
    if (bpView)
        bpView->setBounds(leftCol.removeFromTop(330));
    leftCol.removeFromTop(gap);
    if (distView)
        distView->setBounds(leftCol);

    // --- CENTER COLUMN ---
    if (modView)
        modView->setBounds(centerCol);

    // --- RIGHT COLUMN ---
    if (mmView)
        mmView->setBounds(rightCol.removeFromTop(180));
    rightCol.removeFromTop(4);

    auto row2Bounds = rightCol.removeFromTop(115);
    auto flgBounds = row2Bounds.removeFromLeft(row2Bounds.getWidth() / 2 - 2);
    auto meqBounds = row2Bounds.removeFromRight(flgBounds.getWidth());
    if (flgView)
        flgView->setBounds(flgBounds);
    if (meqView)
        meqView->setBounds(meqBounds);
    rightCol.removeFromTop(4);

    if (compView)
        compView->setBounds(rightCol.removeFromTop(130));
    rightCol.removeFromTop(4);

    if (masterView)
        masterView->setBounds(rightCol);
}

void WvZzDistortionAudioProcessorEditor::mouseDown(const juce::MouseEvent&)
{
    if (helpOverlay.isVisible())
        helpOverlay.dismiss();
}

void WvZzDistortionAudioProcessorEditor::showHelp(const juce::String& topicId, juce::Rectangle<int> targetScreenBounds)
{
    auto topic = helpRepo.getTopic(topicId);
    helpOverlay.displayTopic(topic, targetScreenBounds, &contentContainer);
}

void WvZzDistortionAudioProcessorEditor::hideHelp()
{
    helpOverlay.dismiss();
}

bool WvZzDistortionAudioProcessorEditor::isHelpVisible() const noexcept
{
    return helpOverlay.isVisible();
}

} // namespace wvzz
