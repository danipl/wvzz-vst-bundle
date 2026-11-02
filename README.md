# WvZz Modular Audio Plugin Bundle (JUCE 8 / C++20)

> A modern, modular, high-performance audio plugin suite built on SOLID architectural principles. Powered by zero-allocation DSP modules, resolution-independent vector UI, and a unified real-time routing pipeline.

Flagship release: **WvZz Distortion** for **macOS** (Universal Binary `arm64` & `x86_64`) as **VST3** and **Standalone** application.

---

## Architecture & SOLID Principles

The repository is organized into a reusable static library `WvZzModules` and standalone plugin targets (`Plugins/WvZzDistortion/`):

- **Single Responsibility (SRP):** Headless DSP engines (`IDSPModule`), parameter declarations/smoothing, and GUI presentations (`IModuleView`) are decoupled into isolated classes.
- **Open/Closed (OCP):** New audio modules and multi-instance module chains (e.g., dual distortion stages) can be plugged in without modifying host plugin code or shared routing logic.
- **Liskov Substitution (LSP):** All audio modules implement `IPluginModule` and can be instantiated, bound to APVTS, and managed polymorphically.
- **Interface Segregation (ISP):** Distinct interfaces for real-time DSP (`IDSPModule`), visual telemetry (`IMeterSource`), and editor view creation (`IModuleView`).
- **Dependency Inversion (DIP):** Modules communicate through a pre-allocated `ProcessContext` passing audio buffers and control-rate modulation offsets. Zero direct coupling between audio processing stages.
- **Multi-Instance Capability:** Every module accepts a configurable `instancePrefix` (e.g. `slot1_dist_`, `slot2_dist_`) enabling stacking identical modules without parameter ID collisions.

---

## Modules Overview

1. **BPFilter (`Source/Modules/BPFilter/`):** Andy Simper Cytomic ZDF State Variable Filters with low/high cut linking, band-reject energy extraction into auxiliary buffers, and post-MMFilter recombination.
2. **Distortion (`Source/Modules/Distortion/`):** 4 parallel saturation circuits (Mech, Tube, Bitcrusher, Xcita) running through a 4x oversampling polyphase IIR pipeline with DC blocker.
3. **MMFilter (`Source/Modules/MMFilter/`):** 12 filter topologies (`LP 12/24dB`, `BP 12/24dB`, `HP 12/24dB`, `Notch`, `Peak`, `Comb +`, `Comb -`, `RingMod`, and `Vowel / Formant` morphing across `A-E-I-O-U`) with envelope follower sidechain modulation.
4. **Flanger (`Source/Modules/Flanger/`):** 3rd-order Lagrange fractional delay interpolation with 90° quadrature stereo LFO and feedback.
5. **MagicEQ (`Source/Modules/MagicEQ/`):** Resonant sub-bass peaking booster (60–120 Hz) with complementary anti-mud dip at 2 * tune and Phat saturation mode.
6. **Compressor (`Source/Modules/Compressor/`):** One-knob dynamic leveling with auto-makeup gain, dual ballistics (Normal 3.5:1 vs Phat 12:1 smash), and gain reduction telemetry.
7. **Modulation (`Source/Modules/Modulation/`):** Dual LFOs with 7 waveforms (Free and Host Tempo Sync), Macro X/Y Pad, and 21-target control-rate modulation delivery.
8. **Master (`Source/Modules/Master/`):** Input gain stage, dry circular delay line latency compensation matching distortion oversampling, wet/dry blend, output volume trim (-36 to +12 dB), peak/RMS metering, and safety hard clipper.

---

## Quick Start (Using `make`)

A [`Makefile`](Makefile) is provided in the repository root for one-command builds and installation:

```bash
# 1. Build and install WvZz Distortion VST3 to ~/Library/Audio/Plug-Ins/VST3/
make install-vst3

# 2. Build Standalone desktop application:
make standalone

# 3. Run zero-allocation DSP test suite:
make test

# 4. View available targets:
make help
```

---

## User Manual & Documentation

For detailed architecture descriptions, module parameters, and DAW setup instructions, refer to:

👉 **[Read the Full User Manual (MANUAL.md)](MANUAL.md)**

---

## License

This project is created for music production and educational purposes under the WvZz brand.
