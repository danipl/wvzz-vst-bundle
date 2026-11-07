# Proposal: Compressor Quality Improvements

## Why

The current Compressor module in WvZz uses a simplified 1-pole linear peak detector and unaveraged `tanh` clipping. While functional, the linear detector causes unnatural pumping and choking on complex transients, failing to deliver the authentic smooth dynamics expected of an optical-style compressor. Furthermore, raw `tanh` clipping in Phat Mode generates inharmonic digital aliasing at standard sample rates (44.1 kHz / 48 kHz). Upgrading to decibel-domain dual-stage optical ballistics and zero-latency Anti-Derivative Anti-Aliasing (ADAA) saturation elevates the sonic quality and musicality to professional studio standards without altering the existing parameter layout or introducing latency.

## What Changes

- **Dual-Stage Optical Release Ballistics**: Replace the single exponential release with two-stage program-dependent optical ballistics: a fast initial release (~50% of gain reduction over 40–70 ms) followed by a gentle, program-dependent memory tail.
- **Decibel-Domain Level Detection & Smoothing**: Move level evaluation and gain smoothing into the decibel domain to achieve constant dB/second release decay, matching human loudness perception.
- **1st-Order Anti-Derivative Anti-Aliasing (ADAA) in Phat Mode**: Replace raw `tanh` saturation with analytical 1st-order ADAA soft clipping using the antiderivative $F_1(x) = \ln(\cosh(x))$, suppressing aliasing foldback with zero group delay.
- **Lock-Free Block-Rate Meter Telemetry**: Relocate atomic gain reduction writes out of the per-sample inner loop, storing peak gain reduction once per block to prevent audio thread CPU cache contention.
- **Sub-Block Parameter Smoothing**: Eliminate whole-block parameter jumps (`skip(numSamples)`) by interpolating parameters across sub-blocks, eliminating zipper noise during automation or LFO modulation.

## Capabilities

### New Capabilities
None.

### Modified Capabilities
- `dsp-engine`: Refines `Requirement: Optical Compressor with Phat Mode` to specify decibel-domain level detection, dual-stage optical release ballistics, and anti-aliased (ADAA) Phat mode soft clipping.

## Impact

- **Affected Code**: `Source/Modules/Compressor/CompressorDSP.h`, `Source/Modules/Compressor/CompressorDSP.cpp`, and `Tests/DSPTests.cpp`.
- **APIs & Parameters**: Fully backward-compatible. No parameters are renamed, added, or removed. Presets and automation bindings remain identical.
- **Latency & Allocations**: Preserves zero latency and zero heap allocations on the audio thread.
