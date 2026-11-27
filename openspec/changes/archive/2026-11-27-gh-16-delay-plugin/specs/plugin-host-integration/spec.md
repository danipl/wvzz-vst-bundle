# Spec Delta

## ADDED Requirements

### Requirement: WvZz Delay Plugin Identity
The build SHALL produce WvZz Delay as a Universal Binary (`arm64` and `x86_64`) VST3 and Standalone application with `PLUGIN_CODE WzDl`, `PRODUCT_NAME "WvZz Delay"`, and manufacturer code `WvZz`.

#### Scenario: Packaging includes WvZz Delay
- **WHEN** `make package` runs
- **THEN** `WvZz Delay.vst3` and `WvZz Delay.app` are found, contain `arm64` and `x86_64`, and pass `codesign --verify --deep --strict`

### Requirement: WvZz Delay Tail Length
WvZz Delay SHALL report a tail length computed from its current delay time and feedback: the time until the repeats fall by `60 dB`, at least one delay time and at most `60` seconds.

#### Scenario: Tail grows with feedback
- **WHEN** the delay time is `500 ms`
- **THEN** the reported tail with feedback `0.9` is longer than with feedback `0.2`, and with feedback `1.0` or more it is `60` seconds
