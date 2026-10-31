---
name: gsdlc-ticket-kickoff
description: "Generic (any project) ticket kickoff for YouTrack or GitHub Issues + OpenSpec: use when the user wants to start, analyze, implement, approve, or resume a ticket — e.g. 'Analyze <ID>', 'Let's do <ID>', 'Work on / Implement / Start <ID>', 'Proceed <ID>', 'Continue <ID>', 'Vamos a hacer <ID>', '/gsdlc-ticket-kickoff <ID>'. Detects where an earlier session stopped, reads the ticket, creates the branch from the fresh default branch, checks drift against the code and openspec/specs/, writes and validates the OpenSpec change (proposal, specs, design, tasks) BEFORE asking for approval, reports with links to those files, and on approval re-validates, implements automatically, and runs the commit / push / PR close-out. Defers to a project-local sdlc-ticket-kickoff. Not for hands-on testing of a finished branch (gsdlc-manual-test)."
user-invocable: true
argument-hint: "<ticket id>"
---

# gsdlc Ticket Kickoff & Alignment

> **Step 0 (mandatory):** follow `~/.agents/skills/gsdlc-config/references/step0.md` with job
> `ticket-kickoff`. A project-local `sdlc-ticket-kickoff` wins.
>
> **Communication contract:** TL;DR first, plain English, every question or gate ends with
> `**Recommendation:** <option> — <reason>`.

**The plan lives in files, not in the chat.** The OpenSpec change (`openspec/changes/<change>/`, name
from `{{openspec.change_name}}`, default `<id>-<slug>`) is written and validated before the user is
asked to approve. Between approval and implementation the user may switch model or harness: the next
session finds the change on disk (Phase 0), validates it again, and implements.

Placeholders: `<ID>` ticket id, `<id>` lowercase, `<change>` change folder name, `<BASE>` =
`{{repo.remote}}/{{repo.default_branch}}` (default `origin/main`), `<branch>` from
`{{repo.branch_pattern}}`.

```text
Trigger <ID>
  -> Phase 0 resume check
       no change ---------> 1 fetch ticket -> 2 branch -> 3 drift -> (3.5 grill) -> 4 write+validate change
                                                                              -> 5 report -> STOP for approval
       plan, approved ----> 5.5 re-validate -> implement -> (manual test) -> 6 close-out
       plan, no approval -> 5 report again
       tasks partly done -> 5.5
       all tasks done ----> 6
```

If OpenSpec is disabled (Step 0), write the same four sections into one plan file
`.agents/plans/<id>-<slug>.md` instead of a change folder, and skip the CLI validation (keep the project
rules check).

---

## Core principles

1. **Fixed output.** Use the headers, tables, and order of the report templates. Not applicable →
   `N/A — <reason>`.
2. **No application code before approval.** Before **Proceed**, only: create/switch the ticket branch,
   write files inside the change folder, and commit them locally (no push). No tracker writes.
3. **Spec-anchored planning.** Ground checks in `openspec/config.yaml` and the living specs.
4. **Files are the source of truth.** After Phase 4, the chat summarizes and links; every later step
   re-reads the files.

---

## Phase 0: Resume check (always first)

1. `openspec list --json` → an active change whose name starts with `<id>-` (ignore case)?
2. If yes: `git branch --list "*<ID>*" "*<id>*"` and `openspec instructions apply --change "<change>" --json`.
   Switch to the ticket branch if needed (warn first if the tree is dirty).
3. Route:

| What you find | State | Next |
| :--- | :--- | :--- |
| No active change | New ticket | Phase 1 |
| Change, `0/N` tasks, message approves ("Proceed <ID>", "approve", "go ahead") | Approved, not started | Phase 1 (ticket only), Phase 2, then 5.5. Skip Phase 3 |
| Change, `0/N` tasks, no approval | Awaiting approval | Show the Phase 5 report again; offer to redo Phase 3 if `<BASE>` moved since the plan commit |
| Some tasks done | In progress | Phase 5.5 |
| All tasks done | Ready for close-out | Phase 6 |

Output only when a change was found:

```markdown
### Phase 0 — Resume
| Field | Value |
| :--- | :--- |
| Change | `openspec/changes/<change>/` |
| Branch | `<branch>` |
| Plan commit | `<sha>` — `<plan commit subject>` |
| Tasks | `<done>/<total>` |
| State | Awaiting approval / Approved, not started / In progress / Ready for close-out |
| Next step | Phase <n> — <one line> |
```

---

## Phase 1: Fetch the ticket

Read `<ID>` with the tracker adapter: summary, description, fields, comments, linked ADRs.
With `tracker.type: none`, use the user's text as the ticket and invent no id (ask for a short slug).

---

## Phase 2: Branch from the fresh default branch

1. `git fetch {{repo.remote}} {{repo.default_branch}}`
2. `git status --porcelain` — warn on uncommitted changes.
3. Name: `{{repo.branch_pattern}}` with `{type}` from `{{repo.branch_types}}` (feature for new
   behavior, bugfix for fixes, break for breaking changes).
4. Already on it → confirm. Exists → `git checkout <branch>`. New → `git checkout -b <branch> <BASE>`.

```markdown
### Phase 2 — Branch Status
| Field | Value |
| :--- | :--- |
| Active branch | `<branch>` |
| Base | `<BASE>` @ <sha>, fetched <date> |
| Action | Created / Switched / Already active / Needs rebase |
| Working tree | Clean / Dirty (<files>) |
| Warning | N/A / "Uncommitted changes — confirm before switch" |
```

---

## Phase 3: Code alignment & OpenSpec drift

Never assume the code matches the day the ticket was written.

1. **Read the ticket:** Problem, Goal, Scope, DoD, `## AI Agent Prompt`, linked ADRs.
2. **Read the living specs** the ticket touches (`openspec list --specs`, `openspec show <spec> --type spec`),
   plus every spec in `{{openspec.key_specs}}` for the areas involved. Load the project skills named in
   `{{rules.related_skills}}` for those areas.
3. **Inspect the code** (CodeGraph first when `.codegraph/` exists): prior implementations, renamed files,
   changed contracts. Write down each drift (old assumption → current reality).

```markdown
### Phase 3 — Alignment & Drift
| Area | Status | Evidence |
| :--- | :--- | :--- |
| Prior implementation | None / Partial / Done | `file:line` |
| Drift | None / Found | old → now |
| Ticket prompt usable as-is | Yes / Needs adjustment | reason |
| Specs loaded | <spec ids> | paths |
```

## Phase 3.5: Grill open decisions (only when needed)

Run when Phase 3 leaves user decisions open (drift that changes intent, ambiguous scope or DoD, materially
different designs). Load `gsdlc-grill` with the ticket and Phase 3 findings, skip its grounding steps, grill
only the open branches. Feed the Decision Record into Phase 4. Otherwise note
`Grill: skipped — no open decisions`.

---

## Phase 4: Write the OpenSpec change (before approval)

### 4.1 Scaffold

```bash
openspec new change "<change>"      # skip when Phase 0 found it
```

### 4.2 Write the four artifacts

For each artifact in the order the CLI reports as `ready`:

```bash
openspec status --change "<change>" --json
openspec instructions <artifact-id> --change "<change>" --json
```

| File | Content |
| :--- | :--- |
| `proposal.md` | Why (ticket + ADRs), In Scope / Out of Scope, drift reconciliation, capabilities affected |
| `specs/<capability>/spec.md` | `ADDED` / `MODIFIED` / `REMOVED Requirements`, RFC 2119 words, `#### Scenario:` WHEN/THEN |
| `design.md` | Layer by layer (`{{layers}}`), API contracts, file changes table, decisions, trade-offs, Open Questions |
| `tasks.md` | `- [ ] X.Y` checkboxes ordered by layer (openspec/config.yaml task rule, else `{{layers}}` order, ending with tests and format/lint); each task names its verification |

### 4.3 Validation loop (before approval)

1. `openspec validate "<change>" --strict --json`
2. **Project rules check** (the CLI does not check these): every rule under `rules:` in
   `openspec/config.yaml`, plus `{{rules.*}}` that apply.

On failure: fix format and structure yourself and run both again. If a fix would change scope, behavior, or
a confirmed decision, do not guess: add it to Open Questions (`design.md` + report section 6). Stop after
3 failing rounds, report `Validation: FAIL`, recommend **Refine**.

### 4.4 Commit the plan locally (no push)

```bash
git add openspec/changes/<change>/
git commit -m "<commit_pattern with summary 'OpenSpec plan: <ticket title>'>"
```

After a **Refine**: update, re-run 4.3, new commit `… OpenSpec plan: refine <what>`.

---

## Phase 5: Kickoff report & STOP

````markdown
# Ticket Analysis: [<ID>] <Title>

> **TL;DR** — NEEDS YOUR DECISION: <one sentence>. **You need to:** <one action>.

## Overview
<1–3 lines: problem and target.>

### Ticket & Branch
| Field | Value |
| :--- | :--- |
| Ticket | [<ID> — <Title>](<ID-URL>) |
| Stage · Priority · Type | `<stage>` · `<priority>` · Feature / Bug / Breaking |
| Active Branch | `<branch>` (base `<BASE>`, tree clean) |

---

## 1. Description
<2–5 paragraphs: context, user value, technical motivation. No lists.>

<Optional diagram — see Diagram rules.>

## 2. Codebase Alignment & Drift
<Phase 3 table + narrative reconciling ticket text with the code.>

## 3. Functional Implications
Short; detail in `design.md`.
- **Roles affected:** <from `{{project.roles}}`>
- **API contracts:** one line per new or changed route
- **UI / screens:** names and navigation paths

## 4. Non-Functional Implications
One line each; detail in `design.md`. Security & permissions · Sensitive data · Performance · Reliability ·
Observability · Localization · Project invariants touched (`{{rules.invariants}}`).

## 5. Implementation Plan (OpenSpec)
| Field | Value |
| :--- | :--- |
| Change | `openspec/changes/<change>/` |
| Files | [proposal.md](…) · [design.md](…) · [tasks.md](…) · specs: `<capability>` |
| Validation | `--strict`: PASS / FAIL (<n>) · Project rules: PASS / FAIL |
| Plan commit | `<sha>` (local, not pushed) |

| Layer | Tasks | What changes |
| :--- | :--- | :--- |

## 6. Open Questions
<Decision blocks, or `N/A — No open questions.`>

---

## Awaiting Your Approval
- **Proceed:** the agent validates again and implements automatically.
- **Refine:** the agent updates the files, validates, and shows this report again.
- **Clarify:** answer the open questions first.

You can switch model or harness now; in the new session say `Proceed <ID>`.

**Recommendation:** <Proceed | Refine | Clarify> — <reason>.
````

### Diagram rules (section 1)

Optional. Draw one only when it explains faster than text: the change crosses layers/components, 3+ actors
interact, an entity changes state, or relations change. Skip for small or linear changes (no `N/A`).
Type from content: flow → sequence; lifecycle → state; tables → ER; branching logic → flowchart; layout →
box diagram. Rendering: a visual tool if the harness has one (plus a one-line summary); else Mermaid if the
chat renders it; else an ASCII diagram in a `text` block. When unsure, ASCII.

> **STOP.** Do not modify application code until the user says **Proceed**.

---

## Phase 5.5: Approval → validate → implement

1. **Tracker:** move `<ID>` to the in-progress stage (skip if already there).
2. **Reload the plan from the files.** Check user edits after the plan commit:
   `git status --porcelain openspec/changes/<change>/` and `git diff <plan-commit> -- openspec/changes/<change>/`.
   Edited files are the plan; mention them in one line.
3. **Validate again** (same two checks as 4.3). Pass → step 4 at once, do not ask again. Fail → fix format
   yourself; ask one question at a time when a fix changes scope or decisions; stop after 3 rounds. Commit
   plan fixes (`… OpenSpec plan: validation fixes`).
4. **Implement automatically** following `openspec-apply-change` when the project has it (else the CLI
   apply flow in `references/openspec.md`): `tasks.md` layer by layer, verify each task, tick `- [x]`.
   - **Harness has sub-agents and `{{delegates}}` agents exist:** delegate by layer, do not write code in
     the main session. Run independent backend/frontend work in parallel, otherwise in dependency order.
     Each agent gets the change path and its task numbers (they start with no context).
   - **Otherwise:** implement in this session, layer by layer.
5. **Review before close-out:** check each diff against `design.md`, the delta specs, and the DoD. Fix small
   problems directly; re-delegate larger ones. Confirm every task is ticked.

---

## Phase 6: Close-out — commit, approval gate, push & PR

1. **Self-verification:** run `{{commands.format}}`, `{{commands.typecheck}}`, `{{commands.lint}}`,
   `{{commands.test}}` (long ones in the background). Optional: local review with `gsdlc-pr-review`.
2. **Commit:** stage code and `openspec/changes/<change>/` by path (nothing unrelated);
   message from `{{repo.commit_pattern}}`.
3. **Optional manual test** with `gsdlc-manual-test`. Recommend it when the change is user-visible (screen,
   endpoint, permission, status change, message); recommend skipping for refactor/docs/tests/CI. Ask once.
   When it runs, commit its report file and use the latest SHA below.
4. **Push/PR approval gate (mandatory):**

```markdown
### Implementation Complete & Pre-Push Verification
| Field | Value |
| :--- | :--- |
| **Ticket** | [<ID> — <Title>](<ID-URL>) |
| **Branch** | `<branch>` |
| **Commit** | `<sha>` — `<subject>` |
| **OpenSpec Tasks** | `<N>/<N>` complete |
| **Verification** | format PASS · lint PASS · tests PASS (<counts>) |
| **Manual Test** | `<passed>/<total>` (`<report>`) / Skipped — <reason> |

1. **Push + create PR** against `{{repo.default_branch}}`.
2. **Push only.**
3. **Stay local.**

**Recommendation:** <1 | 2 | 3> — <reason>.
```

5. **Create the PR (option 1):** base `{{repo.default_branch}}`; title from `{{repo.pr_title_pattern}}`; fill
   every section of `{{repo.pr_template}}` when it exists, with honest checkboxes; for GitHub Issues add
   `Closes #<n>`. Move the ticket to the review stage. Then:

```markdown
# PR Published: [<ID>] <Title>

> **TL;DR** — DONE: <one sentence>. **You need to:** <one action>.

| Field | Value |
| :--- | :--- |
| **Pull Request** | [#<n> — <title>](<pr url>) |
| **Ticket** | [<ID>](<ID-URL>) — stage `<in progress>` → `<review>` (verified) |
| **Branch** | `<branch>` → `{{repo.default_branch}}` |
| **OpenSpec Change** | `openspec/changes/<change>/` (`<N>/<N>` tasks) |

**Next step:** review with `gsdlc-pr-review`; after approval, "close it" archives + merges.
```
