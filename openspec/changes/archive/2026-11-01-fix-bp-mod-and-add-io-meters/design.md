# Design

## Context
See `proposal.md` for background and motivation. The CamelPhat plugin architecture consists of:
- `CamelPhatAudioProcessor`: Real-time DSP orchestration across sub-blocks of up to 32 samples, hosting APVTS parameters and DSP modules.
- `BPFilter`: Cytomic ZDF State Variable Filter bandpass / band-reject splitter.
- `ModulationEngine`: Control-rate LFO 1, LFO 2, and X/Y Pad offset generator mapping onto a 21-target enum.
- `MasterSection`: Output gain, latency-compensated dry/wet mixing, and atomic output VU metering (`IMeterSource`).
- `CamelPhatAudioProcessorEditor`: Resizable vector UI with custom LookAndFeel, module cards, and visualizers.

## Goals / Non-Goals

**Goals:**
- Guarantee that `bp_low_cut` never exceeds `bp_high_cut` at both the UI slider level and the DSP processing level.
- Eliminate host parameter notification calls (`setValueNotifyingHost`) from the real-time audio thread (`processBlock`).
- Accurately align the `PassbandVisualizer` rectangle with non-linear slider thumb positions.
- Deliver non-zero, active modulation offsets from LFO 1, LFO 2, and X/Y Pad to target DSP parameters every 32 samples.
- Provide real-time stereo peak and RMS input level metering alongside output metering in the Master section without audio thread allocations.

**Non-Goals:**
- Re-architecting the APVTS parameter layout or adding new modulatable targets.
- Replacing the Master Section layout with an external or top-bar oscilloscope/spectrum analyzer.
- Changing filter DSP topology or oversampling ratios.

## Decisions

### Decision 1: Two-Layer Frequency Clamping for BP Filter Cutoffs
- **Rationale**: Relying only on UI sliders leaves the DSP vulnerable to DAW automation or preset loading where `bp_low_cut > bp_high_cut`. Therefore, clamping must occur at two layers:
  1. **UI Layer**: In `PluginEditor`, `bpLowCutSlider.onValueChange` and `bpHighCutSlider.onValueChange` clamp the slider positions so that Low Cut cannot exceed High Cut, and High Cut cannot drop below Low Cut.
  2. **DSP Layer**: In `BPFilter::process()`, `safeHigh` is clamped with `std::max(safeLow, safeHigh)` so the passband never inverts and the rejected band never duplicates frequencies even during rapid modulation or automation.
- **Alternatives Considered**:
  - *Push mode (Low Cut pushes High Cut)*: Can cause runaway sliders when approaching 20 kHz and complicates parameter automation recording in hosts.
  - *Clamp mode (Low Cut stops at High Cut)*: Cleaner, predictable, and host-friendly.

### Decision 2: Message-Thread Link Handling
- **Rationale**: Calling `p->setValueNotifyingHost(...)` from `updateSmootherTargets()` in `processBlock()` is a direct violation of real-time audio thread safety in JUCE. It can cause priority inversion, lock contention, and recursive slider notifications.
- **Approach**: Move the ratio-preserving `bp_link` adjustment into the UI slider callbacks or an asynchronous `ParameterAttachment` on the message thread. The audio thread strictly reads the atomic parameter values and applies smoother targets.

### Decision 3: Proportion-of-Length Slider Mapping for Passband Visualizer
- **Rationale**: The frequency sliders use `freqSkew = NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.25f)`. `slider.getValue() / 20000.0f` produces values skewed heavily toward zero, making the drawn box misaligned with the visual slider handles.
- **Approach**: Use `lowSlider.valueToProportionOfLength(lowSlider.getValue())` and `highSlider.valueToProportionOfLength(highSlider.getValue())` to calculate exact pixel positions that match the slider knobs.

### Decision 4: Direct Block-Rate Modulation Offset Propagation
- **Rationale**: `ModulationEngine` evaluates modulation every 32 samples. Per-sample interpolation via `stepInterpolation()` was never called and is unnecessary since DSP modules already consume block-averaged modulation at the 32-sample boundary.
- **Approach**: In `ModulationEngine::updateBlockControl()`, copy `newTargets` directly into `targetOffsetsCurrent` at the end of each evaluation. `getOffset(target)` will immediately return the calculated offset, making all 21 targets responsive to LFOs and the X/Y pad.

### Decision 5: Atomic Input Meter Source
- **Rationale**: To reuse `StereoVUMeterComponent`, we implement an `IMeterSource` class (`InputMeterSource`) inside `CamelPhatAudioProcessor`. Right after `subIn` is multiplied by `inputGainLinear` in `processBlock()`, block peak and sum-of-squares are measured and stored in atomic floats with ballistics matching `MasterSection`.
- **UI Integration**: In `masterInner` within `PluginEditor::resized()`, divide the meter area into two columns: `inputMeter` (left, labeled "IN") and `outputMeter` (right, labeled "OUT").

## Risks / Trade-offs

- **[Risk]** Linked sliders in UI could fight user mouse drags if both send notifications.
  → **Mitigation**: Use a reentrancy flag (`isUpdatingFromLink`) when updating companion slider values programmatically.
- **[Risk]** Large modulation depths could theoretically push modulated cutoffs out of bounds.
  → **Mitigation**: Modulated cutoff calculations in `processBlock` are already clamped between 20.0 Hz and 20000.0 Hz; add a check ensuring `modHighCut >= modLowCut`.
- **[Risk]** Space constraints in the Master module card when hosting two meters.
  → **Mitigation**: Allocate 50px width total (24px per stereo meter with 2px gap), which comfortably fits alongside the three rotary knobs (`inputGainKnob`, `masterMixKnob`, `masterVolKnob`).
