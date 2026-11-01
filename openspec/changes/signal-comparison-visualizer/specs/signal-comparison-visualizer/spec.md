# Spec Delta

## Purpose

Provides real-time visual comparison between the dry input signal and the processed output signal across time-domain, frequency-domain, and differential analysis modes.

## ADDED Requirements

### Requirement: Real-Time Dual-Trace Capture
The system SHALL capture incoming raw audio and final post-master audio into pre-allocated lock-free ring buffers on the audio thread with zero memory allocations.

#### Scenario: Audio processing without allocation
- **WHEN** audio is processed through `processBlock()`
- **THEN** raw input samples and final output samples are written to lock-free FIFOs without allocating heap memory or acquiring locks

### Requirement: Dual Oscilloscope Time-Domain View
The system SHALL render an oscilloscope view displaying both input and output waveforms overlaid on a shared zero-crossing coordinate plane with contrasting high-visibility colors.

#### Scenario: Real-time waveform rendering
- **WHEN** the oscilloscope mode is selected during active audio playback
- **THEN** the input waveform is rendered in neon cyan and the output waveform is rendered in neon orange on the same axis at up to 60 FPS

### Requirement: Dual Spectrum Analyzer Frequency-Domain View
The system SHALL compute and render frequency spectrum magnitude curves (20 Hz to 20 kHz) for both input and output signals on the UI thread using FFT with Hann windowing.

#### Scenario: Spectrum curve overlay
- **WHEN** the spectrum analyzer mode is selected
- **THEN** both input and output magnitude curves are drawn overlaid across a logarithmic frequency scale from 20 Hz to 20 kHz

### Requirement: Delta Difference Visualization
The system SHALL calculate and render the difference signal between the output and input to isolate the spectral and harmonic content added or modified by the DSP pipeline.

#### Scenario: Difference curve calculation
- **WHEN** the delta mode is selected
- **THEN** the visualizer displays the differential signal magnitude, revealing the added harmonic saturation, flanger notches, and filter resonance

### Requirement: Visualizer Mode Switching and Controls
The system SHALL provide interactive controls to switch between Oscilloscope (default), Spectrum Analyzer, and Delta views, along with vertical gain scaling and display freeze functionality.

#### Scenario: Mode selection and freeze toggle
- **WHEN** user clicks a mode button or the freeze toggle
- **THEN** the display immediately transitions to the selected analysis mode, and toggling freeze pauses live buffer updating to allow static waveform inspection
