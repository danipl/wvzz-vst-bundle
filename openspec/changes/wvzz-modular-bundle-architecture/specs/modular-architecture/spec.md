# Spec Delta: modular-architecture

## Purpose

Defines the reusable, SOLID-compliant audio module architecture enabling independent digital signal processing, parameter management, UI view generation, and inter-module routing across the WvZz plugin bundle.

## ADDED Requirements

### Requirement: Segregated Plugin Module Contract
The architecture SHALL encapsulate audio processing modules through a segregated contract (`IPluginModule`) separating headless real-time audio processing (`IDSPModule`) from UI view creation (`IModuleView`). Real-time audio routines SHALL NOT instantiate GUI components, depend on window handles, or trigger memory allocations on the audio thread.

#### Scenario: Real-time headless module instantiation
- **WHEN** an audio module is instantiated and prepared for offline rendering or headless unit tests
- **THEN** audio processing and parameter binding operate without requiring any GUI component allocation or display server connection

### Requirement: Unified ProcessContext Audio and Modulation Routing
The DSP processing pipeline SHALL pass a pre-allocated `ProcessContext` structure to every module's `process()` method. The context SHALL provide direct access to the main stereo audio buffer, read-only pre-split sidechain input, auxiliary band-reject buffer, and a dynamic modulation offset lookup function (`getModulationOffset`).

#### Scenario: Auxiliary audio bus exchange
- **WHEN** a filter splitter module outputs rejected frequency bands into the auxiliary band-reject buffer
- **THEN** downstream summation stages recombine the auxiliary buffer with the main audio path without thread locks or dynamic buffer reallocations

#### Scenario: Dynamic modulation target lookup
- **WHEN** modulation sources (LFOs or X/Y pads) modulate an active parameter
- **THEN** the target module resolves its modulation offset from the `ProcessContext` and applies it to its internal smoothed target value in real time

### Requirement: Namespaced Parameter IDs and Multi-Instance Support
Each audio module SHALL accept a configurable instance prefix (defaulting to its canonical prefix such as `dist`, `bp`, `mm`, `flg`, `meq`, `comp`, `mod`, or `master`). All registered parameter IDs, value tree identifiers, and UI attachments SHALL prepend this prefix, allowing multiple instances of the same module class to operate concurrently within a single host processor without identifier collisions.

#### Scenario: Canonical prefix registration
- **WHEN** a module is initialized with its default prefix (e.g. `dist`)
- **THEN** parameter IDs match standard canonical names (`dist_mech`, `dist_tube`, `dist_bit`, `dist_xcita`) ensuring backward compatibility with existing presets

#### Scenario: Multi-instance prefixed registration
- **WHEN** two instances of a module are initialized with distinct prefixes (e.g. `slot1_dist` and `slot2_dist`)
- **THEN** both modules register separate, uniquely namespaced parameter layouts in the host APVTS without collision

### Requirement: Self-Contained Module View and Visual Updates
Each module SHALL provide an on-demand UI factory (`createView()`) returning an `IModuleView` component. The module view SHALL encapsulate its own card frame, header title, accent badge, header bypass toggle, contextual help trigger (`?`), and local control layout within its content bounds, and SHALL expose a visual update callback invoked periodically by the host editor's timer.

#### Scenario: Module UI component layout
- **WHEN** the host plugin editor instantiates and resizes a module view component
- **THEN** the module positions its internal knobs, sliders, and toggles relative to its card content area without relying on absolute parent window coordinates

#### Scenario: Visual animation updates
- **WHEN** the host editor's 60 FPS refresh timer fires
- **THEN** each active module view receives a visual update trigger to animate local meters and visualizers without running independent background timers

### Requirement: Modular Smart Parameter Randomization
Every DSP module SHALL implement a module-specific parameter randomization method (`randomize()`). Randomization SHALL perturb synthesis and effect parameters within musical bounds while strictly respecting module bypass states and stability limits.

#### Scenario: Triggering module randomization
- **WHEN** a global or module-level randomization event is dispatched
- **THEN** the module updates its modulatable parameters within musical ranges without causing uncontrolled filter runaway or clipping
