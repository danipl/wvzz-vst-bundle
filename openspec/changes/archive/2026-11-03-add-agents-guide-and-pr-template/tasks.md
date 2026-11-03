# Tasks

## 1. Author `AGENTS.md` Operating Guide

- [x] 1.1 Create root `AGENTS.md` with project overview, build/test verification commands (`make build`, `make test`, `make package`, `openspec validate --all`), and the "Load Only What You Need" context routing table; verify `AGENTS.md` exists and links to valid repository paths.
- [x] 1.2 Document the mandatory OpenSpec setup and end-to-end lifecycle (`/opsx-explore`, `/opsx-propose`, `/opsx-apply`, `/opsx-sync`, `/opsx-archive`) in `AGENTS.md`, including local CLI check/install instructions (`@fission-ai/openspec@latest`) and the CI unarchived-changes gate; verify all commands match `.github/workflows/pr.yml`.
- [x] 1.3 Document Git branch creation from an updated `main` (`git checkout main && git pull --ff-only origin main`), branch naming conventions (`<type>/<kebab-case-description>`), `release-please` Conventional Commits rules, C++20/JUCE 8 real-time audio & SOLID invariants, automation-owned file restrictions, and plain-English anti-AI-slop rules in `AGENTS.md`; verify regex and version-bump rules match `.github/workflows/pr.yml` and `docs/CICD.md`.

## 2. Update Pull Request Template and `README.md`

- [x] 2.1 Overhaul `.github/PULL_REQUEST_TEMPLATE.md` to require plain-English descriptions without AI slop, a `Key Files Changed` section, optional GitHub-compatible Mermaid/ASCII diagrams, `Type of Change`, `Changes Made`, `Testing` with `Test Configuration`, unified `Checklist`, `Screenshots`, `Related Issues`, and `Additional Notes`; verify the template renders valid GitHub Flavored Markdown.
- [x] 2.2 Update `README.md` (`Documentation Hub` and `Contributing & Community`) to link to `AGENTS.md` and `.github/PULL_REQUEST_TEMPLATE.md` and warn contributors to follow the OpenSpec lifecycle, branch/commit conventions, architectural invariants, and anti-AI-slop PR template requirements; verify all markdown links in `README.md` resolve and `openspec validate --all` passes.

## Workflow follow-up

- Archive the change (`openspec archive add-agents-guide-and-pr-template`) on a working branch created from an up-to-date `main` before opening/merging the pull request.
- Verify `openspec validate --all` passes with zero active unarchived changes.
