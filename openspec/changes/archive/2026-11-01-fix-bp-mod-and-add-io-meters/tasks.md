# Tasks

## 1. Modulation Engine Offset Activation

- [x] 1.1 In `Source/DSP/ModulationEngine.h`, copy `newTargets` directly into `targetOffsetsCurrent` at the end of `updateBlockControl()`, and verify `getOffset(target)` returns the calculated active modulation values.
- [x] 1.2 Add a unit test in `Tests/DSPTests.cpp` asserting that configuring LFO 1, LFO 2, or the X/Y Pad to target a parameter (such as `MMCutoff` or `DistMech`) results in modulated parameter variations in `processBlock`.

## 2. BP Filter Cutoff Clamping & Link Refactoring

- [x] 2.1 In `Source/DSP/BPFilter.h` and `Source/PluginProcessor.cpp`, clamp `cutoffHigh` and `cutoffLow` so that `cutoffLow <= cutoffHigh` during both processing and target updates, preventing inverted filter cascades and double rejection summing.
- [x] 2.2 In `Source/PluginProcessor.cpp`, remove `setValueNotifyingHost` from `updateSmootherTargets()`, and migrate the `bp_link` frequency ratio preservation logic to the message-thread slider listeners in `Source/PluginEditor.cpp` with reentrancy protection.
- [x] 2.3 In `Source/PluginEditor.cpp`, update `PassbandVisualizer::paint()` to calculate thumb coordinates using `lowSlider.valueToProportionOfLength()` and `highSlider.valueToProportionOfLength()`, matching the non-linear slider track accurately.
- [x] 2.4 In `Source/PluginEditor.cpp`, add clamping to `bpLowCutSlider` and `bpHighCutSlider` `onValueChange` callbacks so that the sliders visually and functionally stop when touching each other.
- [x] 2.5 Add a unit test in `Tests/DSPTests.cpp` verifying BP filter processing stability when Low Cut and High Cut frequencies are set equal or inverted.

## 3. Stereo Input Metering & Master UI Update

- [x] 3.1 In `Source/PluginProcessor.h` and `Source/PluginProcessor.cpp`, add an `InputMeterSource` implementing `camelphat::dsp::IMeterSource`, measuring stereo peak and RMS levels on `subIn` right after `inputGainLinear` is applied.
- [x] 3.2 In `Source/PluginEditor.h` and `Source/PluginEditor.cpp`, add an `inputMeter` component connected to `audioProcessor.getInputMeterSource()`, update existing `vuMeter` to `outputMeter`, and arrange both side-by-side with "IN" and "OUT" labels in the Master Section card.
- [x] 3.3 Verify in `PluginEditor.cpp` that `inputMeter` and `outputMeter` update at 60 FPS without audio-thread blocking or layout clipping.

## 4. Build and Integration Verification

- [x] 4.1 Run `camelphat_dsp_tests` test suite and verify all DSP tests pass.
- [x] 4.2 Build `CamelPhat_Standalone` and `CamelPhat_VST3` targets with Ninja/CMake and verify error-free compilation.
