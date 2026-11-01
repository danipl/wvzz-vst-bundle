# CamelPhat (JUCE 8 / C++20)

> A high-performance, zero-allocation multi-effect sound-shaping plugin recreating the legendary Camel Audio CamelPhat 3 for modern 64-bit DAWs.

Available for **macOS** (Universal Binary `arm64` & `x86_64`) as **VST3**, **Audio Unit (AU)**, and **Standalone** application.

---

## Highlights

- **Zero-Allocation Audio Thread:** Strictly 0 heap allocations (`malloc`/`new`) inside `processBlock()`, verified under unit tests across 1,000 continuous audio blocks.
- **4x Oversampling Engine:** Polyphase minimum-phase IIR oversampling running at zero latency to eliminate distortion aliasing while keeping phase alignment with the dry signal.
- **7 Core DSP Stages:**
  - **Band-Pass Crossover & Splitter:** Isolates drive frequencies while preserving untouched clean low-end and high-end air.
  - **Distortion Suite:** Tube (warm saturation), Mech (hard clip), Bit Crusher, and Xcita (dynamic exciter).
  - **Multi-Mode Filter:** 12dB/24dB Low-Pass, Band-Pass, and High-Pass with envelope follower modulation.
  - **Stereo Flanger:** Analog comb-filtering modulation with bipolar feedback.
  - **Magic EQ:** Specialized low-end sub bass peaking enhancer.
  - **Compressor:** Flexible dynamics with analog optical **Phat Mode**.
  - **Master Section:** Stereo dry/wet blend, gain trim, and 60 FPS animated VU meters.
- **Modern Vector UI & LookAndFeel:** High-DPI dark cyberpunk aesthetic with neon arc dials and dual interactive 2D X/Y vector pads (`Filter X/Y` and `Drive X/Y`).
- **Contextual Help System:** In-plugin plain-language explanation cards accessible via **`?`** buttons on every module.
- **SOLID Architecture:** Modular, interface-segregated C++20 code adhering strictly to SOLID design principles (`IDSPModule`, `IMeterSource`, `IHelpContentProvider`, `IHelpOverlayHost`).

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

### Manual CMake Commands
If you prefer running CMake directly:
```bash
# Configure with CMake and Ninja
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build the .vst3 bundle directly:
cmake --build build --target CamelPhat_VST3

# Build all formats (VST3, AU, App, Tests):
cmake --build build

# Copy .vst3 bundle to user audio plug-ins folder:
mkdir -p ~/Library/Audio/Plug-Ins/VST3
cp -R build/CamelPhat_artefacts/VST3/CamelPhat.vst3 ~/Library/Audio/Plug-Ins/VST3/
```

---

## Project Structure

```
├── CMakeLists.txt              # CMake build definitions (JUCE 8.0.4 FetchContent)
├── MANUAL.md                   # Full compilation, installation, & user guide
├── Source/
│   ├── PluginProcessor.h/.cpp  # JUCE AudioProcessor & real-time audio pipeline
│   ├── PluginEditor.h/.cpp     # Custom AudioProcessorEditor & layout assembly
│   ├── Parameters.h            # APVTS parameter IDs & layout definitions
│   ├── DSP/                    # Decoupled zero-allocation DSP modules
│   │   ├── IDSPModule.h        # Module contracts & IMeterSource interface
│   │   ├── BPFilter.h          # Crossover band-pass / band-reject splitter
│   │   ├── DistortionModule.h  # Tube, Mech, Bit, and Exciter saturators
│   │   ├── MMFilter.h          # State Variable Filter + Envelope Follower
│   │   ├── FlangerModule.h     # Stereo delay line flanger
│   │   ├── MagicEQ.h           # Peaking sub-bass biquad filter
│   │   ├── CompressorModule.h  # VCA compressor with Phat optical mode
│   │   └── MasterSection.h     # Dry/wet mixer, gain, and atomic peak/RMS levels
│   └── UI/                     # Custom vector interface components
│       ├── CamelPhatLookAndFeel # Vector rotary dials, toggles, and combo boxes
│       ├── ModuleCardComponent # Delineated card containers
│       ├── XYPadComponent      # 2D coordinate crosshairs with puck dragging
│       ├── StereoVUMeterComponent # 60 FPS timer-driven stereo level meter
│       ├── IHelpSystem.h       # Contracts for help provider & overlay host
│       ├── HelpContentRepository # Plain-English explanation repository
│       ├── HelpButtonComponent # Interactive "?" vector button
│       └── HelpOverlayComponent # Floating contextual explanation card
└── Tests/
    └── DSPTests.cpp            # Headless quantitative test suite
```

---

## License

This project is created for educational and music production purposes.
All trademarks and product names belong to their respective owners.
