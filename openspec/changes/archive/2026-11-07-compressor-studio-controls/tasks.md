# Tasks: Compressor Studio Controls and Visual Metering

## 1. Parameters & DSP Engine

- [x] 1.1 Register `comp_thresh`, `comp_ratio`, `comp_attack`, and `comp_mix` parameters in `CompressorDSP` and `Parameters.h`, retaining `comp_amount` compatibility
- [x] 1.2 Implement dynamic auto-makeup gain formulation and parallel mix dry/wet blending in `CompressorDSP.cpp`
- [x] 1.3 Add saturation ceiling detection telemetry to `CompressorDSP` to report active Phat soft-clipping to the UI

## 2. Visual Interface & Metering

- [x] 2.1 Refactor `CompressorView` to instantiate and layout 5 rotary controls (`Thresh`, `Ratio`, `Attack`, `Release`, `Mix`) with consistent typography and accent branding
- [x] 2.2 Upgrade `GRMeterComponent` in `CompressorView.cpp` with calibrated decibel ticks (`0` to `-24 dB`), smooth fallback ballistics, floating peak-hold line, dynamic numeric readout, and Phat active glow

## 3. Verification & Testing

- [x] 3.1 Update and expand `Tests/DSPTests.cpp` to verify independent threshold, ratio, attack, and parallel mix behavior
- [x] 3.2 Run `make test` to verify zero-allocation compliance across all tests
- [x] 3.3 Run `make build` and `make package` to verify Universal Binary (`arm64` + `x86_64`) compilation and packaging
- [x] 3.4 Run `openspec validate --all` to verify OpenSpec specification integrity across all changes
