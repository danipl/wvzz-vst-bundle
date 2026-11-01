# CamelPhat User & Developer Manual

A practical guide to compiling, installing, and using CamelPhat in your DAW.

---

## Table of Contents
1. [System Requirements](#1-system-requirements)
2. [Compilation Guide](#2-compilation-guide)
3. [macOS Installation](#3-macos-installation)
4. [Using CamelPhat in Ableton Live & Other DAWs](#4-using-camelphat-in-ableton-live--other-daws)
5. [Signal Flow & Architecture](#5-signal-flow--architecture)
6. [Module Reference & Plain-English Guide](#6-module-reference--plain-english-guide)

---

## 1. System Requirements

- **Operating System:** macOS 11.0 (Big Sur) or newer.
- **Architectures:** Universal Binary (Apple Silicon `arm64` and Intel `x86_64`).
- **Plugin Formats:** VST3, Audio Unit (AU v2 `aumf`), and Standalone application.
- **Build Tools:**
  - Xcode Command Line Tools (`clang` supporting C++20).
  - CMake 3.22+.
  - Ninja build system.

---

## 2. Compilation & Fast Build Guide

### Quick Build with `make` (Recommended)
A root [`Makefile`](Makefile) provides simple commands:

| Command | Action |
| :--- | :--- |
| `make vst3` | Builds the `.vst3` bundle file into `build/CamelPhat_artefacts/VST3/CamelPhat.vst3` |
| `make install-vst3` | Builds and copies the `.vst3` bundle directly to `~/Library/Audio/Plug-Ins/VST3/` |
| `make install-system` | Copies `.vst3` bundle to `/Library/Audio/Plug-Ins/VST3/` (prompts for `sudo` if needed) |
| `make au` | Builds the Audio Unit (`.component`) bundle |
| `make install` | Builds both VST3 and AU and copies them to `~/Library/Audio/Plug-Ins/` |
| `make test` | Runs the quantitative DSP tests and allocation checks |
| `make validate-au` | Runs Apple's `auval` to test Audio Unit host integration |
| `make clean` | Cleans the build folder |

---

### Step-by-Step Manual CMake Build

#### Step 1: Install Build Tools
If not already installed on macOS via Homebrew:
```bash
brew install cmake ninja
```

#### Step 2: Configure the Project
Run CMake with the Ninja generator in the project root:
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```
*Note: CMake automatically downloads and configures JUCE 8.0.4 via FetchContent.*

#### Step 3: Build the `.vst3` Bundle
To build only the `.vst3` bundle:
```bash
cmake --build build --target CamelPhat_VST3
```
The compiled bundle will be located at:
`build/CamelPhat_artefacts/VST3/CamelPhat.vst3`

To build all formats (VST3, AU, Standalone App, and Test suite):
```bash
cmake --build build
```

#### Step 4: Run the Quantitative Test Suite
Verify that all DSP algorithms, filter stabilities, and zero-allocation assertions pass:
```bash
ctest --test-dir build --output-on-failure
```

---

## 3. macOS Installation Guide

On macOS, a `.vst3` plugin is a bundle directory containing the universal binary (`arm64` and `x86_64`) and manifest files.

### Option A: Install for Current User (`~/Library/Audio/Plug-Ins/VST3`)
No root privileges required. Most DAWs (like Ableton Live and Reaper) scan this folder automatically:
```bash
mkdir -p ~/Library/Audio/Plug-Ins/VST3
cp -R build/CamelPhat_artefacts/VST3/CamelPhat.vst3 ~/Library/Audio/Plug-Ins/VST3/
```
*(Or simply run `make install-vst3`)*.

### Option B: Install System-Wide (`/Library/Audio/Plug-Ins/VST3`)
Makes the plugin available to all user accounts on the machine (requires `sudo`):
```bash
sudo mkdir -p /Library/Audio/Plug-Ins/VST3
sudo cp -R build/CamelPhat_artefacts/VST3/CamelPhat.vst3 /Library/Audio/Plug-Ins/VST3/
```
*(Or simply run `make install-system`)*.

### Installing Audio Unit (AU)
To install the Audio Unit format as well:
```bash
mkdir -p ~/Library/Audio/Plug-Ins/Components
cp -R build/CamelPhat_artefacts/AU/CamelPhat.component ~/Library/Audio/Plug-Ins/Components/
killall -9 AudioComponentRegistrar 2>/dev/null || true
```
*(Or simply run `make install`)*.

#### Validate Audio Unit
Run Apple's `auval` utility to confirm validation:
```bash
auval -v aumf Cph3 CamA
```
*Expected output: `AU VALIDATION SUCCEEDED`.*

---

## 4. Using CamelPhat in Ableton Live & Other DAWs

### Ableton Live Setup
1. Launch **Ableton Live**.
2. Open **Settings / Preferences** (`Cmd + ,`) and switch to the **Plug-Ins** tab.
3. Verify that:
   - **Use Audio Units (v2)** is **On**.
   - **Use VST3 Plug-In System Folders** is **On**.
4. Click **Rescan Plug-Ins** (hold `Alt/Option` while clicking for a full deep scan).
5. In Ableton's left sidebar, navigate to:
   - **Plug-ins** > **Audio Units** > **CamelClone** > **CamelPhat**, or
   - **Plug-ins** > **VST3** > **CamelClone** > **CamelPhat**.
6. Drag and drop **CamelPhat** onto any Audio track, Drum rack return, or Master channel.

---

## 5. Signal Flow & Architecture

CamelPhat operates with internal 4x minimum-phase polyphase IIR oversampling (zero latency) to ensure distortion and saturation stay free of digital aliasing.

```
Input Audio 
  │
  ├───────────────────────────────────────────────────────┐ [Global Dry Mix]
  │                                                       │
  ▼                                                       │
[BP Filter / Crossover Splitter]                          │
  ├───────────────► Passband Audio (Center)               │
  │                   │                                   │
  │                   ▼                                   │
  │             [Distortion Stage] (Tube, Mech, Bit, Xcita)
  │                   │                                   │
  │                   ▼                                   │
  │             [Multi-Mode Filter] (LP12, LP24, BP, HP + Env Follower)
  │                   │                                   │
  │                   ▼                                   │
  │             [Stereo Flanger]                          │
  │                   │                                   │
  │                   ▼                                   │
  │             [Magic EQ Peaking Sub Boost]              │
  │                   │                                   │
  │                   ▼                                   │
  │             [Compressor & Phat Mode]                  │
  │                   │                                   │
  └───────────────► Band-Reject Clean (Sub & Air)         │
                      │                                   │
                      ▼                                   │
           [Summing & Output Gain] ◄──────────────────────┘
                      │
                      ▼
               [Stereo VU Meter]
```

---

## 6. Module Reference & Plain-English Guide

### In-Plugin Help System
Click the circular **`?`** button in the header of any module to open an instant explanation bubble. It describes the module's audio role, signal placement, and parameter functions. Click anywhere outside or click **`?`** again to close it.

---

### Module 1: Band-Pass Filter & Crossover Splitter
- **Low Cut (20 Hz - 20 kHz):** Sets the bottom frequency that enters the distortion stage. Cuts sub-bass out of the drive loop to prevent low-end mud.
- **High Cut (20 Hz - 20 kHz):** Sets the top frequency that enters the distortion stage. Prevents harsh high sizzle.
- **Res (0.1 - 10.0):** Adds analog resonance at the cutoff points.
- **BP Mix (0 - 100%):** Controls how much filtered passband audio continues down the effects chain.
- **BR Mix (0 - 100%):** Injects the clean, un-distorted sub-bass and air frequencies directly back into the output mix.

---

### Module 2: Distortion & Bit Crusher
- **Tube (0 - 100%):** Warm, asymmetric vacuum tube saturation. Generates musical second-order harmonics.
- **Mech (0 - 100%):** Aggressive, hard-clipping metallic overdrive. Adds high-order odd harmonics.
- **Bit (0 - 100%):** Digital bit-depth reducer (16-bit down to 2-bit). Produces vintage sampler grit.
- **Xcita (0 - 100%):** Dynamic psychoacoustic exciter that breathes high-end sparkle and brightness into dull tracks.
- **Mix (0 - 100%):** Wet/dry balance for the distortion section.
- **DRIVE X/Y Pad:** Drag the interactive puck to modulate **Tube** (horizontal X-axis) and **Mech** (vertical Y-axis) simultaneously.

---

### Module 3: Multi-Mode Filter & Envelope Follower
- **Mode Dropdown:** Choose between `Low-Pass 12dB`, `Low-Pass 24dB`, `Band-Pass`, or `High-Pass`.
- **Cutoff (20 Hz - 20 kHz):** The cutoff threshold frequency of the filter.
- **Res (0 - 100%):** Resonance boost near the cutoff point.
- **Env Amt (-100% to +100%):** How much the incoming audio volume sweeps the filter cutoff.
- **Attack (0.1 ms - 500 ms) & Release (1 ms - 2000 ms):** Ballistics for the dynamic envelope follower.
- **FILTER X/Y Pad:** Drag the interactive puck to sweep **Cutoff** (horizontal X-axis) and **Resonance** (vertical Y-axis) in real time.

---

### Module 4: Stereo Flanger
- **Rate (0.01 Hz - 10 Hz):** Speed of the low-frequency modulation oscillator.
- **Depth (0 - 100%):** Pitch modulation width of the comb-filtering delay line.
- **Feedb (-95% to +95%):** Bipolar feedback. Positive feedback produces rich jet-plane sweeps; negative feedback produces metallic, hollow textures.
- **Mix (0 - 100%):** Wet/dry blend of the flanger effect.

---

### Module 5: Magic EQ
- **Freq (30 Hz - 120 Hz):** Focuses on the fundamental frequency of your kick drum or sub bass.
- **Amount (0 - 100%):** Boosts the selected low-frequency pocket with a specialized peaking curve designed to deliver punch without causing speaker-flapping mud.

---

### Module 6: Compressor & Phat Mode
- **Thresh (-60 dB to 0 dB):** Volume level where compression starts acting on the signal.
- **Ratio (1:1 to 20:1):** Compression intensity above the threshold.
- **Attack (0.1 ms - 100 ms) & Release (10 ms - 1000 ms):** Dynamics reaction timing.
- **PHAT MODE Toggle:** Engages an analog optical-circuit emulator that adds warm analog glue and round, punchy transient response.
- **Mix (0 - 100%):** Enables built-in parallel (New York) compression.

---

### Module 7: Master Section & VU Meters
- **Dry/Wet (0 - 100%):** Master parallel blend between unprocessed original audio and the entire CamelPhat processing engine.
- **Out Gain (-24 dB to +24 dB):** Clean output level trim to match input gain staging.
- **Stereo VU Meters:** High-precision, 60 FPS animated meters displaying both solid RMS signal volume and peak hold lines.
  - *Green:* Safe operating level (`-60 dB` to `-12 dB`).
  - *Amber:* Nominal loudness (`-12 dB` to `-3 dB`).
  - *Red:* Approaching digital full scale clipping (`0 dB`).
