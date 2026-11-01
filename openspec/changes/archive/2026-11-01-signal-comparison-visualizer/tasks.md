# Tasks

## 1. Lock-Free Audio Tap Infrastructure

- [x] 1.1 Create `Source/DSP/AudioVisualizerSource.h` implementing double-buffered lock-free ring buffers using `juce::AbstractFifo` for input and output capture, and verify FIFO read/write operations with a unit test.
- [x] 1.2 Integrate `AudioVisualizerSource` into `CamelPhatAudioProcessor` (`PluginProcessor.h` / `PluginProcessor.cpp`) tapping raw input before `Input Gain` and final output after the Master section, and verify zero heap allocations during audio processing under headless tests.

## 2. Signal Comparison Visualizer Component

- [x] 2.1 Create `Source/UI/SignalComparisonComponent.h` and `.cpp` with a 60 FPS timer consuming samples from `AudioVisualizerSource` and rendering the dual-trace Oscilloscope time-domain view with overlaid Cyan (input) and Orange (output) waveforms.
- [x] 2.2 Implement the Dual Spectrum Analyzer (FFT) mode using `juce::dsp::FFT` with Hann windowing on the UI thread, mapping 20 Hz to 20 kHz on a logarithmic axis.
- [x] 2.3 Implement the Delta Difference mode rendering the differential curve between output and input to highlight added saturation and filter harmonics.
- [x] 2.4 Add header controls for switching modes (`SCOPE`, `SPECTRUM`, `DELTA`), toggling vertical display gain scale (1x, 2x, 4x), and toggling display freeze.

## 3. UI Layout and View Switcher Integration

- [x] 3.1 Update `PluginEditor.h` and `PluginEditor.cpp` to add a tabbed view switcher (`[ X/Y PAD ]` / `[ MONITOR ]`) in the Center Column card header.
- [x] 3.2 Wire component visibility and bounds management in `PluginEditor::resized()`, ensuring seamless switching between the X/Y Pad and the Signal Monitor within the 1080x680 canvas without affecting LFO 1/2.

## 4. Verification and Build

- [x] 4.1 Run automated test suite via `make test` to verify DSP engine integrity and ensure strict zero-allocation compliance during audio rendering blocks.
- [x] 4.2 Compile VST3, AU, and Standalone targets via `make build` and verify clean compilation without warnings or linker errors.
