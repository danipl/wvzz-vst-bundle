# dsp-engine Specification

## Purpose
Defines the real-time audio digital signal processing pipeline recreating the CamelPhat 3 multi-effect processor signal topology and sonic characteristics.

## Requirements

### Requirement: Band-Pass Splitter Routing
The DSP engine SHALL split incoming stereo audio into a frequency-selective passband signal and a complementary band-reject signal using resonant high-pass and low-pass crossover filters.

#### Scenario: Audio band splitting
- **WHEN** audio is processed through the BP Filter splitter
- **THEN** frequencies within the cutoff window pass to the distortion stage while frequencies outside are diverted to the band-reject parallel feed

### Requirement: 4-Stage Oversampled Distortion
The system SHALL process the passband audio through 4 parallel distortion circuits (Tube, Mech, Bit Crusher, and Xcita) within a 4x minimum-phase polyphase IIR oversampling wrapper and remove direct current bias with a 10 Hz high-pass filter.

#### Scenario: Parallel distortion saturation
- **WHEN** distortion amounts for Tube, Mech, Bit Crusher, or Xcita are increased above zero
- **THEN** harmonic overtones and non-linear saturation are generated with zero introduced algorithmic latency and no DC bias

### Requirement: Modulated Multi-Mode Filter
The system SHALL provide a multi-mode resonant filter (Lowpass, Bandpass, Highpass) with cutoff frequency modulatable by an audio envelope follower.

#### Scenario: Envelope follower cutoff modulation
- **WHEN** transient audio peaks exceed the envelope follower threshold
- **THEN** the filter cutoff frequency modulates dynamically in proportion to the input envelope depth

### Requirement: Stereo Modulated Flanger
The system SHALL apply a modulated delay line flanger with adjustable stereo phase offset and soft-saturating feedback.

#### Scenario: Stereo flanger modulation
- **WHEN** the flanger depth and feedback parameters are engaged
- **THEN** short delay sweeps create comb-filter phasing across left and right channels without runaway feedback oscillation

### Requirement: Band-Reject Summer Node
The system SHALL sum the wet filtered/flanged signal with the band-reject bypass signal according to the Band-Reject Mix control.

#### Scenario: Band-reject path recombining
- **WHEN** the Band-Reject Mix parameter is adjusted
- **THEN** the dry out-of-band frequencies combine with the wet chain without unintended phase cancellation

### Requirement: Magic EQ Sub-Bass Boost
The system SHALL provide a tuned low-frequency resonant boost with warm saturation to enhance sub-bass frequencies.

#### Scenario: Low frequency reinforcement
- **WHEN** Magic EQ amount is increased
- **THEN** sub-bass frequencies below 100 Hz receive resonant gain boost and harmonic warming

### Requirement: Optical Compressor with Phat Mode
The system SHALL provide program-dependent optical dynamic range compression with an optional analog-style Phat non-linear saturation mode.

#### Scenario: Dynamic compression with Phat saturation
- **WHEN** compression threshold is exceeded and Phat mode is enabled
- **THEN** signal dynamics are smoothed with optical attack/release timing and enriched with non-linear saturation

### Requirement: Master Mix and Output Gain
The system SHALL blend the fully processed multi-effect signal with the original dry input buffer and apply master output gain.

#### Scenario: Dry/Wet balance control
- **WHEN** master Mix parameter is set between 0% and 100%
- **THEN** output transitions continuously between the original unprocessed input and the fully processed effect chain
