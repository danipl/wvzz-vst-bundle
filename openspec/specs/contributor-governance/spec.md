# contributor-governance Specification

## Purpose
Defines the operating instructions for AI coding agents and human contributors in `AGENTS.md`, the human-friendly and anti-AI-slop pull request template in `.github/PULL_REQUEST_TEMPLATE.md`, and the contributor onboarding links in `README.md`.

## Requirements

### Requirement: AI Agent Operating Instructions (`AGENTS.md`)
The repository SHALL provide a root `AGENTS.md` file that instructs AI agents on repository architecture, OpenSpec setup and full workflow execution, git branch creation and naming conventions, release-please Conventional Commits rules, non-negotiable audio/SOLID invariants, automation-owned file restrictions, and plain-English communication standards.

#### Scenario: OpenSpec local setup and lifecycle guidance
- **WHEN** an AI agent reads `AGENTS.md` in an environment where the OpenSpec CLI may or may not be installed
- **THEN** `AGENTS.md` provides exact commands to verify or install OpenSpec (`@fission-ai/openspec@latest`) and documents the complete OpenSpec workflow from exploration (`/opsx-explore`), proposal (`/opsx-propose`), and implementation (`/opsx-apply`) through specification sync (`/opsx-sync`) and mandatory archiving (`/opsx-archive`) before merging to `main`

#### Scenario: Branch creation from updated main and naming convention
- **WHEN** an AI agent starts work on a change
- **THEN** `AGENTS.md` instructs the agent to update `main` (`git checkout main && git pull --ff-only origin main`) before creating a branch named `<type>/<kebab-case-description>` where `<type>` is one of `feat`, `fix`, `docs`, `refactor`, `test`, `ci`, or `chore`

#### Scenario: Release-please Conventional Commits and automation-owned files
- **WHEN** an AI agent authors commits or pull requests
- **THEN** `AGENTS.md` requires commit subjects and pull request titles to match `^(fix|feat|docs|refactor|test|ci|chore|revert)(\([^)]+\))?!?: .+`, explains semantic version bump behavior, and forbids manual edits to `.release-please-manifest.json`, `CHANGELOG.md`, `CMakeLists.txt` version stamps, `README.md` release-please badges, `build/`, and `dist/`

### Requirement: Human-Friendly Pull Request Template (`.github/PULL_REQUEST_TEMPLATE.md`)
The repository SHALL provide `.github/PULL_REQUEST_TEMPLATE.md` requiring pull requests to be written in clear, human-friendly plain English without AI filler jargon, and structured with sections for description, change type, behavioral changes made, key files changed with responsibilities, optional GitHub-compatible diagrams (`mermaid` or ASCII), testing and test configuration, OpenSpec/SOLID/real-time audio checklists, screenshots, related issues, and additional notes.

#### Scenario: Pull request author fills out the template
- **WHEN** a contributor or AI agent opens a pull request using `.github/PULL_REQUEST_TEMPLATE.md`
- **THEN** the template guides them to write a plain-English summary free of AI slop terms, list the key files changed and what each file change does, include a GitHub-renderable diagram (`mermaid` or ASCII) when architectural or signal-flow context is useful, and complete the testing and architectural verification checklists

### Requirement: Repository Principles and PR Standards in `README.md`
The root `README.md` SHALL link to `AGENTS.md` and `.github/PULL_REQUEST_TEMPLATE.md` and warn contributors that pull requests MUST adhere to the repository's OpenSpec workflow, branch and Conventional Commit rules, architectural invariants, and human-friendly PR template requirements.

#### Scenario: Contributor or agent reviews `README.md` before contributing
- **WHEN** a contributor or AI agent reads the Documentation Hub or Contributing section in `README.md`
- **THEN** `README.md` directs them to `AGENTS.md` and `.github/PULL_REQUEST_TEMPLATE.md` and states the requirement to follow OpenSpec, Conventional Commits, real-time SOLID principles, and plain-English PR descriptions
