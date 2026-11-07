# WvZz Distortion

> **Flagship Multi-Stage Distortion & Coloration Processor of the WvZz Audio Plugin Bundle**  
> Formats: **VST3** & **Standalone** for macOS Universal Binary (`arm64` & `x86_64`). Built with JUCE 8 and C++20.

---

## 1. What is WvZz Distortion?

**WvZz Distortion** is a specialized color and dynamics processor designed for modern sound design, electronic music, mixing, and aggressive tonal sculpting. Inspired by classic multi-effect saturation units such as CamelPhat 3, WvZz Distortion reimagines the concept with clean modern DSP, a high-resolution vector interface, and strict real-time audio safety.

The plugin provides a curated chain of 8 processing blocks:
1. **BP Filter**: Andy Simper Cytomic Zero-Delay Feedback (ZDF) SVF band-splitter isolating the saturation target frequency range while keeping phase-coherent rejected frequencies in an auxiliary band-reject buffer.
2. **Distortion Unit**: Four serial-cascaded saturation circuits (Mechanical, Tube, Bitcrusher, Exciter) with selectable character modes (`Classic`, `Hard Clip`, `Wavefold`) running in a 4x oversampled polyphase IIR pipeline with anti-aliasing and DC blocking.
3. **Multi-Mode (MM) Filter**: 12 diverse filter topologies (from standard Butterworth-style low/high/band-pass to aggressive comb filters, ring modulation, and five-vowel formant morphing) driven by an internal envelope follower.
4. **Band-Reject Summing Node**: Cleanly recombines the filtered saturation path with the latency-compensated un-distorted auxiliary low/high bands according to the `BR Mix` control.
5. **Stereo Flanger**: Analog-modeled 3rd-order Lagrange fractional delay interpolation with a 90° quadrature stereo LFO for stereo widening and metallic resonances.
6. **Magic EQ**: Musical resonant booster (45–180 Hz) with an anti-mud corrective notch at $2 \times \text{tune}$, optional `PHAT` harmonic saturation, and switchable **Pre-Distortion** routing (`PRE`) for Hardstyle kick-tok sculpting.
7. **One-Knob Compressor**: Musical dynamic leveling with automated makeup gain, peak/RMS gain reduction metering, and switchable `Phat` extreme smash mode (12:1 ratio).
8. **Modulation Matrix & Master Section**: Dual beat-locked LFOs (Free & Host Tempo Sync with 4/4 grid phase lock), Macro X/Y Controller, 22 modulation targets (including `Master Vol`), Sub-Bass Mono Maker (`< 120 Hz`), 0 dBFS Output Clipper, click-free bypass crossfade, and 60 FPS peak/RMS stereo metering.

---

## 2. Why WvZz Distortion?

### The Problem
Traditional distortion and saturation plugins often suffer from three major design compromises:
1. **Full-Spectrum Mud and Aliasing**: Distorting a full-range signal introduces ugly intermodulation distortion (IMD) between low sub-bass and high frequencies, while non-linear saturation generates digital aliasing folding back into the audible spectrum.
2. **Loss of Low-End Punch**: Heavy distortion flattens transients and destroys low-end dynamics, causing kicks, 808s, and basslines to lose impact.
3. **Static, Lifeless Tones**: Distortions without integrated movement sound flat and synthetic across an arrangement.

### How WvZz Distortion Solves It
- **Frequency-Isolated Processing**: By splitting the audio via the Cytomic ZDF **BP Filter**, you distort *only* the specific frequency range you want to color (e.g., the midrange crunch of a bass), leaving sub-bass and ultra-highs clean in the auxiliary band-reject bus.
- **4x Polyphase Oversampling**: All non-linear saturation occurs at 4x the host sample rate using polyphase IIR half-band filters, pushing aliasing harmonics well beyond the Nyquist limit and filtering them out cleanly before downsampling.
- **Circular Delay Latency Compensation**: The master section delays the un-processed dry signal by the exact group delay of the oversampling filters (24 samples at 4x), guaranteeing sample-accurate phase alignment when blending dry and wet signals.
- **Deep Control-Rate Modulation**: An integrated dual-LFO engine and dual-axis X/Y controller allow simultaneous dynamic modulation of 21 plugin parameters without clicking, zipper noise, or CPU penalties.
- **Zero Audio Thread Allocation**: The entire DSP pipeline uses pre-allocated buffers and zero dynamic heap allocations, guaranteeing zero audio dropouts, glitches, or thread contention even under extreme DAW load.

---

## 3. How It Achieves It (Architecture & DSP)

```
                     ┌────────────────────────────────────────┐
                     │          Input Gain Stage              │
                     │          (-24 dB to +24 dB)            │
                     └───────────────────┬────────────────────┘
                                         │
                   ┌─────────────────────┴─────────────────────┐
                   │                                           │
                   ▼ (Main Signal)                             ▼ (Dry Path)
       ┌──────────────────────────┐             ┌─────────────────────────────┐
       │     1. BP Filter         │             │ Circular Buffer Delay Line  │
       │ (ZDF SVF Band Splitter)  │             │ (24 samples delay matching) │
       └───────────┬──────────────┘             └──────────────┬──────────────┘
                   │                                           │
         ┌─────────┴─────────────────────┐                     │
         ▼ (Passband Audio)              ▼ (Rejected Auxiliary)│
┌───────────────────────────┐            │                     │
│ 2. 4x Oversampled         │            │                     │
│    Distortion Unit        │            │                     │
│ (Mech/Tube/Bit/Xcita)     │            │                     │
└──────────┬────────────────┘            │                     │
           ▼                             │                     │
┌───────────────────────────┐            │                     │
│ 3. MM Filter              │            │                     │
│ (12 Topologies + Envelope)│            │                     │
└──────────┬────────────────┘            │                     │
           │                             │                     │
           ▼                             │                     │
     [Summing Node] ◄────────────────────┘                     │
     (Passband + Auxiliary * BR Mix)                           │
           │                                                   │
           ▼                                                   │
┌───────────────────────────┐                                  │
│ 4. Stereo Flanger         │                                  │
│ (Lagrange Fractional)     │                                  │
└──────────┬────────────────┘                                  │
           ▼                                                   │
┌───────────────────────────┐                                  │
│ 5. Magic EQ               │                                  │
│ (Sub Booster + Anti-Mud)  │                                  │
└──────────┬────────────────┘                                  │
           ▼                                                   │
┌───────────────────────────┐                                  │
│ 6. Compressor             │                                  │
│ (Auto-Makeup + GR Meter)  │                                  │
└──────────┬────────────────┘                                  │
           │                                                   │
           ▼ (Wet Signal)                                      │
     [Master Blender] ◄────────────────────────────────────────┘
     (Mix: 0% - 100%)
           │
           ▼
     [Output Trim Stage (-36 dB to +12 dB)]
           │
           ▼
     [Safety Hard Clipper (-0.1 dBFS ceiling)]
           │
           ▼
     [Stereo Output + Peak/RMS Telemetry]
```

### Module Breakdown

#### 1. BP Filter (Andy Simper Cytomic ZDF SVF)
- **Cutoffs**: Low Cut (20 Hz – 20 kHz) and High Cut (20 Hz – 20 kHz) with non-linear logarithmic scaling.
- **Link Function**: Locks the bandwidth between Low Cut and High Cut so you can sweep the passband across the spectrum with a single drag.
- **Resonance**: Dedicated Low Res and High Res resonance peaks (0.1 to 10.0 Q).
- **Auxiliary Band-Reject Bus**: Audio frequencies below Low Cut and above High Cut are extracted and piped into a separate stereo buffer.
- **BR Mix**: Smoothly mixes the rejected frequencies back into the signal post-MMFilter, preserving pristine sub-bass and top-end air.

#### 2. Distortion (4 Parallel Shapers)
- **Mechanical (`Mech`)**: Aggressive, industrial wavefolding and hard odd-harmonic clipping.
- **Tube (`Tube`)**: Warm, asymmetric even-harmonic vacuum tube emulation (`tanh(x + offset)`).
- **Bitcrusher (`Bit`)**: Dynamic wordlength reduction (16-bit down to 2-bit) with continuous quantization steps.
- **Exciter (`Xcita`)**: High-frequency psychoacoustic exciter generating subtle presence, air, and tape-like sheen.
- **Oversampling Engine**: 4x polyphase half-band IIR pipeline eliminates digital aliasing without high CPU load.
- **DC Blocker**: High-pass leaky integrator removes asymmetrical DC offset generated by heavy tube drive.

#### 3. Multi-Mode (MM) Filter
- **12 Topologies**:
  - `LP 12dB` / `LP 24dB`: Smooth resonant low-pass filters.
  - `BP 12dB` / `BP 24dB`: Focused band-pass filters.
  - `HP 12dB` / `HP 24dB`: Aggressive high-pass filters.
  - `Notch`: Narrow band elimination.
  - `Peak`: Parametric bell resonance.
  - `Comb +` / `Comb -`: Positive and negative feedback comb filtering for flanged, physical-string resonances.
  - `RingMod`: Balanced ring modulator for metallic, clangorous sidebands.
  - `Formant`: Formant vowel filter interpolating between human vocal formants `A - E - I - O - U`.
- **Envelope Follower**: Detects the incoming dynamic amplitude envelope and modulates the cutoff frequency with variable attack and release times.

#### 4. Flanger
- **Fractional Delay**: 3rd-order Lagrange interpolation prevents interpolation distortion.
- **Stereo Quadrature LFO**: Left and right delay modulation paths are offset by 90°, creating a wide stereo image.
- **Controls**: Delay time, depth, rate, feedback, and wet/dry mix.

#### 5. Magic EQ
- **Sub Booster**: Tunable low-frequency resonator focused on the sub-bass foundation (60 Hz to 120 Hz).
- **Anti-Mud Dip**: Automatically tracks and attenuates $2 \times \text{tune}$ to prevent buildup in the critical 120–250 Hz range.
- **Phat Mode**: Adds warm saturation to the boosted low end for bass that cuts through small speakers.

#### 6. Compressor
- **One-Knob Operation**: Increasing compression lowers threshold while applying intelligent automatic makeup gain.
- **Dual Modes**:
  - `Normal`: 3.5:1 ratio, medium attack, auto-release for transparent leveling.
  - `Phat`: 12:1 ratio, fast attack, punchy release for crushing drums and aggressive transient shaping.
- **Telemetry**: Real-time gain reduction dB readout fed to the UI.

#### 7. Modulation Engine & X/Y Controller
- **Dual LFOs (LFO 1 & LFO 2)**:
  - 7 Waveforms: Sine, Triangle, Saw Up, Saw Down, Square, Sample & Hold, Random Smooth.
  - Rate Modes: Free running (0.05 Hz – 20 Hz) or Host Tempo Sync ($1/32$ to $8$ bars).
  - Bipolar Depth: -100% to +100%.
- **Interactive X/Y Controller Pad**:
  - Assign X and Y axes to any of the 21 modulatable targets.
  - Independent bipolar depth controls for each axis.
  - Smooth visual tracking puck with live parameter readouts.
- **21 Modulatable Targets**: Low Cut, High Cut, Low Res, High Res, BR Mix, Mech, Tube, Bit, Xcita, MM Cutoff, MM Res, MM Mix, Flanger Depth, Flanger Rate, Flanger Feedback, Flanger Mix, Magic EQ Amount, Compressor Amount, Master In Gain, Master Dry/Wet, Master Out Volume.

#### 8. Master Section & Telemetry
- **Dry/Wet Circular Delay Matching**: Compensates for the 24-sample group delay of the distortion oversampling filters.
- **Safety Hard Clipper**: Prevents digital wrap-around distortion and limits output signals to -0.1 dBFS.
- **Stereo VU / Peak Metering**: 60 FPS hardware-accelerated input and output peak/RMS meters.

---

## 4. Part of the WvZz Audio Plugin Bundle

WvZz Distortion is the flagship release of the **WvZz Modular Audio Plugin Suite**. The entire bundle is constructed on a shared foundation of reusable static modules (`libWvZzModules.a`):

| Plugin Name | Status | Primary Focus | Key Architecture |
| :--- | :--- | :--- | :--- |
| **WvZz Distortion** | **Available** | Multi-circuit distortion, filtering & color | 4-circuit distortion, 12-filter MM, BP split, Flanger, Magic EQ, Compressor |
| *WvZz Delay* | *Planned* | Tape, analog BBD & granular stereo delay | Modular fractional delay lines, diffusers, modulation matrix |
| *WvZz Reverb* | *Planned* | Algorithmic plate, hall & non-linear spaces | Zero-allocation FDN (Feedback Delay Networks), early reflections |
| *WvZz Modulation* | *Planned* | Chorus, ensemble, phaser & rotary tremolo | Multi-stage quadrature LFOs, multi-voice chorus lines |
| *WvZz Dynamics* | *Planned* | Multi-band compressor, limiter & transient shaper | Andy Simper ZDF crossover networks, multi-instance compressors |

---

## 5. Documentation Directory

- **[User Manual (MANUAL.md)](USER_MANUAL.md)**: Comprehensive guide for producers and sound designers, covering GUI navigation, parameter ranges, DAW setup, and preset walkthroughs.
- **[Bundle Root Guide (../../README.md)](../../README.md)**: Root architectural documentation, build scripts, and developer overview.
- **[Developer & Build Manual (../../MANUAL.md)](../../MANUAL.md)**: Deep technical manual covering CMake build targets, universal binary signing, and test harness execution.
