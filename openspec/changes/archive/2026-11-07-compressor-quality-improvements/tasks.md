# Tasks: Compressor Quality Improvements

## 1. Core DSP Enhancements

- [x] 1.1 Implement decibel-domain dual-stage optical ballistics in `CompressorDSP` (`CompressorDSP.h` / `CompressorDSP.cpp`), verifying smooth decay behavior with zero audio-thread allocations
- [x] 1.2 Implement 1st-order Anti-Derivative Anti-Aliasing (ADAA) for Phat Mode `tanh` saturation with asymptotic overflow guard and midpoint fallback, verifying ceiling clamping at `-0.1 dBFS`
- [x] 1.3 Move atomic gain reduction telemetry store from per-sample loop to block-rate peak store, and add sub-block parameter smoothing to prevent zipper noise

## 2. Verification & Testing

- [x] 2.1 Update and expand `Tests/DSPTests.cpp` to assert dual-stage optical decay timing, telemetry responsiveness, and ADAA saturation ceiling bounds
- [x] 2.2 Run `make test` to verify zero-allocation compliance and verify that all test suites pass
- [x] 2.3 Run `make build` and `make package` to verify Universal Binary (`arm64` + `x86_64`) compilation and codesigning
- [x] 2.4 Run `openspec validate --all` to verify OpenSpec specification integrity across all changes
