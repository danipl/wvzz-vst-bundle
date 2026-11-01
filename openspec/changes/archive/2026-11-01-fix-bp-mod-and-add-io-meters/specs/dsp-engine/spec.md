# Spec Delta

## MODIFIED Requirements

### Requirement: Band-Pass Splitter Routing
The DSP engine SHALL split incoming stereo audio into a Passband signal and a Rejected Band signal using two 12 dB/oct (2-pole) Cytomic zero-delay feedback (ZDF) trapezoidal State Variable Filters (one High-Pass at `bp_low_cut` with `bp_low_res`, one Low-Pass at `bp_high_cut` with `bp_high_res`, Q mapped `0.707` to `12.0`), where `Low_Rejected = LowPass_Output(Input, bp_low_cut, bp_low_res)`, `High_Rejected = HighPass_Output(Input, bp_high_cut, bp_high_res)`, `Passband = BandPass_Cascade(Input)`, and `bp_link` locks the logarithmic frequency delta between `bp_low_cut` and `bp_high_cut`. The engine and parameter controls SHALL enforce that `bp_low_cut` never exceeds `bp_high_cut` ($f_{\text{low}} \le f_{\text{high}}$), and companion parameter synchronization under `bp_link` SHALL NOT invoke host parameter notifications from the real-time audio thread.

#### Scenario: Audio band splitting
- **WHEN** audio is processed through the BP Filter splitter with `bp_bypass = 0`
- **THEN** frequencies within the passband cascade pass to the Distortion stage while `Low_Rejected + High_Rejected` are extracted for post-MM-Filter recombination

#### Scenario: Linked cutoff movement
- **WHEN** `bp_link` is enabled (`1`) and either `bp_low_cut` or `bp_high_cut` is adjusted
- **THEN** the companion cutoff updates automatically to preserve the logarithmic frequency ratio within `[20.0 Hz, 20000.0 Hz]` without audio-thread host notification calls

#### Scenario: Cutoff boundary clamping prevents crossover
- **WHEN** user or automation adjusts `bp_low_cut` upward toward `bp_high_cut` or `bp_high_cut` downward toward `bp_low_cut`
- **THEN** the values are clamped such that `bp_low_cut` does not exceed `bp_high_cut`, preventing passband silencing and out-of-band double summation

## ADDED Requirements

### Requirement: Stereo Input Signal Level Metering
The DSP engine SHALL provide lock-free atomic stereo peak and RMS level metering (`IMeterSource`) tapped immediately after the `input_gain` stage before the audio enters the BP filter splitter.

#### Scenario: Input level monitoring during playback
- **WHEN** audio is received at the plugin input
- **THEN** peak and RMS levels for left and right channels are computed after input gain and stored in lock-free atomic variables accessible to the UI at up to 60 FPS
