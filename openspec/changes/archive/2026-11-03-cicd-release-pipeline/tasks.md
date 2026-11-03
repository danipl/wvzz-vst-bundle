# Tasks

## 1. Versioning & Multi-Plugin Packaging Build Setup

- [x] 1.1 Update `CMakeLists.txt` project version to `0.1.0` with inline `# x-release-please-version` comment and verify `cmake -B build` configures `CMAKE_PROJECT_VERSION` as `0.1.0`
- [x] 1.2 Create `release-please-config.json` (`release-type: "simple"`, `bump-minor-pre-major: true`, `include-component-in-tag: false`, `extra-files: ["CMakeLists.txt", "README.md"]`) and `.release-please-manifest.json` (`{".": "0.1.0"}`), and verify both files are valid JSON via `jq .`
- [x] 1.3 Add `package` target to `Makefile` that discovers all `.vst3` and `.app` bundles under `build/*_artefacts/Release/`, verifies Universal Binary architectures (`lipo -archs`) and ad-hoc code signatures (`codesign --verify --deep --strict`), and creates `dist/WvZz-VST3-v$(VERSION)-macOS-universal.zip` and `dist/WvZz-Standalone-v$(VERSION)-macOS-universal.zip` via `ditto -c -k --keepParent`; verify by running `make package` and inspecting the generated `.zip` archives

## 2. Pull Request Quality Gates Workflow (`pr.yml`)

- [x] 2.1 Create `.github/workflows/pr.yml` with the `conventional` job (linting commit messages and PR titles against `^(fix|feat|docs|refactor|test|ci|chore|revert)(\([^)]+\))?!?: .+`), the `openspec` job (verifying zero unarchived changes in `openspec/changes/` outside `archive/` and running `openspec validate --all`), and the `build-and-test` job on `macos-latest` (with JUCE FetchContent caching, CMake Release build, `wvzz_dsp_tests`, `lipo -archs`, and `codesign --verify --deep --strict`); verify workflow syntax and test the shell logic locally

## 3. Release Please & Multi-Plugin Release Workflows (`release-please.yml` & `release.yml`)

- [x] 3.1 Create `.github/workflows/release-please.yml` running `googleapis/release-please-action@v4` on `main` and chaining `./.github/workflows/release.yml` via `workflow_call` when `release_created == 'true'`; verify YAML syntax and job outputs
- [x] 3.2 Create `.github/workflows/release.yml` supporting `workflow_call` (`inputs.ref`) and `push: tags: ["v*"]` on `macos-latest` (`contents: write`), stamping the tag version into `CMakeLists.txt`, building all targets, running `wvzz_dsp_tests`, packaging `WvZz-VST3-vX.Y.Z-macOS-universal.zip` and `WvZz-Standalone-vX.Y.Z-macOS-universal.zip` with `ditto`, and uploading both archives via `gh release upload`; verify YAML syntax and packaging commands

## 4. Documentation & Repository Ruleset Alignment

- [x] 4.1 Add the latest release badge with `<!-- x-release-please-version -->` and Releasing section to `README.md`, and create `docs/CICD.md` documenting the release flow, Conventional Commits rules, OpenSpec archive gate, and multi-plugin packaging; verify markdown links and version markers
- [x] 4.2 Update the `main-protection` GitHub repository ruleset (`26422904`) via `gh api` to require the `conventional`, `openspec`, and `build-and-test` status checks and remove the unconfigured `CodeQL` / `code_quality` rules while preserving `pull_request` (squash-only, 1 approval), `copilot_code_review`, and Admin/Maintain bypass actors; verify with `gh api repos/danipl/wvzz-vst-bundle/rulesets/26422904`

## Workflow follow-up

- Archive `cicd-release-pipeline` via `openspec archive "cicd-release-pipeline"` before opening/merging the pull request to `main` so the new `openspec` PR gate passes on its own branch.
- Verify `openspec list --json` returns zero active changes and `openspec validate --all` passes after archiving.
