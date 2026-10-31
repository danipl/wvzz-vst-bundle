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
The plugin SHALL expose all user and automation parameters through `AudioProcessorValueTreeState` with atomic raw pointer caching and sample-rate-aware parameter smoothing.

#### Scenario: Continuous parameter automation
- **WHEN** cutoff frequency or gain parameters are swept rapidly by host automation
- **THEN** values smooth smoothly across audio blocks with zero zipper noise or audible clicks

### Requirement: Plugin Host Formats and macOS Validation
The plugin SHALL build as a Universal Binary (`arm64` and `x86_64`) for VST3, AU, and Standalone formats, passing Apple Audio Unit validation (`auval`).

#### Scenario: Host validation scan
- **WHEN** the plugin is scanned by Apple `auval` or loaded in macOS DAW hosts
- **THEN** the plugin passes validation tests and instantiates with valid 4-character codes

### Requirement: State Serialization and Session Recall
The plugin SHALL serialize and restore its entire parameter configuration to and from XML/ValueTree data chunks within host DAW project sessions.

#### Scenario: DAW project save and restore
- **WHEN** a DAW session containing the plugin is saved and reopened
- **THEN** all module parameters, toggles, and mix balances restore to their exact saved values

### Requirement: Automated Offline Verification Harness
The project SHALL include a standalone CLI test target (`camelphat_dsp_tests`) that quantitatively verifies DSP stability, zero-allocation constraints, harmonic distortion, and parameter smoothing.

#### Scenario: Automated CI/CLI test execution
- **WHEN** the test executable is run from the command line
- **THEN** all automated test suites pass without requiring a GUI or external audio host
