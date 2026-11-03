# AGENTS.md

> [!IMPORTANT]
> **READ-FIRST POLICY:** AI coding agents and contributors must read this document before planning, modifying, or creating code in this repository.

## 1. What This Repository Is

`wvzz` (`WvZz Audio Plugin Suite`) is a modular, high-performance audio plugin ecosystem and shared DSP library (`WvZzModules`) crafted in modern C++20 and JUCE 8.

- **Brand & Vendor**: Company: `"WvZz"`, Manufacturer Code: `WvZz`, Root C++ namespace: `wvzz`, UI namespace: `wvzz::ui`.
- **Target Formats**: Strictly **macOS Universal Binary** (`arm64;x86_64`) **VST3** and **Standalone** desktop applications. (AU is not supported).
- **Core Architecture**: Headless, real-time safe DSP modules (`IDSPModule`) decoupled from GUI views (`IModuleView`), orchestrated via pre-allocated `ProcessContext` structures.

---

## 2. Commands & Self-Verification Gate

Run these commands locally to verify changes before committing or opening a pull request:

```bash
make build                # Configures CMake with Ninja and builds all targets (Release)
make test                 # Runs automated zero-allocation DSP tests (wvzz_dsp_tests)
make package              # Verifies Universal Binary (arm64 + x86_64) lipo & codesign
openspec validate --all   # Validates all OpenSpec change and durable specifications
```

> [!NOTE]
> Pull request CI (`.github/workflows/pr.yml`) strictly gates mergeability on Conventional Commits, zero unarchived OpenSpec changes, and Universal Binary build & test passes.

---

## 3. Load Only What You Need

Do not read the whole repository into context. Load only the specifications and docs relevant to your task:

| When your task involves... | Read |
| :--- | :--- |
| **New module or audio DSP math** | [`openspec/specs/dsp-engine/spec.md`](openspec/specs/dsp-engine/spec.md) and [`Source/Modules/`](Source/Modules/) |
| **Plugin architecture / slots / routing** | [`openspec/specs/modular-architecture/spec.md`](openspec/specs/modular-architecture/spec.md) and [`openspec/specs/plugin-host-integration/spec.md`](openspec/specs/plugin-host-integration/spec.md) |
| **LFO, Envelope, or Macro modulation** | [`openspec/specs/modulation-system/spec.md`](openspec/specs/modulation-system/spec.md) |
| **UI components, vector graphics, knobs** | [`openspec/specs/plugin-ui-system/spec.md`](openspec/specs/plugin-ui-system/spec.md) |
| **Visualizers (Oscilloscope, Spectrum)** | [`openspec/specs/signal-comparison-visualizer/spec.md`](openspec/specs/signal-comparison-visualizer/spec.md) |
| **Help system / tooltips / overlays** | [`openspec/specs/component-help-system/spec.md`](openspec/specs/component-help-system/spec.md) |
| **CI/CD, release packaging, semver** | [`docs/CICD.md`](docs/CICD.md) and [`openspec/specs/cicd-release-pipeline/spec.md`](openspec/specs/cicd-release-pipeline/spec.md) |
| **Contributor & PR standards** | This file (`AGENTS.md`) and [`.github/PULL_REQUEST_TEMPLATE.md`](.github/PULL_REQUEST_TEMPLATE.md) |

---

## 4. OpenSpec Workflow (Mandatory — Explore to Archive)

All non-trivial modifications must follow the OpenSpec specification lifecycle.

```text
+-------------------------------------------------------------------------+
| OPENSPEC LIFECYCLE                                                      |
|                                                                         |
|   [Explore] -----> [Propose] -----> [Apply] -----> [Sync & Archive]     |
|  /opsx-explore   /opsx-propose    /opsx-apply       /opsx-archive       |
|                  (proposal,       (code + tests:   (moves change to     |
|                   design,          make build,      changes/archive/    |
|                   specs, tasks)    make test)       & updates specs/)   |
+-------------------------------------------------------------------------+
```

### Local OpenSpec Setup
If OpenSpec is not installed or available in your shell:
1. Verify OpenSpec CLI:
   ```bash
   openspec --version
   ```
2. If command is not found:
   ```bash
   npm install -g @fission-ai/openspec@latest
   ```
   Or invoke dynamically without global installation:
   ```bash
   npx --yes @fission-ai/openspec@latest <command>
   ```
3. Refresh repository agent skills:
   ```bash
   openspec update
   ```

### The 5 Lifecycle Stages
1. **Explore (`/opsx-explore`)**: Thinking and discovery partner. Explore the codebase and specs read-only without implementing.
2. **Propose (`/opsx-propose <change-name>`)**: Scaffolds `openspec/changes/<change-name>/` and generates planning artifacts (`proposal.md`, `specs/<capability>/spec.md`, `design.md`, `tasks.md`).
3. **Apply (`/opsx-apply <change-name>`)**: Implements tasks iteratively, keeping code changes focused and updating task checkboxes (`- [x]`).
4. **Sync (`/opsx-sync`)**: Synchronizes changes in delta specs to durable specifications when implementation decisions diverge.
5. **Archive (`/opsx-archive <change-name>` or `openspec archive <change-name>`)**:
   - **MANDATORY BEFORE MERGING TO `main`**.
   - Archiving merges delta specifications into `openspec/specs/` and moves the change to `openspec/changes/archive/`.
   - CI rejects pull requests if any active directory remains under `openspec/changes/` outside `archive/`.

---

## 5. Git Branching & Release-Please Commits

### Branch Creation from Fresh `main`
Always pull latest `main` before branching:
```bash
git checkout main
git pull --ff-only origin main
git checkout -b <type>/<kebab-case-description>
```

### Branch Naming Convention
Allowed branch prefixes:
- `feat/` — New features or architectural capabilities (e.g. `feat/delay-module`)
- `fix/` — Bug fixes and audio glitch resolutions (e.g. `fix/filter-denormals`)
- `docs/` — Documentation updates and specifications (e.g. `docs/agents-guide`)
- `refactor/` — Code refactoring without behavior changes
- `test/` — Testing improvements or test harness updates
- `ci/` — GitHub Actions, Makefile, or release pipeline changes
- `chore/` — Tooling, dependency, or housekeeping tasks

### Commit Messages & PR Titles (Release-Please Compatible)
Every non-merge commit subject and the **Pull Request title** must conform to Conventional Commits:
```text
^(fix|feat|docs|refactor|test|ci|chore|revert)(\([^)]+\))?!?: .+
```

> [!IMPORTANT]
> Because the repository uses GitHub **Squash and Merge**, the **PR Title becomes the commit subject on `main`** and directly drives automated semantic versioning:

| Commit / PR Title Type | Version Bump | Release-Please Action |
| :--- | :--- | :--- |
| `fix:` | Patch (`v0.1.0` → `v0.1.1`) | Generates release notes, triggers compilation & release asset upload |
| `feat:` | Minor (`v0.1.0` → `v0.2.0`) | Generates release notes, triggers compilation & release asset upload |
| `feat!:` or `BREAKING CHANGE:` | Major (`v1.0.0` or minor while pre-1.0) | Major version bump |
| `docs:`, `chore:`, `refactor:`, `test:`, `ci:` | None | No release triggered |

---

## 6. Invariants — PRs That Break These Are Rejected

1. **Real-Time Audio Safety (Strict Guarantee)**:
   - Absolutely **zero dynamic heap allocations** (`new`, `malloc`, `std::vector::push_back`, `juce::String` instantiation) inside `process()`, `processBlock()`, or any method called on the audio thread.
   - **Zero mutex locks** or thread synchronization on the audio thread.
   - All audio DSP processing methods must be marked `noexcept`.
   - Continuous verification: `make test` executes heap-allocation assertion hooks.
2. **SOLID Architecture & Module Decoupling**:
   - `IDSPModule` (DSP math) must **never** include GUI headers, juce_gui_basics, or know about `IModuleView`.
   - Host engines interact with modules polymorphically via `IPluginModule` and pre-allocated `ProcessContext`.
   - All module parameters must support multi-instance prefixing (`instancePrefix`).
3. **Target Platform Integrity**:
   - Target is macOS Universal Binary (`arm64` Apple Silicon & `x86_64` Intel) for **VST3** and **Standalone** desktop app.
   - All built bundles must pass `codesign --verify --deep --strict`.

---

## 7. Hard Rules — Files Owned by Automation

Never manually edit or hand-bump the following files:
- `CMakeLists.txt` project `VERSION` line (bumped by release-please)
- `README.md` release version badge (`x-release-please-version`)
- `.release-please-manifest.json` (release-please state manifest)
- `CHANGELOG.md` (generated by release-please)
- `build/` and `dist/` directories (ephemeral build artifacts)

Editing `release-please-config.json` is permitted only when intentionally altering release automation rules.

---

## 8. Communication & Pull Request Standard (No AI Slop)

Contributors and AI agents must adhere to strict plain-English communication:
- **Plain English**: Short, direct sentences (under 20 words where possible). Explain what changed and why.
- **No AI Slop / Filler Jargon**: Never use AI buzzwords or filler phrases:
  *Forbidden words*: *delve, leverage, robust, seamless, comprehensive, crucial, holistic, streamline, cutting-edge, game-changer, pivotal, tapestry, "under the hood", "in today's landscape", "out of the box", "testament to"*.
- **Concrete Facts Over Fluff**: State exact file paths, test metrics, and numbers (e.g., "14 tests passed, 0 allocations" rather than "tests look robust").
- **Mandatory PR Template**: Every PR must fill out [`.github/PULL_REQUEST_TEMPLATE.md`](.github/PULL_REQUEST_TEMPLATE.md) completely, including the **Key Files Changed** breakdown and a **GitHub-renderable diagram** (`mermaid` or ASCII) whenever architectural context helps reviewers.
