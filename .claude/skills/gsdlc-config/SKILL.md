---
name: gsdlc-config
description: "Set up or update the per-project settings file (.agents/sdlc.yaml) that the global gsdlc-* SDLC skills read: tracker (YouTrack or GitHub Issues), ticket key, repo and branch rules, OpenSpec, layers, commands, review checks, manual-test stack, release mode. Use when the user says 'set up sdlc for this project', 'configure gsdlc', 'init sdlc config', '/gsdlc-config', or when another gsdlc-* skill finds no sdlc.yaml. Also holds the shared references (Step 0 protocol, tracker adapters, OpenSpec guide, communication standard) the other gsdlc skills load."
user-invocable: true
---

# gsdlc Config — project setup for the generic SDLC skills

The `gsdlc-*` skills are a generic, OpenSpec-based SDLC (ticket → plan → code → review → release).
They know nothing about a specific project. Each repository describes itself once in
**`.agents/sdlc.yaml`**. This skill writes that file.

## Skill family

| Skill | Job |
| :--- | :--- |
| `gsdlc-grill` | Stress-test an idea or plan, one question at a time → Decision Record |
| `gsdlc-ticket-create` | Create tickets / epics / subtasks with a standard structure |
| `gsdlc-ticket-audit` | Check a ticket before work: keep / update / close, right size |
| `gsdlc-ticket-kickoff` | Branch, OpenSpec change, approval, implementation, PR |
| `gsdlc-manual-test` | Guided hands-on test of a branch before the PR |
| `gsdlc-pr-review` | Review a PR or branch, verdict, archive + merge + close ticket |
| `gsdlc-pr-feedback` | Evaluate review comments, update the plan, fix |
| `gsdlc-release` | Semver bump, changelog, tag, GitHub release |

A project-local `sdlc-<job>` skill always wins over `gsdlc-<job>` (see Step 0).

## Shared references (other skills load these)

| File | Content |
| :--- | :--- |
| `references/step0.md` | Step 0 that every gsdlc skill runs first |
| `references/sdlc.template.yaml` | Full config schema with defaults and comments |
| `references/example-youtrack.yaml` | Filled example (Go + React Native, YouTrack, GitHub) |
| `references/tracker-youtrack.md` | YouTrack adapter (MCP → mcporter → REST) |
| `references/tracker-github.md` | GitHub Issues adapter (`gh`, sub-issues, labels / Projects) |
| `references/openspec.md` | OpenSpec commands and ownership rules |
| `references/communication.md` | Default communication standard |

All paths are under `~/.agents/skills/gsdlc-config/`.

---

## Setup workflow

Read-only until the user approves the file.

### 1. Detect

Run from the repository root and collect evidence:

| Value | How to detect |
| :--- | :--- |
| `repo.slug`, `repo.host` | `git remote get-url origin` |
| `repo.default_branch` | `git symbolic-ref refs/remotes/origin/HEAD` (fallback `main`) |
| branch / commit patterns | `git branch -r` and `git log --oneline -50` (look for `type/KEY-123-…`, `[KEY-123] …`) |
| `tracker.key_prefix` | most common `[A-Z]+-[0-9]+` in branch names and commits |
| `tracker.type` | YouTrack MCP server loaded or `youtrack` in `AGENTS.md`/docs → `youtrack`; `gh issue list` works and no other tracker → `github` |
| `openspec.*` | `openspec context --json`; read `openspec/config.yaml` |
| `layers` | top-level folders (`backend/`, `frontend/`, `infra/`, `docs/`) and `openspec/config.yaml` task order |
| `commands.*` | `Makefile` targets, `package.json` scripts, `go.mod`, `pyproject.toml`, CI workflows |
| `delegates` | `.claude/agents/*.md` |
| `rules.*` | `AGENTS.md`, `CLAUDE.md`, `openspec/config.yaml` `context:` |
| `manual_test.*` | `docker-compose.yml`, `.env.example`, `__doc__/` or `docs/` DEVELOPMENT/CONTRIBUTING files |
| `release.*` | `.github/workflows/*` with `workflow_dispatch` + tag, existing tags (`git tag -l`) |
| `communication.standard` | a "Communication Standard" section in `AGENTS.md` |

### 2. Ask only what you could not detect

One question at a time, each with `**Recommendation:**`. Typical open items: tracker type, YouTrack
base URL / project / MCP server name, stage values, release mode.

### 3. Propose

Show the full YAML (start from `references/sdlc.template.yaml`; drop sections that stay at defaults)
and a short table: key, value, evidence. End with
`**Recommendation:** write it — <reason>`.

### 4. Write and check

After approval: write `.agents/sdlc.yaml`. Then check it:

```bash
python3 -c "import sys,yaml; yaml.safe_load(open('.agents/sdlc.yaml'))" 2>/dev/null \
  || ruby -ryaml -e 'YAML.load_file(".agents/sdlc.yaml")'
```

Run one read-only tracker call (read the newest ticket) to prove the tracker settings work.
Do not commit unless the user asks.

### 5. Report

```markdown
# gsdlc Config: <project name>

> **TL;DR** — DONE: wrote `.agents/sdlc.yaml` (<tracker type>, key `<prefix>`). **You need to:** <commit it | nothing>.

| Area | Value | Checked |
| :--- | :--- | :--- |
| Tracker | youtrack `<project>` / github `<repo>` | read <ID>: OK |
| Repo | `<slug>` → `<default branch>` | |
| OpenSpec | root `<root>` / disabled | `openspec context`: OK |
| Commands | fmt / lint / test | |
| Release | <mode> | |

**Next step:** <e.g. "say 'audit TS-123' or 'let's do TS-123'">.
```

## Update workflow

When the file exists and the user wants a change: show the diff of the keys to change, wait for
approval, write, and run the step 4 checks again.
