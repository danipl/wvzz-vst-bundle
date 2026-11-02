<div align="center">

<br/>

<img src="Assets/wvzz-logo.svg" alt="WvZz Audio Plugin Suite Logo" width="220" />

### ⚡ Where Precision DSP Engineering Meets Musical Expression 🎛️

**Open-source, modular VST3 & Standalone audio plugin bundle crafted in modern C++20 and JUCE 8.**  
*Zero-allocation real-time audio • Cytomic ZDF Analog Modeling • Resolution-independent Vector UI*

[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/20)
[![JUCE 8](https://img.shields.io/badge/JUCE-8.0-00e676?style=for-the-badge)](https://juce.com/)
[![Platform](https://img.shields.io/badge/macOS-Universal%20Binary%20(arm64%20%7C%20x86__64)-12161a?style=for-the-badge&logo=apple&logoColor=white)](#)
[![Formats](https://img.shields.io/badge/Formats-VST3%20%7C%20Standalone-00e5ff?style=for-the-badge)](#)
[![Real-Time Safe](https://img.shields.io/badge/Audio%20Thread-Zero%20Allocations-ff9100?style=for-the-badge)](#)

<p align="center">
  <a href="#-about-the-project">About</a> •
  <a href="#-the-wvzz-vst-plugin-bundle">Plugin Bundle</a> •
  <a href="#-shared-dsp-toolbox-wvzzmodules">Core DSP</a> •
  <a href="#-quick-start--installation">Quick Start</a> •
  <a href="#-documentation-hub">Docs</a> •
  <a href="#-contributing--community">Contribute</a>
</p>

---

</div>

## 🎵 About The Project

**WvZz** is an open-source audio engineering initiative where **sound design, musicality, and strict software architecture converge**. 

Modern music producers demand plugins that sound analog, respond dynamically without digital harshness, and run with rock-solid stability under heavy DAW loads. At the same time, audio developers need a clean, decoupled codebase where DSP algorithms can be composed, tested headlessly, and reused across multiple instruments and effects without spaghetti code.

**This repository is not a single monolithic plugin—it is a modular ecosystem (`WvZzModules`) and a growing bundle of studio-grade VST3 and Standalone audio processors.**

### Why WvZz Stands Out:
- **🎧 Compromiseless Sound Quality:** Zero-Delay Feedback (ZDF) state-variable filters, 4x polyphase IIR oversampling, 3rd-order Lagrange fractional delay lines, and sample-accurate circular latency compensation.
- **⚡ Real-Time Audio Guarantee:** Every `processBlock()` execution is strictly `noexcept`, lock-free, and verified by an automated test harness for **zero heap memory allocations** on the audio thread.
- **🎨 High-DPI Vector Interface:** Built purely with hardware-accelerated vector paths (`juce::Path`), scaling razor-sharp from `75%` to `200%` with 60 FPS telemetry, live oscilloscopes, and interactive `?` component help overlays.
- **🧱 SOLID Modular Architecture:** Any DSP module (`IDSPModule`) and its corresponding UI card (`IModuleView`) can be instantiated multiple times with custom parameter prefixes (`slot1_dist_`, `slot2_dist_`) to assemble brand-new plugins in minutes.

---

## 🎛️ The WvZz VST Plugin Bundle

All plugins in the WvZz suite are built as native **macOS Universal Binaries** (`arm64` Apple Silicon & `x86_64` Intel) in **VST3** and **Standalone** formats.

### 1. 🔥 WvZz Distortion *(Available Now — Flagship)*

> **Multi-Stage Frequency-Selective Saturation, Filtering, Modulation & Dynamics Processor**  
> Inspired by classic hardware and legendary multi-effects like *CamelPhat 3*, re-engineered from the ground up for modern bass music, drum bus sculpting, and aggressive sound design.

| Key Capabilities | Highlights | Documentation |
| :--- | :--- | :--- |
| **Band-Split Saturation** | Isolate distortion to specific midrange bands via the Cytomic ZDF **BP Filter** while keeping pristine sub-bass intact in the auxiliary **Band-Reject (`BR Mix`)** bus. | 📖 **[Plugin Overview & DSP Guide](Plugins/WvZzDistortion/README.md)** |
| **4 Parallel Shapers (4x OVS)** | Blend **Mechanical** (wavefolding), **Tube** (asymmetric even harmonics), **Bitcrusher** (16-to-2 bit), and **Xcita** (psychoacoustic air) inside a 4x oversampled pipeline. | 🎛️ **[Producer User Manual](Plugins/WvZzDistortion/USER_MANUAL.md)** |
| **12-Mode MM Filter + Env** | `LP/BP/HP (12/24dB)`, `Notch`, `Peak`, `Comb +/-`, `RingMod`, and `Vowel Formant (A-E-I-O-U)` driven by an internal sidechain envelope follower. | 🧪 **[10 Factory Presets Included](Plugins/WvZzDistortion/USER_MANUAL.md#5-factory-presets-guide)** |
| **Post-FX & Deep Modulation** | Quadrature stereo **Flanger**, sub-resonating **Magic EQ** with automatic anti-mud notch, **One-Knob Phat Compressor**, **Dual Tempo-Sync LFOs**, and a **2D Macro X/Y Pad** routing to 21 targets. | 🛠️ **[Source Code (`Plugins/WvZzDistortion`)](Plugins/WvZzDistortion)** |

<br/>

### 🗺️ Bundle Roadmap & Upcoming Plugins

Because every DSP block lives in our shared `WvZzModules` library, the bundle is actively expanding into a complete production suite:

| Plugin Target | Status | Focus & Sonic Character | Core Architecture |
| :--- | :---: | :--- | :--- |
| **🔥 WvZz Distortion** | ✅ **Released** | Multi-circuit saturation, ZDF band-splitting, 12-mode filtering & Phat compression | `BPFilter` + `Distortion` + `MMFilter` + `Flanger` + `MagicEQ` + `Compressor` |
| **⏱️ WvZz Delay** | 🚧 *Planned* | Tape, analog BBD & granular stereo delay with ducking and filter diffusion | Fractional Lagrange delay lines, ZDF feedback filters, LFO/Env matrix |
| **🌌 WvZz Reverb** | 🚧 *Planned* | Algorithmic plate, lush hall & non-linear shimmer spaces | Zero-allocation Feedback Delay Networks (FDN) & early reflection diffusers |
| **🌊 WvZz Modulation** | 🚧 *Planned* | Dimensional multi-voice chorus, through-zero phaser, flanger & harmonic tremolo | Multi-phase quadrature oscillators & all-pass ZDF phaser cascades |
| **🎚️ WvZz Dynamics** | 🚧 *Planned* | 3-band crossover compressor, transient shaper & master brickwall limiter | Linkwitz-Riley ZDF crossovers & multi-instance `CompressorModule` chains |

---

## 🧩 Shared DSP Toolbox (`WvZzModules`)

At the heart of the repository is `libWvZzModules.a` (`Source/Modules/`), a self-contained library of reusable audio building blocks:

```
┌────────────────────────────────────────────────────────────────────────────┐
│                        Host Plugin (e.g. WvZz Distortion)                  │
│             Aggregates IPluginModule instances & APVTS Parameters          │
└──────┬──────────────┬──────────────┬──────────────┬──────────────┬─────────┘
       │              │              │              │              │
       ▼              ▼              ▼              ▼              ▼
 ┌───────────┐  ┌───────────┐  ┌───────────┐  ┌───────────┐  ┌───────────┐
 │ BPFilter  │  │Distortion │  │ MMFilter  │  │  Flanger  │  │  MagicEQ  │ ...
 │  Module   │  │  Module   │  │  Module   │  │  Module   │  │  Module   │
 └─────┬─────┘  └─────┬─────┘  └─────┬─────┘  └─────┬─────┘  └─────┬─────┘
       │              │              │              │              │
       └──────────────┴──────────────┼──────────────┴──────────────┘
                                     ▼
          ┌─────────────────────────────────────────────────────┐
          │          Pre-Allocated ProcessContext               │
          │ • Main Stereo Audio Bus                             │
          │ • Read-Only Pre-Split Sidechain Bus                 │
          │ • Auxiliary Band-Reject Bus                         │
          │ • Lock-Free 21-Target Modulation Offset Lookup      │
          └─────────────────────────────────────────────────────┘
```

1. **[`BPFilter`](Source/Modules/BPFilter/)**: Andy Simper Cytomic ZDF State Variable Filters with frequency linking (`LINK`), resonance control, and auxiliary band-reject energy extraction.
2. **[`Distortion`](Source/Modules/Distortion/)**: 4 parallel non-linear shapers running through a 4x polyphase IIR oversampler with DC offset blocking.
3. **[`MMFilter`](Source/Modules/MMFilter/)**: 12 filter topologies + dynamic envelope follower with configurable attack, release, and bipolar cutoff modulation.
4. **[`Flanger`](Source/Modules/Flanger/)**: 3rd-order Lagrange fractional delay line modulated by a 90° stereo quadrature LFO.
5. **[`MagicEQ`](Source/Modules/MagicEQ/)**: Tunable 60–120 Hz sub-bass resonator paired with a complementary anti-mud cut at $2 \times \text{tune}$ and optional harmonic drive.
6. **[`Compressor`](Source/Modules/Compressor/)**: Program-dependent dynamic leveler with automatic makeup gain, Normal (`3.5:1`) and Phat (`12:1`) ballistics, and atomic gain-reduction metering.
7. **[`Modulation`](Source/Modules/Modulation/)**: Dual 7-waveform LFOs (Free Hz or Host BPM Sync) + interactive X/Y Vector Pad + musical `Dice` smart parameter randomizer.
8. **[`Master`](Source/Modules/Master/)**: Input trim, 24-sample circular delay dry-path latency compensation, dry/wet blender, output gain, safety `-0.1 dBFS` hard clipper, and 60 FPS Peak/RMS stereo VU telemetry.

---

## 🚀 Quick Start & Installation

### Prerequisites
- **macOS** 11.0 (Big Sur) or newer (Apple Silicon `arm64` or Intel `x86_64`).
- **Xcode Command Line Tools** (`xcode-select --install`), **CMake 3.22+**, and **Ninja** (`brew install cmake ninja`).

### One-Command Build & Install
We provide a root [`Makefile`](Makefile) that handles CMake configuration, Universal Binary compilation, ad-hoc code signing, and VST3 installation:

```bash
# 1. Clone the repository
git clone https://github.com/danipl/wvzz-vst-bundle.git
cd wvzz-vst-bundle

# 2. Build & install WvZz Distortion.vst3 to ~/Library/Audio/Plug-Ins/VST3/
make install-vst3

# 3. (Optional) Build the Standalone desktop application
make standalone

# 4. Run the automated zero-allocation DSP & acoustic test suite
make test
```

<details>
<summary><b>📋 View All Available Makefile Commands</b></summary>

| Command | Description |
| :--- | :--- |
| `make build` | Compiles all libraries, plugins (`VST3` & `Standalone`), and test targets |
| `make vst3` | Builds only `WvZz Distortion.vst3` in `build/WvZzDistortion_artefacts/Release/VST3/` |
| `make standalone` | Builds `WvZz Distortion.app` in `build/WvZzDistortion_artefacts/Release/Standalone/` |
| `make install-vst3` | Builds, signs, and installs `.vst3` into `~/Library/Audio/Plug-Ins/VST3/` |
| `make install-system`| Installs `.vst3` system-wide into `/Library/Audio/Plug-Ins/VST3/` (requires `sudo`) |
| `make test` | Builds and executes `wvzz_dsp_tests` via CTest |
| `make status` | Checks which WvZz plugins are currently installed on your Mac |
| `make clean` | Removes the `build/` directory |

</details>

---

## 📚 Documentation Hub

Whether you are a music producer looking to sculpt your mix or an audio programmer exploring the C++ architecture, start here:

| Document | Audience | What's Inside |
| :--- | :--- | :--- |
| **[WvZz Distortion — Plugin Description](Plugins/WvZzDistortion/README.md)** | Producers & Engineers | Deep dive into **what** the plugin does, **why** it avoids mud/aliasing, and **how** its DSP signal flow operates. |
| **[WvZz Distortion — User Manual](Plugins/WvZzDistortion/USER_MANUAL.md)** | Producers & Musicians | 60-second quick start, UI map, complete parameter reference tables, 12 filter modes, 10 factory presets, and DAW gain-staging tips. |
| **[Bundle Developer & Build Manual](MANUAL.md)** | Audio Developers | System requirements, CMake/Ninja build internals, SOLID interface contracts (`IPluginModule`), and test harness details. |

---

## 🤝 Contributing & Community

**We warmly welcome musicians, sound designers, DSP engineers, and C++ developers to collaborate on the WvZz Plugin Bundle!** 🎶✨

Whether you want to design factory presets, propose a new filter topology, build the next plugin in the bundle, or refine the vector UI, your contributions have a home here.

### Ways You Can Contribute:
- **🎚️ Sound Designers & Producers:** Create and share new presets, test plugins across different DAWs (Ableton, Logic, Bitwig, Reaper, FL Studio), or suggest workflow improvements.
- **🧮 DSP Engineers:** Contribute new `IPluginModule` processors (delays, reverbs, phasers, wavefolders, spectral processors) or optimize existing math routines.
- **🎨 UI/UX Developers:** Enhance our custom `juce::LookAndFeel`, design new real-time visualizers, or expand contextual `?` help tooltips.
- **🐛 Bug Reports & QA:** Report audio glitches, edge-case host behaviors, or platform build issues via GitHub Issues.

### Engineering Standards for Pull Requests:
To keep the bundle studio-ready and reliable for everyone, please ensure your code adheres to our core principles:
1. **Real-Time Audio Safety First:** Never allocate heap memory (`new`, `malloc`, `std::vector::push_back`), lock mutexes, or perform I/O inside `process()` or `processBlock()`.
2. **Respect SOLID Boundaries:** Keep headless DSP math (`IDSPModule`) strictly separated from GUI components (`IModuleView`) so all modules remain testable in CLI environments.
3. **Support Multi-Instance Prefixing:** Prefix all APVTS parameter IDs with `instancePrefix` so modules can be chained multiple times in future plugins.
4. **Verify with Tests:** Run `make test` before submitting your PR and add quantitative unit tests in [`Tests/DSPTests.cpp`](Tests/DSPTests.cpp) for new DSP behaviors.

### How to Get Started:
1. **Fork** the repository and create your feature branch (`git checkout -b feat/amazing-dsp-module`).
2. **Build & Test** locally using `make build && make test`.
3. **Commit** your changes with clear messages and open a **Pull Request**—we're excited to review and jam on your ideas!

---

## 📄 License

Crafted with passion for music production, audio engineering, and open education under the **WvZz** brand.

<div align="center">
  <br/>
  <i>Built with ❤️ for the audio community — Turn it up, mangle the waveform, and keep the sub clean.</i>
</div>
