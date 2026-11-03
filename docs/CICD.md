# CI/CD — Release & Quality Pipeline

This repository automates pull request quality gates and release publishing using GitHub Actions, [release-please](https://github.com/googleapis/release-please), and OpenSpec governance.

## Architecture

```text
PR to main ───────────────────────────────────────────┐
                                                      │
[conventional]          [openspec]             [build-and-test]
- Lint commit messages  - Zero active changes  - macOS Universal Binary
- Lint PR title           in openspec/changes/   (arm64;x86_64)
                          (must be archived)   - Zero-allocation DSP tests
                        - Validate specs       - Codesign verification
                                                      │
                                    Squash-merge to main
                                                      │
                                                      ▼
                            [release-please] opens/updates Release PR
                            - Bumps .release-please-manifest.json
                            - Staves version into CMakeLists.txt & README.md
                            - Generates CHANGELOG.md
                                                      │
                                    Squash-merge Release PR
                                                      │
                                                      ▼
                            - Creates git tag vX.Y.Z
                            - Creates GitHub Release with notes
                            - Chained workflow_call triggers:
                                                      │
                                                      ▼
                            [release.yml]
                            - Builds all VST3 & Standalone plugins
                            - Runs DSP test verification
                            - Packages format-split zip archives:
                              * WvZz-VST3-vX.Y.Z-macOS-universal.zip
                              * WvZz-Standalone-vX.Y.Z-macOS-universal.zip
                            - Uploads assets to GitHub Release
```

## 1. Conventional Commits & Version Bumps

Release please determines the semantic version bump from Conventional Commits:

| Commit Type | Version Bump | Example |
| :--- | :--- | :--- |
| `fix:` | Patch (`0.1.0` → `0.1.1`) | `fix: prevent filter blowup on extreme drive` |
| `feat:` | Minor (`0.1.0` → `0.2.0`) | `feat: add stereo widening module` |
| `feat!:` or `BREAKING CHANGE:` | Major (`0.1.0` → `1.0.0` after 1.0) | `feat!: overhaul ProcessContext interface` |

> Note: With `bump-minor-pre-major: true`, breaking changes before 1.0.0 trigger minor version bumps.

## 2. OpenSpec Gate

Before a PR can merge to `main`, all work in `openspec/changes/<change-id>/` must be archived:
```bash
openspec archive <change-id>
```
Archiving copies delta specifications into `openspec/specs/` and moves the change to `openspec/changes/archive/`. The `openspec` CI job verifies:
1. No unarchived directories exist in `openspec/changes/` (outside `archive/`).
2. All specs pass `openspec validate --all`.

## 3. Multi-Plugin Packaging

The build system automatically discovers and packages all plugins defined via `juce_add_plugin()` in `CMakeLists.txt`:
- **VST3 Bundle**: `dist/WvZz-VST3-vX.Y.Z-macOS-universal.zip`
- **Standalone Applications**: `dist/WvZz-Standalone-vX.Y.Z-macOS-universal.zip`

Local packaging can be tested at any time:
```bash
make package
```
All binaries are compiled as macOS Universal Binaries (`arm64` and `x86_64`) and ad-hoc code-signed.
