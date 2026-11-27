# WvZz Dynamics - User Manual

> Official User Guide for **WvZz Dynamics**, part of the **WvZz Audio Plugin Bundle**.

---

## Table of Contents
1. [Overview & Quick Start](#1-overview--quick-start)
2. [Interface Layout & Navigation](#2-interface-layout--navigation)
3. [Module Parameters Guide](#3-module-parameters-guide)
   - [Sub & Detector](#sub--detector)
   - [Transient Shaper](#transient-shaper)
   - [Multiband OTT](#multiband-ott)
   - [Compressor](#compressor)
   - [Rhythmic Pumper](#rhythmic-pumper)
   - [Maximizer Clipper](#maximizer-clipper)
4. [Global Controls](#4-global-controls)
   - [Top Header Bar](#top-header-bar)
   - [Modulation: LFO 1 & LFO 2](#modulation-lfo-1--lfo-2)
   - [Modulation: X/Y Pad & Monitor](#modulation-xy-pad--monitor)
   - [Master Section](#master-section)
5. [Factory Presets](#5-factory-presets)
6. [Signal Flow & Routing](#6-signal-flow--routing)
7. [Usage Tips](#7-usage-tips)
8. [Troubleshooting & FAQ](#8-troubleshooting--faq)

---

## 1. Overview & Quick Start

**WvZz Dynamics** is a dynamics effect plugin for macOS (`VST3` and `Standalone`). It chains a sub-bass splitter, a transient shaper with gate, a 3-band OTT compressor, a bus compressor, a rhythmic ducker, and a clipper.

### 60-Second Quick Start
1. Load **WvZz Dynamics** as an insert effect on a track or bus.
2. Pick a preset in the **Preset Selector** in the header:
   - Try `Drum Bus VCA Glue` on a drum bus.
   - Try `SuperSaw Hyper-OTT` on a lead stack.
   - Try `Rawstyle Kick Clipper` on a kick.
3. In **SUB & DETECTOR**, turn `LOCK` on to keep the sub-bass out of the OTT and the Compressor.
4. In **TRANSIENT & GATE**, raise **Attack** for more punch.
5. In **HYPER-OTT (3-BAND)**, raise **Depth** for a denser sound.
6. In **MAXIMIZER CLIPPER**, raise **Drive** for loudness.
7. Match the level with **Volume** in the **MASTER** card.

---

## 2. Interface Layout & Navigation

The window has a header bar and three columns.

```text
┌──────────────────────────────────────────────────────────────────────────────────┐
│ [LOGO] DET -> TRANS -> OTT -> COMP -> PUMP -> CLIP -> MASTER [<][PRESET][>][DICE][BYPASS] │
├─────────────────────────┬──────────────────────────┬─────────────────────────────┤
│       LEFT COLUMN       │      CENTER COLUMN       │        RIGHT COLUMN         │
│                         │                          │                             │
│  ┌───────────────────┐  │  ┌────────────────────┐  │  ┌───────────────────────┐  │
│  │  SUB & DETECTOR   │  │  │ [X/Y PAD][MONITOR] │  │  │      COMPRESSOR       │  │
│  │ (LOCK, LISTEN,    │  │  │ X/Y pad with       │  │  │ (Thresh, Ratio, Mode, │  │
│  │  X-Over, Tight,   │  │  │ targets and depths,│  │  │  Attack, Release,     │  │
│  │  SC HPF, Tilt)    │  │  │ or signal monitor  │  │  │  Knee, Mix, PHAT)     │  │
│  └───────────────────┘  │  └────────────────────┘  │  └───────────────────────┘  │
│  ┌───────────────────┐  │  ┌────────────────────┐  │  ┌───────────────────────┐  │
│  │ TRANSIENT & GATE  │  │  │       LFO 1        │  │  │    RHYTHMIC PUMPER    │  │
│  │ (Attack, Sustain, │  │  │ (Wave, SYNC, Rate, │  │  │ (Mode, Division,      │  │
│  │  Gate, Speed)     │  │  │  Sync Div, Depth,  │  │  │  Shape, Amount,       │  │
│  └───────────────────┘  │  │  Target)           │  │  │  Curve, SUB ONLY)     │  │
│  ┌───────────────────┐  │  ├────────────────────┤  │  └───────────────────────┘  │
│  │ HYPER-OTT (3-BAND)│  │  │       LFO 2        │  │  ┌───────────────────────┐  │
│  │ (Depth, Upward,   │  │  │ (Wave, SYNC, Rate, │  │  │   MAXIMIZER CLIPPER   │  │
│  │  Downward, Time,  │  │  │  Sync Div, Depth,  │  │  │ (Mode, Drive,         │  │
│  │  Sheen)           │  │  │  Target)           │  │  │  Ceiling, Mix)        │  │
│  └───────────────────┘  │  └────────────────────┘  │  └───────────────────────┘  │
│                         │                          │  ┌───────────────────────┐  │
│                         │                          │  │        MASTER         │  │
│                         │                          │  │ (In Gain, Dry/Wet,    │  │
│                         │                          │  │  Volume, CLIP, MONO)  │  │
│                         │                          │  └───────────────────────┘  │
└─────────────────────────┴──────────────────────────┴─────────────────────────────┘
```

- **Window size**: the default size is 1080 × 680 px.
- **Resizing**: drag the bottom-right corner. The window scales from 810 × 510 px to 1920 × 1200 px and keeps its aspect ratio.
- **Card controls**: each module card has a bypass toggle in its header and a **`?`** help button.
- **Visual feedback**: a bypassed card dims its controls.

---

## 3. Module Parameters Guide

The modules are listed in signal-chain order. Parameter IDs are the names a host shows for automation.

In every table, a bypass parameter set to `Off` means the module is active.

---

### Sub & Detector

Card title: `SUB & DETECTOR`. This module does two jobs:

1. It splits the sub-bass from the rest of the signal, so later modules do not react to it.
2. It filters the detector signal. The detector signal is the copy of the audio that the Compressor measures.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Header toggle | `det_bypass` | On / Off | Off | Bypasses the module. No sub split and no detector filter. |
| LOCK | `det_sub_lock` | On / Off | On | Sends the band below **X-Over** around the OTT and the Compressor. |
| X-Over | `det_sub_xover` | 40 – 300 Hz | 90 Hz | Sets the crossover frequency of the sub split (24 dB/oct). |
| Tight | `det_sub_tight` | 0.0 – 1.0 | 0.25 | Reduces the sub level when the sub is loud. Works only with **LOCK** on. |
| SC HPF | `det_sc_hp` | 20 – 500 Hz | 80 Hz | High-pass filter on the detector signal. Higher values make the Compressor ignore bass. |
| Tilt | `det_sc_tilt` | -6 – +6 dB | 0 dB | Tilts the detector signal around 1 kHz. Positive values make the Compressor react more to highs. |
| LISTEN | `det_sc_listen` | On / Off | Off | Outputs the filtered detector signal so you can hear it. |

> **Note**: with **LOCK** on, the sub band is added back after the Compressor. The Rhythmic Pumper and the Maximizer Clipper still process it.

---

### Transient Shaper

Card title: `TRANSIENT & GATE`. This module changes the level of the attack and the tail of each hit. It works the same way at any input level. It also has a noise gate.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Header toggle | `trans_bypass` | On / Off | Off | Bypasses the module, including the gate. |
| Attack | `trans_attack` | -1.0 – +1.0 | 0.0 | Positive values boost the attack (up to about +12 dB). Negative values soften it. |
| Sustain | `trans_sustain` | -1.0 – +1.0 | 0.0 | Positive values lift the tail. Negative values shorten it. |
| Speed selector | `trans_speed` | Fast / Punch | Fast | `Fast` follows short hits. `Punch` uses longer envelope and gate times for kicks. |
| Gate | `trans_gate` | -80 – 0 dB | -80 dB | Gate threshold. The gate is off below -78 dB. |

> **Note**: the gate closes 3 dB below its opening threshold and has a short hold time. This stops it from chattering on decaying sounds.

---

### Multiband OTT

Card title: `HYPER-OTT (3-BAND)`. This module splits the signal into three bands: Low (below 120 Hz), Mid (120 Hz to 2.5 kHz), and High (above 2.5 kHz). In each band it turns loud parts down and quiet parts up.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Header toggle | `ott_bypass` | On / Off | Off | Bypasses the module. |
| Depth | `ott_depth` | 0.0 – 1.0 | 0.35 | Blend between the unprocessed and the processed signal. |
| Upward | `ott_up_drive` | 0.0 – 1.0 | 0.5 | Amount of upward compression. Lifts quiet detail between -55 and -22 dBFS. |
| Downward | `ott_down_drive` | 0.0 – 1.0 | 0.5 | Amount of downward compression. Reduces band levels above -12 dBFS. |
| Time | `ott_time` | 0.1 – 2.0 | 1.0 | Scales attack and release. At 1.0, attack is 5 ms and release is 50 ms. |
| Sheen | `ott_sheen` | 0 – +12 dB | 0 dB | Boosts the High band before its compression stage. |

> **Note**: with **LOCK** on in Sub & Detector, the band below **X-Over** does not reach this module.

---

### Compressor

Card title: `COMPRESSOR`. This is a bus compressor with three modes. It measures the filtered detector signal from Sub & Detector. Makeup gain is automatic.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Header toggle | `comp_bypass` | On / Off | Off | Bypasses the module. |
| Thresh | `comp_thresh` | -48 – 0 dB | -18 dB | Level above which compression starts. |
| Ratio | `comp_ratio` | 1.5 – 20.0 | 4.0 | Compression ratio (x:1). |
| Attack | `comp_attack` | 0.1 – 50 ms | 5 ms | Time the compressor takes to react. |
| Release | `comp_release` | 10 – 1500 ms | 120 ms | Time the compressor takes to recover. |
| Knee | `comp_knee` | 0 – 12 dB | 3 dB | Width of the soft knee around the threshold. |
| Mix | `comp_mix` | 0.0 – 1.0 | 1.0 | Blend of compressed and uncompressed signal (parallel compression). |
| Mode selector | `comp_mode` | Opto / VCA / FET | Opto | `Opto`: two-stage release. `VCA`: single-stage. `FET`: attack five times faster. |
| PHAT | `comp_phat` | On / Off | Off | Forces at least 12:1 ratio, at most 0.8 ms attack, a 1 dB knee, and a soft clip at -0.1 dBFS. |
| (no control) | `comp_amount` | 0.0 – 1.0 | 0.0 | Lowers the threshold by up to 36 dB and adds up to 14 dB of makeup. Host automation only. |

The card also shows a gain reduction meter.

---

### Rhythmic Pumper

Card title: `RHYTHMIC PUMPER`. This module turns the level down and up in a repeating pattern. The result sounds like sidechain compression from a kick.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Header toggle | `pump_bypass` | On / Off | Off | Bypasses the module. |
| Amount | `pump_amount` | 0.0 – 1.0 | 0.0 | Depth of the ducking. At 0.0 the module has no effect. |
| Division selector | `pump_sync_div` | 1/1, 1/2, 1/4, 1/8, 1/16, 1/4D, 1/8D, 1/4T, 1/8T | 1/4 | Length of one ducking cycle. `D` is dotted and `T` is triplet. |
| Shape selector | `pump_shape` | Classic Duck / Hard Kick Clear / Reverse Bass / Festival Swell | Classic Duck | Shape of the gain curve over one cycle (see below). |
| Curve | `pump_curve` | 0.1 – 0.9 | 0.5 | Changes the recovery of the shape. `Reverse Bass` ignores it. |
| Mode selector | `pump_mode` | Tempo Sync / Audio / Ext SC | Tempo Sync | `Tempo Sync` follows the host beat. `Audio / Ext SC` restarts the cycle on each trigger. |
| SUB ONLY | `pump_sub_only` | On / Off | Off | Ducks only the signal below 180 Hz. |

#### Shapes
1. `Classic Duck`: smooth dip at the start of the cycle. **Curve** sets how much of the cycle the recovery takes.
2. `Hard Kick Clear`: silence for the first 15% of the cycle, then a ramp up.
3. `Reverse Bass`: low level for the first 20%, a swell up to 70%, then a fast cut.
4. `Festival Swell`: a rise across the whole cycle. **Curve** sets how steep it is.

#### Modes
- **Tempo Sync**: while the host plays, the cycle is locked to the host beat position. While the host is stopped, the cycle runs free at the host tempo.
- **Audio / Ext SC**: the cycle restarts when the detector signal rises above -24 dBFS. The cycle length still comes from the division and the host tempo.

---

### Maximizer Clipper

Card title: `MAXIMIZER CLIPPER`. This module adds gain and then clips the peaks at the ceiling. The clipping curves are anti-aliased.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Header toggle | `clip_bypass` | On / Off | Off | Bypasses the module. |
| Drive | `clip_drive` | 0 – +18 dB | 0 dB | Gain into the clipper. More drive means more loudness and more distortion. |
| Ceiling | `clip_ceiling` | -12 – 0 dB | -0.1 dB | Maximum output level of the clipped signal. |
| Mode selector | `clip_mode` | Hard Clip / Warm Tube / Hardstyle Crunch | Hard Clip | `Hard Clip`: flat cut. `Warm Tube`: soft tanh curve. `Hardstyle Crunch`: asymmetric curve. |
| Mix | `clip_mix` | 0.0 – 1.0 | 1.0 | Blend of the clipped signal and the signal before the drive. |

> **Note**: with **Mix** below 1.0, the unclipped part can exceed the ceiling.

---

## 4. Global Controls

### Top Header Bar

- **Logo and subtitle**: shows `DYNAMICS  //  MULTIBAND & RHYTHMIC ENGINE`.
- **Signal flow label**: shows `DET -> TRANS -> OTT -> COMP -> PUMP -> CLIP -> MASTER`.
- **Preset Selector**: lists the 20 factory presets. The `<` and `>` buttons step through them.
- **DICE**: sends a randomize request to the six processing modules. It does not touch Modulation or Master.
- **BYPASS**: master bypass. See `master_bypass` in the [Master Section](#master-section).

---

### Modulation: LFO 1 & LFO 2

The two LFOs sit in the lower part of the center column. Each LFO moves one target.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Wave (LFO 1) | `lfo1_wave` | Sine / Triangle / Saw Down / Saw Up / Square / Sample & Hold / Smooth Random | Sine | Waveform of LFO 1. |
| SYNC (LFO 1) | `lfo1_sync` | On / Off | Off | Off: free rate in Hz. On: rate follows the host tempo. |
| Rate (LFO 1) | `lfo1_rate` | 0.02 – 30 Hz | 1.0 Hz | Speed of LFO 1 when **SYNC** is off. |
| Sync Div (LFO 1) | `lfo1_sync_div` | 24 divisions (see below) | 1/4 | Cycle length of LFO 1 when **SYNC** is on. |
| Target (LFO 1) | `lfo1_target` | 16 choices (see below) | None | Parameter that LFO 1 moves. |
| Depth (LFO 1) | `lfo1_depth` | -1.0 – +1.0 | 0.0 | Amount and direction of the modulation. |
| Wave (LFO 2) | `lfo2_wave` | Same 7 waveforms | Sine | Waveform of LFO 2. |
| SYNC (LFO 2) | `lfo2_sync` | On / Off | Off | Off: free rate in Hz. On: rate follows the host tempo. |
| Rate (LFO 2) | `lfo2_rate` | 0.02 – 30 Hz | 1.0 Hz | Speed of LFO 2 when **SYNC** is off. |
| Sync Div (LFO 2) | `lfo2_sync_div` | 24 divisions (see below) | 1/4 | Cycle length of LFO 2 when **SYNC** is on. |
| Target (LFO 2) | `lfo2_target` | 16 choices (see below) | None | Parameter that LFO 2 moves. |
| Depth (LFO 2) | `lfo2_depth` | -1.0 – +1.0 | 0.0 | Amount and direction of the modulation. |

**Sync divisions**: `4/1`, `2/1`, `1/1`, `1/2`, `1/4`, `1/8`, `1/16`, `1/32`. Each one also exists as dotted (`D`) and triplet (`T`).

**Targets**: `None`, `Sub X-Over`, `SC HPF`, `Trans Attack`, `Trans Sustain`, `OTT Depth`, `OTT Upward`, `OTT Sheen`, `Comp Thresh`, `Comp Mix`, `Pump Amount`, `Pump Curve`, `Clip Drive`, `Master Mix`, `LFO 2 Rate`, `Master Vol`.

---

### Modulation: X/Y Pad & Monitor

The top of the center column has two tabs: `X/Y PAD` and `MONITOR`.

**X/Y PAD tab** (card title `MODULATION & X/Y`): drag the puck to move two targets at once.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Puck, horizontal | `xy_x` | 0.0 – 1.0 | 0.5 | Horizontal position of the puck. |
| Puck, vertical | `xy_y` | 0.0 – 1.0 | 0.5 | Vertical position of the puck. |
| X Target | `xy_x_target` | Same 16 targets as the LFOs | None | Parameter that the horizontal axis moves. |
| Y Target | `xy_y_target` | Same 16 targets as the LFOs | None | Parameter that the vertical axis moves. |
| X Depth | `xy_x_depth` | -1.0 – +1.0 | 0.0 | Amount and direction of the horizontal modulation. |
| Y Depth | `xy_y_depth` | -1.0 – +1.0 | 0.0 | Amount and direction of the vertical modulation. |

**MONITOR tab** (card title `ANALYSIS & MONITOR`): replaces the pad with a display that compares the input and the output signal. The plugin also feeds it the total gain reduction of the chain. The LFO panels stay visible.

---

### Master Section

Card title: `MASTER`. The BYPASS button is in the header bar.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| In Gain | `input_gain` | -24 – +24 dB | 0 dB | Gain before all modules. It also changes the dry signal. |
| Dry/Wet | `master_mix` | 0.0 – 1.0 | 1.0 | 0.0 is the dry signal only. 1.0 is the processed signal only. |
| Volume | `master_vol` | -36 – +12 dB | 0 dB | Output level. Applied before MONO and CLIP. |
| MONO | `master_mono_bass` | On / Off | Off | Sums everything below 120 Hz to mono. |
| CLIP | `master_clip` | On / Off | On | Hard clip at 0 dBFS on the output. |
| BYPASS (header) | `master_bypass` | On / Off | Off | Crossfades to the dry signal in 10 ms. |

The card also shows peak and RMS output meters.

---

## 5. Factory Presets

WvZz Dynamics has 20 factory presets.

1. **Euphoric Lead Wall**: lead stacks. Medium OTT with high sheen, VCA compression, 1/4 ducking, warm clipping.
2. **Tok & Tail Kick Glue**: kicks. Strong attack boost, Opto compression with PHAT, 6 dB of hard clip.
3. **Reverse Bass Pumper**: reverse bass lines. `Reverse Bass` pump shape at 95% and `Hardstyle Crunch` clipping.
4. **Screech OTT Maximizer**: screech leads. Heavy OTT, FET compression with PHAT, 8 dB of crunch clipping.
5. **Rawstyle Kick Clipper**: raw kicks. Maximum attack boost, 10:1 FET with PHAT, 10 dB of crunch clipping.
6. **Mainstage Drop Bus**: drop buses. Medium OTT, parallel VCA compression, light 1/4 ducking, hard clip.
7. **Big Room Kick Snap**: big room kicks. Attack boost in `Punch` speed, 5:1 VCA, hard clip.
8. **Festival Sub Lock**: bass and sub. Tight sub control, shorter tails, `Hard Kick Clear` ducking.
9. **Pluck Transient Pop**: plucks. Strong attack boost, short sustain, FET compression.
10. **SuperSaw Hyper-OTT**: supersaws. Deep OTT with strong sheen and 70% 1/4 ducking.
11. **Clap & Snare Smash**: claps and snares. More attack and more sustain, 6:1 FET, hard clip.
12. **Drum Bus VCA Glue**: drum buses. Gentle 3:1 VCA compression with light OTT.
13. **Hi-Hat Air & Sparkle**: hi-hats. High sheen, shorter sustain, Opto compression.
14. **Tight Gate Percussion**: percussion. Gate at -36 dB and very short sustain.
15. **Euphoric Vocal Up-Lift**: vocals. Strong upward OTT, soft Opto compression, light ducking.
16. **Piano & Pluck Leveler**: pianos and plucks. Mild OTT and 2.5:1 Opto compression.
17. **Sidechain Pad Swell**: pads. Softer attack and `Festival Swell` ducking at 85%.
18. **Club Loudness Maximizer**: full mixes for clubs. VCA compression and 6 dB of hard clip.
19. **Parallel Bus Slam**: parallel bus compression. 8:1 FET at 50% mix and 7 dB of hard clip.
20. **Clean Mastering Glue**: mastering. 2:1 VCA, light OTT, and mild warm clipping.

A preset sets only the parameters that it stores. All other parameters keep their current value. For example, no preset stores Compressor attack, release, or knee. Only `Tight Gate Percussion` stores the gate threshold.

---

## 6. Signal Flow & Routing

```text
[Main In] ──► [Input Gain] ──┬──────────────────────────────► (dry copy for Master)
                             │
                             ├──► (detector copy) ◄── or [Sidechain In], if the bus is active
                             │            │
                             ▼            ▼
                    [1. Sub & Detector: split sub, filter detector]
                             │            │
              (sub, if LOCK) │            │ (filtered detector)
        ┌────────────────────┤            │
        │                    ▼            │
        │           [2. Transient Shaper + Gate]
        │                    │            │
        │                    ▼            │
        │           [3. Multiband OTT]    │
        │                    │            │
        │                    ▼            │
        │           [4. Compressor] ◄─────┤ (level detection)
        │                    │            │
        └──────────────► [Sum]            │
                             │            │
                             ▼            │
                    [5. Rhythmic Pumper] ◄┘ (trigger in Audio / Ext SC mode)
                             │
                             ▼
                    [6. Maximizer Clipper]
                             │
                             ▼
                    [Master: Dry/Wet ► Volume ► MONO ► CLIP ► Bypass]
                             │
                             ▼
                        [Stereo Out]
```

### External Sidechain
The plugin has an optional stereo **Sidechain** input bus. It is disabled by default.

1. Enable the sidechain input in your host and route a track to it (for example, the kick).
2. The Compressor now measures the sidechain signal instead of the main input.
3. Set the Rhythmic Pumper **Mode** to `Audio / Ext SC` to trigger the ducking from the same signal.
4. Use **SC HPF** and **Tilt** to filter the sidechain. Turn **LISTEN** on to check the result.

Without an active sidechain bus, the detector uses the main input after **In Gain**.

### Latency and Tail
The plugin reports no latency and no tail to the host.

---

## 7. Usage Tips

- **Start with the level.** Use **In Gain** so that the Compressor and the OTT see a healthy level. Their thresholds are fixed or absolute.
- **Stop bass pumping.** Raise **SC HPF** until the Compressor stops reacting to the kick or the sub.
- **Keep the sub clean.** Turn **LOCK** on and set **X-Over** just above the sub fundamental.
- **Gate before you lift.** Upward compression raises noise. Set **Gate** above the noise floor before you raise **Upward**.
- **Use parallel settings.** Lower **Mix** on the Compressor or the Clipper to keep some of the original dynamics.
- **Compare at equal loudness.** Use **Volume** to match the bypassed level, then toggle **BYPASS**.
- **Pump only the lows.** Turn **SUB ONLY** on to duck the bass and keep the highs steady.
- **Reset after a preset.** Presets do not store every parameter. Check **PHAT**, **Gate**, and the Pumper **Mode** after you switch.
- **Leave headroom.** Master **CLIP** is on by default and cuts at 0 dBFS. Lower **Volume** if you need a lower peak level.

---

## 8. Troubleshooting & FAQ

**Q: Why is there no AU (Audio Unit) version?**  
A: WvZz Dynamics is built as VST3 and Standalone only. Use a VST3 host or the Standalone app.

**Q: Why do I hear a thin, filtered sound?**  
A: **LISTEN** is on in SUB & DETECTOR. It sends the detector signal to the output. Turn it off.

**Q: Why does the Rhythmic Pumper do nothing?**  
A: Check that **Amount** is above 0.0. In `Audio / Ext SC` mode, the trigger signal must rise above -24 dBFS.

**Q: Why does the Compressor not react to my sidechain track?**  
A: Check that the sidechain input bus is enabled in the host. Then check **SC HPF**. A high value removes most of a kick.

**Q: Why does the sound change when I move the X/Y pad?**  
A: An axis has a target and a depth. Set **X Depth** and **Y Depth** to 0.0 to turn the pad off.
