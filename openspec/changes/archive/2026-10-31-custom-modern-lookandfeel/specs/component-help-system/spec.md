# Spec Delta

## Purpose
Defines the in-plugin contextual help system, interactive explanation buttons, and plain-language guidance overlays for all sound-shaping components.

## ADDED Requirements

### Requirement: Interactive Component Help Triggers
The UI system SHALL render an interactive help button ("?") on every DSP module panel header and primary complex control group.

#### Scenario: User clicks help button
- **WHEN** user clicks or hovers the "?" button on any module panel
- **THEN** the system activates contextual explanation mode for that specific component

### Requirement: Contextual Plain-Language Explanations
The help system SHALL display a clear, accessible overlay detailing the module's DSP purpose, signal routing position, parameter functions, and recommended sonic applications.

#### Scenario: Reading module explanation
- **WHEN** the help card for a module is opened
- **THEN** the overlay presents short, jargon-free explanations formatted with title, description, and key parameter tips

### Requirement: Intuitive Help Overlay Dismissal
The help overlay SHALL close automatically whenever the user clicks outside the overlay bounds, clicks the "?" button a second time, or presses the Escape key.

#### Scenario: Closing help card
- **WHEN** user clicks anywhere outside the active help bubble or clicks the dismiss button
- **THEN** the overlay disappears smoothly without interrupting active audio playback or knob tweaks

### Requirement: Decoupled Help Content Provider
Help descriptions and parameter guides SHALL be managed via a decoupled content repository interface rather than hardcoded inside UI painting routines.

#### Scenario: Help content query
- **WHEN** a help component queries content for a module identifier
- **THEN** the content provider returns structured metadata containing title, overview, and parameter notes
