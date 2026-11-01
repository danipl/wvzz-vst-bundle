# Design

## Context

CamelPhat 3 operates with a strictly zero-allocation, lock-free real-time audio thread. The UI is implemented with JUCE 8 using custom vector components laid out in a 3-column, 1080x680 resizable canvas. The Center Column currently houses an interactive X/Y Pad and dual LFO modulation panels. See `proposal.md` for background and motivation.

## Goals / Non-Goals

**Goals:**
- Provide zero-allocation, lock-free streaming of raw DAW input audio and final post-master audio from `processBlock()` to the GUI.
- Implement a high-performance vector visualizer supporting three swappable views: Oscilloscope (default), Spectrum Analyzer (FFT), and Delta (difference).
- Render both signals overlaid on the same coordinate plane with high-contrast neon styling (Neon Cyan for Input, Neon Orange for Output).
- Seamlessly integrate into the Center Column with a tabbed switch between the X/Y Pad and the Signal Monitor.

**Non-Goals:**
- Modifying audio routing, DSP algorithms, or adding latency.
- Audio recording, export to WAV/CSV, or DAW sidechain input visualization.
- Resizing the main plugin canvas beyond the existing 1080x680 base coordinate system.

## Decisions

### Decision 1: Lock-Free Double Ring Buffer (`AbstractFifo`) for Audio Capture
- **Choice**: Implement an `AudioVisualizerSource` class inside `Source/DSP/` holding two pre-allocated float ring buffers (e.g. 8192 samples each) managed by `juce::AbstractFifo`.
- **Rationale**: `juce::AbstractFifo` is wait-free and lock-free for single-producer single-consumer patterns. Audio thread pushes samples in chunks without waiting; UI thread pops available samples on its timer.
- **Alternatives Considered**:
  - `std::mutex` with double-buffering: Rejected because taking mutexes in `processBlock()` causes priority inversion and audio dropouts.
  - JUCE `AudioVisualiserComponent`: Rejected because it only supports a single input channel stream and lacks dual-trace overlay, FFT frequency-domain curves, and custom CamelPhat LookAndFeel styling.

### Decision 2: UI-Side FFT Computation
- **Choice**: Perform FFT calculations exclusively on the GUI thread inside `SignalComparisonComponent` using `juce::dsp::FFT` (order 10 = 1024 points) and a pre-calculated Hann window table.
- **Rationale**: Keeps the audio thread purely dedicated to sound processing. The UI thread easily handles a 1024-point FFT at 30–60 FPS with negligible CPU impact (< 1%).
- **Alternatives Considered**: Computing FFT on audio thread or background worker thread: Rejected as unnecessary complexity; modern CPUs execute a 1024-point float FFT in microseconds on the GUI thread.

### Decision 3: Center Column Header Tab Integration
- **Choice**: Add a segmented pill button (`[ X/Y PAD ]` / `[ MONITOR ]`) in the Center Column card header.
- **Rationale**: The X/Y Pad area is 360 x 270 px—the largest open visual space in the interface. Placing the visualizer here maintains the 1080x680 layout, avoids shrinking existing knob cards, and keeps LFO 1/2 accessible at all times.
- **Alternatives Considered**:
  - Expand plugin height with a permanent bottom strip: Rejected because it breaks standard DAW window sizing and preset screenshots.
  - Floating modal dialog: Rejected because producers want to see parameters and visual feedback simultaneously while adjusting knobs.

### Decision 4: Signal Normalization and Visual Overlay
- **Choice**: Plot both Input and Output directly over the same center zero-crossing line with semi-transparent area shading for Input and a bright stroke for Output. Provide a `Scale` toggle (1x, 2x, 4x) to boost quiet signals visually.
- **Rationale**: Direct overlay allows immediate recognition of distortion clipping (tops of waves flattened) and compression leveling (reduced peak-to-RMS spread).

## Risks / Trade-offs

- **[Risk] High GUI CPU load during continuous FFT/waveform path rendering** → **Mitigation**: Use fixed-size decimation (step size calculated from pixel width) so waveform paths have at most ~350 points; skip FFT calculation when Oscilloscope mode is active.
- **[Risk] Audio thread dropouts if FIFO overflows** → **Mitigation**: Audio thread performs non-blocking writes; if FIFO is full, incoming samples are safely dropped until the UI reads.
- **[Risk] Glitches during sample rate change** → **Mitigation**: Reset and re-allocate FIFO buffers in `prepareToPlay(sampleRate, samplesPerBlock)`.

## Open Questions

- None. All requirements and architectural choices are resolved.
