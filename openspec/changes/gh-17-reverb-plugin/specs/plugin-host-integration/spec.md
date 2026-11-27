# Spec Delta

## ADDED Requirements

### Requirement: WvZz Reverb Plugin Identity
The build SHALL produce WvZz Reverb as a Universal Binary (`arm64` and `x86_64`) VST3 and Standalone application with `PLUGIN_CODE WzRv`, `PRODUCT_NAME "WvZz Reverb"`, and manufacturer code `WvZz`.

#### Scenario: Packaging includes all five plugins
- **WHEN** `make package` runs
- **THEN** 5 VST3 bundles and 5 Standalone apps are found, including `WvZz Reverb.vst3` and `WvZz Reverb.app`, each with `arm64` and `x86_64` and a valid signature

### Requirement: WvZz Reverb Tail Length
WvZz Reverb SHALL report a tail length equal to its pre-delay plus its decay time plus the early-reflections length.

#### Scenario: Tail follows the decay setting
- **WHEN** `rev_decay` is `5 s` and `rev_predelay` is `100 ms`
- **THEN** the reported tail is between `5.1` and `5.3` seconds
