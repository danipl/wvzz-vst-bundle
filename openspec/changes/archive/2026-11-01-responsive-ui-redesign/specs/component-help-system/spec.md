# Spec Delta

## MODIFIED Requirements

### Requirement: Contextual Plain-Language Explanations
The help system SHALL display a clear, accessible overlay detailing the module's DSP purpose, signal routing position, parameter functions, and recommended sonic applications, anchored accurately to the triggered `?` button in the scaled editor canvas at any window size.

#### Scenario: Reading module explanation
- **WHEN** the help card for a module is opened at default (`100%`) or resized (`75%`–`200%`) window dimensions
- **THEN** the overlay presents short, jargon-free explanations formatted with title, description, and key parameter tips, clamped within the visible canvas bounds adjacent to the clicked `?` button
