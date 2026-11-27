# Spec Delta

## ADDED Requirements

### Requirement: WvZz Modulation Plugin Identity
The build SHALL produce WvZz Modulation as a Universal Binary (`arm64` and `x86_64`) VST3 and Standalone application with `PLUGIN_CODE WzMd`, `PRODUCT_NAME "WvZz Modulation"`, and manufacturer code `WvZz`.

#### Scenario: Packaging includes WvZz Modulation
- **WHEN** `make package` runs
- **THEN** `WvZz Modulation.vst3` and `WvZz Modulation.app` are found, contain `arm64` and `x86_64`, and pass `codesign --verify --deep --strict`

### Requirement: Tail Length Reporting
A plugin whose effects keep sounding after the input stops SHALL report a tail length to the host that covers its longest feedback decay. WvZz Modulation SHALL report `2.0` seconds.

#### Scenario: Host asks for the tail length
- **WHEN** the host calls the tail length query on WvZz Modulation
- **THEN** the plugin returns `2.0` seconds
