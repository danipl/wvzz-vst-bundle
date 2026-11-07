# Design: Compressor Quality Improvements

## Context

`CompressorDSP` operates as an `IDSPModule` and `IMeterSource` inside the WvZz audio pipeline, processing stereo audio blocks via `ProcessContext` under strict real-time guarantees (zero dynamic heap allocations, zero locks, `noexcept`). See `proposal.md` for problem background and motivation.

## Goals / Non-Goals

**Goals:**
- Implement decibel-domain dual-stage optical release ballistics to deliver smooth, program-dependent dynamics with rapid transient punch recovery.
- Implement 1st-order Anti-Derivative Anti-Aliasing (ADAA) for Phat Mode `tanh` saturation to eliminate audible digital aliasing without adding latency.
- Move atomic telemetry writes from the inner per-sample loop to a single store per audio block.
- Smooth modulatable parameters (`comp_amount`, `comp_release`) without whole-block stepping to prevent zipper noise.
- Maintain full compatibility with existing parameter IDs, APVTS bindings, and zero-allocation requirements.

**Non-Goals:**
- Modifying parameter IDs or adding new public controls (preserves full preset compatibility).
- Multi-channel expansion beyond stereo (left/right).
- Oversampling pipelines that introduce group delay / latency compensation requirements.

## Decisions

### 1. Decibel-Domain Gain Smoothing vs. Linear Envelope Detection
- **Decision**: Compute raw detector input peaks in dB, evaluate static gain reduction in dB, and perform envelope filtering directly on the gain reduction signal (`grDb`).
- **Rationale**: Smoothing in the decibel domain creates constant decay in dB/second. This mirrors human loudness perception and avoids the unnatural, sluggish decay tails produced by linear-domain exponential filters.
- **Alternatives Considered**: Keeping linear-domain detection with an ad-hoc polynomial curve (rejected: does not provide consistent dB/sec ballistic control across varying input levels).

### 2. Dual-Stage Optical Release Ballistics Model
- **Decision**: Model the electro-optical attenuator (T4 photocell behavior) using a two-state release system:
  - **Fast Stage ($GR_{\text{fast}}$)**: Rebounds rapidly with a fixed time constant ($\tau_{\text{fast}} \approx 50\text{ ms}$) to recover dynamic punch immediately following transients.
  - **Slow Stage ($GR_{\text{slow}}$)**: Decays based on the user-selected `comp_release` ($10\text{--}1500\text{ ms}$) to smoothly hold sustained energy without modulation distortion.
  - **Composite Gain Reduction**: $GR_{\text{total}}[n] = 0.5 \cdot GR_{\text{fast}}[n] + 0.5 \cdot GR_{\text{slow}}[n]$.
- **Rationale**: This closely approximates classic optical compressor hardware, providing punch on drum hits while remaining transparent on basslines and full mixes.
- **Alternatives Considered**: Single variable-coefficient release (rejected: more prone to instability and harder to tune predictably).

### 3. Analytical 1st-Order ADAA for Phat Mode Saturation
- **Decision**: Apply 1st-order Anti-Derivative Anti-Aliasing to the `tanh` soft-clip ceiling at $-0.1\text{ dBFS}$.
  - Antiderivative: $F_1(x) = \int \tanh(x)\,dx = \ln(\cosh(x))$.
  - Ill-conditioning fallback: When $|x[n] - x[n-1]| < 10^{-5}$, evaluate midpoint $\tanh\left(\frac{x[n] + x[n-1]}{2}\right)$ to prevent floating-point division by zero.
  - Large-argument asymptotic protection: For $|x| > 16.0$, use $\ln(\cosh(x)) \approx |x| - \ln(2)$ to avoid numerical overflow in `std::cosh`.
- **Rationale**: ADAA suppresses harmonic foldback aliasing by $15\text{--}25\text{ dB}$ across high frequencies while introducing **zero latency** and minimal CPU overhead.
- **Alternatives Considered**: Polyphase $2\times$ oversampling (rejected: requires group delay latency compensation in `ProcessContext` and FIR/IIR filtering overhead).

### 4. Block-Rate Telemetry Store
- **Decision**: Accumulate peak (most negative) gain reduction in a local float across the block and store to `gainReductionDb` once at the end of `process()`.
- **Rationale**: Eliminates 512 to 2048 atomic writes per audio block, removing unnecessary CPU cache invalidations on the real-time audio thread.

### 5. Sub-Block Parameter Interpolation
- **Decision**: Process incoming blocks in sub-blocks of 32 samples (or interpolate smoothed values per sample) instead of advancing parameters by `skip(numSamples)`.
- **Rationale**: Prevents audible clicks and zipper noise when `comp_amount` is modulated by LFOs or automated by the host.

## Risks / Trade-offs

- **[Risk] ADAA numerical precision with floats**: Dividing by $(x[n] - x[n-1])$ can introduce precision loss when samples are nearly identical.
  - *Mitigation*: Threshold delta check ($10^{-5}$) with midpoint evaluation fallback and asymptotic approximation for large inputs.
- **[Risk] Telemetry responsiveness**: Updating meter atomic once per block instead of per sample.
  - *Mitigation*: At typical buffer sizes (64 to 512 samples at 44.1 kHz), block intervals are $1.4\text{ to }11.6\text{ ms}$, which is faster than standard 60 FPS GUI refresh rates (16.6 ms). Tracking the block peak preserves visual transient peaks accurately.
