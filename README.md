# CamelPhat 3 (JUCE 8 / C++20)

> A high-performance, zero-allocation multi-effect sound-shaping plugin faithfully recreating the signal chain, DSP behavior, and modulation architecture of **Camel Audio CamelPhat 3** for modern 64-bit DAWs.

Available for **macOS** (Universal Binary `arm64` & `x86_64`) as **VST3**, **Audio Unit (AU v2)**, and **Standalone** application.

---

## Highlights

- **Zero-Allocation Audio Thread:** Strictly 0 heap allocations (`malloc`/`new`) inside `processBlock()`, verified under headless unit tests across continuous audio rendering blocks.
- **4x Oversampling Engine:** 4x polyphase IIR oversampling with latency compensation on the global dry signal path to eliminate digital aliasing while maintaining phase alignment.
- **7 Core DSP Stages:**
  - **BP Filter (Bandpass & Band-Reject Split):** Andy Simper Cytomic ZDF State Variable Filters with Low/High cut link and band-reject extraction.
  - **4-Stage Parallel Distortion:** Mech (1.2 kHz pre-emphasis with sine wavefolding), Tube (analog saturation with DC blocker), Bitcrusher (base sample rate decimation), and Xcita (2.8 kHz HPF with full-wave rectification).
  - **12-Mode Multi-Mode Filter:** 12 filter topologies (`LP 12/24dB`, `BP 12/24dB`, `HP 12/24dB`, `Notch`, `Peak`, `Comb +`, `Comb -`, `RingMod`, and `Vowel / Formant` morphing across `A-E-I-O-U`) with pre-BP envelope follower modulation.
  - **Stereo Flanger:** 3rd-order Lagrange fractional delay interpolation with 90° quadrature stereo LFO and fixed 0.55 feedback.
  - **Magic EQ:** Resonant sub-bass peaking booster (60-120 Hz) with complimentary anti-mud dip at 2 * tune, and Phat Mode with internal resonant saturation.
  - **One-Knob Compressor:** Auto-makeup gain and dual ballistics (Normal 3.5:1 leveling vs Phat 12:1 smash with -0.1 dBFS ceiling).
  - **Master Section:** Dry latency compensation, output volume trim (-36 to +12 dB), Dice smart randomizer, and 60 FPS animated VU meters.
- **Modulation System:**
  - **LFO 1 & LFO 2:** Control-rate modulation with 7 waveforms (Sine, Triangle, Saw Down, Saw Up, Square, S&H, Smooth Random), Free and Host Tempo Sync.
  - **21-Target Matrix:** Cross-modulation of LFO 2 Rate by LFO 1, BP Both Cuts, and individual module parameters.
  - **Macro X/Y Pad:** 2D interactive vector surface with assignable targets and depth controls.
- **10 Factory Presets:** Included built-in presets covering classic bass mangling, breakbeat smashing, and lo-fi textures.
- **Contextual Help System:** In-plugin plain-language explanation cards accessible via **`?`** buttons on every module.

---

## Documentation & User Guide

For detailed compilation instructions, DAW setup (Ableton Live, Logic Pro, Reaper), and an in-depth module-by-module breakdown, please refer to the complete manual:

👉 **[Read the Full User & Developer Manual (MANUAL.md)](MANUAL.md)**

---

## Quick Start (Using `make`)

A [`Makefile`](Makefile) is provided in the repository root for one-command builds and installation:

```bash
# 1. Build the .vst3 bundle and install to ~/Library/Audio/Plug-Ins/VST3/
make install-vst3

# Or build and install both VST3 and AU bundles:
make install

# Or install VST3 system-wide to /Library/Audio/Plug-Ins/VST3/ (requires sudo):
make install-system

# Run tests:
make test
```

---

## License

This project is created for educational and music production purposes.
All trademarks and product names belong to their respective owners.
