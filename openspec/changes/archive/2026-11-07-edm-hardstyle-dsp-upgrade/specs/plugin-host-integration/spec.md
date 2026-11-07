## MODIFIED Requirements

### Requirement: Automated Offline Verification Harness
The project SHALL include a standalone CLI test target (`wvzz_dsp_tests`) that quantitatively verifies zero-allocation processing, isolated module processing, multi-instance parameter prefixing, all 12 MM Filter topologies, 4-circuit serial distortion and character modes (`Classic`, `Hard Clip`, `Wavefold`), oversampling dry/wet and Band-Reject latency alignment, Pre-Distortion Magic EQ routing, Sub-Bass Mono Maker and 0 dBFS Master Clipper, host BPM LFO sync and `Master Vol` modulation routing, Dice randomizer constraints, and 15-preset recall.

#### Scenario: Automated CI/CLI test execution
- **WHEN** `wvzz_dsp_tests` is executed from the command line
- **THEN** all automated test suites pass with zero heap allocations on the audio thread

### Requirement: 10 Factory Presets and Program Management
The plugin SHALL expose 15 built-in factory presets (`Neuro Reese Mangler`, `Phat Breakbeat Smash`, `Sub Weight Enhancer`, `Acid Band-Reject Sweep`, `Lo-Fi Bit Crunch`, `Warm Tube Bus Glue`, `Stepped Comb Resonator`, `Talking Vowel Wobble`, `Metallic RingMod Perc`, `Wide Quadrature Flanger`, `Hardstyle Kick Tok & Crunch`, `Euphoric Supersaw Excite`, `Raw Screech Comb Screamer`, `Reverse Bass Pump`, and `Big Room Festival Kick`) accessible via standard host program APIs and XML/JSON preset serialization.

#### Scenario: Selecting a factory preset
- **WHEN** a user or host selects program index `0` through `14`
- **THEN** all APVTS parameters update to the selected factory preset configuration and `getProgramName()` returns the preset's name
