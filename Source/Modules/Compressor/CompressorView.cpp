#include "CompressorView.h"
#include "CompressorDSP.h"

namespace wvzz::modules {

void CompressorView::GRMeterComponent::updateLevels()
{
    float rawGrDb = 0.0f;
    if (meterSource != nullptr)
        rawGrDb = meterSource->getGainReduction(); // Negative or 0.0f

    // Scale: 0 dB at top to -24 dB at bottom
    const float targetNorm = juce::jlimit(0.0f, 1.0f, -rawGrDb / 24.0f);

    // Ballistics: fast peak descent, smooth exponential fallback decay
    constexpr float decay = 0.88f;
    if (targetNorm > currentGrNorm)
        currentGrNorm = targetNorm;
    else
        currentGrNorm = currentGrNorm * decay;

    // Peak hold decay
    constexpr float peakHoldDecay = 0.96f;
    if (currentGrNorm > peakHoldNorm)
        peakHoldNorm = currentGrNorm;
    else
        peakHoldNorm = peakHoldNorm * peakHoldDecay;

    displayGrDb = -currentGrNorm * 24.0f;

    if (compDsp != nullptr)
        isSaturating = compDsp->isSaturating();

    repaint();
}

void CompressorView::GRMeterComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    // Left label: "GR"
    auto labelArea = bounds.removeFromLeft(24.0f);
    g.setColour(ui::WvZzLookAndFeel::textSecondary);
    g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
    g.drawText("GR", labelArea, juce::Justification::centredLeft, false);

    // Right label: live numeric readout e.g. "-4.2 dB"
    auto readoutArea = bounds.removeFromRight(46.0f);
    if (currentGrNorm > 0.02f)
    {
        g.setColour(isSaturating ? ui::WvZzLookAndFeel::coralRed : ui::WvZzLookAndFeel::honeyGold);
        g.setFont(juce::FontOptions(8.5f, juce::Font::bold));
        g.drawText(juce::String::formatted("%.1f dB", displayGrDb),
                   readoutArea, juce::Justification::centredRight, false);
    }
    else
    {
        g.setColour(ui::WvZzLookAndFeel::textSecondary.withAlpha(0.4f));
        g.setFont(juce::FontOptions(8.5f, juce::Font::plain));
        g.drawText("0.0 dB", readoutArea, juce::Justification::centredRight, false);
    }

    bounds.reduce(4.0f, 2.0f);
    auto trackBounds = bounds;

    // Track background
    g.setColour(ui::WvZzLookAndFeel::cardBg.darker(0.4f));
    g.fillRoundedRectangle(trackBounds, 3.0f);
    g.setColour(ui::WvZzLookAndFeel::cardBorder);
    g.drawRoundedRectangle(trackBounds, 3.0f, 1.0f);

    const float trackW = trackBounds.getWidth() - 2.0f;
    const float trackH = trackBounds.getHeight() - 2.0f;
    const float trackX = trackBounds.getX() + 1.0f;
    const float trackY = trackBounds.getY() + 1.0f;

    // Horizontal calibrated tick marks at 0, -3, -6, -12, -18, -24 dB
    const float tickDbs[] = { 3.0f, 6.0f, 12.0f, 18.0f };
    g.setColour(ui::WvZzLookAndFeel::cardBorder.withAlpha(0.5f));
    for (float db : tickDbs)
    {
        const float tickX = trackX + trackW * (db / 24.0f);
        g.drawVerticalLine(static_cast<int>(tickX), trackY, trackY + trackH);
    }

    // Active Gain Reduction fill (extends horizontally from left 0 dB)
    if (currentGrNorm > 0.005f)
    {
        const float fillW = trackW * currentGrNorm;
        auto fillRect = juce::Rectangle<float>(trackX, trackY, fillW, trackH);

        juce::Colour leftColor = ui::WvZzLookAndFeel::honeyGold;
        juce::Colour rightColor = isSaturating ? ui::WvZzLookAndFeel::coralRed.brighter(0.2f) : ui::WvZzLookAndFeel::coralRed;

        juce::ColourGradient grGrad(leftColor, trackX, trackY,
                                    rightColor, trackX + trackW, trackY, false);
        g.setGradientFill(grGrad);
        g.fillRoundedRectangle(fillRect, 2.0f);
    }

    // Floating Peak-Hold Line
    if (peakHoldNorm > 0.01f)
    {
        const float peakX = trackX + trackW * peakHoldNorm;
        g.setColour(ui::WvZzLookAndFeel::coralRed.brighter(0.3f));
        g.fillRect(peakX - 1.0f, trackY, 2.0f, trackH);
    }
}

CompressorView::CompressorView(juce::AudioProcessorValueTreeState& apvts,
                               const juce::String& prefix,
                               const CompressorDSP* compDsp,
                               ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      compressorDSP(compDsp),
      card("COMPRESSOR", ui::WvZzLookAndFeel::honeyGold)
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
        helpButton = std::make_unique<ui::HelpButtonComponent>("compressor", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    // 5 Rotary Knobs: Thresh, Ratio, Attack, Release, Mix
    initRotary(threshKnob,  paramId("thresh"),  "Thresh",  ui::WvZzLookAndFeel::honeyGold);
    initRotary(ratioKnob,   paramId("ratio"),   "Ratio",   ui::WvZzLookAndFeel::honeyGold);
    initRotary(attackKnob,  paramId("attack"),  "Attack",  ui::WvZzLookAndFeel::honeyGold);
    initRotary(releaseKnob, paramId("release"), "Release", ui::WvZzLookAndFeel::honeyGold);
    initRotary(mixKnob,     paramId("mix"),     "Mix",     ui::WvZzLookAndFeel::honeyGold);

    // Phat Mode Toggle
    phatToggle.button.setButtonText("PHAT");
    phatToggle.button.getProperties().set("isPill", true);
    phatToggle.button.getProperties().set("accentColor", static_cast<int>(ui::WvZzLookAndFeel::honeyGold.getARGB()));
    phatToggle.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsState, paramId("phat"), phatToggle.button);
    addAndMakeVisible(phatToggle.button);

    // GR Meter
    grMeter = std::make_unique<GRMeterComponent>(compressorDSP, compressorDSP);
    addAndMakeVisible(*grMeter);
}

void CompressorView::initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText, juce::Colour accent)
{
    rc.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rc.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    rc.slider.getProperties().set("accentColor", static_cast<int>(accent.getARGB()));
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

void CompressorView::paint(juce::Graphics&)
{
}

void CompressorView::updateVisuals()
{
    if (grMeter != nullptr)
        grMeter->updateLevels();

    if (compressorDSP != nullptr && phatToggle.button.getToggleState())
    {
        if (compressorDSP->isSaturating())
            phatToggle.button.getProperties().set("accentColor", static_cast<int>(ui::WvZzLookAndFeel::coralRed.getARGB()));
        else
            phatToggle.button.getProperties().set("accentColor", static_cast<int>(ui::WvZzLookAndFeel::honeyGold.getARGB()));
        phatToggle.button.repaint();
    }
}

void CompressorView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();

    // Top Row: Horizontal Gain Reduction Meter + PHAT Pill Button
    auto topRow = content.removeFromTop(20);
    phatToggle.button.setBounds(topRow.removeFromRight(50).reduced(0, 1));
    topRow.removeFromRight(8); // Gap

    if (grMeter != nullptr)
        grMeter->setBounds(topRow.reduced(0, 1));

    content.removeFromTop(3); // Margin between top meter bar and knobs

    // Bottom Row: 5 Knobs filling the full width and remaining height
    auto layoutKnob = [](RotaryControl& rc, juce::Rectangle<int> r) {
        int labelH = 14;
        auto knobArea = r.reduced(1);
        rc.label.setBounds(knobArea.removeFromBottom(labelH));
        rc.slider.setBounds(knobArea);
    };

    const int numKnobs = 5;
    const int knobWidth = content.getWidth() / numKnobs;

    layoutKnob(threshKnob,  content.removeFromLeft(knobWidth));
    layoutKnob(ratioKnob,   content.removeFromLeft(knobWidth));
    layoutKnob(attackKnob,  content.removeFromLeft(knobWidth));
    layoutKnob(releaseKnob, content.removeFromLeft(knobWidth));
    layoutKnob(mixKnob,     content);
}

} // namespace wvzz::modules
