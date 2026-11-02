# Design: WvZz Plugin Bundle & Modular SOLID VST3 Architecture

## Context

See `proposal.md` for motivation. Currently, the JUCE codebase in `Source/` defines all parameters in a global `Parameters.h`, manages 18 smoothers in `CamelPhatAudioProcessor`, and manually constructs cards, knobs, and attachments in `CamelPhatAudioProcessorEditor`. To transform this into a bundle of plugins under the **WvZz** brand that can be reused in future VSTs or a multi-FX rack host, the modules must be decoupled, self-contained, and strictly compliant with SOLID principles.

## Goals / Non-Goals

**Goals:**
- **Single Responsibility (SRP)**: Keep real-time DSP, parameter registration/smoothing, and UI view presentation decoupled. Headless DSP runs with zero GUI dependencies.
- **Open/Closed (OCP)**: Allow new modules or multiple instances of an existing module to be added without modifying host processors, editors, or existing module implementations.
- **Liskov Substitution (LSP)**: All modules implement `IPluginModule` and can be manipulated generically by any plugin processor or rack host.
- **Interface Segregation (ISP)**: Segregate audio processing (`IDSPModule`), level metering (`IMeterSource`), and graphical presentation (`IModuleView`).
- **Dependency Inversion (DIP)**: Modules interact with host routing and modulation exclusively via the abstract `ProcessContext` structure.
- **Zero-Allocation Audio Thread**: Maintain zero heap allocations, zero locks, and strict `noexcept` guarantees in all real-time processing blocks.
- **Clean Public Repository**: Provide only standard `openspec-*` skills and workflows for community collaboration, configured for GitHub issues on `danipl/wvzz-vst-bundle`.

**Non-Goals:**
- Building the second VST plugin or a dynamic multi-slot rack plugin in this change.
- Supporting Apple Audio Unit (AU) validation or Windows/Linux platforms.
- Modifying underlying DSP equations, transfer curves, or factory preset parameter values.

## Decisions

### 1. The `IPluginModule` Triad Contract
Each audio section (`BPFilter`, `Distortion`, `MMFilter`, `Flanger`, `MagicEQ`, `Compressor`, `Modulation`, `Master`) is encapsulated in its own directory under `Source/Modules/<ModuleName>/`:
- **`IDSPModule`**: Headless DSP interface providing:
  - `registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)`
  - `bindParameters(juce::AudioProcessorValueTreeState& apvts)`
  - `prepare(const juce::dsp::ProcessSpec& spec)`
  - `reset() noexcept`
  - `process(ProcessContext& ctx) noexcept`
  - `randomize(juce::Random& rng)`
- **`IModuleView`**: Base `juce::Component` created on-demand via `module.createView(apvts, helpHost)`. It encapsulates:
  - Standard card frame (`ModuleCardComponent`) with header title, accent color, bypass toggle, and `?` help button.
  - Self-contained `resized()` layout arranging controls relative to `getContentBounds()`.
  - Visual update hook `void updateVisuals()` called by the host editor's 60 FPS timer.
- **`IPluginModule`**: Descriptor binding the module ID, display name, and DSP/View factories.

*Alternative Considered*: Putting `juce::Component` directly inside the DSP class. Rejected because it couples real-time audio threads and headless CLI tests to GUI headers.

### 2. Pre-Allocated `ProcessContext` for Decoupled Routing
Instead of hardcoding cross-module dependencies in `PluginProcessor`:
- `ProcessContext` provides:
  - `juce::AudioBuffer<float>& mainBuffer` (in-place audio processing)
  - `const juce::AudioBuffer<float>& sidechainInputBuffer` (pre-BP tap for envelope followers)
  - `juce::AudioBuffer<float>& auxBandRejectBuffer` (buffer for BP splitter and recombination)
  - `bool& hasAuxSplit` (active split flag)
  - `float getModulationOffset(int targetId) const noexcept` (fast lookup for LFO/XY offsets)
- Modules query `ctx.getModulationOffset(...)` internally to offset their smoothed parameters, completely decoupling them from the host processor.

*Alternative Considered*: Arbitrary node-graph DAG routing engine. Rejected for now as over-engineering for the current pipeline.

### 3. Configurable Parameter ID Prefixing (`instancePrefix`)
Each module accepts an optional `instancePrefix` in its constructor:
- Default prefix matches the canonical ID (`"dist"`, `"bp"`, `"mm"`, etc.), generating `"dist_mech"`, `"bp_low_cut"`, etc.
- In multi-slot racks, instances receive distinct prefixes (`"slot1_dist"`, `"slot2_dist"`).
- Guarantees multi-instance support while preserving 100% backward compatibility with all 10 existing factory presets and existing spec scenarios.

### 4. Vector `WvZzLogoComponent` & Assets
- Implement `WvZzLogoComponent` in `Source/Core/UI/WvZzLogoComponent.h` using pure `juce::Path` vector geometry. Renders the geometric `WvZz` waveform emblem and product title badge with infinite sharpness across 75% to 200% window scaling.
- Export the matching vector logo to `Assets/wvzz-logo.svg` for repository documentation and GitHub README badges.

### 5. Directory Layout & CMake Library Structure
- `Source/Core/`: Base module interfaces, `ProcessContext`, `WvZzLookAndFeel`, vector UI controls (`XYPad`, `StereoVUMeter`, `SignalComparison`), `WvZzLogoComponent`.
- `Source/Modules/`: Self-contained audio modules (`BPFilter/`, `Distortion/`, `MMFilter/`, `Flanger/`, `MagicEQ/`, `Compressor/`, `Modulation/`, `Master/`).
- `Plugins/WvZzDistortion/`: The flagship multi-effect VST3 & Standalone plugin target.
- `CMakeLists.txt`: Defines static library target `WvZzModules` containing `Source/Core` and `Source/Modules`, linked by `WvZzDistortion` and the headless test runner `wvzz_dsp_tests`.

### 6. Repository Governance & Community Workflow
- Remove non-standard `gsdlc-*` directories from `.agents/skills/` and `.claude/skills/`.
- Track standard OpenSpec skills in `.agents/skills/openspec-*` and `.agents/workflows/opsx-*.md`, with symlinks in `.claude/`.
- Configure `.gitignore` to prevent personal SDLC files from being committed.
- Set git remote to `git@github.com:danipl/wvzz-vst-bundle.git`.

## Risks / Trade-offs

- **[Risk] Control layout shifting when moving from absolute editor coordinates to module-local `getContentBounds()`**
  → *Mitigation*: Carefully translate existing control proportions and bounds inside each module view's `resized()`, keeping the overall 3-column `1080x680` canvas identical.
- **[Risk] Audio-thread latency or overhead with `ProcessContext` modulation lookup**
  → *Mitigation*: Use a pre-allocated flat array with `O(1)` atomic index lookups for `getModulationOffset`, ensuring zero heap allocation and minimal CPU cycles.
- **[Risk] Parameter smoother synchronization during rapid host automation**
  → *Mitigation*: Each module owns its own `juce::SmoothedValue` instances and updates targets once per block inside its `process()` method.
