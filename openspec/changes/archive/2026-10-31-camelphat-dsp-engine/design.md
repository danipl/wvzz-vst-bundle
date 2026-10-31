# Design

## Context
See `proposal.md` for background and motivation. This design specifies the technical architecture for the macOS VST3/AU/Standalone audio engine recreating the CamelPhat 3 multi-effect processor using C++20, JUCE 8.0.4, and CMake.

## Goals / Non-Goals

**Goals:**
- Implement a zero-allocation, real-time safe audio processing pipeline across 6 core DSP stages, a parallel band-reject dry path, and a master section.
- Wrap the 4-stage distortion section in 4x minimum-phase polyphase IIR oversampling to eliminate aliasing with zero algorithmic latency.
- Enforce full numerical safety using Topology-Preserving Transform (TPT / SVF) filter structures, 10 Hz DC blocking, soft-saturated feedback, denormal flushing, and master NaN/Inf protection.
- Manage all parameters via `juce::AudioProcessorValueTreeState` with cached atomic float pointers and sample-rate-aware parameter smoothing.
- Provide a standalone CLI verification target (`camelphat_dsp_tests`) that quantitatively asserts zero allocations, harmonic distortion, filter stability, and phase coherence.
- Provide a working interim `juce::GenericAudioProcessorEditor` for immediate auditioning in DAWs while passing Apple `auval`.

**Non-Goals:**
- Custom vector GUI, LookAndFeel, X/Y pads, and animated VU meters (explicitly deferred to Stage 2).
- Reverse-engineering legacy 32-bit `.phat` / `.fxp` binary preset formats (using native JUCE XML/ValueTree state).
- Windows or Linux builds for initial delivery.

## Decisions

### 1. Build System & JUCE 8 Management
- **Decision**: Use CMake `FetchContent` to fetch JUCE 8.0.4 from GitHub.
- **Rationale**: Enables reproducible, frictionless builds without manual git submodule initialization.
- **Configuration**:
  - `CMAKE_CXX_STANDARD 20`
  - `CMAKE_OSX_ARCHITECTURES "arm64;x86_64"`
  - `CMAKE_OSX_DEPLOYMENT_TARGET "11.0"`
  - Plugin formats: `VST3 AU Standalone`
  - Plugin codes: Manufacturer `CamA`, Plugin `Cph3`
  - Automated post-build copy: `COPY_PLUGIN_AFTER_BUILD TRUE`

### 2. Signal Routing & Buffer Layout
- **Decision**: Pre-allocate all scratch buffers in `prepareToPlay(double sampleRate, int samplesPerBlock)`.
- **Buffer Architecture**:
  - `globalDryBuffer`: Holds a copy of the raw stereo input for the final Master Dry/Wet mix.
  - `bandRejectBuffer`: Holds the out-of-band audio rejected by the BP Filter splitter.
  - `processBlock()` executes in-place on `juce::AudioBuffer<float>&` without dynamic reallocations.

```text
[Input Buffer] 
    ├──► Copy to globalDryBuffer
    ▼
[BP Filter Splitter] ──► bandRejectBuffer (scaled by BR Mix)
    │
    ▼ (Passband)
[4-Stage Distortion] (4x Polyphase IIR Oversampling + 10 Hz DC Block)
    ▼
[MM Filter] (SVF Lowpass/Bandpass/Highpass + Envelope Follower)
    ▼
[Flanger] (Stereo Modulated Delay + Tanh Feedback)
    ▼
[Summing Node] (Wet Path + bandRejectBuffer)
    ▼
[Magic EQ] (Resonant Sub-Bass Boost + Warmth)
    ▼
[Compressor] (Optical Detector + Phat Saturation)
    ▼
[Master Mix] (Crossfade with globalDryBuffer) ──► [Stereo Output & Atomic VU Feed]
```

### 3. Oversampling & Phase Alignment
- **Decision**: Use `juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR` at 4x factor.
- **Rationale**: Polyphase IIR filters introduce 0 samples of algorithmic latency and eliminate pre-ringing on punchy transients (critical for kick drums and bass). Because latency is zero, summing the parallel Band-Reject path and Global Dry path causes no phase comb-filtering.
- **Alternatives Considered**: Linear-phase FIR was rejected because its group delay would require circular delay-matching lines on both the Band-Reject and Global Dry paths and add host latency.

### 4. DSP Module Implementations
- **BP Filter Splitter**: Two 2nd-order TPT/SVF filters (High-pass at Low Cut, Low-pass at High Cut) creating a bandpass signal for the wet path and the inverted band-reject signal for the dry bypass path.
- **4-Stage Distortion**:
  - *Tube*: Soft analog saturation curve ($f(x) = \tanh(k \cdot x)$).
  - *Mech*: Asymmetric hard clipping with metallic odd-harmonic emphasis.
  - *Bit Crusher*: Sample rate reduction (zero-order hold decimation) combined with bit depth reduction ($f(x) = \text{round}(x \cdot 2^b) / 2^b$).
  - *Xcita*: High-frequency bandpass dynamic exciter saturation.
  - *DC Blocker*: 10 Hz 1-pole high-pass filter post-distortion to remove DC bias before feeding downstream stages.
- **MM Filter**: Multi-Mode State Variable Filter with smooth cut-off modulation from an analog-modeled envelope follower (attack/release ballistics).
- **Flanger**: Stereo fractional delay lines modulated by a quadrature LFO with adjustable stereo phase spread, bounded by soft $\tanh$ saturation in the feedback path.
- **Magic EQ**: Resonant low-shelf peaking boost centered around 60–80 Hz with soft non-linear saturation for analog low-end thickness.
- **Compressor**: Optical detector modeling with program-dependent attack/release ballistics, variable threshold/ratio, and a "Phat" toggle adding parallel harmonic drive.
- **Master Section**: Linear dry/wet crossfade against `globalDryBuffer`, output volume gain, and atomic float storage of peak and RMS levels for the Stage 2 VU meters.

### 5. Real-Time Safety & APVTS Parameter Architecture
- **Decision**: Strict zero heap allocation in the audio thread.
- **Implementation**:
  - `juce::ScopedNoDenormals` active during `processBlock()`.
  - Cache atomic raw parameter pointers (`std::atomic<float>*`) during initialization; no string lookups or map queries during audio processing.
  - Parameters smoothed via `juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>` (or Multiplicative for frequencies) with 20ms ramp time.
  - Master output block passes through a safety clamp replacing any non-finite (`isnan`/`isinf`) float with 0.0f.

### 6. Automated Testing Harness (`camelphat_dsp_tests`)
- **Decision**: A lightweight C++ console executable target testing the processor offline without a DAW.
- **Verification Suites**:
  1. *Allocation Test*: Overrides global `new` and `malloc` with thread-local assertion flags during 1,000 blocks of `processBlock()` to prove zero heap allocations.
  2. *Harmonic Analysis*: Feeds a 1 kHz sine wave into each distortion circuit and verifies expected harmonic energy.
  3. *Stability & NaN Stress Test*: Feeds +12 dBFS impulses and maximum resonance sweeps to confirm the output remains finite and bounded.
  4. *Parameter Smoothing Test*: Verifies continuous sample-level transitions during rapid cutoff jumps.

### 7. SOLID Interface Contracts & Zero-Allocation Biquads
- **Decision**: Define segregated abstract interfaces in `Source/DSP/IDSPModule.h`:
  - `IDSPModule` (SRP/LSP): Enforces lifecycle contracts (`prepare` and `reset`).
  - `IAudioProcessorModule` (ISP): Standardizes in-place real-time buffer processing.
  - `IMeterSource` (ISP/DIP): Segregates atomic UI metering from DSP processing logic.
- **MagicEQ Optimization**: Uses a stack-calculated Transposed Direct Form II biquad filter rather than JUCE's `IIR::Coefficients` heap pointer, ensuring 0 heap allocations during parameter sweeps.
- **AU Component Architecture**: Configured as `aumf` (Music Effect with MIDI input) to support MIDI CC / X/Y control while passing Apple `auval`.

## Risks / Trade-offs

- **[Risk: Summing cancellation between wet and band-reject paths]** → *Mitigation:* Minimum-phase polyphase IIR oversampling guarantees zero latency, keeping phase aligned.
- **[Risk: CPU spikes on silent audio tails]** → *Mitigation:* `juce::ScopedNoDenormals` flushes subnormal floats to zero on both arm64 and x86_64.
- **[Risk: Accidental allocations in audio thread]** → *Mitigation:* Automated test asserts 0 allocations across 1,000 blocks.

## Open Questions
- Deferred to Stage 2: Vintage vector graphics styling, custom LookAndFeel, dual X/Y vector control pads, and factory preset authoring.
