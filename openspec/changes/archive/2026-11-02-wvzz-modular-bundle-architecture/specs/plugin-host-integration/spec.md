# Spec Delta: plugin-host-integration

## MODIFIED Requirements

### Requirement: APVTS Parameter Architecture and Smoothing
The plugin SHALL aggregate its parameter set (`bp_*`, `dist_*`, `mm_*`, `flg_*`, `meq_*`, `comp_*`, `master_*`, `lfo1_*`, `lfo2_*`, `xy_*`, and `input_gain`) through modular delegation where each `IPluginModule` contributes its parameters to the host `AudioProcessorValueTreeState`, caches its atomic raw pointers, and internally drives 10ms–20ms linear or exponential smoothing ramps across audio processing blocks.

#### Scenario: Continuous parameter automation
- **WHEN** any continuous cutoff, drive, mix, or volume parameter is swept rapidly by host automation
- **THEN** parameter smoothers interpolate values across samples within each block with zero zipper noise or step discontinuities

### Requirement: Plugin Host Formats and macOS Validation
The plugin SHALL build as a Universal Binary (`arm64` and `x86_64`) targeting macOS VST3 and Standalone application formats under the WvZz brand (`COMPANY_NAME "WvZz"`, `PLUGIN_MANUFACTURER_CODE WvZz`, `PLUGIN_CODE WzDs`, `PRODUCT_NAME "WvZz Distortion"`).

#### Scenario: Host validation scan
- **WHEN** the VST3 bundle is scanned by macOS DAW hosts or the Standalone application is launched
- **THEN** the plugin loads successfully, identifies as WvZz Distortion with manufacturer code WvZz and plugin code WzDs, and processes audio without validation errors

### Requirement: Automated Offline Verification Harness
The project SHALL include a standalone CLI test target (`wvzz_dsp_tests`) that quantitatively verifies zero-allocation processing, isolated module processing, multi-instance parameter prefixing, all 12 MM Filter topologies, 4-circuit distortion harmonic generation, oversampling dry/wet latency alignment, LFO & X/Y modulation routing, Dice randomizer constraints, and 10-preset recall.

#### Scenario: Automated CI/CLI test execution
- **WHEN** `wvzz_dsp_tests` is executed from the command line
- **THEN** all automated test suites pass with zero heap allocations on the audio thread
