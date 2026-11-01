#include "PluginEditor.h"

CamelPhatAudioProcessorEditor::CamelPhatAudioProcessorEditor(CamelPhatAudioProcessor& proc)
    : AudioProcessorEditor(proc),
      audioProcessor(proc),
      bpFilterCard("BAND-PASS FILTER", camelphat::ui::CamelPhatLookAndFeel::neonCyan),
      distCard("DISTORTION & CRUSH", camelphat::ui::CamelPhatLookAndFeel::neonAmber),
      mmFilterCard("MULTI-MODE FILTER", camelphat::ui::CamelPhatLookAndFeel::neonGreen),
      flangerCard("FLANGER", camelphat::ui::CamelPhatLookAndFeel::neonCyan),
      magicEqCard("MAGIC EQ", camelphat::ui::CamelPhatLookAndFeel::neonAmber),
      compCard("COMPRESSOR & PHAT", camelphat::ui::CamelPhatLookAndFeel::neonGreen),
      masterCard("MASTER OUTPUT", camelphat::ui::CamelPhatLookAndFeel::neonGreen),
      bpHelp("bp_filter", *this),
      distHelp("distortion", *this),
      mmHelp("mm_filter", *this),
      flangerHelp("flanger", *this),
      eqHelp("magic_eq", *this),
      compHelp("compressor", *this),
      masterHelp("master", *this),
      xyFilterHelp("xy_filter", *this),
      xyDistHelp("xy_dist", *this),
      filterXYPad(proc.getAPVTS(),
                  camelphat::ParameterIDs::mmCutoff,
                  camelphat::ParameterIDs::mmResonance,
                  "FILTER X/Y", "Cutoff", "Res",
                  camelphat::ui::CamelPhatLookAndFeel::neonGreen),
      distXYPad(proc.getAPVTS(),
                camelphat::ParameterIDs::distTube,
                camelphat::ParameterIDs::distMech,
                "DRIVE X/Y", "Tube", "Mech",
                camelphat::ui::CamelPhatLookAndFeel::neonAmber),
      vuMeter(proc.getMasterSection())
{
    setLookAndFeel(&lookAndFeel);

    // 1. Add Card Containers
    addAndMakeVisible(bpFilterCard);
    addAndMakeVisible(distCard);
    addAndMakeVisible(mmFilterCard);
    addAndMakeVisible(flangerCard);
    addAndMakeVisible(magicEqCard);
    addAndMakeVisible(compCard);
    addAndMakeVisible(masterCard);

    // 2. Add Help Buttons
    addAndMakeVisible(bpHelp);
    addAndMakeVisible(distHelp);
    addAndMakeVisible(mmHelp);
    addAndMakeVisible(flangerHelp);
    addAndMakeVisible(eqHelp);
    addAndMakeVisible(compHelp);
    addAndMakeVisible(masterHelp);
    addAndMakeVisible(xyFilterHelp);
    addAndMakeVisible(xyDistHelp);

    // 3. Add XY Pads & VU Meter
    addAndMakeVisible(filterXYPad);
    addAndMakeVisible(distXYPad);
    addAndMakeVisible(vuMeter);

    // 4. Initialise Rotary Sliders & Attachments
    // BP Filter
    initRotary(bpCutLow, camelphat::ParameterIDs::bpCutoffLow, "Low Cut");
    initRotary(bpCutHigh, camelphat::ParameterIDs::bpCutoffHigh, "High Cut");
    initRotary(bpRes, camelphat::ParameterIDs::bpResonance, "Res");
    initRotary(bpMix, camelphat::ParameterIDs::bpMix, "BP Mix");
    initRotary(bpRejectMix, camelphat::ParameterIDs::bandRejectMix, "BR Mix");

    // Distortion
    initRotary(distTube, camelphat::ParameterIDs::distTube, "Tube");
    initRotary(distMech, camelphat::ParameterIDs::distMech, "Mech");
    initRotary(distBit, camelphat::ParameterIDs::distBit, "Bit");
    initRotary(distXcita, camelphat::ParameterIDs::distXcita, "Xcita");
    initRotary(distMix, camelphat::ParameterIDs::distMix, "Mix");

    // MM Filter
    initRotary(mmCutoff, camelphat::ParameterIDs::mmCutoff, "Cutoff");
    initRotary(mmRes, camelphat::ParameterIDs::mmResonance, "Res");
    initRotary(mmEnvAmount, camelphat::ParameterIDs::mmEnvAmount, "Env Amt");
    initRotary(mmEnvAttack, camelphat::ParameterIDs::mmEnvAttack, "Attack");
    initRotary(mmEnvRelease, camelphat::ParameterIDs::mmEnvRelease, "Release");

    mmModeBox.addItem("Low-Pass 12dB", 1);
    mmModeBox.addItem("Band-Pass", 2);
    mmModeBox.addItem("High-Pass", 3);
    mmModeBox.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mmModeBox);
    mmModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        proc.getAPVTS(), camelphat::ParameterIDs::mmMode, mmModeBox);

    // Flanger
    initRotary(flangerRate, camelphat::ParameterIDs::flangerRate, "Rate");
    initRotary(flangerDepth, camelphat::ParameterIDs::flangerDepth, "Depth");
    initRotary(flangerFeedback, camelphat::ParameterIDs::flangerFeedback, "Feedb");
    initRotary(flangerMix, camelphat::ParameterIDs::flangerMix, "Mix");

    // Magic EQ
    initRotary(magicFreq, camelphat::ParameterIDs::magicEqFrequency, "Freq");
    initRotary(magicAmount, camelphat::ParameterIDs::magicEqAmount, "Amount");

    // Compressor
    initRotary(compThresh, camelphat::ParameterIDs::compThreshold, "Thresh");
    initRotary(compRatio, camelphat::ParameterIDs::compRatio, "Ratio");
    initRotary(compAttack, camelphat::ParameterIDs::compAttack, "Attack");
    initRotary(compRelease, camelphat::ParameterIDs::compRelease, "Release");
    initRotary(compMix, camelphat::ParameterIDs::compMix, "Mix");

    compPhatToggle.setButtonText("PHAT MODE");
    addAndMakeVisible(compPhatToggle);
    compPhatAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        proc.getAPVTS(), camelphat::ParameterIDs::compPhatMode, compPhatToggle);

    // Master
    initRotary(masterMix, camelphat::ParameterIDs::masterMix, "Dry/Wet");
    initRotary(masterGain, camelphat::ParameterIDs::masterOutputGain, "Out Gain");

    // 5. Help Overlay (always top)
    addAndMakeVisible(helpOverlay);

    // Modern widescreen layout
    setSize(920, 600);
    setResizable(false, false);
}

CamelPhatAudioProcessorEditor::~CamelPhatAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void CamelPhatAudioProcessorEditor::initRotary(RotaryControl& ctrl, const juce::String& paramId, const juce::String& labelText)
{
    ctrl.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ctrl.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    ctrl.slider.setPopupDisplayEnabled(true, false, this);
    addAndMakeVisible(ctrl.slider);

    ctrl.label.setText(labelText, juce::dontSendNotification);
    ctrl.label.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    ctrl.label.setJustificationType(juce::Justification::centred);
    ctrl.label.setColour(juce::Label::textColourId, camelphat::ui::CamelPhatLookAndFeel::textSecondary);
    addAndMakeVisible(ctrl.label);

    ctrl.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), paramId, ctrl.slider);
}

void CamelPhatAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background gradient
    juce::ColourGradient bgGrad(camelphat::ui::CamelPhatLookAndFeel::bgBase.brighter(0.04f), 0, 0,
                                camelphat::ui::CamelPhatLookAndFeel::bgBase, 0, static_cast<float>(getHeight()), false);
    g.setGradientFill(bgGrad);
    g.fillAll();

    // Top Title Bar
    g.setColour(camelphat::ui::CamelPhatLookAndFeel::cardBorder.withAlpha(0.4f));
    g.drawHorizontalLine(44, 0.0f, static_cast<float>(getWidth()));

    // Title & Logo
    g.setColour(camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    g.drawText("CAMELPHAT", 16, 0, 160, 44, juce::Justification::centredLeft);

    g.setColour(camelphat::ui::CamelPhatLookAndFeel::textSecondary);
    g.setFont(juce::FontOptions(11.0f, juce::Font::plain));
    g.drawText("MULTI-EFFECT SOUND SHAPER // VST3 & AU & STANDALONE", 170, 0, 400, 44, juce::Justification::centredLeft);
}

void CamelPhatAudioProcessorEditor::resized()
{
    auto totalArea = getLocalBounds().withTrimmedTop(48).reduced(12);

    // Layout configuration:
    // Left Column: Band-Pass Filter (Top), Multi-Mode Filter (Bottom)
    // Middle Column: Distortion (Top), Flanger & Magic EQ (Middle), Dual X/Y Pads (Bottom)
    // Right Column: Compressor (Top), Master Section with VU Meter (Bottom)

    const int colGap = 10;
    const int rowGap = 10;

    int leftW = 280;
    int rightW = 220;
    int midW = totalArea.getWidth() - leftW - rightW - (2 * colGap);

    auto leftCol   = totalArea.removeFromLeft(leftW);
    totalArea.removeFromLeft(colGap);
    auto midCol    = totalArea.removeFromLeft(midW);
    totalArea.removeFromLeft(colGap);
    auto rightCol  = totalArea;

    // Helper for laying out a rotary slider + label under it
    auto layoutKnob = [](RotaryControl& ctrl, juce::Rectangle<int> area) {
        int labelH = 14;
        auto knobArea = area.reduced(2, 1);
        ctrl.label.setBounds(knobArea.removeFromBottom(labelH));
        ctrl.slider.setBounds(knobArea);
    };

    // -------------------------------------------------------------
    // LEFT COLUMN: BP Filter (Top: 240px) & MM Filter (Bottom: Rest)
    // -------------------------------------------------------------
    auto bpBounds = leftCol.removeFromTop(240);
    bpFilterCard.setBounds(bpBounds);
    bpHelp.setBounds(bpBounds.getRight() - 26, bpBounds.getY() + 5, 18, 18);

    auto bpInner = bpFilterCard.getContentBounds();
    // 2 rows of knobs
    auto bpRow1 = bpInner.removeFromTop(bpInner.getHeight() / 2);
    auto bpRow2 = bpInner;

    int bpKnobW = bpRow1.getWidth() / 3;
    layoutKnob(bpCutLow, bpRow1.removeFromLeft(bpKnobW));
    layoutKnob(bpCutHigh, bpRow1.removeFromLeft(bpKnobW));
    layoutKnob(bpRes, bpRow1);

    int bpKnobW2 = bpRow2.getWidth() / 2;
    layoutKnob(bpMix, bpRow2.removeFromLeft(bpKnobW2));
    layoutKnob(bpRejectMix, bpRow2);

    leftCol.removeFromTop(rowGap);

    // MM Filter
    auto mmBounds = leftCol;
    mmFilterCard.setBounds(mmBounds);
    mmHelp.setBounds(mmBounds.getRight() - 26, mmBounds.getY() + 5, 18, 18);

    auto mmInner = mmFilterCard.getContentBounds();
    mmModeBox.setBounds(mmInner.removeFromTop(24).reduced(20, 0));
    mmInner.removeFromTop(6);

    auto mmRow1 = mmInner.removeFromTop(mmInner.getHeight() / 2);
    auto mmRow2 = mmInner;

    int mmKnobW = mmRow1.getWidth() / 2;
    layoutKnob(mmCutoff, mmRow1.removeFromLeft(mmKnobW));
    layoutKnob(mmRes, mmRow1);

    int mmKnobW3 = mmRow2.getWidth() / 3;
    layoutKnob(mmEnvAmount, mmRow2.removeFromLeft(mmKnobW3));
    layoutKnob(mmEnvAttack, mmRow2.removeFromLeft(mmKnobW3));
    layoutKnob(mmEnvRelease, mmRow2);

    // -------------------------------------------------------------
    // MIDDLE COLUMN: Distortion (Top), Flanger/Magic EQ (Mid), XY Pads (Bottom)
    // -------------------------------------------------------------
    auto distBounds = midCol.removeFromTop(120);
    distCard.setBounds(distBounds);
    distHelp.setBounds(distBounds.getRight() - 26, distBounds.getY() + 5, 18, 18);

    auto distInner = distCard.getContentBounds();
    int distKnobW = distInner.getWidth() / 5;
    layoutKnob(distTube, distInner.removeFromLeft(distKnobW));
    layoutKnob(distMech, distInner.removeFromLeft(distKnobW));
    layoutKnob(distBit, distInner.removeFromLeft(distKnobW));
    layoutKnob(distXcita, distInner.removeFromLeft(distKnobW));
    layoutKnob(distMix, distInner);

    midCol.removeFromTop(rowGap);

    // Mid Row: Flanger & Magic EQ side-by-side
    auto midRowBounds = midCol.removeFromTop(120);
    auto flangerBounds = midRowBounds.removeFromLeft(midRowBounds.getWidth() * 6 / 10);
    midRowBounds.removeFromLeft(colGap);
    auto eqBounds = midRowBounds;

    // Flanger
    flangerCard.setBounds(flangerBounds);
    flangerHelp.setBounds(flangerBounds.getRight() - 26, flangerBounds.getY() + 5, 18, 18);
    auto flangerInner = flangerCard.getContentBounds();
    int flangerKnobW = flangerInner.getWidth() / 4;
    layoutKnob(flangerRate, flangerInner.removeFromLeft(flangerKnobW));
    layoutKnob(flangerDepth, flangerInner.removeFromLeft(flangerKnobW));
    layoutKnob(flangerFeedback, flangerInner.removeFromLeft(flangerKnobW));
    layoutKnob(flangerMix, flangerInner);

    // Magic EQ
    magicEqCard.setBounds(eqBounds);
    eqHelp.setBounds(eqBounds.getRight() - 26, eqBounds.getY() + 5, 18, 18);
    auto eqInner = magicEqCard.getContentBounds();
    int eqKnobW = eqInner.getWidth() / 2;
    layoutKnob(magicFreq, eqInner.removeFromLeft(eqKnobW));
    layoutKnob(magicAmount, eqInner);

    midCol.removeFromTop(rowGap);

    // Bottom Row: Dual X/Y Pads
    auto xyBounds = midCol;
    int padW = (xyBounds.getWidth() - colGap) / 2;
    auto pad1Area = xyBounds.removeFromLeft(padW);
    xyBounds.removeFromLeft(colGap);
    auto pad2Area = xyBounds;

    filterXYPad.setBounds(pad1Area);
    xyFilterHelp.setBounds(pad1Area.getRight() - 26, pad1Area.getY() + 5, 18, 18);

    distXYPad.setBounds(pad2Area);
    xyDistHelp.setBounds(pad2Area.getRight() - 26, pad2Area.getY() + 5, 18, 18);

    // -------------------------------------------------------------
    // RIGHT COLUMN: Compressor (Top: 270px), Master & VU Meter (Bottom)
    // -------------------------------------------------------------
    auto compBounds = rightCol.removeFromTop(270);
    compCard.setBounds(compBounds);
    compHelp.setBounds(compBounds.getRight() - 26, compBounds.getY() + 5, 18, 18);

    auto compInner = compCard.getContentBounds();
    compPhatToggle.setBounds(compInner.removeFromTop(24).reduced(24, 0));
    compInner.removeFromTop(4);

    auto compRow1 = compInner.removeFromTop(compInner.getHeight() / 2);
    auto compRow2 = compInner;

    int compKnobW1 = compRow1.getWidth() / 3;
    layoutKnob(compThresh, compRow1.removeFromLeft(compKnobW1));
    layoutKnob(compRatio, compRow1.removeFromLeft(compKnobW1));
    layoutKnob(compMix, compRow1);

    int compKnobW2 = compRow2.getWidth() / 2;
    layoutKnob(compAttack, compRow2.removeFromLeft(compKnobW2));
    layoutKnob(compRelease, compRow2);

    rightCol.removeFromTop(rowGap);

    // Master Section & VU Meter
    auto masterBounds = rightCol;
    masterCard.setBounds(masterBounds);
    masterHelp.setBounds(masterBounds.getRight() - 26, masterBounds.getY() + 5, 18, 18);

    auto masterInner = masterCard.getContentBounds();
    // Meter on the right
    auto meterArea = masterInner.removeFromRight(36).reduced(0, 4);
    vuMeter.setBounds(meterArea);

    masterInner.removeFromRight(10);
    int masterKnobH = masterInner.getHeight() / 2;
    layoutKnob(masterMix, masterInner.removeFromTop(masterKnobH));
    layoutKnob(masterGain, masterInner);
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
