# Proposal: Compressor Studio Controls and Visual Metering

## Why

The current Compressor module only exposes a macro "Amount" knob that hard-couples threshold and auto-makeup gain, while attack and ratio are fixed to preset values. Professional producers require independent control over Threshold, Ratio, Attack, Release, and parallel Mix to shape transients, tame dynamic peaks, and apply parallel compression. In addition, the current Gain Reduction meter lacks calibrated decibel markings, smooth fallback ballistics, and a peak-hold indicator, making precise visual monitoring difficult. Exposing dedicated studio controls and calibrated meter ballistics elevates the module to full studio channel-strip standards while maintaining real-time safety and preset compatibility.

## What Changes

- **Dedicated Dynamic Parameters**:
  - **Threshold** (`comp_thresh`): Adjustable from `-48.0 dB` to `0.0 dB` (default `-18.0 dB`).
  - **Ratio** (`comp_ratio`): Adjustable from `1.5:1` to `20.0:1` (default `4.0:1`).
  - **Attack** (`comp_attack`): Adjustable from `0.1 ms` to `50.0 ms` (default `5.0 ms`).
  - **Release** (`comp_release`): Adjustable from `10.0 ms` to `1500.0 ms` (default `120.0 ms`).
  - **Mix** (`comp_mix`): Built-in parallel wet/dry compression blend from `0.0%` to `100.0%` (default `100.0%`).
- **Smart Auto-Makeup Gain**: Dynamically computes makeup gain based on threshold and ratio ($G_{\text{makeup}} \approx -\text{threshold} \times (1 - 1/\text{ratio}) \times 0.5$) to maintain consistent perceived volume during adjustment.
- **Backward Compatibility**: Keeps `comp_amount` registered in the parameter layout as an additive drive macro or fallback, ensuring legacy presets load without parameter displacement.
- **Enhanced Visual Metering in `CompressorView`**:
  - **Analog Ballistics & Peak Hold**: Fast peak descent with exponential fallback decay and a floating peak-hold line.
  - **Calibrated Decibel Scale Ticks**: Subtle horizontal calibration lines at `0`, `-3`, `-6`, `-12`, `-18`, and `-24 dB`.
  - **Dynamic Numeric Readout**: Displays the live gain reduction (e.g. `-4.2 dB`) beneath the meter.
  - **Active Saturation Glow**: Illuminates the `PHAT` button with an active warm glow when peaks trigger the ADAA soft-clip ceiling.

## Capabilities

### New Capabilities
None.

### Modified Capabilities
- `dsp-engine`: Updates `Requirement: Optical Compressor with Phat Mode` to specify independent Threshold, Ratio, Attack, Release, and parallel Mix parameters with auto-makeup calculation.
- `plugin-ui-system`: Updates `Requirement: Live LFO Waveform Visualizers and Gain Reduction Meter` to mandate calibrated decibel scale ticks, smooth ballistics with floating peak-hold line, and numeric gain reduction readouts in `CompressorView`.

## Impact

- **Affected Code**:
  - `Source/Modules/Compressor/CompressorDSP.h`, `CompressorDSP.cpp`
  - `Source/Modules/Compressor/CompressorView.h`, `CompressorView.cpp`
  - `Source/Parameters.h` (registration of `comp_thresh`, `comp_ratio`, `comp_attack`, `comp_mix`)
  - `Tests/DSPTests.cpp`
- **APIs & Host Integration**: Backward-compatible parameter additions; multi-instance prefixing preserved via `paramId()`.
- **Real-Time Safety**: Zero allocations and `noexcept` preserved across all new DSP controls.
