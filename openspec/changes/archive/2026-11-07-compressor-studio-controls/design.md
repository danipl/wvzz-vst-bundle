# Design: Compressor Studio Controls and Visual Metering

## Context

`CompressorDSP` provides optical-style dynamics processing within the WvZz DSP pipeline, and `CompressorView` renders its user interface within the 130px height right-column module card. See `proposal.md` for background motivation.

## Goals / Non-Goals

**Goals:**
- Expose dedicated studio controls for Threshold (`comp_thresh`), Ratio (`comp_ratio`), Attack (`comp_attack`), Release (`comp_release`), and parallel Mix (`comp_mix`).
- Compute dynamic auto-makeup gain to maintain consistent perceived volume across variable threshold and ratio settings.
- Implement parallel dry/wet crossfading within the real-time audio thread without extra latency.
- Refactor `CompressorView` to display 5 rotary knobs with consistent typography and accent branding.
- Implement responsive visual meter ballistics (instant attack down, smooth decay up) with a floating peak-hold line, calibrated scale ticks (`0` to `-24 dB`), and live numeric decibel readout.
- Add active visual feedback when Phat Mode saturation soft-clips transients.

**Non-Goals:**
- External sidechain bus routing or UI selector in this change.
- Multi-band dynamics splitting.

## Decisions

### 1. Parameter Registration & Legacy Compatibility
- **Decision**: Register `comp_thresh` (`-48.0 dB` to `0.0 dB`, default `-18.0 dB`), `comp_ratio` (`1.5:1` to `20.0:1`, default `4.0:1`), `comp_attack` (`0.1 ms` to `50.0 ms`, default `5.0 ms`), and `comp_mix` (`0.0%` to `100.0%`, default `100.0%`). Retain `comp_amount` in the layout as a secondary additive macro, defaulting to 0.0, to prevent DAW preset displacement.
- **Rationale**: Gives pro audio engineers direct control over individual dynamic envelope parameters while maintaining full preset backward compatibility.

### 2. Dynamic Auto-Makeup Formulation
- **Decision**: Calculate makeup gain as:
  $$G_{\text{makeup}}(\text{dB}) = -0.5 \times \text{thresholdDb} \times \left(1 - \frac{1}{\text{ratio}}\right)$$
- **Rationale**: As threshold is lowered or ratio increased, gain reduction naturally deepens. This formula automatically restores peak and RMS loudness, eliminating the need for constant manual makeup gain adjustments.

### 3. In-Module Parallel Compression
- **Decision**: Crossfade dry and wet paths using a linear blend:
  $$\text{out} = \text{in} \times (1.0 - \text{mix}) + \text{wet} \times \text{mix}$$
- **Rationale**: Since `CompressorDSP` introduces zero group delay (ADAA saturation is zero-latency), blending dry input with compressed wet audio produces phase-coherent parallel compression (New York compression) without comb filtering.

### 4. 5-Knob Responsive View Layout
- **Decision**: Arrange the 5 knobs horizontally across the card content area:
  - Width: ~350 px divided across 5 knobs (~70 px per knob column).
  - Height: ~75 px for rotary knob + label.
  - Right area: 40 px reserved for the calibrated vertical GR meter and numeric readout.
  - Top-right header area: `PHAT` pill toggle button.

### 5. Calibrated Meter Ballistics & Peak-Hold
- **Decision**: Maintain visual ballistics inside `CompressorView` updated at 60 FPS:
  - Instant attack: tracks incoming negative gain reduction immediately.
  - Release decay: exponential fallback factor $\approx 0.88$ per frame ($60\text{ Hz}$).
  - Peak hold: floating line holding the maximum negative peak, decaying at $\approx 0.96$ per frame.
  - Calibration: horizontal tick marks at $0, -3, -6, -12, -18, -24\text{ dB}$, with a live text readout (e.g. `-4.2 dB`) beneath the meter.
- **Decision on Saturation Glow**: Monitor whether output samples trigger the $-0.1\text{ dBFS}$ ADAA soft-clip ceiling and illuminate a warm honey/coral glow on the Phat toggle when active.

## Risks / Trade-offs

- **[Risk] Visual crowding with 5 knobs**: Squeezing 5 knobs into a ~350 px area could make labels overlap on smaller window scales.
  - *Mitigation*: Use compact 9.5 pt bold typography for parameter labels and maintain clean margins matching `WvZzLookAndFeel`.
- **[Risk] Audio-thread safety with parallel mix**: Adding mix math must not allocate or lock.
  - *Mitigation*: Pure float arithmetic `in * (1 - mix) + wet * mix` marked `noexcept`.
