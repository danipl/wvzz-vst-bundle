# Design

## Context

See `proposal.md` for motivation. The WvZz repository is a C++20 / CMake / JUCE 8.0.4 monorepo targeting macOS Universal Binaries (`arm64;x86_64`) in VST3 and Standalone formats. GitHub repository merge settings are already configured as squash-only (`allow_squash_merge: true`, `allow_merge_commit: false`, `allow_rebase_merge: false`, `squash_merge_commit_title: "COMMIT_OR_PR_TITLE"`) with an active `main-protection` branch ruleset where Repository Admin and Maintain roles have bypass permissions (`bypass_mode: "always"`).

We are adapting the three-workflow release architecture from `danipl/opencode-jev` (`pr.yml`, `release-please.yml`, `release.yml`) to a CMake multi-plugin audio bundle with OpenSpec governance.

## Goals / Non-Goals

**Goals:**
- Enforce Conventional Commits, OpenSpec change archival, spec validation, Universal Binary compilation, and zero-allocation DSP unit tests on every PR to `main`.
- Automate semantic versioning from baseline `0.1.0` using `release-please` without manual tag pushing or manual version edits.
- Automatically discover, verify, and package every plugin built under `build/*_artefacts/Release/` into two format-split archives (`WvZz-VST3-vX.Y.Z-macOS-universal.zip` and `WvZz-Standalone-vX.Y.Z-macOS-universal.zip`) and attach them to the GitHub Release.
- Align the `main-protection` ruleset status checks with `pr.yml` jobs so no broken or unarchived PR can merge.

**Non-Goals:**
- Apple Developer ID paid notarization or Apple AudioUnit (AU) builds (AU is explicitly unsupported per project architecture constraints, and ad-hoc signing is the current standard).
- Windows or Linux plugin builds.

## Decisions

### 1. Chained `workflow_call` from `release-please.yml` to `release.yml`
- **Choice**: Invoke `.github/workflows/release.yml` directly as a reusable workflow (`uses: ./.github/workflows/release.yml`) inside `release-please.yml` when `steps.rp.outputs.release_created == 'true'`, while also keeping `on: push: tags: ["v*"]` on `release.yml`.
- **Rationale**: GitHub Actions loop protection prevents tags created by `GITHUB_TOKEN` (which `release-please` uses) from triggering `on: push: tags` workflows. Chaining via `workflow_call` guarantees the release build runs automatically on merge of the Release PR without requiring a Personal Access Token (PAT), while `on: push: tags: ["v*"]` still supports human-pushed emergency tags.
- **Alternatives Considered**: Using a custom PAT or GitHub App token for `release-please` (rejected: introduces secret rotation overhead and extra token management compared to `workflow_call`).

### 2. `release-please` Strategy: `release-type: "simple"` with `extra-files`
- **Choice**: Configure `release-please-config.json` with `"release-type": "simple"`, `"bump-minor-pre-major": true`, `"include-component-in-tag": false`, and `"extra-files": ["CMakeLists.txt", "README.md"]`. Initialize `.release-please-manifest.json` at `"0.1.0"`.
- **Rationale**: Unlike `opencode-jev` (which is a Node package with `package.json`), `wvzz` uses `CMakeLists.txt` as its build manifest. Using `release-type: "simple"` combined with `# x-release-please-version` on line 3 of `CMakeLists.txt` (`project(WvZzBundle VERSION 0.1.0 LANGUAGES C CXX) # x-release-please-version`) and `<!-- x-release-please-version -->` in `README.md` updates all version references inside the Release PR commit itself. Additionally, `release.yml` includes an explicit `sed` version-stamping step from the tag (`${REF#v}`) in the ephemeral runner checkout as a safety net for manual emergency tags.
- **Alternatives Considered**: Maintaining a dummy `package.json` (rejected: pollutes a C++ repository with unused Node metadata).

### 3. OpenSpec Archive Gate in `pr.yml`
- **Choice**: Add an `openspec` job in `pr.yml` that:
  1. Verifies no subdirectories exist under `openspec/changes/` other than `archive` (or uses `openspec list --json` to assert `.changes | length == 0`).
  2. Runs `npx --yes @fission-ai/openspec@latest validate --all` to validate all specs under `openspec/specs/`.
- **Rationale**: In the OpenSpec workflow, feature specs are developed inside `openspec/changes/<change-id>/` and must be archived via `openspec archive` (which syncs delta specs into `openspec/specs/` and moves the change to `openspec/changes/archive/`) before the PR merges to `main`. Checking both the absence of active unarchived changes and the validity of `openspec/specs/` guarantees `main` always reflects the canonical specification state.
- **Alternatives Considered**: Auto-archiving inside CI on PR merge (rejected: archiving mutates files in the repository and requires spec validation review before merge).

### 4. Multi-Plugin Discovery and Format-Split Packaging via `ditto`
- **Choice**: Both `Makefile` (`make package`) and `release.yml` dynamically iterate over `build/*_artefacts/Release/VST3/*.vst3` and `build/*_artefacts/Release/Standalone/*.app`, stage them into `dist/VST3/` and `dist/Standalone/`, and compress each directory using `ditto -c -k --keepParent`:
  - `WvZz-VST3-vX.Y.Z-macOS-universal.zip`
  - `WvZz-Standalone-vX.Y.Z-macOS-universal.zip`
- **Rationale**: When new plugins (`WvZzDelay`, `WvZzReverb`, etc.) are added via `juce_add_plugin()` in `CMakeLists.txt`, JUCE automatically outputs them to `build/<Target>_artefacts/Release/{VST3,Standalone}/`. Wildcard discovery requires zero CI config edits when adding new plugins. Using macOS `ditto -c -k` instead of standard `zip` preserves macOS bundle structure, extended attributes, and ad-hoc code signatures.

### 5. JUCE FetchContent Caching in GitHub Actions
- **Choice**: Cache the CMake FetchContent source directory (`build/_deps/juce-src`) keyed on the JUCE tag (`8.0.4`) in `pr.yml` and `release.yml` using `actions/cache@v4`.
- **Rationale**: Avoids cloning JUCE 8.0.4 on every CI run and reduces macOS runner time.

### 6. GitHub Ruleset (`main-protection`) Alignment
- **Choice**: Update repository ruleset `26422904` (`main-protection`) so that:
  - Required status checks enforce `conventional`, `openspec`, and `build-and-test`.
  - Unconfigured `code_scanning` (`CodeQL`) and `code_quality` rules are removed (since C++ CodeQL default setup is unsupported and `build-and-test` enforces compiler warnings and headless DSP verification), while keeping `copilot_code_review`, `pull_request` (1 approval, squash-only), and Admin/Maintain bypass actors.

## Risks / Trade-offs

- **[Risk] Self-blocking PR when introducing `pr.yml` for the first time** → Before merging the PR that introduces `cicd-release-pipeline`, this change (`openspec/changes/cicd-release-pipeline/`) must itself be archived into `openspec/changes/archive/` on the branch so the new `openspec` CI gate passes on its own PR.
- **[Risk] macOS runner build duration for Universal Binaries (`arm64;x86_64`)** → Mitigated by Ninja parallel builds and caching `build/_deps` via `actions/cache@v4`.
