#include "PluginEditor.h"

namespace wvzz {

void WvZzReverbAudioProcessorEditor::ContentContainer::paint(juce::Graphics& g)
{
    g.fillAll(ui::WvZzLookAndFeel::bgBase);

    // Subtle dark gradient from top to bottom
    juce::ColourGradient grad(
        ui::WvZzLookAndFeel::cardBg.brighter(0.04f), 0.0f, 0.0f,
        ui::WvZzLookAndFeel::cardBg.darker(0.12f), 0.0f, static_cast<float>(getHeight()), false);
    g.setGradientFill(grad);
    g.fillRect(getLocalBounds());

    // Top Header hairline divider
    g.setColour(ui::WvZzLookAndFeel::cardBorder.withAlpha(0.4f));
    g.drawHorizontalLine(44, 10.0f, static_cast<float>(getWidth() - 10));
}

WvZzReverbAudioProcessorEditor::WvZzReverbAudioProcessorEditor(WvZzReverbAudioProcessor& proc)
    : AudioProcessorEditor(&proc),
      audioProcessor(proc),
      logoComponent("REVERB")
{
    setLookAndFeel(&lookAndFeel);

    addAndMakeVisible(contentContainer);

    // Header Controls
    contentContainer.addAndMakeVisible(logoComponent);

    subTitleLabel.setText("REVERB  //  PLATE, HALL & SHIMMER", juce::dontSendNotification);
    subTitleLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    subTitleLabel.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::textSecondary);
    contentContainer.addAndMakeVisible(subTitleLabel);

    signalFlowLabel.setText("EARLY REFLECTIONS -> REVERB (FDN) -> MASTER", juce::dontSendNotification);
    signalFlowLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    signalFlowLabel.setJustificationType(juce::Justification::centred);
    signalFlowLabel.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::skyCyan.withAlpha(0.6f));
    contentContainer.addAndMakeVisible(signalFlowLabel);

    prevPresetButton.setButtonText("<");
    prevPresetButton.onClick = [this] {
        const int total = audioProcessor.getNumPrograms();
        if (total > 0)
        {
            const int next = (audioProcessor.getCurrentProgram() - 1 + total) % total;
            audioProcessor.setCurrentProgram(next);
            presetSelector.setSelectedId(next + 1, juce::dontSendNotification);
        }
    };
    contentContainer.addAndMakeVisible(prevPresetButton);

    const auto& presets = reverbfx::getFactoryPresets();
    for (int i = 0; i < static_cast<int>(presets.size()); ++i)
        presetSelector.addItem(presets[static_cast<size_t>(i)].name, i + 1);
    presetSelector.setSelectedId(audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
    presetSelector.onChange = [this] {
        const int id = presetSelector.getSelectedId();
        if (id > 0)
            audioProcessor.setCurrentProgram(id - 1);
    };
    contentContainer.addAndMakeVisible(presetSelector);

    nextPresetButton.setButtonText(">");
    nextPresetButton.onClick = [this] {
        const int total = audioProcessor.getNumPrograms();
        if (total > 0)
        {
            const int next = (audioProcessor.getCurrentProgram() + 1) % total;
            audioProcessor.setCurrentProgram(next);
            presetSelector.setSelectedId(next + 1, juce::dontSendNotification);
        }
    };
    contentContainer.addAndMakeVisible(nextPresetButton);

    diceButton.setButtonText("DICE");
    diceButton.onClick = [this] { audioProcessor.randomizeParameters(); };
    contentContainer.addAndMakeVisible(diceButton);

    initToggle(masterBypassToggle, "master_bypass", "BYPASS", false, true, ui::WvZzLookAndFeel::coralRed);

    // Create Module Views
    reverbView  = audioProcessor.getReverbModule().createView(audioProcessor.getAPVTS(), this);
    earlyView   = audioProcessor.getEarlyReflectionsModule().createView(audioProcessor.getAPVTS(), this);
    modView     = audioProcessor.getModulationModule().createView(audioProcessor.getAPVTS(), this);
    masterView  = audioProcessor.getMasterModule().createView(audioProcessor.getAPVTS(), this);

    contentContainer.addAndMakeVisible(*reverbView);
    contentContainer.addAndMakeVisible(*earlyView);
    contentContainer.addAndMakeVisible(*modView);
    contentContainer.addAndMakeVisible(*masterView);

    contentContainer.addChildComponent(helpOverlay);

    setSize(1080, 680);
    setResizable(true, true);
    setResizeLimits(810, 510, 1920, 1200);
    getConstrainer()->setFixedAspectRatio(1080.0 / 680.0);

    startTimerHz(60);
}

WvZzReverbAudioProcessorEditor::~WvZzReverbAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void WvZzReverbAudioProcessorEditor::initToggle(ToggleControl& ctrl, const juce::String& paramId, const juce::String& text,
                                                bool isHeaderBypass, bool isPill, juce::Colour accent)
{
    ctrl.button.setButtonText(text);
    if (isHeaderBypass)
        ctrl.button.getProperties().set("isHeaderBypass", true);
    if (isPill)
        ctrl.button.getProperties().set("isPill", true);
    ctrl.button.getProperties().set("accentColor", static_cast<int>(accent.getARGB()));
    ctrl.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), paramId, ctrl.button);
    contentContainer.addAndMakeVisible(ctrl.button);
}

void WvZzReverbAudioProcessorEditor::timerCallback()
{
    const int expectedId = audioProcessor.getCurrentProgram() + 1;
    if (presetSelector.getSelectedId() != expectedId)
        presetSelector.setSelectedId(expectedId, juce::dontSendNotification);

    for (auto* view : { reverbView.get(), earlyView.get(), modView.get(), masterView.get() })
    {
        if (view != nullptr)
            view->updateVisuals();
    }
}

void WvZzReverbAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(ui::WvZzLookAndFeel::bgBase);
}

void WvZzReverbAudioProcessorEditor::resized()
{
    constexpr int baseWidth = 1080;
    constexpr int baseHeight = 680;

    contentContainer.setBounds(0, 0, baseWidth, baseHeight);

    const float scaleX = static_cast<float>(getWidth()) / static_cast<float>(baseWidth);
    const float scaleY = static_cast<float>(getHeight()) / static_cast<float>(baseHeight);
    const float scale = juce::jmin(scaleX, scaleY);

    const float offsetX = (static_cast<float>(getWidth())  - static_cast<float>(baseWidth)  * scale) * 0.5f;
    const float offsetY = (static_cast<float>(getHeight()) - static_cast<float>(baseHeight) * scale) * 0.5f;

    contentContainer.setTransform(juce::AffineTransform::scale(scale).translated(offsetX, offsetY));

    // Header layout
    logoComponent.setBounds(14, 7, 130, 30);
    subTitleLabel.setBounds(150, 12, 230, 20);
    signalFlowLabel.setBounds(384, 10, 250, 24);

    prevPresetButton.setBounds(640, 9, 24, 24);
    presetSelector.setBounds(668, 9, 200, 24);
    nextPresetButton.setBounds(872, 9, 24, 24);
    diceButton.setBounds(906, 9, 56, 24);
    masterBypassToggle.button.setBounds(972, 9, 70, 24);

    // 3-Column Layout: Left (320px), Center (408px), Right (320px)
    constexpr int colY = 48;
    constexpr int totalHeight = 680 - colY - 8; // 624px

    // Left Column: Reverb (full height)
    reverbView->setBounds(10, colY, 320, totalHeight);

    // Center Column: X/Y pad, monitor, decay-curve display tab, and the two LFO panels
    modView->setBounds(336, colY, 408, totalHeight);

    // Right Column: Early Reflections (462px), Master (156px)
    earlyView->setBounds(750, colY, 320, 462);
    masterView->setBounds(750, colY + 468, 320, 156);
}

void WvZzReverbAudioProcessorEditor::mouseDown(const juce::MouseEvent&)
{
    if (helpOverlay.isVisible())
        helpOverlay.dismiss();
}

void WvZzReverbAudioProcessorEditor::showHelp(const juce::String& topicId, juce::Rectangle<int> targetScreenBounds)
{
    auto topic = helpRepo.getTopic(topicId);
    helpOverlay.displayTopic(topic, targetScreenBounds, &contentContainer);
}

void WvZzReverbAudioProcessorEditor::hideHelp()
{
    helpOverlay.dismiss();
}

bool WvZzReverbAudioProcessorEditor::isHelpVisible() const noexcept
{
    return helpOverlay.isVisible();
}

} // namespace wvzz
