# Tasks: WvZz Plugin Bundle & Modular SOLID VST3 Architecture

## 1. Repository Governance & Skill Cleanup

- [x] 1.1 Remove non-standard `gsdlc-*` directories from `.agents/skills/` and `.claude/skills/`, retaining only the 6 standard `openspec-*` skills and workflows, and verify via directory listing
- [x] 1.2 Update `.gitignore` to ignore personal `gsdlc-*` skills and `.agents/sdlc.yaml`, and verify via `git status`
- [x] 1.3 Configure git remote `origin` to `git@github.com:danipl/wvzz-vst-bundle.git` and verify via `git remote -v`
- [x] 1.4 Update `openspec/config.yaml` to codify WvZz brand constraints, macOS VST3/Standalone targets, and mandatory SOLID principles

## 2. Core Framework & Brand Vector Assets

- [x] 2.1 Create `Source/Core/Interfaces/` with `IDSPModule.h`, `IModuleView.h`, `IPluginModule.h`, and `ProcessContext.h`
- [x] 2.2 Migrate `CamelPhatLookAndFeel` to `Source/Core/UI/WvZzLookAndFeel` with namespace `wvzz::ui` and verify clean compilation
- [x] 2.3 Implement programmatic vector `Source/Core/UI/WvZzLogoComponent.h` using `juce::Path` with responsive scaling
- [x] 2.4 Create `Assets/wvzz-logo.svg` containing the identical vector brand emblem for documentation and verify file generation
- [x] 2.5 Migrate shared UI controls (`XYPadComponent`, `StereoVUMeterComponent`, `SignalComparisonComponent`, `HelpOverlayComponent`) into `Source/Core/UI/`

## 3. Modular Audio Component Refactoring (`Source/Modules/`)

- [x] 3.1 Implement `Source/Modules/BPFilter/` with `BPFilterDSP`, `BPFilterView`, and `BPFilterModule` supporting configurable `instancePrefix`
- [x] 3.2 Implement `Source/Modules/Distortion/` with 4-stage oversampled DSP, `DistortionView`, and `DistortionModule`
- [x] 3.3 Implement `Source/Modules/MMFilter/` with 12-topology filter, pre-BP sidechain envelope follower, `MMFilterView`, and `MMFilterModule`
- [x] 3.4 Implement `Source/Modules/Flanger/` with stereo quadrature delay DSP, `FlangerView`, and `FlangerModule`
- [x] 3.5 Implement `Source/Modules/MagicEQ/` with resonant sub-bass boost and Phat mode, `MagicEQView`, and `MagicEQModule`
- [x] 3.6 Implement `Source/Modules/Compressor/` with auto-makeup optical compression, `CompressorView` (with GR meter), and `CompressorModule`
- [x] 3.7 Implement `Source/Modules/Modulation/` with dual LFOs, X/Y assignment engine, `ModulationView`, and `ModulationModule`
- [x] 3.8 Implement `Source/Modules/Master/` with oversampling latency delay compensation, I/O level metering, `MasterView`, and `MasterModule`

## 4. Flagship Plugin & Preset Assembly (`Plugins/WvZzDistortion/`)

- [x] 4.1 Implement `Plugins/WvZzDistortion/PluginProcessor` composing the 8 modules, APVTS layout aggregation, and zero-allocation `ProcessContext` processing
- [x] 4.2 Implement `Plugins/WvZzDistortion/PluginEditor` with `WvZzLogoComponent`, header preset toolbar, and 3-column `IModuleView` layout
- [x] 4.3 Migrate all 10 factory presets into `Plugins/WvZzDistortion/Presets/FactoryPresets.h` with `wvzz` namespace and verify parameter ID consistency

## 5. Build Automation & Verification Harness

- [x] 5.1 Refactor `CMakeLists.txt` for `WvZzBundle` (`FORMATS VST3 Standalone`, static library `WvZzModules`, target `WvZzDistortion`, test target `wvzz_dsp_tests`)
- [x] 5.2 Update `Tests/DSPTests.cpp` to verify individual modules, multi-instance parameter prefixing, and full plugin DSP with zero heap allocations
- [x] 5.3 Update `Makefile`, `README.md`, and `MANUAL.md` to reflect `WvZz Distortion` targets and verify `make help`
- [x] 5.4 Execute `make test` (`wvzz_dsp_tests`) and verify 100% test pass rate with zero heap allocations
- [x] 5.5 Execute `make vst3` and `make standalone` to build macOS Universal Binary bundles and verify bundle structures

## 6. Community GitHub Integration

- [x] 6.1 Create `.github/ISSUE_TEMPLATE/` with templates for bug reports, module requests, and plugin ideas
- [x] 6.2 Create `.github/PULL_REQUEST_TEMPLATE.md` with OpenSpec change and SOLID module checklists

## Workflow follow-up

- Validate the OpenSpec change using `openspec validate wvzz-modular-bundle-architecture --strict`.
- Apply tasks through `openspec-apply-change` upon user approval.
- Archive the change after verification and sync specs to main.
