## MODIFIED Requirements

### Requirement: Plugin Host Formats and macOS Validation
The plugin suite SHALL build all flagship targets as Universal Binaries (`arm64` and `x86_64`) targeting macOS VST3 and Standalone application formats under the WvZz brand (`COMPANY_NAME "WvZz"`, `PLUGIN_MANUFACTURER_CODE WvZz`), producing:
- `WvZzDistortion` (`PLUGIN_CODE WzDs`, `PRODUCT_NAME "WvZz Distortion"`)
- `WvZzDynamics` (`PLUGIN_CODE WzDy`, `PRODUCT_NAME "WvZz Dynamics"`, with stereo main input/output and optional stereo external sidechain input bus)

#### Scenario: Host validation scan
- **WHEN** the VST3 bundles (`WvZz Distortion.vst3` and `WvZz Dynamics.vst3`) are scanned by macOS DAW hosts or the Standalone applications are launched
- **THEN** both plugins load successfully, identify with manufacturer code `WvZz` and their respective plugin codes (`WzDs` and `WzDy`), and process audio without validation errors

### Requirement: Automated Offline Verification Harness
The project SHALL include a standalone CLI test target (`wvzz_dsp_tests`) that quantitatively verifies zero-allocation processing, isolated module processing, multi-instance parameter prefixing, all 12 MM Filter topologies, 4-circuit serial distortion and character modes (`Classic`, `Hard Clip`, `Wavefold`), oversampling dry/wet and Band-Reject latency alignment, Pre-Distortion Magic EQ routing, Sub-Bass Mono Maker and 0 dBFS Master Clipper, host BPM LFO sync and `Master Vol` modulation routing, Dice randomizer constraints, 15-preset recall for `WvZz Distortion`, and full DSP + 20-preset verification for `WvZz Dynamics` (`DynDetector`, `TransientShaper`, `MultibandOTT`, `Compressor` Opto/VCA/FET modes, `RhythmicPumper`, and `MaximizerClipper`).

#### Scenario: Automated CI/CLI test execution
- **WHEN** `wvzz_dsp_tests` is executed from the command line
- **THEN** all automated test suites for both `WvZz Distortion` and `WvZz Dynamics` pass with zero heap allocations on the audio thread

### Requirement: 10 Factory Presets and Program Management
Each plugin in the suite SHALL expose built-in factory presets accessible via standard host program APIs and XML/JSON preset serialization:
- **WvZz Distortion (15 presets)**: `Neuro Reese Mangler`, `Phat Breakbeat Smash`, `Sub Weight Enhancer`, `Acid Band-Reject Sweep`, `Lo-Fi Bit Crunch`, `Warm Tube Bus Glue`, `Stepped Comb Resonator`, `Talking Vowel Wobble`, `Metallic RingMod Perc`, `Wide Quadrature Flanger`, `Hardstyle Kick Tok & Crunch`, `Euphoric Supersaw Excite`, `Raw Screech Comb Screamer`, `Reverse Bass Pump`, and `Big Room Festival Kick`.
- **WvZz Dynamics (20 presets)**: `Euphoric Lead Wall`, `Tok & Tail Kick Glue`, `Reverse Bass Pumper`, `Screech OTT Maximizer`, `Rawstyle Kick Clipper`, `Mainstage Drop Bus`, `Big Room Kick Snap`, `Festival Sub Lock`, `Pluck Transient Pop`, `SuperSaw Hyper-OTT`, `Clap & Snare Smash`, `Drum Bus VCA Glue`, `Hi-Hat Air & Sparkle`, `Tight Gate Percussion`, `Euphoric Vocal Up-Lift`, `Piano & Pluck Leveler`, `Sidechain Pad Swell`, `Club Loudness Maximizer`, `Parallel Bus Slam`, and `Clean Mastering Glue`.

#### Scenario: Selecting a factory preset
- **WHEN** a user or host selects a valid program index in `WvZz Distortion` (`0`–`14`) or `WvZz Dynamics` (`0`–`19`)
- **THEN** all APVTS parameters update to the selected factory preset configuration and `getProgramName()` returns the preset's name
