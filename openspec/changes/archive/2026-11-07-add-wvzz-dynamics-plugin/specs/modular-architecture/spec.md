## MODIFIED Requirements

### Requirement: Unified ProcessContext Audio and Modulation Routing
The DSP processing pipeline SHALL pass a pre-allocated `ProcessContext` structure to every module's `process()` method. The context SHALL provide direct access to the main stereo audio buffer, configurable sidechain detector audio buffer (`sidechainInputBuffer`), auxiliary split buffer (`auxBandRejectBuffer`), a dynamic modulation offset lookup function (`getModulationOffset`) supporting both `Distortion` and `Dynamics` modulation target enums, live host transport state (`bpm`, `ppqPosition`, `isPlaying`), active oversampling latency (`distortionLatencySamples`), and cumulative gain reduction telemetry (`cumulativeGainReductionDb`) for real-time visualization.

#### Scenario: Auxiliary audio bus exchange
- **WHEN** a filter or crossover splitter module outputs isolated frequency bands into the auxiliary buffer and downstream stages introduce processing or latency
- **THEN** downstream summation stages delay and recombine the auxiliary buffer with the main audio path without thread locks or dynamic buffer reallocations

#### Scenario: Dynamic modulation target lookup
- **WHEN** modulation sources (LFOs or X/Y pads) evaluate synced rates and target offsets under either the Distortion or Dynamics target profile
- **THEN** the modulation module reads `bpm`, `ppqPosition`, and `isPlaying` from `ProcessContext` and populates modulation offsets for all downstream modules in real time
