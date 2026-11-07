## MODIFIED Requirements

### Requirement: LFO Free and Host Tempo Sync Timing
The system SHALL support both Free running rate (`0.02 Hz` to `30.0 Hz`) and Host Tempo Sync divisions (`4/1`, `2/1`, `1/1`, `1/2`, `1/4`, `1/8`, `1/16`, `1/32` including Dotted and Triplet variants) for LFO 1 and LFO 2, reading live host transport `bpm` and beat position (`ppqPosition`) from `ProcessContext` on every processing sub-block.

#### Scenario: Host tempo-synced LFO rate
- **WHEN** LFO sync mode is enabled and the host provides transport `bpm` (e.g., `128.0` or `150.0 BPM`) and `ppqPosition`
- **THEN** the LFO cycle frequency dynamically tracks the live host `bpm` (locked to `ppqPosition` when transport is playing) and falls back to free accumulation at `120.0 BPM` if host tempo is unavailable

### Requirement: 21-Target Modulation Routing Matrix
The system SHALL allow LFO 1, LFO 2, and the X/Y Pad axes to modulate any parameter in the 22-item Modulatable Targets Enum (`None`, `BP Low Cut`, `BP High Cut`, `BP Both Cuts`, `BP Low Res`, `BP High Res`, `BP BR Mix`, `Dist Mech`, `Dist Tube`, `Dist Bit`, `Dist Xcita`, `MM Cutoff`, `MM Res`, `MM Mix`, `Flanger Rate`, `Flanger Amount`, `Magic EQ Amount`, `Magic EQ Tune`, `Comp Amount`, `Master Mix`, `LFO 2 Rate` for LFO 1 only, and `Master Vol`).

#### Scenario: LFO 1 cross-modulating LFO 2 Rate
- **WHEN** LFO 1 target is set to `LFO 2 Rate` with a non-zero depth
- **THEN** LFO 1's output dynamically shifts the instantaneous oscillation rate of LFO 2

#### Scenario: BP Both Cuts modulation
- **WHEN** a modulator targets `BP Both Cuts`
- **THEN** both `bp_low_cut` and `bp_high_cut` shift simultaneously in logarithmic frequency space

#### Scenario: X/Y Pad and LFO combined modulation
- **WHEN** both an LFO and an X/Y axis target the same parameter with non-zero depth
- **THEN** the modulation engine sums their offsets and supplies the combined offset to the audio processor

#### Scenario: Beat-synced Master Volume pumping and gating
- **WHEN** an LFO or X/Y axis targets `Master Vol` with non-zero depth
- **THEN** the master output gain is modulated in real time to produce rhythmic sidechain pumping, reverse-bass swells, or trance gating
