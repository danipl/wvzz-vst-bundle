# Proposal

## Why

When working with sound-shaping multi-effects like distortion, dual filters, compression, and modulation, producers need immediate visual confirmation of how the processing chain transforms the audio. Currently, CamelPhat 3 only provides small stereo VU peak/RMS meters in the Master section. Adding a dedicated real-time comparative visualizer will allow users to directly inspect waveform changes (clipping, compression squaring, transients), frequency spectrum alterations (filter cuts, EQ boosts, harmonic generation), and net signal differences side-by-side on the same coordinate plane.

## What Changes

- **Dual-Trace Signal Tap in DSP Engine**: Tap audio directly at the DAW input buffer (pre-input-gain raw input) and at the final output buffer (post-master-section, post-volume) using pre-allocated, lock-free ring buffers (`juce::AbstractFifo`) ensuring 0 heap allocations and no locks on the real-time audio thread.
- **Center Card View Switcher**: Introduce tabbed navigation in the Center Column header allowing users to toggle between `[ X/Y PAD ]` and `[ SIGNAL MONITOR ]` without altering the standard 1080x680 plugin layout.
- **Three Swappable Visualizer Modes**:
  - **Oscilloscope (Default)**: Real-time time-domain waveform display plotting both Input and Output on the same zero-crossing axis using high-contrast neon colors (Cyan for Input, Orange for Output).
  - **Spectrum Analyzer (FFT)**: 20 Hz to 20 kHz log-frequency spectrum curve overlay computed via UI-thread FFT with Hann windowing.
  - **Delta / Difference View**: Real-time visualization of the difference signal (`Output - Input`), isolating exactly what harmonics, filters, and dynamic changes CamelPhat added.
- **Visualizer Controls**: Provide interactive view mode selector pills (`Scope`, `Spectrum`, `Delta`), timebase/gain scaling, and a freeze frame toggle for precise waveform analysis.

## Capabilities

### New Capabilities
- `signal-comparison-visualizer`: Real-time capture, analysis, and rendering of comparative input vs output signals across time-domain (oscilloscope), frequency-domain (spectrum analyzer), and delta difference views.

### Modified Capabilities
- `plugin-ui-system`: Update the Center Column layout requirements to support tabbed switching between the interactive X/Y Pad and the Signal Monitor.

## Impact

- **DSP / Audio Thread**: `PluginProcessor` adds lock-free ring buffers (`juce::AbstractFifo`) to push sub-block samples of raw input and final output. Zero allocation in `processBlock()` is strictly preserved.
- **UI Components**:
  - New component `SignalComparisonComponent` in `Source/UI/`.
  - Updated `PluginEditor` to host the tabbed center view switcher and manage component visibility and 60 Hz timer updates.
- **Dependencies**: Uses JUCE's built-in `juce::dsp::FFT` and `juce::AbstractFifo`; no new external libraries required.
