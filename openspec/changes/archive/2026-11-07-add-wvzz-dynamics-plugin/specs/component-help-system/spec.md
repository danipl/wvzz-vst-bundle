## MODIFIED Requirements

### Requirement: Decoupled Help Content Provider
Help descriptions and parameter guides for both `WvZz Distortion` and `WvZz Dynamics` modules (`det`, `trans`, `ott`, `comp`, `pump`, `clip`, `mod`, `master`, `bp`, `dist`, `mm`, `flg`, `meq`) SHALL be managed via a decoupled content repository interface (`HelpContentRepository`) rather than hardcoded inside UI painting routines.

#### Scenario: Help content query
- **WHEN** a help component in either `WvZz Distortion` or `WvZz Dynamics` queries content for a module identifier
- **THEN** the content provider returns structured metadata containing title, overview, and parameter notes for that module
