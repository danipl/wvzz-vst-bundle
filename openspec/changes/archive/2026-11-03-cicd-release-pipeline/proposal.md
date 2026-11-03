# Proposal

## Why

The WvZz repository currently builds and tests plugins manually and lacks automated pull request quality gates, OpenSpec archive enforcement, and automated semantic release publishing. As the bundle expands from `WvZzDistortion` to multiple plugins (`WvZzDelay`, `WvZzReverb`, `WvZzModulation`, `WvZzDynamics`), we need a deterministic CI/CD pipeline modeled after `danipl/opencode-jev` that enforces quality on every PR to `main` and automatically compiles, packages, and publishes all VST3 and Standalone macOS Universal Binary plugins through `release-please`.

## What Changes

- **PR Quality Gates Workflow (`.github/workflows/pr.yml`)**:
  - **Conventional Commits & PR Title Linting (`conventional`)**: Validate commit messages and squash-merge PR titles against Conventional Commits (`fix|feat|docs|refactor|test|ci|chore|revert`).
  - **OpenSpec Archive & Spec Validation Gate (`openspec`)**: Enforce that no unarchived OpenSpec changes remain in `openspec/changes/` (outside `openspec/changes/archive/`) on PRs targeting `main`, and run `openspec validate --all` across all living specifications.
  - **macOS Universal Binary Build & Headless DSP Test Gate (`build-and-test`)**: Compile all CMake targets in `Release` mode on `macos-latest`, execute the zero-allocation `wvzz_dsp_tests` suite via CTest, verify Universal Binary architectures (`arm64` and `x86_64`) via `lipo -archs`, and verify ad-hoc bundle signatures via `codesign --verify --deep --strict`.
- **Automated Release Please Workflow (`.github/workflows/release-please.yml`, `release-please-config.json`, `.release-please-manifest.json`)**:
  - Baseline repository version at `0.1.0` (`bump-minor-pre-major: true`, `release-type: "simple"`, `include-component-in-tag: false`).
  - Configure `extra-files` to automatically stamp `CMakeLists.txt` (`project(WvZzBundle VERSION ...)` with `# x-release-please-version`) and `README.md` release badges (`<!-- x-release-please-version -->`).
  - Chain `.github/workflows/release.yml` via `workflow_call` whenever `release_created == 'true'` so automated `GITHUB_TOKEN` tags immediately trigger binary compilation and asset publishing.
- **Multi-Plugin Compilation & Release Packaging Workflow (`.github/workflows/release.yml` & `Makefile`)**:
  - Checkout the release tag `vX.Y.Z` on `macos-latest`, stamp the tag version into `CMakeLists.txt`, compile all registered plugin targets (`*_VST3` and `*_Standalone`), and run `wvzz_dsp_tests`.
  - Automatically discover and bundle all built plugins across `build/*_artefacts/Release/` into two format-specific macOS Universal Binary archives using `ditto`:
    - `WvZz-VST3-vX.Y.Z-macOS-universal.zip` (containing all `.vst3` bundles)
    - `WvZz-Standalone-vX.Y.Z-macOS-universal.zip` (containing all `.app` bundles)
  - Upload both `.zip` archives to the corresponding GitHub Release via `gh release upload`.
- **CI/CD Documentation (`docs/CICD.md` & `README.md`)**:
  - Document the end-to-end release pipeline, Conventional Commits bump rules, OpenSpec archive requirements, and artifact packaging structure.

## Capabilities

### New Capabilities
- `cicd-release-pipeline`: Automated pull request quality gates (Conventional Commits, OpenSpec archive enforcement, macOS Universal Binary compilation, and zero-allocation DSP testing) and `release-please` driven compilation, multi-plugin bundling, and GitHub Release publishing.

### Modified Capabilities

None.

## Impact

- **Build & Versioning (`CMakeLists.txt`, `Makefile`)**: `CMakeLists.txt` version initialized at `0.1.0` with `# x-release-please-version` annotation; `Makefile` extended to support multi-plugin packaging across `build/*_artefacts/Release/`.
- **GitHub Actions (`.github/workflows/pr.yml`, `.github/workflows/release-please.yml`, `.github/workflows/release.yml`)**: Adds automated PR validation and release publishing pipelines.
- **Release Bookkeeping (`release-please-config.json`, `.release-please-manifest.json`, `CHANGELOG.md`)**: Introduces automated semantic versioning and changelog generation on `main`.
- **Documentation (`README.md`, `docs/CICD.md`)**: Adds release version badge and full CI/CD architecture documentation.
