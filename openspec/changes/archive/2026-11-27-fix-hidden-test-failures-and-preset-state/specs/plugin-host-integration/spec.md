# Spec Delta

## ADDED Requirements

### Requirement: Selected Preset Saved with Session State
Every plugin of the bundle SHALL save the index of the selected factory preset with its state and SHALL restore it on load. A state saved without the index SHALL load with preset index `0`.

#### Scenario: Preset index survives a reload
- **WHEN** a plugin has its last preset selected, its state is saved, another preset is selected, and the saved state is loaded
- **THEN** the plugin reports the last preset as current and all parameter values equal the saved ones

### Requirement: Test Checks Run in the Release Build
The automated test target SHALL evaluate every check in the build configuration used by `make test` and CI, and SHALL end with a non-zero exit code and the failed condition on the first failed check.

#### Scenario: A failed check fails the run
- **WHEN** a condition checked by a test is false in a Release build
- **THEN** the test binary prints the condition with file and line and exits with code `1`
