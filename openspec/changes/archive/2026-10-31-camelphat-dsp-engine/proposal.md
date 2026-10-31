# Proposal

## Why
Camel Audio's CamelPhat 3 is one of the most revered multi-effect processors in electronic music production, renowned for its punchy low-end saturation, warm tube drive, and analog-style dynamic control. Because 32-bit legacy plugins no longer run on modern 64-bit Apple Silicon macOS environments, producers lack access to its exact topology and sonic character. This change implements the core DSP engine and host integration for a modern, zero-allocation, 64-bit macOS VST3/AU/Standalone plugin.

## What Changes
- **Build System**: Setup CMake infrastructure targeting macOS Universal Binary (`arm64` and `x86_64`) on macOS 11.0+, fetching JUCE 8.0.4 via `FetchContent`.
- **Plugin Host Integration**: Setup stereo in/out audio processor supporting VST3, AU, and Standalone formats with standard 4-character codes (`CamA` / `Cph3`), ad-hoc codesigning, and interim generic parameter editor.
- **APVTS State & Parameter Model**: Expose all modulatable parameters through `juce::AudioProcessorValueTreeState` with cached atomic float pointers and `juce::SmoothedValue` smoothing to eliminate zipper noise.
- **Band-Pass Splitter**: Dual resonant HPF + LPF splitter separating the incoming audio into passband (processed wet) and band-reject (parallel clean) paths.
- **4-Stage Distortion Module**: Parallel Tube, Mech, Bit Crusher, and Xcita circuits operating inside a 4x minimum-phase polyphase IIR oversampling wrapper with a 10 Hz DC blocking highpass filter.
- **Multi-Mode Filter**: Resonant filter (Lowpass, Bandpass, Highpass) with cut-off actively modulated by an integrated envelope follower.
- **Stereo Flanger**: Modulated delay line with LFO, stereo phase offset, and soft-saturating feedback loop.
- **Band-Reject Summer**: Summer node recombining the wet modulated signal with the band-reject path scaled by the BR Mix control.
- **Magic EQ**: Resonant low-end boost tuned for bass enhancement with warm saturation.
- **Optical Compressor**: Program-dependent compressor featuring an analog "Phat" saturation toggle.
- **Master Section**: Dry/Wet balance against the global pre-effect input buffer, master output gain, and atomic peak/RMS feed for VU metering.
- **Automated DSP Test Suite**: Dedicated `camelphat_dsp_tests` CLI console target verifying zero heap allocations in `processBlock()`, harmonic overtone generation, filter stability, and phase coherence.

## Capabilities

### New Capabilities
- `dsp-engine`: Real-time audio DSP pipeline executing the 6 core processing stages, 4x IIR oversampler, band-reject parallel split, and numerical stability protections.
- `plugin-host-integration`: Audio plugin host lifecycle management (VST3, AU, Standalone), APVTS parameter tree, state serialization/recall, and zero-allocation audio thread safety.

### Modified Capabilities
*(None - Greenfield repository with no existing specs).*

## Impact
- **Dependencies**: Adds CMake FetchContent dependency on JUCE 8.0.4.
- **APIs**: Implements standard JUCE `AudioProcessor` interfaces and APVTS state management.
- **Performance**: Enforces strict zero-allocation real-time safety inside `processBlock()` and zero-latency oversampling.
- **Validation**: AU plugin passes Apple Audio Unit validation (`auval`) on macOS.
