# Step 0 — Load the project (every gsdlc skill runs this first)

Run these steps in order before any phase of a `gsdlc-*` skill. Keep the output to one short
"Project" line unless something is missing.

## 0.1 Project-local skill wins

The `gsdlc-*` skills are global copies. A repository may have its own version of the same job.
Look in the repository root (`git rev-parse --show-toplevel`) for a skill with the same job:

| gsdlc skill | Project skill names to look for |
| :--- | :--- |
| `gsdlc-<job>` | `sdlc-<job>` in `.agents/skills/`, `.agent/skills/`, `.claude/skills/`, `.opencode/skills/`, `.codex/skills/` |

Also honor `overrides:` in `.agents/sdlc.yaml` (map `<job>: <project skill name>`).

If a project skill exists: **stop here**, say in one line
`Using the project skill <name> instead of gsdlc-<job>.`, load that skill, and follow it.

## 0.2 Load `.agents/sdlc.yaml`

Search in this order and use the first file found: `.agents/sdlc.yaml`, `.sdlc.yaml`, `.claude/sdlc.yaml`.

- **Found:** read it. Refer to its values as `{{path.to.key}}`. Missing keys use the defaults in
  `~/.agents/skills/gsdlc-config/references/sdlc.template.yaml`.
- **Not found:** say so in one line and offer to run `gsdlc-config` (it detects most values and writes
  the file after the user approves). **Recommendation:** run it — it takes about one minute and every
  later SDLC step depends on it. If the user declines, continue with values detected now
  (git remote, default branch, `openspec/` presence) and ask only for the values this skill needs.

## 0.3 Communication standard

Load `{{communication.standard}}` when set; otherwise
`~/.agents/skills/gsdlc-config/references/communication.md`. It is mandatory for every report.

## 0.4 Tracker adapter

| `tracker.type` | Load |
| :--- | :--- |
| `youtrack` | `~/.agents/skills/gsdlc-config/references/tracker-youtrack.md` |
| `github` | `~/.agents/skills/gsdlc-config/references/tracker-github.md` |
| `none` | No tracker. Tickets are free text from the user; skip tracker writes and say so in reports |

Ticket ids match `{{tracker.key_prefix}}-<number>` (case-insensitive). In the skills, `<ID>` is the id
as written (`TS-123`) and `<id>` is lowercase (`ts-123`). `<ID-URL>` is `{{tracker.issue_url}}` with the
id filled in.

## 0.5 OpenSpec

If `{{openspec.enabled}}` is true, load `~/.agents/skills/gsdlc-config/references/openspec.md`.
If `openspec context --json` reports no root, treat OpenSpec as disabled for this run, say so in the
report, and never run `openspec init` on your own.

## 0.6 Project rules

Read, when present: the root `AGENTS.md` / `CLAUDE.md`, `openspec/config.yaml` (`context` and `rules`),
and `{{rules.*}}` in `sdlc.yaml`. They are constraints for every phase. Do not copy them into the chat.
