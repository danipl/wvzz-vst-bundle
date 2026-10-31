# OpenSpec in the gsdlc workflow

OpenSpec is a spec tool (CLI `openspec`). It keeps two things in `{{openspec.root}}/`:

- **Living specs** — `specs/<capability>/spec.md`: what the product does today.
- **Changes** — `changes/<name>/` with `proposal.md`, `specs/` (delta specs: what the change adds,
  modifies, or removes), `design.md`, and `tasks.md`. Archiving a change merges its delta specs into
  the living specs and moves the folder to `changes/archive/YYYY-MM-DD-<name>/`.

Steps: **explore → propose → apply → archive**.

## Main rule

One ticket (or one subtask) = one OpenSpec change = one focused agent session = one pull request.
Change name: `{{openspec.change_name}}` (default `<id>-<slug>`, lowercase, e.g. `ts-640-artist-aftercare`).

## Read-only commands

Add `--store {{openspec.store}}` to every command when it is set.

| Command | What it tells you |
| :--- | :--- |
| `openspec context --json` | The OpenSpec root. No root → OpenSpec is off for this run |
| `openspec list --specs --json` | Living spec capabilities and requirement counts |
| `openspec show "<spec>" --type spec --json --no-scenarios` | Short view: purpose and requirement texts |
| `openspec show "<spec>" --type spec` | Full spec with scenarios |
| `openspec list --json` | Active changes (match ticket id in the name, ignore case) |
| `openspec list --archived --json` | Archived changes (`YYYY-MM-DD-<id>-…`) |
| `openspec status --change "<name>" --json` | Which artifacts exist and are done |
| `openspec instructions <artifact> --change "<name>" --json` | Template + project rules for one artifact |
| `openspec instructions apply --change "<name>" --json` | Task progress |
| `openspec show "<name>" --type change --json --deltas-only` | Requirements an active change adds / modifies / removes |
| `openspec validate "<name>" --strict [--json]` | Whether an active change is valid |
| `openspec validate --specs` | Whether the living specs are valid |

Archived changes cannot be opened with `show`: read `changes/archive/<name>/proposal.md` and its `specs/`.

## Writing commands (only in the skill phase that owns them)

| Command | Owner |
| :--- | :--- |
| `openspec new change "<name>"` | `gsdlc-ticket-kickoff` Phase 4 |
| edits to a change's files | kickoff, `gsdlc-pr-feedback`, `gsdlc-manual-test` Phase 6 |
| `openspec archive "<name>"` | `gsdlc-pr-review` close-out |
| `openspec init` / `openspec update` | never, unless the user asks |

## Companion OpenSpec skills

`openspec init` installs `openspec-*` skills and `/opsx:*` commands in a project
(`openspec-explore`, `openspec-propose`, `openspec-apply-change`, `openspec-update-change`,
`openspec-sync-specs`, `openspec-archive-change`). When they exist in the project, follow them for
apply / update / archive. When they do not, do the same with the CLI:

- **Apply:** `openspec instructions apply --change <name> --json`, then do `tasks.md` in order and tick
  each task `- [x]` after its verification passes.
- **Update:** edit the artifact files, then `openspec validate <name> --strict`.
- **Archive:** `openspec archive <name> --yes` (it merges delta specs), then `openspec validate --specs`.

## Project rules

`{{openspec.root}}/config.yaml` may have `context:` and `rules:` (per artifact) and `operations:`.
The CLI does not check `rules:`. The skills check them by hand ("project rules check").
If a `tasks` rule defines a layer order, use it; otherwise use `{{layers}}` order.
