# Spec Delta: plugin-ui-system

## MODIFIED Requirements

### Requirement: Live LFO Waveform Visualizers and Gain Reduction Meter
The UI system SHALL render live waveform and phase visualizers in the LFO 1 and LFO 2 sub-panels, a vertical passband shading indicator between the BP Filter dual vertical cutoff sliders, glowing `PHAT` pill toggles for Magic EQ and Compressor Phat modes, a header `Dice` randomizer button, and a Compressor visualizer featuring a 5-knob layout (`Threshold`, `Ratio`, `Attack`, `Release`, `Mix`), calibrated decibel scale tick marks (`0`, `-3`, `-6`, `-12`, `-18`, `-24 dB`), analog meter ballistics with a floating peak-hold line, a dynamic numeric decibel readout, and visual saturation glow on Phat clipping.

#### Scenario: Visual feedback during modulation and compression
- **WHEN** audio is compressed and LFO 1/2 are running
- **THEN** the LFO sub-panels animate the active oscillator waveform and phase while the Compressor meter displays real-time attenuation in dB with calibrated scale marks, smooth ballistic decay, and a floating peak-hold indicator

#### Scenario: Passband visualizer slider tracking
- **WHEN** user moves either the Low Cut or High Cut vertical slider
- **THEN** the passband shaded rectangle updates smoothly matching the physical slider thumbs on the non-linear logarithmic scale without vertical inversion or offset error

#### Scenario: Calibrated compressor metering and numeric readout
- **WHEN** audio compression occurs in the Compressor module
- **THEN** the GR meter animates with rapid descent and smooth fallback across the calibrated `-24 dB` scale, and the numeric readout updates with the active gain reduction value
