# Spec Delta: dsp-engine

## MODIFIED Requirements

### Requirement: Optical Compressor with Phat Mode
The system SHALL provide a feed-forward optical compressor supporting independent parameters for Threshold `comp_thresh` (`-48.0` to `0.0 dB`, default `-18.0 dB`), Ratio `comp_ratio` (`1.5:1` to `20.0:1`, default `4.0:1`), Attack `comp_attack` (`0.1` to `50.0 ms`, default `5.0 ms`), Release `comp_release` (`10.0` to `1500.0 ms`, default `120.0 ms`), and parallel Mix `comp_mix` (`0.0` to `1.0`, default `1.0`), alongside legacy `comp_amount` drive compatibility, operating with decibel-domain dual-stage optical release ballistics, dynamic auto-makeup compensation, and a selectable Phat Mode (`comp_phat = 1`) featuring a post-makeup 1st-order ADAA `tanh` soft-clip ceiling at `-0.1 dBFS`.

#### Scenario: Dynamic compression with Phat saturation
- **WHEN** `comp_thresh` is set below signal level and `comp_phat` is toggled between `0` and `1`
- **THEN** the compressor applies gain reduction based on `comp_thresh`, `comp_ratio`, `comp_attack`, and `comp_release`, switching between clean leveling and aggressive brickwall saturation with a `-0.1 dBFS` ADAA soft-clip ceiling

#### Scenario: Dual-stage optical release behavior
- **WHEN** a transient exceeds the compressor threshold and subsequently falls below threshold
- **THEN** the gain reduction decays in the decibel domain with a fast initial recovery stage (~50% of attenuation over 40–70 ms) followed by an exponential program-dependent tail stage scaled by `comp_release`

#### Scenario: Anti-aliased Phat saturation ceiling
- **WHEN** audio is processed in Phat Mode (`comp_phat = 1`) with peak levels exceeding the `-0.1 dBFS` ceiling
- **THEN** the saturation stage evaluates the 1st-order ADAA antiderivative of `tanh` across consecutive samples, suppressing digital foldback aliasing with zero latency

#### Scenario: Parallel compression blend
- **WHEN** `comp_mix` is adjusted between `0.0` and `1.0`
- **THEN** the output linearly crossfades between the uncompressed input audio and the compressed/saturated wet signal
