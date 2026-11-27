# WvZz Modulation - User Manual

> Official User Guide for **WvZz Modulation**, part of the **WvZz Audio Plugin Bundle**.

---

## Table of Contents
1. [Overview & Quick Start](#1-overview--quick-start)
2. [Interface Layout & Navigation](#2-interface-layout--navigation)
3. [Module Parameters Guide](#3-module-parameters-guide)
   - [Chorus](#chorus)
   - [Phaser](#phaser)
   - [Flanger](#flanger)
   - [Tremolo](#tremolo)
4. [Global Controls](#4-global-controls)
   - [Top Header Bar](#top-header-bar)
   - [Modulation: LFO 1 & LFO 2](#modulation-lfo-1--lfo-2)
   - [Modulation: X/Y Pad, Monitor & Scope](#modulation-xy-pad-monitor--scope)
   - [Master Section](#master-section)
5. [Factory Presets](#5-factory-presets)
6. [Signal Flow & Routing](#6-signal-flow--routing)
7. [Usage Tips](#7-usage-tips)
8. [Troubleshooting & FAQ](#8-troubleshooting--faq)

---

## 1. Overview & Quick Start

**WvZz Modulation** is a modulation effect plugin for macOS (`VST3` and `Standalone`). It chains a chorus, a phaser, a flanger, and a tremolo.

The plugin exposes 54 parameters: 8 for Chorus, 8 for Phaser, 7 for Flanger, 7 for Tremolo, 18 for Modulation, and 6 for Master.

### 60-Second Quick Start
1. Load **WvZz Modulation** as an insert effect on a track or bus.
2. Pick a preset in the **Preset Selector** in the header:
   - Try `Lush Stereo Chorus` on a pad or a clean guitar.
   - Try `Vintage 4-Stage Phaser` on an electric piano.
   - Try `Jet Flanger` on drums or a noise sweep.
3. Look at the card headers. A preset turns on only the effects that it uses. The other cards stay dimmed.
4. Click the bypass toggle in a card header to turn another effect on.
5. Set **Rate** and **Depth** on the active card to taste.
6. Click the `SCOPE` tab in the center column to see the LFO of each effect.
7. Match the level with **Volume** in the **MASTER** card.

---

## 2. Interface Layout & Navigation

The window has a header bar and three columns.

```text
┌──────────────────────────────────────────────────────────────────────────────────┐
│ [LOGO] CHORUS -> PHASER -> FLANGER -> TREMOLO -> MASTER [<][PRESET][>][DICE][BYPASS] │
├─────────────────────────┬──────────────────────────┬─────────────────────────────┤
│       LEFT COLUMN       │      CENTER COLUMN       │        RIGHT COLUMN         │
│                         │                          │                             │
│  ┌───────────────────┐  │  ┌────────────────────┐  │  ┌───────────────────────┐  │
│  │      CHORUS       │  │  │ [X/Y PAD][MONITOR] │  │  │        FLANGER        │  │
│  │ (Rate, Depth,     │  │  │ [SCOPE]            │  │  │ (Rate, Amount,        │  │
│  │  Delay, Spread,   │  │  │ X/Y pad, signal    │  │  │  Feedback, Depth,     │  │
│  │  Feedback, Mix,   │  │  │ monitor, or LFO    │  │  │  Center, THRU-ZERO)   │  │
│  │  Voices)          │  │  │ phase scope        │  │  └───────────────────────┘  │
│  └───────────────────┘  │  └────────────────────┘  │  ┌───────────────────────┐  │
│  ┌───────────────────┐  │  ┌────────────────────┐  │  │        TREMOLO        │  │
│  │      PHASER       │  │  │       LFO 1        │  │  │ (Rate, Depth, Stereo, │  │
│  │ (Rate, Depth,     │  │  │ (Wave, SYNC, Rate, │  │  │  X-Over, Shape, Mode) │  │
│  │  Center, Feedback,│  │  │  Sync Div, Depth,  │  │  └───────────────────────┘  │
│  │  Stereo, Mix,     │  │  │  Target)           │  │  ┌───────────────────────┐  │
│  │  Stages)          │  │  ├────────────────────┤  │  │        MASTER         │  │
│  └───────────────────┘  │  │       LFO 2        │  │  │ (In Gain, Dry/Wet,    │  │
│                         │  │ (same controls)    │  │  │  Volume, CLIP, MONO)  │  │
│                         │  └────────────────────┘  │  └───────────────────────┘  │
└─────────────────────────┴──────────────────────────┴─────────────────────────────┘
```

- **Window size**: the default size is 1080 × 680 px.
- **Resizing**: drag the bottom-right corner. The window scales from 810 × 510 px to 1920 × 1200 px and keeps its aspect ratio.
- **Card controls**: each effect card has a bypass toggle in its header and a **`?`** help button.
- **Visual feedback**: a bypassed card dims its controls.

### The Three Center Tabs

The top of the center column has three tabs. The two LFO panels below stay visible in every tab.

| Tab | Card title | What it shows |
| :--- | :--- | :--- |
| `X/Y PAD` | `MOD & X/Y` | The X/Y pad with its two targets and two depth knobs. |
| `MONITOR` | `MONITOR` | A display that compares the input and the output signal. The plugin opens on this tab. |
| `SCOPE` | `SCOPE` | The LFO phase scope (see below). |

### The SCOPE Tab

The scope has four lanes, one per effect: `CHORUS`, `PHASER`, `FLANGER`, `TREMOLO`. Each lane uses the color of its card.

In each lane:

- **The curve** is one cycle of the LFO of that effect. Chorus, Phaser, and Flanger use a sine. Tremolo follows its **Shape** selector.
- **The curve height** follows the depth of the effect. More **Depth** gives a taller curve.
- **The filled dot** is the LFO position of the left channel. A thin vertical line marks it.
- **The ring** is the LFO position of the right channel.
- **The label** says `ACTIVE` or `BYPASSED`. A bypassed lane is dimmed.

When the dot and the ring are in the same place, left and right move together. The further apart they are, the wider the stereo movement.

> **Note**: the Flanger lane also shows `BYPASSED` when **Amount** is 0.0, because the Flanger does nothing at that value.

---

## 3. Module Parameters Guide

The modules are listed in signal-chain order. Parameter IDs are the names a host shows for automation.

In every table, a bypass parameter set to `Off` means the module is active. All four effects are bypassed by default. A factory preset turns on the effects that it uses.

---

### Chorus

Card title: `CHORUS`. This module adds delayed copies of the signal. An LFO (low-frequency oscillator, a slow repeating wave) changes each delay time. This shifts the pitch a little and makes the sound wider and thicker.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Header toggle | `chor_bypass` | On / Off | On | Bypasses the module. |
| Rate | `chor_rate` | 0.05 – 10 Hz | 0.8 Hz | Speed of the LFO. |
| Depth | `chor_depth` | 0.0 – 1.0 | 0.5 | How far the LFO moves the delay time. 1.0 is ±5 ms. |
| Delay | `chor_delay` | 2 – 30 ms | 12 ms | Base delay time of the voices. |
| Voices | `chor_voices` | 1 / 2 / 3 / 4 | 3 | Number of delayed copies. The voices are spaced evenly over the LFO cycle. |
| Spread | `chor_spread` | 0.0 – 1.0 | 0.7 | Shifts the LFO of the right channel. 1.0 is 90 degrees. |
| Feedback | `chor_feedback` | 0.0 – 0.7 | 0.0 | Sends the first voice back into the delay. Higher values add a resonant tone. |
| Mix | `chor_mix` | 0.0 – 1.0 | 0.5 | Blend of the dry signal and the chorus voices. |

> **Note**: the voices are scaled by `1 / sqrt(voices)`. More voices sound thicker, not louder.

---

### Phaser

Card title: `PHASER`. This module sends the signal through a row of all-pass filters. An all-pass filter changes the phase of the signal, not its level. Mixed with the dry signal, this makes notches that an LFO sweeps up and down.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Header toggle | `phs_bypass` | On / Off | On | Bypasses the module. |
| Rate | `phs_rate` | 0.02 – 10 Hz | 0.5 Hz | Speed of the sweep. |
| Depth | `phs_depth` | 0.0 – 1.0 | 0.7 | Width of the sweep. 1.0 is two octaves above and below **Center**. |
| Center | `phs_center` | 100 – 4000 Hz | 800 Hz | Middle frequency of the sweep. |
| Stages | `phs_stages` | 2 / 4 / 6 / 8 / 12 | 4 | Number of all-pass filters. More stages make more notches. |
| Feedback | `phs_feedback` | -0.95 – +0.95 | 0.5 | Sends the output back to the input. Higher values make sharper peaks. Negative values change the tone. |
| Stereo | `phs_stereo` | 0.0 – 1.0 | 0.5 | Shifts the LFO of the right channel. 1.0 is 180 degrees. |
| Mix | `phs_mix` | 0.0 – 1.0 | 0.5 | Blend of the dry and the filtered signal. The notches are deepest at 0.5. |

> **Note**: the swept frequency stays between 20 Hz and 45% of the sample rate.

---

### Flanger

Card title: `FLANGER`. This module mixes the signal with a very short delayed copy. An LFO sweeps the delay time. The result is a moving comb-filter sound, often called a "jet" sound.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Header toggle | `flg_bypass` | On / Off | On | Bypasses the module. |
| Rate | `flg_rate` | 0.01 – 20 Hz | 0.4 Hz | Speed of the sweep. |
| Amount | `flg_amount` | 0.0 – 1.0 | 0.0 | Blend of the dry and the delayed signal. At 0.0 the module has no effect. The effect is strongest at 0.5. |
| Feedback | `flg_feedback` | -0.95 – +0.95 | 0.55 | Sends the delayed signal back into the delay. Higher values make a sharper, more metallic sound. |
| Depth | `flg_depth` | 0 – 5 ms | 2.0 ms | How far the LFO moves the delay time above and below **Center**. |
| Center | `flg_center` | 0.5 – 10 ms | 2.5 ms | Middle delay time of the sweep. |
| THRU-ZERO | `flg_tzero` | On / Off | Off | Delays the dry signal by **Center**, so the sweep passes through zero delay. |

> **Note**: the LFO of the right channel is always 90 degrees ahead of the left. The delay time never goes below one sample.

#### Through-Zero Mode
1. With **THRU-ZERO** off, the delayed copy is always later than the dry signal.
2. With **THRU-ZERO** on, the plugin delays the dry signal by **Center**.
3. The swept copy now moves from before the dry signal to after it.
4. At the crossing point, the two signals line up. This gives the strong "tape flange" sound.

Set **Depth** equal to or higher than **Center** so the sweep reaches the crossing point.

---

### Tremolo

Card title: `TREMOLO`. This module moves the level up and down with an LFO. It can also move the left and right channels in opposite directions (auto-pan).

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Header toggle | `trem_bypass` | On / Off | On | Bypasses the module. |
| Rate | `trem_rate` | 0.1 – 20 Hz | 4.0 Hz | Speed of the level change. |
| Depth | `trem_depth` | 0.0 – 1.0 | 0.5 | How far the level drops. At 1.0 the level falls to silence at the lowest point. |
| Shape | `trem_shape` | Sine / Triangle / Square | Sine | Waveform of the LFO. `Square` is smoothed over 2 ms to avoid clicks. |
| Mode | `trem_mode` | Classic / Harmonic | Classic | `Classic` moves the whole signal. `Harmonic` moves lows and highs in opposite phase. |
| Stereo | `trem_stereo` | 0.0 – 1.0 | 0.0 | Shifts the LFO of the right channel. 1.0 is 180 degrees, which gives an auto-pan. |
| X-Over | `trem_xover` | 200 – 2000 Hz | 800 Hz | Split frequency between lows and highs. Used only in `Harmonic` mode. |

#### Modes
- **Classic**: the level of the whole signal moves between full level and `1 - Depth`.
- **Harmonic**: the signal is split at **X-Over** into a low band and a high band. When the low band is loud, the high band is quiet, and the reverse. The overall level changes less, and the tone moves instead.

---

## 4. Global Controls

### Top Header Bar

- **Logo and subtitle**: shows `MODULATION  //  CHORUS, PHASER, FLANGER & TREMOLO`.
- **Signal flow label**: shows `CHORUS -> PHASER -> FLANGER -> TREMOLO -> MASTER`.
- **Preset Selector**: lists the 10 factory presets. The `<` and `>` buttons step through them.
- **DICE**: sets 21 effect knobs to random values inside safe limits. See below.
- **BYPASS**: master bypass. See `master_bypass` in the [Master Section](#master-section).

**What DICE changes**: all knobs on the Chorus and Phaser cards, the five Flanger knobs, and the four Tremolo knobs.

**What DICE does not change**: the bypass toggles, **Voices**, **Stages**, **Shape**, **Mode**, **THRU-ZERO**, Modulation, and Master. DICE does not turn an effect on. You hear a change only on effects that are already active.

---

### Modulation: LFO 1 & LFO 2

The two LFOs sit in the lower part of the center column. They are separate from the LFOs inside the four effects. Each one moves one target.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Wave (LFO 1) | `lfo1_wave` | Sine / Triangle / Saw Down / Saw Up / Square / Sample & Hold / Smooth Random | Sine | Waveform of LFO 1. |
| SYNC (LFO 1) | `lfo1_sync` | On / Off | Off | Off: free rate in Hz. On: rate follows the host tempo. |
| Rate (LFO 1) | `lfo1_rate` | 0.02 – 30 Hz | 1.0 Hz | Speed of LFO 1 when **SYNC** is off. |
| Sync Div (LFO 1) | `lfo1_sync_div` | 24 divisions (see below) | 1/4 | Cycle length of LFO 1 when **SYNC** is on. |
| Target (LFO 1) | `lfo1_target` | 15 choices (see below) | None | Parameter that LFO 1 moves. |
| Depth (LFO 1) | `lfo1_depth` | -1.0 – +1.0 | 0.0 | Amount and direction of the modulation. |
| Wave (LFO 2) | `lfo2_wave` | Same 7 waveforms | Sine | Waveform of LFO 2. |
| SYNC (LFO 2) | `lfo2_sync` | On / Off | Off | Off: free rate in Hz. On: rate follows the host tempo. |
| Rate (LFO 2) | `lfo2_rate` | 0.02 – 30 Hz | 1.0 Hz | Speed of LFO 2 when **SYNC** is off. |
| Sync Div (LFO 2) | `lfo2_sync_div` | 24 divisions (see below) | 1/4 | Cycle length of LFO 2 when **SYNC** is on. |
| Target (LFO 2) | `lfo2_target` | 15 choices (see below) | None | Parameter that LFO 2 moves. |
| Depth (LFO 2) | `lfo2_depth` | -1.0 – +1.0 | 0.0 | Amount and direction of the modulation. |

**Sync divisions**: `4/1`, `2/1`, `1/1`, `1/2`, `1/4`, `1/8`, `1/16`, `1/32`. Each one also exists as dotted (`D`) and triplet (`T`).

**Targets**: `None` plus 14 targets.

| # | Target | What moves | Reach at full depth |
| :--- | :--- | :--- | :--- |
| 1 | `Chorus Rate` | Chorus **Rate** | 3 octaves up or down |
| 2 | `Chorus Depth` | Chorus **Depth** | ±1.0 |
| 3 | `Chorus Mix` | Chorus **Mix** | ±1.0 |
| 4 | `Phaser Rate` | Phaser **Rate** | 3 octaves up or down |
| 5 | `Phaser Center` | Phaser **Center** | 2 octaves up or down |
| 6 | `Phaser Feedback` | Phaser **Feedback** | ±1.0 |
| 7 | `Phaser Mix` | Phaser **Mix** | ±1.0 |
| 8 | `Flanger Rate` | Flanger **Rate** | 3 octaves up or down |
| 9 | `Flanger Amount` | Flanger **Amount** | ±1.0 |
| 10 | `Tremolo Rate` | Tremolo **Rate** | 3 octaves up or down |
| 11 | `Tremolo Depth` | Tremolo **Depth** | ±1.0 |
| 12 | `Master Mix` | Master **Dry/Wet** | ±1.0 |
| 13 | `LFO 2 Rate` | Speed of LFO 2 | 3 octaves up or down |
| 14 | `Master Vol` | Master **Volume** | ±24 dB |

The modulated value always stays inside the range of its parameter. The knob on screen does not move.

`LFO 2 Rate` works only as the target of LFO 1.

**Tempo sync**: while the host plays, a synced LFO is locked to the host beat position. While the host is stopped, it runs free at the host tempo.

---

### Modulation: X/Y Pad, Monitor & Scope

The top of the center column has three tabs: `X/Y PAD`, `MONITOR`, and `SCOPE`.

**X/Y PAD tab** (card title `MOD & X/Y`): drag the puck to move two targets at once. The center of the pad means no change.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Puck, horizontal | `xy_x` | 0.0 – 1.0 | 0.5 | Horizontal position of the puck. |
| Puck, vertical | `xy_y` | 0.0 – 1.0 | 0.5 | Vertical position of the puck. |
| X Target | `xy_x_target` | Same 15 choices as the LFOs | None | Parameter that the horizontal axis moves. |
| Y Target | `xy_y_target` | Same 15 choices as the LFOs | None | Parameter that the vertical axis moves. |
| X Depth | `xy_x_depth` | -1.0 – +1.0 | 0.0 | Amount and direction of the horizontal modulation. |
| Y Depth | `xy_y_depth` | -1.0 – +1.0 | 0.0 | Amount and direction of the vertical modulation. |

**MONITOR tab** (card title `MONITOR`): replaces the pad with a display that compares the input and the output signal. The plugin opens on this tab.

**SCOPE tab** (card title `SCOPE`): replaces the pad with the LFO phase scope. See [The SCOPE Tab](#the-scope-tab).

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

WvZz Modulation has 10 factory presets. The plugin starts on preset 1.

| # | Preset | Use case | Effects turned on |
| :--- | :--- | :--- | :--- |
| 1 | **Lush Stereo Chorus** | Pads, keys, clean guitars. Three voices, medium depth, wide spread. | Chorus |
| 2 | **Dimension Wide** | Widening without clear pitch movement. Four voices, slow and shallow, full spread, 35% mix. | Chorus |
| 3 | **Vintage 4-Stage Phaser** | Electric pianos and guitars. Four stages, 0.4 Hz, centered at 700 Hz. | Phaser |
| 4 | **Deep 12-Stage Sweep** | Slow sweeps on pads and synths. Twelve stages, 0.08 Hz, high feedback. | Phaser |
| 5 | **Jet Flanger** | Drums, noise, and risers. Slow sweep with 0.85 feedback. | Flanger |
| 6 | **Through-Zero Tape Flange** | Tape-style flanging on a full mix or drums. **THRU-ZERO** on, light negative feedback. | Flanger |
| 7 | **Harmonic Tremolo Shimmer** | Guitars and keys. `Harmonic` mode at 5.5 Hz, split at 700 Hz. | Tremolo |
| 8 | **Auto-Pan Pulse** | Left-right movement on hats, arps, and pads. `Classic` mode, triangle shape, 2 Hz, full stereo. | Tremolo |
| 9 | **Ensemble Strings** | String machines and synth pads. Four-voice chorus with a light 2-stage phaser. | Chorus, Phaser |
| 10 | **Psychedelic Swirl** | Heavy movement for leads and effects. All four effects at moderate settings. | Chorus, Phaser, Flanger, Tremolo |

### How a Preset Loads
1. The plugin sets every parameter to its default. This includes Modulation and Master.
2. The plugin applies the values that the preset stores.

So a preset always sounds the same, whatever you loaded before. An effect that the preset does not use goes back to bypassed.

> **Note**: loading a preset also resets **In Gain**, **Volume**, **Dry/Wet**, the LFOs, and the X/Y pad. No factory preset uses LFO 1, LFO 2, or the X/Y pad.

---

## 6. Signal Flow & Routing

```text
[Main In] ──► [Input Gain] ──┬──────────────► (dry copy for Master)
                             │
                             ▼
                    [1. Chorus]
                             │
                             ▼
                    [2. Phaser]
                             │
                             ▼
                    [3. Flanger]
                             │
                             ▼
                    [4. Tremolo]
                             │
                             ▼
                    [Master: Dry/Wet ► Volume ► MONO ► CLIP ► Bypass]
                             │
                             ▼
                        [Stereo Out]
```

- The order of the effects is fixed.
- A bypassed effect passes the signal through unchanged.
- Each effect has its own LFO. LFO 1, LFO 2, and the X/Y pad are extra sources that move the targets.
- The plugin has a stereo input and a stereo output. It has no sidechain input.

### Latency and Tail
- **Latency**: the plugin reports no latency to the host.
- **Through-zero**: with **THRU-ZERO** on, the Flanger delays its dry signal by **Center** (0.5 to 10 ms). The host does not compensate this delay.
- **Tail**: the plugin reports a tail of 2.0 seconds. This covers the decay of the Flanger at maximum feedback.

---

## 7. Usage Tips

- **Start with one effect.** Turn on one card, set it, then add the next. Four active effects can sound busy.
- **Use the scope for stereo.** Open the `SCOPE` tab and move **Spread** or **Stereo**. Watch the ring move away from the dot.
- **Keep the low end steady.** Chorus and phaser can make bass unstable. Turn **MONO** on, or lower **Dry/Wet**.
- **Lower Mix for a soft chorus.** A **Mix** of 0.3 to 0.4 widens the sound without clear pitch movement.
- **Raise Stages for a deeper phaser.** Use 2 or 4 stages for a soft sweep. Use 8 or 12 for a strong one.
- **Tune the flanger with Center.** A short **Center** sounds bright and metallic. A long **Center** sounds lower and hollow.
- **Check timing with THRU-ZERO.** The delay is short, but it can smear a tight drum bus. Compare with the mode off.
- **Keep Dry/Wet at 1.0 with THRU-ZERO.** The master dry copy is not delayed. Mixing it in changes the flange sound.
- **Make an auto-pan.** Set Tremolo **Stereo** to 1.0 in `Classic` mode. Use the `Triangle` shape for an even movement.
- **Sync with LFO 1.** The effect rates are in Hz. For tempo movement, turn **SYNC** on in LFO 1 and target a mix or depth.
- **Watch the feedback.** High **Feedback** on the Phaser or the Flanger raises the level. Lower **Volume** if the output clips.
- **Save your own settings in the host.** Loading a factory preset resets every parameter.

---

## 8. Troubleshooting & FAQ

**Q: Why is there no AU (Audio Unit) version?**  
A: WvZz Modulation is built as VST3 and Standalone only. Use a VST3 host or the Standalone app.

**Q: I turned the Flanger on. Why do I hear nothing?**  
A: **Amount** is 0.0 by default. Raise it to about 0.5. The Flanger presets set it for you.

**Q: Why does DICE change nothing?**  
A: DICE does not turn effects on. Turn on at least one card first, then click DICE.

**Q: Why did my Volume and LFO settings change when I picked a preset?**  
A: A preset resets every parameter to its default before it applies its values. This includes Master and Modulation.

**Q: Why does the X-Over knob on the Tremolo do nothing?**  
A: **X-Over** works only in `Harmonic` mode. Set **Mode** to `Harmonic`.

**Q: Why is the track a little late with THRU-ZERO on?**  
A: Through-zero delays the dry signal by **Center**. The plugin does not report this delay to the host. Lower **Center** or turn the mode off.

**Q: Why does a lane in the SCOPE tab say BYPASSED?**  
A: That effect is bypassed. For the Flanger, it also appears when **Amount** is 0.0.

**Q: Why does the sound change when I move the X/Y pad?**  
A: An axis has a target and a depth. Set **X Depth** and **Y Depth** to 0.0 to turn the pad off.
