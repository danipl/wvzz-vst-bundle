#include "RhythmicPumperView.h"
#include "RhythmicPumperDSP.h"

namespace wvzz::modules {

RhythmicPumperView::RhythmicPumperView(juce::AudioProcessorValueTreeState& apvts,
                                       const juce::String& prefix,
                                       const RhythmicPumperDSP* /*pumpDsp*/,
                                       ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("RHYTHMIC PUMPER", ui::WvZzLookAndFeel::electricViolet)
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

    if (helpHost != nullptr)
    {
        helpButton = std::make_unique<ui::HelpButtonComponent>("rhythmic_pumper", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    // Sub Only Toggle
    initToggle(subOnlyToggle, paramId("sub_only"), "SUB ONLY", true, ui::WvZzLookAndFeel::electricViolet);

    // Mode Selector (Tempo Sync / Audio SC)
    modeSelector.addItem("Tempo Sync", 1);
    modeSelector.addItem("Audio / Ext SC", 2);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvtsState, paramId("mode"), modeSelector);
    addAndMakeVisible(modeSelector);

    // Sync Div & Shape Selectors
    const auto& syncDivs = getPumpSyncDivNames();
    for (int i = 0; i < syncDivs.size(); ++i)
        syncDivSelector.addItem(syncDivs[i], i + 1);
    syncDivAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvtsState, paramId("sync_div"), syncDivSelector);
    addAndMakeVisible(syncDivSelector);

    const auto& shapes = getPumpShapeNames();
    for (int i = 0; i < shapes.size(); ++i)
        shapeSelector.addItem(shapes[i], i + 1);
    shapeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvtsState, paramId("shape"), shapeSelector);
    addAndMakeVisible(shapeSelector);

    // Knobs
    initRotary(amountKnob, paramId("amount"), "Amount", ui::WvZzLookAndFeel::electricViolet);
    initRotary(curveKnob,  paramId("curve"),  "Curve",  ui::WvZzLookAndFeel::electricViolet);
}

void RhythmicPumperView::initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText, juce::Colour accent)
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

void RhythmicPumperView::initToggle(ToggleControl& tc, const juce::String& paramId, const juce::String& text,
                                    bool isPill, juce::Colour accent)
{
    tc.button.setButtonText(text);
    tc.button.getProperties().set("isPill", isPill);
    tc.button.getProperties().set("accentColor", static_cast<int>(accent.getARGB()));
    tc.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsState, paramId, tc.button);
    addAndMakeVisible(tc.button);
}

void RhythmicPumperView::paint(juce::Graphics&)
{
}

void RhythmicPumperView::updateVisuals()
{
}

void RhythmicPumperView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();

    // Top Row: Sub Only pill + Mode Combo
    auto topRow = content.removeFromTop(20);
    subOnlyToggle.button.setBounds(topRow.removeFromLeft(68).reduced(0, 1));
    modeSelector.setBounds(topRow.removeFromRight(95).reduced(0, 1));

    content.removeFromTop(4);

    // Middle Row: Sync Div and Shape Combos
    auto midRow = content.removeFromTop(20);
    syncDivSelector.setBounds(midRow.removeFromLeft(60).reduced(0, 1));
    midRow.removeFromLeft(4);
    shapeSelector.setBounds(midRow.reduced(0, 1));

    content.removeFromTop(4);

    auto layoutKnob = [](RotaryControl& rc, juce::Rectangle<int> r) {
        int labelH = 14;
        auto knobArea = r.reduced(1);
        rc.label.setBounds(knobArea.removeFromBottom(labelH));
        rc.slider.setBounds(knobArea);
    };

    const int kw = content.getWidth() / 2;
    layoutKnob(amountKnob, content.removeFromLeft(kw));
    layoutKnob(curveKnob,  content);
}

} // namespace wvzz::modules
