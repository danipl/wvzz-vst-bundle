# Proposal

## Why
While the core DSP engine is fully verified and functional, the plugin currently uses JUCE's basic generic parameter editor. Music producers and audio engineers need an intuitive, visually modern, and responsive user interface that evokes the classic CamelPhat workflow while providing immediate contextual understanding through built-in help explanations for every sound-shaping module.

## What Changes
- **Custom Modern LookAndFeel (`CamelPhatLookAndFeel`)**: Implement a cohesive vector theme featuring custom dark/neon styling, ergonomic rotary knobs with value readouts, illuminated toggle buttons, styled combo boxes, and distinct module grouping cards.
- **Contextual Component Help System ("?" Info Overlay)**: Equip every DSP module header and complex control group with an interactive "?" button. Clicking or hovering reveals an elegant, readable tooltip or overlay card explaining what the component does, its signal path role, and practical sonic tips.
- **Dual Vector X/Y Control Pads**: Implement interactive, hardware-accelerated X/Y modulation pads (Pad 1: Filter Cutoff vs Resonance; Pad 2: Tube Drive vs Mech Drive) with coordinate crosshairs and smooth dragging.
- **Animated Stereo VU Meters**: Implement a 60 FPS timer-driven stereo meter component reading atomic peak and RMS levels from the DSP engine's `IMeterSource` interface without blocking the audio thread.
- **APVTS Parameter Bindings**: Connect all 30 APVTS parameters to custom UI components using `juce::AudioProcessorValueTreeState::SliderAttachment`, `ButtonAttachment`, and `ComboBoxAttachment`.

## Capabilities

### New Capabilities
- `plugin-ui-system`: Modern vector-based graphical user interface and custom LookAndFeel implementing ergonomic controls, module cards, dual X/Y pads, and real-time metering.
- `component-help-system`: Contextual in-plugin documentation and interactive "?" help overlays explaining module DSP behavior and control functionality in plain language.

### Modified Capabilities
*(None - Existing `dsp-engine` and `plugin-host-integration` capabilities remain unchanged; the UI binds cleanly on top of their established contracts).*

## Impact
- **Processor**: Replaces `GenericAudioProcessorEditor` in `CamelPhatAudioProcessor::createEditor()` with `CamelPhatAudioProcessorEditor`.
- **Performance**: GUI runs strictly on the message thread; uses atomic readouts for metering with zero impact on real-time audio threads.
- **Dependencies**: Uses JUCE 8 `juce_gui_basics` and `juce_gui_extra` vector drawing primitives (zero external asset bloat; fully scalable at any display DPI).
