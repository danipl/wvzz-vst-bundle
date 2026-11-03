# cicd-release-pipeline Specification

## Purpose

Defines the automated continuous integration quality gates for pull requests targeting `main` and the automated semantic versioning, multi-plugin Universal Binary compilation, packaging, and GitHub Release publishing pipeline.

## Requirements

### Requirement: Conventional Commits and Pull Request Title Gate
The CI pipeline SHALL validate all non-merge commit subjects in a pull request or push to `main`, as well as the pull request title itself, against the Conventional Commits format (`^(fix|feat|docs|refactor|test|ci|chore|revert)(\([^)]+\))?!?: .+`).

#### Scenario: Pull request with valid Conventional Commits
- **WHEN** a pull request is opened or updated with commit subjects and a PR title matching `^(fix|feat|docs|refactor|test|ci|chore|revert)(\([^)]+\))?!?: .+`
- **THEN** the `conventional` check passes

#### Scenario: Pull request with non-conforming commit or title
- **WHEN** any non-merge commit subject in the PR range or the PR title does not match the Conventional Commits pattern
- **THEN** the `conventional` check fails with a descriptive error message explaining the expected format and bump semantics

### Requirement: OpenSpec Archive and Specification Validation Gate
The CI pipeline SHALL enforce that pull requests targeting `main` and pushes to `main` have zero unarchived active OpenSpec changes remaining in `openspec/changes/` (outside `openspec/changes/archive/`) and that all specifications in `openspec/specs/` pass `openspec validate --all`.

#### Scenario: All OpenSpec changes archived and valid
- **WHEN** a pull request to `main` has no unarchived change directories in `openspec/changes/` and `openspec validate --all` succeeds
- **THEN** the `openspec` check passes

#### Scenario: Unarchived OpenSpec change present in pull request
- **WHEN** a pull request to `main` contains an active change directory under `openspec/changes/<change-id>/` that has not been archived into `openspec/changes/archive/`
- **THEN** the `openspec` check fails and instructs the contributor to archive the change before merging to `main`

### Requirement: Pull Request Universal Binary Build and Zero-Allocation DSP Test Gate
The CI pipeline SHALL compile all CMake targets in `Release` mode on macOS, execute the automated `wvzz_dsp_tests` test suite, verify that every built `.vst3` and `.app` binary contains both `arm64` and `x86_64` Mach-O architectures, and verify that every bundle passes strict code signature verification.

#### Scenario: Pull request build and verification succeeds
- **WHEN** `pr.yml` executes the `build-and-test` job on macOS
- **THEN** all plugin targets compile in `Release` mode, `wvzz_dsp_tests` passes with zero audio-thread heap allocations, and every `.vst3` and `.app` bundle verifies as a signed `arm64` + `x86_64` Universal Binary

### Requirement: Automated Semantic Versioning via Release Please
The repository SHALL automate semantic version bumps, `CHANGELOG.md` updates, `CMakeLists.txt` and `README.md` version stamping, git tag creation (`vX.Y.Z`), and GitHub Release creation on `main` using `release-please` starting from baseline version `0.1.0` with pre-major minor bumping enabled (`bump-minor-pre-major: true`).

#### Scenario: Release-qualifying commit pushed to main
- **WHEN** a `feat:`, `fix:`, `revert:`, or breaking commit is merged to `main`
- **THEN** `release-please` opens or updates a release pull request updating `.release-please-manifest.json`, `CHANGELOG.md`, `CMakeLists.txt`, and `README.md`

#### Scenario: Release pull request merged to main
- **WHEN** the `release-please` pull request is squash-merged into `main`
- **THEN** `release-please` creates the `vX.Y.Z` tag and GitHub Release with generated release notes and immediately chains the release compilation workflow via `workflow_call`

### Requirement: Multi-Plugin Format-Split Release Bundling and Publishing
When a release tag `vX.Y.Z` is published (either chained from `release-please` or via a manual `v*` tag push), the release pipeline SHALL stamp version `X.Y.Z` into `CMakeLists.txt`, build and test all plugins in `Release` mode, package all built `.vst3` bundles into `WvZz-VST3-vX.Y.Z-macOS-universal.zip` and all built `.app` bundles into `WvZz-Standalone-vX.Y.Z-macOS-universal.zip` preserving macOS bundle metadata, and attach both archives to the `vX.Y.Z` GitHub Release.

#### Scenario: Automated compilation and asset upload on release
- **WHEN** `release.yml` runs for tag `vX.Y.Z`
- **THEN** all plugins under `build/*_artefacts/Release/VST3/*.vst3` are bundled into `WvZz-VST3-vX.Y.Z-macOS-universal.zip`, all applications under `build/*_artefacts/Release/Standalone/*.app` are bundled into `WvZz-Standalone-vX.Y.Z-macOS-universal.zip`, and both archives are uploaded to the GitHub Release for `vX.Y.Z`
