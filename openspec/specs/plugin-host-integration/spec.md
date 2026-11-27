# plugin-host-integration Specification

## Purpose
Defines the plugin host lifecycle integration, APVTS parameter architecture, zero-allocation real-time audio thread constraints, and automated verification harness.

## Requirements

### Requirement: Zero-Allocation Audio Processing
The plugin SHALL execute the entire `processBlock()` real-time audio callback without triggering any dynamic memory heap allocations, deallocations, or blocking system calls.

#### Scenario: Real-time audio rendering under load
- **WHEN** audio buffers of varying block sizes (32 to 1024 samples) are processed
- **THEN** zero heap allocations occur and all working buffers remain strictly pre-allocated

### Requirement: APVTS Parameter Architecture and Smoothing
The plugin SHALL aggregate its parameter set (`bp_*`, `dist_*`, `mm_*`, `flg_*`, `meq_*`, `comp_*`, `master_*`, `lfo1_*`, `lfo2_*`, `xy_*`, and `input_gain`) through modular delegation where each `IPluginModule` contributes its parameters to the host `AudioProcessorValueTreeState`, caches its atomic raw pointers, and internally drives 10ms–20ms linear or exponential smoothing ramps across audio processing blocks.

#### Scenario: Continuous parameter automation
- **WHEN** any continuous cutoff, drive, mix, or volume parameter is swept rapidly by host automation
- **THEN** parameter smoothers interpolate values across samples within each block with zero zipper noise or step discontinuities

### Requirement: Plugin Host Formats and macOS Validation
The plugin suite SHALL build all flagship targets as Universal Binaries (`arm64` and `x86_64`) targeting macOS VST3 and Standalone application formats under the WvZz brand (`COMPANY_NAME "WvZz"`, `PLUGIN_MANUFACTURER_CODE WvZz`), producing:
- `WvZzDistortion` (`PLUGIN_CODE WzDs`, `PRODUCT_NAME "WvZz Distortion"`)
- `WvZzDynamics` (`PLUGIN_CODE WzDy`, `PRODUCT_NAME "WvZz Dynamics"`, with stereo main input/output and optional stereo external sidechain input bus)

#### Scenario: Host validation scan
- **WHEN** the VST3 bundles (`WvZz Distortion.vst3` and `WvZz Dynamics.vst3`) are scanned by macOS DAW hosts or the Standalone applications are launched
- **THEN** both plugins load successfully, identify with manufacturer code `WvZz` and their respective plugin codes (`WzDs` and `WzDy`), and process audio without validation errors

### Requirement: State Serialization and Session Recall
The plugin SHALL serialize and restore its entire parameter, modulation, and active preset state to and from XML/JSON data chunks within host DAW project sessions.

#### Scenario: DAW project save and restore
- **WHEN** a DAW session containing the plugin is saved and reopened
- **THEN** all module parameters, bypass toggles, LFO/XY assignments, and mix balances restore to their exact saved values

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

### Requirement: WvZz Modulation Plugin Identity
The build SHALL produce WvZz Modulation as a Universal Binary (`arm64` and `x86_64`) VST3 and Standalone application with `PLUGIN_CODE WzMd`, `PRODUCT_NAME "WvZz Modulation"`, and manufacturer code `WvZz`.

#### Scenario: Packaging includes WvZz Modulation
- **WHEN** `make package` runs
- **THEN** `WvZz Modulation.vst3` and `WvZz Modulation.app` are found, contain `arm64` and `x86_64`, and pass `codesign --verify --deep --strict`

### Requirement: Tail Length Reporting
A plugin whose effects keep sounding after the input stops SHALL report a tail length to the host that covers its longest feedback decay. WvZz Modulation SHALL report `2.0` seconds.

#### Scenario: Host asks for the tail length
- **WHEN** the host calls the tail length query on WvZz Modulation
- **THEN** the plugin returns `2.0` seconds

### Requirement: WvZz Delay Plugin Identity
The build SHALL produce WvZz Delay as a Universal Binary (`arm64` and `x86_64`) VST3 and Standalone application with `PLUGIN_CODE WzDl`, `PRODUCT_NAME "WvZz Delay"`, and manufacturer code `WvZz`.

#### Scenario: Packaging includes WvZz Delay
- **WHEN** `make package` runs
- **THEN** `WvZz Delay.vst3` and `WvZz Delay.app` are found, contain `arm64` and `x86_64`, and pass `codesign --verify --deep --strict`

### Requirement: WvZz Delay Tail Length
WvZz Delay SHALL report a tail length computed from its current delay time and feedback: the time until the repeats fall by `60 dB`, at least one delay time and at most `60` seconds.

#### Scenario: Tail grows with feedback
- **WHEN** the delay time is `500 ms`
- **THEN** the reported tail with feedback `0.9` is longer than with feedback `0.2`, and with feedback `1.0` or more it is `60` seconds

### Requirement: WvZz Reverb Plugin Identity
The build SHALL produce WvZz Reverb as a Universal Binary (`arm64` and `x86_64`) VST3 and Standalone application with `PLUGIN_CODE WzRv`, `PRODUCT_NAME "WvZz Reverb"`, and manufacturer code `WvZz`.

#### Scenario: Packaging includes all five plugins
- **WHEN** `make package` runs
- **THEN** 5 VST3 bundles and 5 Standalone apps are found, including `WvZz Reverb.vst3` and `WvZz Reverb.app`, each with `arm64` and `x86_64` and a valid signature

### Requirement: WvZz Reverb Tail Length
WvZz Reverb SHALL report a tail length equal to its pre-delay plus its decay time plus the early-reflections length.

#### Scenario: Tail follows the decay setting
- **WHEN** `rev_decay` is `5 s` and `rev_predelay` is `100 ms`
- **THEN** the reported tail is between `5.1` and `5.3` seconds
