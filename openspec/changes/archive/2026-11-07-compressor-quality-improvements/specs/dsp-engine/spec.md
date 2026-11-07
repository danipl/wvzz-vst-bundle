# Spec Delta: dsp-engine

## MODIFIED Requirements

### Requirement: Optical Compressor with Phat Mode
The system SHALL provide a one-knob feed-forward compressor where `comp_amount` (`0.0`–`1.0`) simultaneously drives Threshold (`0 dB` down to `-36 dB`) and calculated Auto-Makeup Gain (`0 dB` to `+14 dB`) with adjustable release `comp_release` (`10.0`–`1500.0 ms`, default `120.0 ms`), operating with decibel-domain dual-stage optical release ballistics (fast initial release for ~50% gain recovery, followed by a program-dependent memory tail) in Normal Mode (`comp_phat = 0`: Attack `3.5 ms`, Ratio `3.5:1`, Soft Knee `6 dB`) or Phat Mode (`comp_phat = 1`: Attack `0.8 ms`, Ratio `12:1`, Hard Knee `1 dB`, plus a post-makeup 1st-order Anti-Derivative Anti-Aliasing (ADAA) `tanh` soft-clip ceiling at `-0.1 dBFS`).

#### Scenario: Dynamic compression with Phat saturation
- **WHEN** `comp_amount` is increased and `comp_phat` is toggled between `0` and `1`
- **THEN** the compressor switches between smooth `3.5:1` leveling (`3.5 ms` attack, `6 dB` knee) and aggressive `12:1` brickwall smashing (`0.8 ms` attack, `1 dB` knee, `-0.1 dBFS` soft-clip ceiling) with automatic makeup gain

#### Scenario: Dual-stage optical release behavior
- **WHEN** a transient exceeds the compressor threshold and subsequently falls below threshold
- **THEN** the gain reduction decays in the decibel domain with a fast initial recovery stage (~50% of attenuation over 40–70 ms) followed by an exponential program-dependent tail stage scaled by `comp_release`

#### Scenario: Anti-aliased Phat saturation ceiling
- **WHEN** audio is processed in Phat Mode (`comp_phat = 1`) with peak levels exceeding the `-0.1 dBFS` ceiling
- **THEN** the saturation stage evaluates the 1st-order ADAA antiderivative of `tanh` across consecutive samples, suppressing digital foldback aliasing with zero latency
