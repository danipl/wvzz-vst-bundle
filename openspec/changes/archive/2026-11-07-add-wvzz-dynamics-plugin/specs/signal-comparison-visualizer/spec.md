## MODIFIED Requirements

### Requirement: Visualizer Mode Switching and Controls
The system SHALL provide interactive controls to switch between Oscilloscope (default), Spectrum Analyzer, Delta, and Gain Reduction (`GR`) views, along with vertical gain scaling, display freeze functionality, and a live Crest Factor (Peak-to-RMS dB) telemetry badge.

#### Scenario: Mode selection and freeze toggle
- **WHEN** user clicks a mode button (`SCOPE`, `SPECTRUM`, `DELTA`, or `GR`) or the freeze toggle
- **THEN** the display immediately transitions to the selected analysis mode, and toggling freeze pauses live buffer updating to allow static waveform and gain reduction inspection

#### Scenario: Live Gain Reduction trace and Crest Factor readout
- **WHEN** `GR` mode is selected during active dynamics processing
- **THEN** the visualizer renders the output waveform alongside a scrolling calibrated Gain Reduction history trace (`0` to `-24 dB`) and displays the live output Crest Factor in dB
