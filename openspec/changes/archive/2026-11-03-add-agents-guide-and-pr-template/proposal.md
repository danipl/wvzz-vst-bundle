# Proposal

## Why

As the WvZz audio plugin repository grows into a collaborative multi-plugin ecosystem with automated CI quality gates and `release-please` versioning, AI coding agents and human contributors need a single, authoritative operating manual (`AGENTS.md`) and a structured, human-friendly pull request template (`.github/PULL_REQUEST_TEMPLATE.md`). Establishing explicit instructions for OpenSpec setup and lifecycle execution, branch and commit conventions, real-time audio invariants, and plain-English PR authoring eliminates AI-generated filler ("AI slop") and prevents CI gate failures on pull requests.

## What Changes

- **Create `AGENTS.md`**:
  - Mandate the OpenSpec workflow from exploration (`/opsx-explore`), proposal (`/opsx-propose`), and implementation (`/opsx-apply`) through specification sync (`/opsx-sync`) and mandatory archiving (`/opsx-archive`) before merging to `main`.
  - Document local OpenSpec CLI verification and installation steps (`@fission-ai/openspec@latest`) when OpenSpec is not yet installed locally.
  - Enforce creating working branches from an up-to-date `main` branch (`git checkout main && git pull --ff-only origin main`) using the repository's branch naming convention (`<type>/<kebab-case-description>`).
  - Specify Conventional Commits and PR title formatting rules required by `.github/workflows/pr.yml` and `release-please` semantic versioning.
  - Define non-negotiable C++20 / JUCE 8 architectural invariants (zero audio-thread heap allocations, lock-free `noexcept` `processBlock()`, SOLID `IDSPModule` / `IModuleView` separation, `instancePrefix` parameter IDs, Universal Binary `arm64;x86_64` VST3/Standalone targets) and automation-owned files that must never be hand-edited.
  - Provide a fast context routing map ("Load Only What You Need") and plain-English communication rules that ban AI filler jargon.
- **Overhaul `.github/PULL_REQUEST_TEMPLATE.md`**:
  - Add clear authoring instructions banning AI slop jargon and requiring plain-English explanations of what changed and why.
  - Include structured sections modeled after collaborative engineering PRs: `Description`, `Type of Change`, `Changes Made`, `Key Files Changed` (highlighting important files and their responsibilities), `Architecture / Signal Flow Diagram` (using GitHub-compatible Mermaid or ASCII diagrams when helpful for context), `Testing` (with `Test Configuration`), `Checklist` (OpenSpec, SOLID, and real-time audio gates), `Screenshots / Audio Examples`, `Related Issues`, and `Additional Notes`.
- **Update `README.md`**:
  - Reference `AGENTS.md` in the Documentation Hub and Contributing sections.
  - Explicitly instruct contributors and AI agents to follow the OpenSpec lifecycle, branch/commit conventions, architectural principles, and the `.github/PULL_REQUEST_TEMPLATE.md` structure (human-friendly descriptions, no AI slop, key files breakdown, and diagrams where useful).

## Capabilities

### New Capabilities
- `contributor-governance`: Defines the repository's AI agent operating instructions (`AGENTS.md`), OpenSpec workflow and setup guidance, branch and commit conventions, pull request template requirements (`.github/PULL_REQUEST_TEMPLATE.md`), and contributor documentation in `README.md`.

### Modified Capabilities
<!-- None -->

## Impact

- **Added Files**: `AGENTS.md`, `openspec/specs/contributor-governance/spec.md` (upon archive).
- **Modified Files**: `.github/PULL_REQUEST_TEMPLATE.md`, `README.md`.
- **Systems Affected**: Repository documentation, AI agent onboarding, and GitHub Pull Request authoring workflow (no C++ DSP or UI runtime code changes).
