# Spec Delta

## MODIFIED Requirements

### Requirement: Stereo Modulated Flanger
The system SHALL apply a stereo flanger: a short delay line with 3rd-order fractional interpolation and a `90°` left/right LFO offset. Parameters: `flg_center` (`0.5–10 ms`, default `2.5`), `flg_depth` (`0–5 ms`, default `2.0`), `flg_feedback` (`-0.95 to 0.95`, default `0.55`), `flg_tzero` (default off), `flg_rate` (`0.01–20 Hz`, default `0.4`), `flg_amount` (`0–1`), and `flg_bypass` (default `1`). In WvZz Distortion it runs after the Band-Reject summing node.

#### Scenario: Stereo flanger modulation
- **WHEN** `flg_bypass` is `0` and `flg_amount` is increased above `0.0`
- **THEN** the 3rd-order interpolated delay sweeps `flg_center ± flg_depth` in stereo quadrature with `flg_feedback` feedback

#### Scenario: Default values keep the previous sound
- **WHEN** `flg_feedback`, `flg_depth`, `flg_center`, and `flg_tzero` are at their defaults
- **THEN** the output equals the output of the flanger with fixed `2.5 ms` center, `±2.0 ms` depth, and `0.55` feedback

#### Scenario: Through-zero mode
- **WHEN** `flg_tzero` is on
- **THEN** the dry path is delayed by `flg_center`, so the wet signal sweeps before and after the dry signal and the comb notches pass through zero delay

#### Scenario: Negative feedback
- **WHEN** `flg_feedback` is negative
- **THEN** the feedback signal is inverted and the output stays finite
