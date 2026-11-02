# WvZz Modular Audio Plugin Bundle (JUCE 8 / C++20)

> A modern, modular, high-performance audio plugin suite built on SOLID architectural principles. Powered by zero-allocation DSP modules, resolution-independent vector UI, and a unified real-time routing pipeline.

The repository is built as an extensible **bundle of audio plugins**, sharing a common foundation of high-performance DSP and UI modules (`WvZzModules`), with individual plugin targets housed under the `Plugins/` directory.

---

## Plugin Suite Overview

| Plugin Target | Formats | Status | Description | Documentation |
| :--- | :--- | :--- | :--- | :--- |
| **WvZz Distortion** | VST3, Standalone | **Available (Flagship)** | Multi-circuit saturation, Andy Simper ZDF bandpass/reject splitter, 12-mode MM filter, flanger, sub-bass Magic EQ, and one-knob dynamics. | [Plugin Overview](Plugins/WvZzDistortion/README.md) • [User Manual](Plugins/WvZzDistortion/USER_MANUAL.md) |
| *WvZz Delay* | VST3, Standalone | *Planned* | Modular fractional delay lines, analog tape flutter, diffusion networks, and dynamic ducking. | Coming soon |
| *WvZz Reverb* | VST3, Standalone | *Planned* | Feedback delay network (FDN) reverberator with non-linear decay damping and early reflection spatialization. | Coming soon |
| *WvZz Modulation* | VST3, Standalone | *Planned* | Multi-voice dimensional chorus, quadrature flanging, through-zero phaser, and rotary speaker emulator. | Coming soon |
| *WvZz Dynamics* | VST3, Standalone | *Planned* | Multi-band compressor, transient designer, and master-grade brickwall limiter. | Coming soon |

---

## Architecture & SOLID Principles

The project decouples core audio and UI functionality into the reusable static library **`WvZzModules`** (`Source/`), shared by all plugins in the `Plugins/` tree:

- **Single Responsibility (SRP):** Headless DSP engines (`IDSPModule`), parameter declarations/smoothing, and GUI presentations (`IModuleView`) are decoupled into isolated classes.
- **Open/Closed (OCP):** New audio modules and multi-instance module chains (e.g., dual distortion stages) plug into the audio engine without modifying host processor code or shared routing logic.
- **Liskov Substitution (LSP):** All audio modules implement `IPluginModule` and can be instantiated, bound to APVTS, and managed polymorphically.
- **Interface Segregation (ISP):** Distinct interfaces for real-time DSP (`IDSPModule`), visual telemetry (`IMeterSource`), and editor view creation (`IModuleView`).
- **Dependency Inversion (DIP):** Modules communicate through a pre-allocated `ProcessContext` passing audio buffers and control-rate modulation offsets. Zero direct coupling between audio processing stages.
- **Multi-Instance Capability:** Every module accepts a configurable `instancePrefix` (e.g. `slot1_dist_`, `slot2_dist_`) enabling stacking identical modules without parameter ID collisions.
- **Real-Time Safety:** Zero heap allocations, zero locks, and `noexcept` guarantees in all real-time audio callbacks across every plugin in the suite.

---

## Shared Core Modules (`Source/Modules/`)

The bundle shares a modular DSP toolbox:

1. **BPFilter (`Source/Modules/BPFilter/`):** Andy Simper Cytomic ZDF State Variable Filters with low/high cut linking, band-reject energy extraction into auxiliary buffers, and post-MMFilter recombination.
2. **Distortion (`Source/Modules/Distortion/`):** 4 parallel saturation circuits (Mech, Tube, Bitcrusher, Xcita) running through a 4x oversampling polyphase IIR pipeline with DC blocker.
3. **MMFilter (`Source/Modules/MMFilter/`):** 12 filter topologies (`LP 12/24dB`, `BP 12/24dB`, `HP 12/24dB`, `Notch`, `Peak`, `Comb +`, `Comb -`, `RingMod`, and `Vowel / Formant` morphing across `A-E-I-O-U`) with envelope follower sidechain modulation.
4. **Flanger (`Source/Modules/Flanger/`):** 3rd-order Lagrange fractional delay interpolation with 90° quadrature stereo LFO and feedback.
5. **MagicEQ (`Source/Modules/MagicEQ/`):** Resonant sub-bass peaking booster (60–120 Hz) with complementary anti-mud dip at $2 \times \text{tune}$ and Phat saturation mode.
6. **Compressor (`Source/Modules/Compressor/`):** One-knob dynamic leveling with auto-makeup gain, dual ballistics (Normal 3.5:1 vs Phat 12:1 smash), and gain reduction telemetry.
7. **Modulation (`Source/Modules/Modulation/`):** Dual LFOs with 7 waveforms (Free and Host Tempo Sync), Macro X/Y Pad, and 21-target control-rate modulation delivery.
8. **Master (`Source/Modules/Master/`):** Input gain stage, dry circular delay line latency compensation matching distortion oversampling, wet/dry blend, output volume trim (-36 to +12 dB), peak/RMS metering, and safety hard clipper.

---

## Quick Start & Build System

The project uses CMake 3.22+ and Ninja, targeting **macOS Universal Binary** (`arm64` Apple Silicon & `x86_64` Intel).

A [`Makefile`](Makefile) in the root repository provides common build shortcuts:

```bash
# 1. Build and install WvZz Distortion VST3 to ~/Library/Audio/Plug-Ins/VST3/
make install-vst3

# 2. Build Standalone desktop application:
make standalone

# 3. Run the zero-allocation DSP quantitative test harness:
make test

# 4. View all available make targets:
make help
```

---

## Documentation Links

- **WvZz Distortion Docs**:
  - 📖 **[Plugin Technical Description & Architecture](Plugins/WvZzDistortion/README.md)**: Deep dive into the 4-circuit distortion engine, oversampling, and band-splitting.
  - 🎛️ **[User Manual](Plugins/WvZzDistortion/USER_MANUAL.md)**: Step-by-step producer guide, parameter descriptions, factory presets, and DAW workflows.
- **Bundle & Developer Docs**:
  - 🛠️ **[Developer & Build Guide (MANUAL.md)](MANUAL.md)**: Detailed compilation instructions, code architecture, and CI testing details.

---

## License

Created under the WvZz brand for music production, audio engineering, and creative sound design.
