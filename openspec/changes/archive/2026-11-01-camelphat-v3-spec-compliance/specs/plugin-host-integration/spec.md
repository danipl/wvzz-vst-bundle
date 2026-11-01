# Spec Delta

## ADDED Requirements

### Requirement: 10 Factory Presets and Program Management
The plugin SHALL expose 10 built-in factory presets (`Neuro Reese Mangler`, `Phat Breakbeat Smash`, `Sub Weight Enhancer`, `Acid Band-Reject Sweep`, `Lo-Fi Bit Crunch`, `Warm Tube Bus Glue`, `Stepped Comb Resonator`, `Talking Vowel Wobble`, `Metallic RingMod Perc`, and `Wide Quadrature Flanger`) accessible via standard host program APIs and XML/JSON preset serialization.

#### Scenario: Selecting a factory preset
- **WHEN** a user or host selects program index `0` through `9`
- **THEN** all APVTS parameters update to the selected factory preset configuration and `getProgramName()` returns the preset's name

## MODIFIED Requirements

### Requirement: APVTS Parameter Architecture and Smoothing
The plugin SHALL expose the complete CamelPhat 3 specification parameter set (`bp_*`, `dist_*`, `mm_*`, `flg_*`, `meq_*`, `comp_*`, `master_*`, `lfo1_*`, `lfo2_*`, `xy_*`, and `input_gain`) through `AudioProcessorValueTreeState` with atomic raw pointer caching and sample-accurate 10ms–20ms linear or exponential smoothing ramps applied across audio blocks.

#### Scenario: Continuous parameter automation
- **WHEN** any continuous cutoff, drive, mix, or volume parameter is swept rapidly by host automation
- **THEN** parameter smoothers interpolate values across samples within each block with zero zipper noise or step discontinuities

### Requirement: State Serialization and Session Recall
The plugin SHALL serialize and restore its entire parameter, modulation, and active preset state to and from XML/JSON data chunks within host DAW project sessions.

#### Scenario: DAW project save and restore
- **WHEN** a DAW session containing the plugin is saved and reopened
- **THEN** all module parameters, bypass toggles, LFO/XY assignments, and mix balances restore to their exact saved values

### Requirement: Automated Offline Verification Harness
The project SHALL include a standalone CLI test target (`camelphat_dsp_tests`) that quantitatively verifies zero-allocation processing, all 12 MM Filter topologies, 4-circuit distortion harmonic generation, oversampling dry/wet latency alignment, LFO & X/Y modulation routing, Dice randomizer constraints, and 10-preset recall.

#### Scenario: Automated CI/CLI test execution
- **WHEN** `camelphat_dsp_tests` is executed from the command line
- **THEN** all automated test suites pass with zero heap allocations on the audio thread
