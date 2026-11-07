## MODIFIED Requirements

### Requirement: Unified ProcessContext Audio and Modulation Routing
The DSP processing pipeline SHALL pass a pre-allocated `ProcessContext` structure to every module's `process()` method. The context SHALL provide direct access to the main stereo audio buffer, read-only pre-split sidechain input, auxiliary band-reject buffer, a dynamic modulation offset lookup function (`getModulationOffset`), live host transport state (`bpm`, `ppqPosition`, `isPlaying`), and active distortion oversampling latency (`distortionLatencySamples`).

#### Scenario: Auxiliary audio bus exchange
- **WHEN** a filter splitter module outputs rejected frequency bands into the auxiliary band-reject buffer and downstream oversampled distortion introduces group delay
- **THEN** downstream summation stages delay and recombine the auxiliary buffer with the main audio path using `distortionLatencySamples` without thread locks or dynamic buffer reallocations

#### Scenario: Dynamic modulation target lookup
- **WHEN** modulation sources (LFOs or X/Y pads) evaluate synced rates and target offsets
- **THEN** the modulation module reads `bpm`, `ppqPosition`, and `isPlaying` from `ProcessContext` and populates modulation offsets for all downstream modules in real time
