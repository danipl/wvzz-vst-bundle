# Proposal

## Why

The current CamelPhat plugin baseline implements a simplified 7-module prototype whose parameter IDs, signal-flow summing order, filter topologies, distortion transfer functions, compressor architecture, modulation system, and GUI layout diverge from the official CamelPhat 3 Technical Specification. Aligning the codebase with the full specification is required to deliver an authentic, phase-coherent, modulatable CamelPhat 3 DSP clone with 10 factory presets and a resizable 3-column interface.

## What Changes

- **BREAKING**: Replace prototype APVTS parameter IDs and ranges in `Source/Parameters.h` with the exact CamelPhat 3 specification parameter contract (`bp_*`, `dist_*`, `mm_*`, `flg_*`, `meq_*`, `comp_*`, `master_*`, `lfo1_*`, `lfo2_*`, `xy_*`, and `input_gain`), including per-sample/sub-block 10ms–20ms parameter smoothing.
- **Signal Flow & Bypass Architecture**: Move the Band-Reject (`bp_br_mix`) summing node to execute immediately after Module 3 (`MM Filter`) and before Module 4 (`Flanger`), tap the `MM Filter` Envelope Follower from the Plugin Input (pre-BP Filter), and add individual hardware-free bypass toggles for every module plus a click-free crossfaded `master_bypass`.
- **Module 1 (BP Filter)**: Implement dual 12 dB/oct Cytomic ZDF trapezoidal SVFs extracting `Low_Rejected = LowPass_Output(Input, bp_low_cut, bp_low_res)`, `High_Rejected = HighPass_Output(Input, bp_high_cut, bp_high_res)`, and `Passband = BandPass_Cascade(Input)`, with independent Low/High resonance (`Q: 0.707` to `12.0`) and log-frequency `bp_link` locking.
- **Module 2 (Distortion)**: Run `Bitcrusher` (`16-bit` to `4-bit`, `1x` to `32x` sample-and-hold) at the base sample rate before 4x oversampling, and implement the exact specification transfer functions and filters for `Mech` (`1.2 kHz` +6 dB pre-emphasis, asymmetric sine wavefolding + negative hard clip, de-emphasis), `Tube` (`tanh(drive * x + 0.1 * x^2) - tanh(0.1 * x^2)` with DC blocker), and `Xcita` (`2.8 kHz` 2-pole Butterworth HPF, `tanh(drive * (0.7 * x_hf + 0.3 * abs(x_hf)))`, `10 Hz` DC blocker).
- **Module 3 (MM Filter)**: Expand from 3 basic filter modes to all 12 required topologies (`LP 12dB`, `LP 24dB` with internal `tanh` feedback saturation, `BP 12dB`, `BP 24dB`, `HP 12dB`, `HP 24dB`, `Notch`, `Peak`, `Comb +`, `Comb -`, `RingMod`, and `Vowel / Formant` morphing across `A-E-I-O-U`), add `mm_mix`, extend `mm_env_rel` to `5000 ms`, and increase `mm_env_depth` to `±6 octaves`.
- **Module 4 (Flanger)**: Upgrade delay line to 3rd-order Lagrange/Hermite fractional interpolation with `2.5 ms` center delay, `±2.0 ms` sweep depth, `0.55` fixed feedback, `90°` stereo quadrature LFO phase offset, `flg_rate` up to `20.0 Hz`, and `flg_bypass` defaulting to `1`.
- **Module 5 (Magic EQ)**: Implement the Pultec-style resonant low-end boost at `meq_tune` (`60`–`120 Hz`, `0` to `+12 dB`) with a complimentary dip at `2 * meq_tune` (`0` to `-2.5 dB`), and `meq_phat` mode placing `tanh` inside the resonant state feedback path alongside a `+3 dB` high-shelf boost at `8 kHz`.
- **Module 6 (Compressor)**: Replace the 5-knob compressor with the specification's one-knob feed-forward compressor where `comp_amount` drives Threshold (`0` to `-36 dB`) and Auto-Makeup Gain (`0` to `+14 dB`), switching between Normal Mode (`3.5 ms` attack, `3.5:1` ratio, `6 dB` soft knee) and Phat Mode (`0.8 ms` attack, `12:1` ratio, `1 dB` hard knee, `-0.1 dBFS` `tanh` ceiling).
- **Modulation System (LFO 1, LFO 2, and X/Y Pad)**: Add two control-rate LFOs (updated every 32 samples with per-sample linear interpolation, 7 waveforms including Sample & Hold and cubic Smooth Random, Free `0.02`–`30 Hz` and Host Tempo Sync `4/1`–`1/32` with Dotted/Triplet variants) and a single macro X/Y Controller Pad (`xy_x`, `xy_y`, `xy_x_depth`, `xy_y_depth`) targeting the 21-item Modulatable Targets Enum (including `LFO 2 Rate` from LFO 1).
- **Master Section, Latency Compensation, Dice & Presets**: Delay the global dry signal path by the oversampler's group delay to prevent phase cancellation on `master_mix`, add the constrained `Dice` smart randomizer, and implement 10 factory presets with XML/JSON serialization.
- **Resizable 3-Column GUI**: Reorganize the editor into a `900 x 560 px` fixed-aspect-ratio window scalable from `75%` to `200%` matching Section 6 (BP dual vertical sliders with passband shading, Center X/Y Pad with target dropdowns + LFO 1/2 live waveform visualizers, Right MM Filter + Flanger + Magic EQ/Comp glowing `P` toggles + Comp GR LED meter + Master VU/Peak meter & Dice button) while keeping the contextual `?` help overlays.

## Capabilities

### New Capabilities
- `modulation-system`: Control-rate LFO 1 and LFO 2 generators (32-sample control rate with per-sample linear interpolation, 7 waveforms, Free and Host Tempo Sync with Dotted/Triplet divisions), the 21-item Modulatable Targets routing matrix (including cross-modulation of LFO 2 Rate by LFO 1), and the assignable macro X/Y Controller Pad with bipolar/unipolar depth offsets.

### Modified Capabilities
- `dsp-engine`: Updates signal flow routing (`BR Mix` summing node pre-Flanger, pre-BP envelope follower tap, per-module bypass toggles), BP Filter ZDF SVF band-reject math and `bp_link`, 4-circuit Distortion equations and base-rate Bitcrusher placement, 12-topology MM Filter, 3rd-order interpolated Flanger, Magic EQ dip and Phat feedback saturation, one-knob auto-makeup Compressor, and Master dry latency compensation and Dice randomization.
- `plugin-host-integration`: Updates the APVTS parameter schema to match the specification parameter tables, reports oversampling latency to the host, adds 10 factory presets with XML/JSON serialization, and expands automated unit tests in `Tests/DSPTests.cpp`.
- `plugin-ui-system`: Updates the editor to a `900 x 560 px` fixed-aspect-ratio resizable window (`75%`–`200%`) with the specification's 3-column layout, BP vertical dual cutoff sliders with passband shading, single assignable X/Y Pad, LFO 1 & 2 live waveform visualizers, glowing `P` buttons, Compressor gain-reduction LED meter, and Master Dice button.

## Impact

- **Code**: `Source/Parameters.h`, `Source/PluginProcessor.h`, `Source/PluginProcessor.cpp`, `Source/DSP/BPFilter.h`, `Source/DSP/DistortionModule.h`, `Source/DSP/MMFilter.h`, `Source/DSP/FlangerModule.h`, `Source/DSP/MagicEQ.h`, `Source/DSP/CompressorModule.h`, `Source/DSP/MasterSection.h`, new `Source/DSP/ModulationEngine.h`, new `Source/Presets/FactoryPresets.h`, `Source/PluginEditor.h`, `Source/PluginEditor.cpp`, `Source/UI/*`, and `Tests/DSPTests.cpp`.
- **Compatibility**: Breaking change to saved DAW parameter automation IDs from the prototype; establishes the canonical v3.0 parameter and preset format.
