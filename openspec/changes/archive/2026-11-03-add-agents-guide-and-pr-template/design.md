# Design

## Context

See `proposal.md` for motivation. The repository already enforces three automated CI checks in `.github/workflows/pr.yml` (`conventional`, `openspec`, and `build-and-test`) and automated semantic releases via `release-please` in `.github/workflows/release-please.yml`. However, AI coding agents and new contributors currently lack a root `AGENTS.md` entry point explaining how these gates, the OpenSpec workflow, and the C++20/JUCE 8 real-time audio architecture fit together, and `.github/PULL_REQUEST_TEMPLATE.md` only contains a minimal checklist without guidance on human-friendly descriptions, file breakdowns, or architectural diagrams.

## Goals / Non-Goals

**Goals:**
- Provide a self-contained, scannable `AGENTS.md` at the repository root combining OpenSpec setup/workflow rules, Git branching and `release-please` commit conventions, C++20/JUCE 8 real-time audio invariants, and a context routing table.
- Upgrade `.github/PULL_REQUEST_TEMPLATE.md` to enforce plain-English, zero-AI-slop descriptions, a dedicated `Key Files Changed` section, optional GitHub-renderable diagrams (`mermaid` or ASCII), and comprehensive test/architectural checklists modeled on `AthenixLab/tattoostudio#483`.
- Surface both `AGENTS.md` and the PR template standards prominently in `README.md`.

**Non-Goals:**
- Changing existing GitHub Actions workflows (`.github/workflows/pr.yml`, `release-please.yml`, `release.yml`) or C++ source code.
- Adding external PR linting bots beyond the existing `pr.yml` checks.

## Decisions

1. **Structure of `AGENTS.md` (Combining `opencode-jev` Conciseness with `tattoostudio` Workflow Rigor)**
   - *Decision*: Organize `AGENTS.md` into eight focused sections:
     1. **What This Repository Is** (modular C++20 / JUCE 8 VST3 & Standalone audio plugin bundle and `WvZzModules` static library).
     2. **Commands & Self-Verification Gate** (`make build`, `make test`, `make package`, `openspec validate --all`).
     3. **Load Only What You Need** (table mapping tasks to `openspec/specs/<capability>/spec.md` and docs).
     4. **OpenSpec Workflow (Mandatory — Explore to Archive)** (including local CLI check `openspec --version`, installation via `npm install -g @fission-ai/openspec@latest` or `npx --yes @fission-ai/openspec@latest`, and the 5-stage flow: `/opsx-explore` -> `/opsx-propose` -> `/opsx-apply` -> `/opsx-sync` -> `/opsx-archive`).
     5. **Git Branching & Release-Please Commits** (`git checkout main && git pull --ff-only origin main`, `<type>/<kebab-case-slug>` branch naming, Conventional Commits regex, PR title squash-merge rule, and semver bump table).
     6. **Invariants — PRs That Break These Are Rejected** (zero audio-thread allocations, `noexcept` lock-free `processBlock()`, SOLID separation between `IDSPModule` and `IModuleView`, `instancePrefix` parameter IDs, macOS Universal Binary `arm64;x86_64` VST3 & Standalone only).
     7. **Hard Rules — Files Owned by Automation** (never hand-edit `.release-please-manifest.json`, `CHANGELOG.md`, `CMakeLists.txt` version, or `README.md` release badge; never commit `build/` or `dist/`).
     8. **Communication & Pull Request Standard (No AI Slop)** (plain English, short sentences, concrete paths and test counts, explicit list of banned AI filler words, adherence to `.github/PULL_REQUEST_TEMPLATE.md`).
   - *Rationale*: Gives any AI agent immediate operational clarity on the first read without bloating context.

2. **Structure of `.github/PULL_REQUEST_TEMPLATE.md`**
   - *Decision*: Include an HTML comment header reminding authors of the Plain English / No AI Slop policy and Conventional Commits PR title requirement, followed by:
     - `## Description` (human-friendly summary of problem, solution, and OpenSpec archive path)
     - `## Type of Change`
     - `## Changes Made` (logical behavioral bullet points)
     - `## Key Files Changed` (Markdown table of important files modified/added and what they do)
     - `## Architecture / Signal Flow Diagram (if applicable)` (instructions and placeholder for GitHub-compatible ```` ```mermaid ```` or ```` ```text ```` diagrams)
     - `## Testing` (checkboxes + `Test Configuration` for OS, Architecture, and DAW/Standalone host)
     - `## Checklist` (unified OpenSpec, SOLID, Real-Time Audio, and Code Quality checklist)
     - `## Screenshots / Visuals (if applicable)`
     - `## Related Issues`
     - `## Additional Notes`
   - *Rationale*: Matches the proven structure of `AthenixLab/tattoostudio/pull/483` while preserving `WvZz`-specific real-time audio, Universal Binary, and OpenSpec checks.

## Risks / Trade-offs

- **[Risk] Contributors or agents forget to archive the OpenSpec change before opening/merging the PR** → Mitigation: Documented prominently in `AGENTS.md`, `README.md`, and `.github/PULL_REQUEST_TEMPLATE.md`, and already enforced by the `openspec` job in `.github/workflows/pr.yml`.
- **[Risk] Mermaid syntax errors in GitHub PR descriptions** → Mitigation: Explicitly note in the PR template and `AGENTS.md` to quote Mermaid node labels containing parentheses or special characters (e.g., `A["BPFilter (ZDF)"]`) or use plain ASCII fenced blocks (`text`).
