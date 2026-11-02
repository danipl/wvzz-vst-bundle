# Proposal: WvZz Plugin Bundle & Modular SOLID VST3 Architecture

## Why

The current codebase is built as a single monolithic plugin (`CamelPhat 3`). While the underlying DSP algorithms are functional, all parameter declarations, smoothers, UI controls, and help content are hardcoded into central monolithic classes (`CamelPhatAudioProcessor`, `CamelPhatAudioProcessorEditor`, and `Parameters.h`). This prevents extracting and reusing audio modules across a suite of plugins, building single-module mini-plugins, or composing a future multi-FX rack host (in the style of Kilohearts Snapins, Soundtoys, or FabFilter).

Transforming the repository into the **WvZz** plugin bundle with a strictly modular, SOLID-compliant architecture enables individual modules (`BPFilter`, `Distortion`, `MMFilter`, `Flanger`, `MagicEQ`, `Compressor`, `Modulation`, `Master`) to be compiled into a shared library, reused across multiple VST3 plugins, and tested in isolation. Rebranding to **WvZz** with **WvZz Distortion** as the flagship release establishes a clean, modern foundation for community collaboration and future bundle expansion.

## What Changes

- **Repository & Brand Migration**:
  - Rebrand all plugin identifiers, metadata, and namespaces from `CamelPhat` / `camelphat` to **`WvZz`** (`COMPANY_NAME "WvZz"`, `PLUGIN_MANUFACTURER_CODE WvZz`, root C++ namespace `wvzz`).
  - Rename the first multi-effect plugin target to **`WvZz Distortion`** (`PRODUCT_NAME "WvZz Distortion"`, `PLUGIN_CODE WzDs`, target `WvZzDistortion`).
  - Restructure project directory hierarchy into:
    - `Source/Core/` (shared interfaces, `ProcessContext`, `WvZzLookAndFeel`, vector UI controls, `WvZzLogoComponent`)
    - `Source/Modules/` (self-contained modules: `BPFilter`, `Distortion`, `MMFilter`, `Flanger`, `MagicEQ`, `Compressor`, `Modulation`, `Master`)
    - `Plugins/WvZzDistortion/` (first flagship VST3 & Standalone plugin target)
    - `Assets/` (vector SVG brand logo for docs)
- **Target Formats**:
  - Focus strictly on macOS Universal Binary (`arm64;x86_64`) **VST3** and **Standalone** desktop application formats (`FORMATS VST3 Standalone`).
  - **BREAKING**: Remove AU (`.component`) bundle generation and Apple `auval` validation targets from build automation.
- **SOLID Modular Architecture**:
  - Introduce `IPluginModule` contract encapsulating headless real-time `IDSPModule`, APVTS parameter registration/binding, internal parameter smoothing, module-level randomization, and on-demand `IModuleView` (`juce::Component`) UI generation.
  - Introduce zero-allocation `ProcessContext` for audio bus routing (`mainBuffer`, `sidechainInputBuffer`, `auxBandRejectBuffer`) and decoupled modulation lookup (`getModulationOffset`).
  - Provide configurable `instancePrefix` per module (defaulting to canonical IDs `"bp"`, `"dist"`, `"mm"`, etc.) enabling multi-instance rack usage while preserving 100% compatibility with all 10 factory presets.
  - Compile core and modules into a reusable CMake static library (`WvZzModules`) linked by `WvZzDistortion` and the headless test harness.
- **Universal Brand Identity & UI**:
  - Implement programmatic vector `WvZzLogoComponent` rendered in the plugin header bar via `juce::Path` with dynamic scaling.
  - Implement matching `Assets/wvzz-logo.svg` for repository documentation.
  - Provide self-contained `IModuleView` cards owning their header title, accent color, header bypass toggle, `?` help button, `HelpEntry` topic, local layout, and 60 FPS visual update hook.
- **OpenSpec Governance & Community Collaboration**:
  - Codify SOLID and modular architectural rules in `openspec/config.yaml`.
  - Maintain only official `openspec-*` skills and workflows in `.agents/skills/`, `.agents/workflows/`, and `.claude/`.
  - Delete non-standard `gsdlc-*` workflows from repo and add ignore rules for personal configurations.
  - Rename offline test harness to `wvzz_dsp_tests` to verify module isolation, zero-allocation processing, multi-instance prefixing, and full plugin integration.

## Capabilities

### New Capabilities
- `modular-architecture`: Defines the segregated `IPluginModule` triad (`IDSPModule`, `IModuleView`, descriptor), `ProcessContext` audio/modulation routing, parameter prefixing (`instancePrefix`), and module-level smart randomization.

### Modified Capabilities
- `plugin-host-integration`: Host format requirements updated to macOS VST3 and Standalone Universal Binaries (`arm64;x86_64`) dropping AU/auval; brand updated to WvZz (`WvZz` manufacturer code, `WzDs` plugin code, product name `WvZz Distortion`); APVTS modular parameter aggregation and verification via `wvzz_dsp_tests`.
- `plugin-ui-system`: Updated for `wvzz::ui::WvZzLookAndFeel`, unified vector `WvZzLogoComponent`, self-contained `IModuleView` card components with header bypass and help triggers, and editor layout composition.

## Impact

- **Build System**: `CMakeLists.txt` updated to project `WvZzBundle`, targets `WvZzModules` (static library), `WvZzDistortion` (JUCE VST3 + Standalone), and `wvzz_dsp_tests`. `Makefile` updated to `WvZz` commands.
- **Source Code**: All `Source/*` files refactored and relocated under `Source/Core/`, `Source/Modules/`, and `Plugins/WvZzDistortion/`. All namespaces migrated to `wvzz::`.
- **Public Repo**: Git remote points to `git@github.com:danipl/wvzz-vst-bundle.git`. Repo skills cleaned to only standard `openspec-*`.
- **Backward Compatibility**: Audio DSP algorithms and factory preset parameter values remain numerically identical to CamelPhat 3 reference specifications.
