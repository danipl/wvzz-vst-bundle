# Spec Delta

## Purpose
Defines the visual design language, custom LookAndFeel vector styling, interactive controls, and real-time metering interface of the plugin editor.

## ADDED Requirements

### Requirement: Modern Vector Rotary Knobs
The UI system SHALL render custom vector rotary knobs featuring dynamic arc progress tracks, center needle pointers, value readouts, and linear vertical drag tracking.

#### Scenario: User adjusts knob value
- **WHEN** user clicks and drags a rotary knob vertically or uses the mouse wheel
- **THEN** the knob arc fills smoothly and updates the underlying APVTS parameter value in real time

### Requirement: Modular Section Grouping
The plugin editor SHALL organize the signal chain into clearly delineated visual cards representing BP Filter, Distortion, MM Filter, Flanger, Magic EQ, Compressor, and Master sections.

#### Scenario: Visual layout hierarchy
- **WHEN** the plugin window opens in any host DAW
- **THEN** controls for each DSP stage are grouped into distinct, titled module panels with clear hierarchy

### Requirement: Interactive Dual X/Y Vector Pads
The system SHALL provide two interactive 2D coordinate pads that control paired parameters simultaneously with visual crosshairs and draggable pucks.

#### Scenario: X/Y pad dragging
- **WHEN** user drags the puck inside the Filter X/Y pad or Distortion X/Y pad
- **THEN** both horizontal and vertical target parameters update simultaneously with immediate audible effect

### Requirement: Responsive 60 FPS Stereo VU Metering
The UI system SHALL provide smooth, hardware-accelerated stereo peak and RMS level meters driven by a UI timer reading atomic levels from the DSP master stage.

#### Scenario: Audio playback metering
- **WHEN** audio is processed through the plugin
- **THEN** the stereo meters animate fluidly at up to 60 FPS with peak hold and smooth ballistics without causing audio thread dropouts

### Requirement: Bidirectional Parameter Synchronization
All graphical controls SHALL maintain bidirectional synchronization with host DAW automation and preset recall using APVTS parameter attachments.

#### Scenario: Host automation movement
- **WHEN** a DAW automation lane modulates a parameter
- **THEN** the corresponding GUI knob, slider, or toggle updates its displayed position synchronously without fighting the host
