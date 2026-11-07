# WvZz Distortion - User Manual

> Official User Guide for **WvZz Distortion**, part of the **WvZz Audio Plugin Bundle**.

---

## Table of Contents
1. [Overview & Quick Start](#1-overview--quick-start)
2. [Interface Layout & Navigation](#2-interface-layout--navigation)
3. [Module Parameters Guide](#3-module-parameters-guide)
   - [Top Header Bar](#top-header-bar)
   - [BP Filter (Bandpass & Band-Reject)](#bp-filter)
   - [Distortion Unit](#distortion-unit)
   - [MM Filter (Multi-Mode)](#mm-filter)
   - [Flanger](#flanger)
   - [Magic EQ](#magic-eq)
   - [Compressor](#compressor)
   - [Center Control: X/Y Pad & Visualizer](#center-control-xy-pad--visualizer)
   - [Modulation: LFO 1 & LFO 2](#modulation-lfo-1--lfo-2)
   - [Master Section & Output](#master-section--output)
4. [Signal Flow & Routing Concepts](#4-signal-flow--routing-concepts)
5. [Factory Presets Guide](#5-factory-presets-guide)
6. [DAW Setup & Best Practices](#6-daw-setup--best-practices)
7. [Troubleshooting & FAQ](#7-troubleshooting--faq)

---

## 1. Overview & Quick Start

**WvZz Distortion** is a multi-stage creative color and dynamics effect plugin for macOS (`VST3` and `Standalone`). It combines frequency-selective band-splitting, four parallel distortion circuits, twelve filter topologies, stereo flanging, specialized low-end enhancement, and one-knob compression.

### 60-Second Quick Start:
1. Load **WvZz Distortion** as an insert effect on an audio track (drums, synth, 808, bassline, or guitar).
2. Use the **Preset Selector** in the top header bar to test sounds:
   - Try `Warm Tube Bus Glue` on a drum bus or mix bus.
   - Try `Neuro Reese Mangler` or `Sub Weight Enhancer` on an 808 or synth bass.
   - Try `Acid Band-Reject Sweep` on an arpeggio or percussion loop.
3. Turn on the **BP Filter** (`ON` button) and adjust **Low Cut** and **High Cut** to narrow down which frequencies you want to distort.
4. Dial in the four distortion knobs: **Mech** for bite, **Tube** for warmth, **Bit** for digital crunch, and **Xcita** for sparkle.
5. Bring in **BR Mix** (Band-Reject Mix) to blend untouched sub-bass and high frequencies back into the signal.
6. Drag the central **X/Y Pad** to morph the sound live.

---

## 2. Interface Layout & Navigation

The interface uses a clean, responsive vector layout designed around a 3-column architecture:

```
┌──────────────────────────────────────────────────────────────────────────────────┐
│  [WvZz LOGO]   BP ➔ DIST ➔ MM+BR ➔ FLG ➔ EQ ➔ COMP ➔ MASTER   [PRESET] [DICE] [BYPASS] │
├─────────────────────────┬──────────────────────────┬─────────────────────────────┤
│       LEFT COLUMN       │      CENTER COLUMN       │        RIGHT COLUMN         │
│                         │                          │                             │
│  ┌───────────────────┐  │  ┌────────────────────┐  │  ┌───────────────────────┐  │
│  │     BP FILTER     │  │  │ [ X/Y PAD ] [SCOPE]│  │  │       MM FILTER       │  │
│  │ (Cutoffs, Link,   │  │  │ Interactive Puck   │  │  │ (12 Topologies, Res,  │  │
│  │  Resonance, BR)   │  │  │ Target Assignments │  │  │  Env Follower, Mix)   │  │
│  └───────────────────┘  │  └────────────────────┘  │  └───────────────────────┘  │
│  ┌───────────────────┐  │  ┌────────────────────┐  │  ┌───────────────────────┐  │
│  │    DISTORTION     │  │  │       LFO 1        │  │  │    STEREO FLANGER     │  │
│  │ (Mech, Tube, Bit, │  │  │ (Wave, Rate, Sync, │  │  └───────────────────────┘  │
│  │  Xcita Saturation)│  │  │  Depth, Target)    │  │  ┌───────────────────────┐  │
│  └───────────────────┘  │  ├────────────────────┤  │  │       MAGIC EQ        │  │
│                         │  │       LFO 2        │  │  │ (Sub Peaking & Phat)  │  │
│                         │  │ (Wave, Rate, Sync, │  │  └───────────────────────┘  │
│                         │  │  Depth, Target)    │  │  ┌───────────────────────┐  │
│                         │  └────────────────────┘  │  │      COMPRESSOR       │  │
│                         │                          │  │ (Amount, Phat, GR dB) │  │
│                         │                          │  └───────────────────────┘  │
│                         │                          │  ┌───────────────────────┐  │
│                         │                          │  │     MASTER SECTION    │  │
│                         │                          │  │ (Gain, Mix, Vol, VU)  │  │
│                         │                          │  └───────────────────────┘  │
└─────────────────────────┴──────────────────────────┴─────────────────────────────┘
```

- **Resizing**: Click and drag the bottom-right resize corner. The UI scales smoothly from 75% (810 × 510 px) to 200% (1920 × 1200 px) with high-DPI crisp vector graphics.
- **Card Controls**: Each module card features an independent **ON/OFF** toggle and a **`?`** contextual help button that displays inline parameter documentation.
- **Visual Feedback**: Bypassed modules dim their controls to provide clear visual status.

---

## 3. Module Parameters Guide

### Top Header Bar

- **WvZz Vector Monogram**: Scalable brand badge.
- **Signal Flow Summary**: Visual breadcrumb path showing the serial audio chain: `BP ➔ DIST ➔ MM + BR ➔ FLG ➔ EQ ➔ COMP ➔ MASTER` (or `BP ➔ EQ ➔ DIST ➔ MM + BR ➔ FLG ➔ COMP ➔ MASTER` when Magic EQ is in Pre-Distortion mode).
- **Preset Selector**: Choose from 15 factory presets or step through them using the `<` and `>` buttons.
- **Dice (Smart Randomizer)**: Randomizes parameters across all active modules within musical bounds without destabilizing filters or triggering harsh feedback.
- **Master Bypass**: Globally bypasses all processing stages, letting the dry input pass directly to the output.

---

### BP Filter (Bandpass & Band-Reject Splitter)

The BP Filter isolates the frequency band that will feed the Distortion and MM Filter stages.

| Parameter | Range | Default | Description |
| :--- | :--- | :--- | :--- |
| **Bypass (`bp_bypass`)** | On / Off | Off (Active) | Activates or bypasses the filter stage. |
| **Low Cut (`bp_low_cut`)** | 20 Hz – 20 kHz | 20 Hz | Frequencies below this point are routed to the Band-Reject auxiliary bus. |
| **High Cut (`bp_high_cut`)** | 20 Hz – 20 kHz | 20 kHz | Frequencies above this point are routed to the Band-Reject auxiliary bus. |
| **Link (`bp_link`)** | On / Off | Off | Locks the distance between Low Cut and High Cut for one-touch band sweeps. |
| **Low Res (`bp_low_res`)** | 0.1 – 10.0 | 0.707 | Resonance peak at the Low Cut corner frequency. |
| **High Res (`bp_high_res`)** | 0.1 – 10.0 | 0.707 | Resonance peak at the High Cut corner frequency. |
| **BR Mix (`bp_br_mix`)** | 0% – 100% | 0% | Blends the rejected auxiliary low/high audio back in after the MM Filter stage. |

> **Pro-Tip**: To distort only the aggressive midrange of an 808 without muddying the clean sub, set **Low Cut** to 120 Hz, **High Cut** to 2 kHz, and turn **BR Mix** up to 100%. The sub stays untouched and 100% clean!

---

### Distortion Unit

Multi-mode non-linear saturation running in a serial cascaded topology inside a 4x oversampled polyphase pipeline.

| Parameter | Range | Default | Description |
| :--- | :--- | :--- | :--- |
| **Bypass (`dist_bypass`)** | On / Off | Off (Active) | Bypasses the distortion stage. |
| **Mode (`dist_mode`)** | Classic / Hard Clip / Wavefold | Classic | Selects distortion algorithm: **Classic** (smooth arctan soft clipping), **Hard Clip** (brickwall clipping at ±1.0 for aggressive EDM/Hardstyle edge), or **Wavefold** (mathematical sine wavefolding for metallic screeches). |
| **Mech (`dist_mech`)** | 0% – 100% | 0% | Industrial clipping drive. In Classic mode, applies odd-harmonic saturation; in Hard Clip mode, pushes signals hard against the ±1.0 boundary; in Wavefold mode, determines folding recursion depth. |
| **Tube (`dist_tube`)** | 0% – 100% | 0% | Warm vacuum tube saturation generating musical even harmonics. Adds body and analog thickness before subsequent clipping stages. |
| **Bit (`dist_bit`)** | 0% – 100% | 0% | Dynamic digital bitcrushing (reduces bit depth from 16-bit to 2-bit). Gritty digital texture. |
| **Xcita (`dist_xcita`)** | 0% – 100% | 0% | High-frequency psychoacoustic exciter. Adds brightness, presence, and air without harshness. |

> **Cascaded Topology**: The distortion unit cascades from **Tube Drive** into **Mech Drive (or selected Mode)**, followed by **Bitcrush** and **Exciter**. This allows smooth pre-warming from Tube before slamming into Hard Clip or Wavefold.

---

### MM Filter (Multi-Mode Filter)

A versatile downstream filter featuring 12 unique topologies and dynamic envelope follower modulation.

| Parameter | Range | Default | Description |
| :--- | :--- | :--- | :--- |
| **Bypass (`mm_bypass`)** | On / Off | Off (Active) | Bypasses the MM Filter stage. |
| **Type (`mm_type`)** | 12 Topologies | LP 12dB | Selects the active filter algorithm (see below). |
| **Cutoff (`mm_cutoff`)** | 20 Hz – 20 kHz | 1000 Hz | Filter corner or center frequency. |
| **Resonance (`mm_res`)** | 0.0 – 1.0 | 0.2 | Filter peak resonance or internal feedback. |
| **Env Attack (`mm_env_attack`)** | 1 ms – 500 ms | 10 ms | Attack time for the envelope follower sidechain. |
| **Env Release (`mm_env_release`)** | 10 ms – 2000 ms| 100 ms | Release time for the envelope follower sidechain. |
| **Env Mod (`mm_env_mod`)** | -100% – +100% | 0% | Bipolar amount of envelope follower modulation applied to Cutoff. |
| **Mix (`mm_mix`)** | 0% – 100% | 100% | Wet/dry balance of the MM Filter output. |

#### Filter Topologies:
1. `LP 12dB`: 2-pole resonant low-pass.
2. `LP 24dB`: 4-pole steep resonant low-pass.
3. `BP 12dB`: 2-pole band-pass filter.
4. `BP 24dB`: 4-pole steep band-pass filter.
5. `HP 12dB`: 2-pole high-pass filter.
6. `HP 24dB`: 4-pole steep high-pass filter.
7. `Notch`: Narrow band elimination.
8. `Peak`: Parametric bell filter.
9. `Comb +`: Positive feedback comb filter for flanging and string resonances.
10. `Comb -`: Negative feedback comb filter for hollow, woody resonances.
11. `RingMod`: Diode ring modulator for clangorous metallic tones.
12. `Formant`: Vocal formant filter morphing between vowels A, E, I, O, U as cutoff sweeps.

---

### Flanger

Stereo fractional delay flanger with a 90° quadrature LFO.

| Parameter | Range | Default | Description |
| :--- | :--- | :--- | :--- |
| **Bypass (`flg_bypass`)** | On / Off | Off (Active) | Bypasses the flanger. |
| **Rate (`flg_rate`)** | 0.05 Hz – 10.0 Hz | 0.2 Hz | Modulation frequency. |
| **Depth (`flg_depth`)** | 0% – 100% | 50% | Sweep depth of the delay modulation. |
| **Feedback (`flg_feedback`)** | -95% – +95% | 40% | Bipolar feedback for metallic swooshes and resonance. |
| **Mix (`flg_mix`)** | 0% – 100% | 0% | Wet/dry balance of the flanger. |

---

### Magic EQ

A low-frequency enhancer that injects solid weight into kicks and basslines, or creates aggressive kick "tok" transients.

| Parameter | Range | Default | Description |
| :--- | :--- | :--- | :--- |
| **Bypass (`meq_bypass`)** | On / Off | Off (Active) | Bypasses Magic EQ. |
| **Pre (`meq_pre`)** | On / Off | Off (Post) | Toggles Pre-Distortion EQ routing. When ON, routes Magic EQ before the Distortion unit to overdrive specific bass fundamentals (producing Hardstyle kick "tok" transients). When OFF, routes post-distortion for clean low-end enhancement. |
| **Amount (`meq_amount`)** | 0% – 100% | 0% | Boost intensity of the sub-bass resonator. |
| **Tune (`meq_tune`)** | 45 Hz – 180 Hz | 80 Hz | Target fundamental sub frequency. Automatically cuts $2 \times \text{tune}$ to avoid boxy low-mid buildup. Extended range supports low sub-bass (45 Hz) up to upper-bass punch (180 Hz). |
| **Phat (`meq_phat`)** | On / Off | Off | Adds warm harmonic saturation to the sub frequencies. |

---

### Compressor

One-knob dynamic leveling with auto-makeup gain and real-time gain reduction metering.

| Parameter | Range | Default | Description |
| :--- | :--- | :--- | :--- |
| **Bypass (`comp_bypass`)** | On / Off | Off (Active) | Bypasses the compressor. |
| **Amount (`comp_amount`)** | 0% – 100% | 0% | Increases compression ratio and reduces threshold while auto-raising makeup gain. |
| **Phat (`comp_phat`)** | On / Off | Off | Toggles aggressive smash mode (12:1 ratio and fast attack) vs Normal mode (3.5:1 ratio). |
| **GR Meter** | 0 to -30 dB | 0 dB | Real-time display of gain attenuation. |

---

### Center Control: X/Y Pad & Visualizer

The center panel features two views switchable via top tabs:
1. **X/Y Controller Pad**:
   - **Puck**: Click and drag to modulate two targets simultaneously.
   - **X Target / Y Target**: Choose any of the 22 modulatable targets (including **Master Vol**) from the dropdown lists.
   - **X Depth / Y Depth**: Bipolar depth controls (-100% to +100%) setting the influence of pad movement.
2. **Signal Visualizer**:
   - Real-time comparison display showing waveform or spectrum changes across the processing chain.

---

### Modulation: LFO 1 & LFO 2

Two independent low-frequency oscillators designed for rhythmic movement.

| Parameter | Range | Default | Description |
| :--- | :--- | :--- | :--- |
| **Waveform** | 7 Types | Sine | Sine, Triangle, Saw Up, Saw Down, Square, Sample & Hold, Random Smooth. |
| **Sync** | Free / Sync | Free | Free running in Hertz or locked to DAW host tempo subdivisions. When DAW playback starts, the LFO phase locks precisely to host quarter-note (PPQ) beat positions for jitter-free rhythmic drops. |
| **Rate** | 0.05 – 20 Hz / 1/32 – 8 bars | 1.0 Hz | Speed of modulation. |
| **Depth** | -100% – +100% | 0% | Bipolar modulation depth. |
| **Target** | 22 Targets | None | Parameter modulated by this LFO (including Master Volume). |

---

### Master Section & Output

- **Input Gain (`input_gain`)**: -24 dB to +24 dB trim before entering any processing stages.
- **Dry/Wet Mix (`master_mix`)**: 0% (pure dry) to 100% (fully processed). The dry path is phase-aligned via a dynamic delay buffer matching the 4x polyphase oversampling latency.
- **Output Volume (`master_volume`)**: -36 dB to +12 dB output attenuation or boost. Fully modulatable via LFOs and the X/Y pad.
- **Mono Bass (`master_mono_bass`)**: Toggles a 120 Hz zero-delay feedback SVF crossover that sums frequencies below 120 Hz to mono while leaving wide stereo information intact above 120 Hz. Vital for club sound system compatibility.
- **Master Clipper (`master_clip`)**: Engaging `CLIP` enables a 0.0 dBFS hard peak clipper that squares off transients for maximum loudness and eliminates inter-sample clipping on EDM master buses.
- **Click-Free Bypass**: Master bypass employs a 10 ms linear crossfade buffer to eliminate audio pops and clicks during live DAW automation.
- **Stereo VU & Peak Meters**: Dual high-precision visual meters showing input and output signal dynamics.

---

## 4. Signal Flow & Routing Concepts

Understanding the signal flow allows you to harness the full potential of WvZz Distortion:

```
[Input Gain]
     │
     ▼
[BP Filter] ─────────── (Auxiliary Band-Reject Bus: Frequencies outside passband)
     │                                                      │
     ├──────────────────────────┐                           │
     │ (Normal Routing)         │ (meq_pre = 1)             │
     │                          ▼                           │
     │                    [Magic EQ]                        │
     │                          │                           │
     ▼                          ▼                           │
[Distortion Unit (Tube ➔ Mech/Mode ➔ Bit ➔ Xcita)]          │
     │                                                      │
     ▼                                                      │
[MM Filter]                                                 │
     │                                                      │
     ▼                                                      │
[Summing Node] ◄────────────────────────────────────────────┘ (Multiplied by BR Mix)
     │                                                        (Delayed to match oversampler)
     ▼
[Stereo Flanger]
     │
     ├──────────────────────────┐
     │ (Normal Routing)         │ (meq_pre = 1: Bypassed here)
     ▼                          │
[Magic EQ]                      │
     │                          │
     ▼ ◄────────────────────────┘
[Compressor]
     │
     ▼
[Dry/Wet Blender] ◄─── [Latency-Compensated Dry Signal]
     │
     ▼
[Sub-Bass Mono Maker (120 Hz ZDF SVF Crossover, if MONO active)]
     │
     ▼
[Master Clipper (0.0 dBFS Hard Clip, if CLIP active)]
     │
     ▼
[Output Volume & 10ms Click-Free Bypass Crossfade]
     │
     ▼
[Stereo Output]
```

---

## 5. Factory Presets Guide

WvZz Distortion ships with 15 carefully designed factory presets covering multiple production genres:

1. **Neuro Reese Mangler**: Heavy mechanical and tube distortion with aggressive envelope-follower modulation on a 24dB low-pass filter. Perfect for drum & bass and dubstep reese basses.
2. **Phat Breakbeat Smash**: Phat compression mode paired with subtle tube drive and a touch of Magic EQ sub boost. Instantly fattens vintage drum loops.
3. **Sub Weight Enhancer**: Clean bandpass isolation that keeps the sub intact via BR Mix while applying subtle exciter and Magic EQ tuned to 80 Hz.
4. **Acid Band-Reject Sweep**: Sync'd LFO sweeping the MM Filter cutoff while the auxiliary band-reject bus maintains the groove's punch.
5. **Lo-Fi Bit Crunch**: Downsampled bitcrushing with vintage comb filtering and subtle tape flanging.
6. **Warm Tube Bus Glue**: Transparent even-order tube warmth and gentle 3.5:1 compression for glueing instrument groups or mix buses.
7. **Stepped Comb Resonator**: Sample & Hold LFO modulating a positive comb filter to generate rhythmic melodic overtones.
8. **Talking Vowel Wobble**: Formant filter modulated by an LFO morphing between vowels for vowel-talk basslines.
9. **Metallic RingMod Perc**: High-speed ring modulation and mechanical wavefolding for transforming percussion into industrial hits.
10. **Wide Quadrature Flanger**: 90° out-of-phase stereo delay modulation for rich dimensional widening without center-channel cancellation.
11. **Hardstyle Rawstyle Kick**: Pre-Distortion Magic EQ tuned to 60 Hz pushing into brickwall Hard Clip mode, with sub-bass mono collapse and master peak clipping. The definitive Hardstyle kick engine.
12. **Euphoric Lead Thickener**: Warm tube pre-drive cascading into vocal formant filtering and 90° quadrature flanging for massive stadium lead supersaws.
13. **Hardcore Frenchcore Tok**: Pre-Distortion Magic EQ tuned to 120 Hz driving high-intensity Wavefold distortion and a steep 12dB high-pass filter for piercing transient punch.
14. **BigRoom Festival Drop**: Hard Clip mode, parallel BP band-reject sub preservation, aggressive Phat compression, and master clipping for festival-ready drop kicks.
15. **PsyTrance Rolling Bass**: Tempo-sync'd LFO modulating the BP Filter low cut corner with sub-bass mono summing enabled for punchy, rolling basslines.

---

## 6. DAW Setup & Best Practices

### Supported Hosts
Tested and validated on macOS (Intel and Apple Silicon):
- **Ableton Live 11 & 12** (VST3)
- **Logic Pro** (via Standalone or AU/VST3 bridges)
- **FL Studio 21+** (VST3)
- **Bitwig Studio 5+** (VST3)
- **Reaper 6 & 7** (VST3)
- **Cubase 12 & 13** (VST3)

### Gain Staging Tips
- **Watch the Input Meter**: Feed signals at around -18 to -12 dBFS nominal peak. If a signal is too quiet or loud, use the **In Gain** knob in the Master section to calibrate.
- **Drive vs. Makeup**: As you increase **Mech** or **Tube**, the distortion circuits will boost gain. Balance the wet signal with the **Out Vol** control or engage the **Compressor** to tame transient peaks.
- **Oversampling Quality**: Because the plugin runs 4x polyphase oversampling natively, you get pristine highs without setting your entire DAW project to 192 kHz.

---

## 7. Troubleshooting & FAQ

**Q: Why don't I see an AU (Audio Unit) version in Logic Pro?**  
A: WvZz Distortion is strictly built as a modern VST3 and Standalone format. Use a VST3 host, wrapper, or the Standalone application.

**Q: Can I use the plugin live on stage?**  
A: Yes! The entire DSP engine guarantees zero heap memory allocations on the audio processing thread, resulting in ultra-low latency, rock-solid stability, and zero glitches.

**Q: Why does the sound change when I move the X/Y pad?**  
A: Check the X Target and Y Target dropdown menus. If they are assigned to parameters like Cutoff or Drive, moving the puck alters those settings in real time. Set X/Y Depth to 0% if you wish to freeze the pad.
