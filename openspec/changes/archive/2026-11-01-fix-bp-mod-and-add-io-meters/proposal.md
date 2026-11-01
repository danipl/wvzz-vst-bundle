# Proposal

## Why
Users encountered three functional and usability limitations in the CamelPhat plugin:
1. In the BP Filter & Split section, the Low Cut and High Cut frequencies can cross each other ($f_{\text{low}} > f_{\text{high}}$), which collapses the passband to silence and creates phase cancellation and double energy in the rejected band. Furthermore, the `bp_link` synchronization mechanism currently triggers host parameter notification calls from the real-time audio thread, causing audio glitches.
2. In the Modulation section, modifying LFO or X/Y Pad controls has zero effect on audio because the modulation target offset buffer is never updated from its initial zero state during control-rate execution.
3. The plugin lacks visual feedback for the input signal, providing only an output VU meter in the master section and leaving users unaware of incoming signal headroom and gain staging.

## What Changes
- Enforce boundary constraints between `bp_low_cut` and `bp_high_cut` so that Low Cut cannot exceed High Cut and High Cut cannot drop below Low Cut ($f_{\text{low}} \le f_{\text{high}}$).
- Move the `bp_link` frequency tracking logic off the real-time audio thread and eliminate audio-thread `setValueNotifyingHost` invocations.
- Correct the `PassbandVisualizer` vertical coordinate calculations to use JUCE normalisable range skew mapping so that the visual passband corresponds accurately to physical slider thumb positions.
- Connect the modulation engine's calculated target offsets directly into the control-rate target offsets buffer (`targetOffsetsCurrent`) so that LFO 1, LFO 2, and the X/Y Pad audibly modulate their configured targets across the 21-target routing matrix.
- Introduce an atomic input metering source in the DSP engine measuring stereo peak and RMS levels immediately following the input gain stage.
- Update the Master section UI to feature dual stereo VU meters (Input and Output) side by side with consistent ballistics, peak hold, and dB scales.

## Capabilities

### New Capabilities
*(None)*

### Modified Capabilities
- `dsp-engine`: Enforce cutoff bounds ($f_{\text{low}} \le f_{\text{high}}$) on the Band-Pass splitter, eliminate real-time audio thread parameter notification in `bp_link`, and expose atomic stereo input metering from the input gain stage.
- `modulation-system`: Ensure control-rate calculated modulation offsets are active and routed to target parameters every 32 samples for LFO 1, LFO 2, and the X/Y Pad.
- `plugin-ui-system`: Update the Master section layout to include dual Stereo VU meters (IN and OUT), and correct the BP Filter passband visualizer thumb position mapping for non-linear frequency sliders.

## Impact
- **DSP Engine**: `BPFilter` and `CamelPhatAudioProcessor` enforce frequency clamping and provide input metering without allocations or audio-thread blocking.
- **Modulation Engine**: `ModulationEngine` propagates calculated offsets to `getOffset(target)` without breaking modulation routing.
- **UI System**: `MasterSection` layout in `PluginEditor` accommodates both IN and OUT meters, and `PassbandVisualizer` accurately draws between the two slider positions.
