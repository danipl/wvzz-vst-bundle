---
name: gsdlc-ticket-audit
description: "Generic (any project) pre-work audit of one ticket, an epic with its subtasks, or one subtask in YouTrack or GitHub Issues — e.g. 'audit <ID>', 'revisa el ticket', 'revisa los tickets y el ADR', 'is this ticket still worth doing', 'should this be an epic', '/gsdlc-ticket-audit'. Explains in plain English what the ticket asks for and what already exists in the code, then decides (1) keep, update, or close and (2) one ticket or epic + subtasks sized to fit one AI agent session. Checks epic links, ADR coverage, and OpenSpec alignment. Read-only until the user approves. Defers to a project-local sdlc-ticket-audit skill when one exists. Not for creating (gsdlc-ticket-create) or implementing (gsdlc-ticket-kickoff)."
user-invocable: true
---

# gsdlc Ticket Audit

> **Step 0 (mandatory):** follow `~/.agents/skills/gsdlc-config/references/step0.md` with job
> `ticket-audit`. A project-local `sdlc-ticket-audit` wins.
>
> **Communication contract:** TL;DR first, plain English, every question ends with
> `**Recommendation:** <option> — <reason>`.

This skill answers three questions before anyone works on a ticket:

1. **What does it ask for, and what already exists?** (context)
2. **Is it worth doing as written?** (Decision 1: keep, update, or close)
3. **Is it the right size?** (Decision 2: one ticket, or an epic with subtasks)

It is **read-only** until the user approves the proposed changes.

---

## 0. Where this skill sits

| Stage | Skill | Job |
| :--- | :--- | :--- |
| Idea not decided yet | `gsdlc-grill` | Ask questions until the design is clear |
| Write a new ticket | `gsdlc-ticket-create` | Create the ticket or epic |
| **Check a ticket before work** | **`gsdlc-ticket-audit`** | **Context, keep/update/close, right size** |
| Build the ticket | `gsdlc-ticket-kickoff` | Branch, OpenSpec change, code, PR |

**Main rule: one ticket (or subtask) = one OpenSpec change.** An epic never gets one change for all its
subtasks, so Decision 2 also decides how many OpenSpec changes the work will produce.

---

## 1. Read the ticket(s)

Use the tracker adapter (Step 0). Read the ticket, its comments, and any ADR it links
(`{{adr.location}}`: tracker article, or a file under `{{adr.path}}`).

| Mode | How to detect | What to read |
| :--- | :--- | :--- |
| **Single ticket** | Not a subtask, has no subtasks, title does not start with `[Epic]` | Ticket, comments, linked ADR |
| **Epic** | Title starts with `[Epic]`, epic label, or it has subtasks | Epic, every subtask, comments, ADR |
| **Subtask** | It has a parent epic | Subtask, parent epic, sibling titles, ADR |

In **Subtask** mode, also say in section 4 whether the subtask is the right size inside its epic, and
offer a full epic audit in section 7.

---

## 2. Where the project stands today

Never trust that the code still matches the day the ticket was written. For each **Scope** and
**Definition of Done** item (for an epic: each subtask), check:

1. **Code:** CodeGraph (`codegraph_explore`) first when `.codegraph/` exists, then grep / file reads.
2. **OpenSpec:** the read-only commands in `references/openspec.md` (context, list specs, show specs,
   active and archived changes, status, deltas, validate). Match ticket ids in change names ignoring
   case; search related tickets' ids too.
3. **Git:** `git log --oneline --grep "<ID>"` and `git log` on the main files involved.
4. **Other tickets:** search with 2–3 key words. Look for duplicates and tickets that replace this one.

Never run writing OpenSpec commands here (`new change`, `archive`, `init`, `update`).

Give each item exactly one status, with evidence (path, spec, commit, ticket id):

| Status | Meaning |
| :--- | :--- |
| `Done` | The code already does this. Cite `file:line` or the commit. |
| `Partly done` | Some of it exists. Say what is missing. |
| `Not started` | Nothing exists yet. |
| `Conflicts` | The code, a spec, or an ADR now says something different. Say what. |
| `Unclear` | The ticket is too vague to check. Say what is missing. |

---

## 3. Decision 1 — Keep, update, or close

| Verdict | Choose it when |
| :--- | :--- |
| **CLOSE** | Any of: (a) all Scope items are `Done`; (b) another open ticket covers the same work; (c) a later ADR, spec, or product decision removed the need; (d) the benefit is too small for the cost — no user, security, data, or developer-time gain you can name; (e) it rests on a wrong assumption about the code. |
| **UPDATE** | The goal is still worth it, but the text is wrong or weak: items `Done` or `Conflicts`; old paths or names; untestable DoD; missing sections (Problem, Goal, Scope, Out of Scope, DoD, AI Agent Prompt); dependencies on closed or wrong tickets. |
| **KEEP** | The goal is worth it, Scope matches the code today, every DoD item is testable. |

Rules:
- Two verdicts fit → the stronger wins: CLOSE > UPDATE > KEEP.
- CLOSE (d): name the cost and the missing benefit concretely. Never close only because work is hard.
- Epic: one verdict for the epic and one per subtask.
- UPDATE: list each change as `before → after`.
- UPDATE with an active OpenSpec change: the change must be updated too (openspec update). List it.
- CLOSE with an active OpenSpec change: never delete it. Ask whether to archive it or leave it.
- Goal right but contradicts a living spec → UPDATE: the ticket must say its change needs `MODIFIED` or
  `REMOVED` requirements.

---

## 4. Decision 2 — One ticket, or epic + subtasks

### 4.1 Why size matters (explain this plainly in the report)

An AI agent works on one ticket in one session. Everything it reads goes into its **context window**
(the limited working memory of one session). When a ticket needs more than fits comfortably, the agent
forgets early details, misses requirements, makes edits that do not match, and one large PR is hard to
review and roll back. When a ticket is split **too much**, each agent re-reads the same files, pieces
cannot be tested alone, and work spreads over many PRs.

**Target:** one ticket = one OpenSpec change = one focused agent session = one PR testable on its own.

### 4.2 Measure (show numbers, not just the result)

| Signal | One ticket is fine when | Split when |
| :--- | :--- | :--- |
| **Separate outcomes** (a tester checks each alone) | 1 | 2+ that can ship separately |
| **Files to change** | ~12 or fewer | more than ~12 |
| **Files to read first** (code + specs + ADR) | ~20 or fewer | more than ~20, or many unrelated areas |
| **Layers** (`{{layers}}`) | backend + frontend of the same feature | 3+, or INFRA/migration needing its own rollout |
| **Spec capabilities touched** | 1–2 | 3+ |
| **Tasks in `tasks.md`** (estimate) | ~15 or fewer | more than ~15 |
| **Open design decisions** | none, or small ones the ticket answers | decisions shared by several parts (needs an ADR) |
| **Order of delivery** | one PR | part B can only start after part A is merged |

Backend + frontend of one small feature still fit one ticket, especially when `{{delegates}}` splits
them across sub-agents.

### 4.3 Decide

- **SINGLE TICKET** — every signal is "fine".
- **EPIC + SUBTASKS** — 2+ separate outcomes, a required delivery order, or an ADR-level decision; or
  2+ other signals in "split".
- Signals disagree → pick the option where each piece passes "fine" on its own.

Existing epic: `KEEP STRUCTURE` | `SPLIT <subtask>` | `MERGE <subtasks>` (each changes 1–3 files, reads
the same files, cannot be tested alone) | `COLLAPSE TO SINGLE TICKET`.

### 4.4 Shape the subtasks

Each proposed subtask: one testable outcome; `gsdlc-ticket-create` naming
(`<EpicCode>.<n> <LAYER>: <Summary>`); its dependencies (usually DB → BE → FE → TEST/DOCS); its own size
estimate; the spec capabilities its own change will touch; no overlap with another subtask.

---

## 5. Consistency checks

Run every row. Mark `N/A` with a reason when it does not apply. Never drop a row.

| # | Check | What passes |
| :--- | :--- | :--- |
| 1 | Required sections | Problem, Goal, Scope, Out of Scope, DoD present; DoD testable |
| 2 | AI Agent Prompt | One `## AI Agent Prompt` at the end, one fenced block, paths match the code today |
| 3 | Subtask links | Each subtask has its parent; epic has the right number of children |
| 4 | ADR ticket list | The ADR lists every subtask |
| 5 | Decision coverage | Each ADR decision is covered by a ticket; each ticket traces to a decision |
| 6 | Dependencies | Epic order matches each ticket's Dependencies section (the ticket wins) |
| 7 | Contradictions | Main flow end to end: no step needs something another step forbids |
| 8 | Data model & project rules | `{{rules.data_model}}` and `{{rules.invariants}}` hold |
| 9 | OpenSpec alignment | No conflict with living specs; behavior changes say `MODIFIED` / `REMOVED` |
| 10 | OpenSpec change state | No archived change already did it; an active change matches and validates `--strict` |

---

## 6. Report — always this format

Never add, drop, or reorder sections. When one does not apply, write one line saying why.
Write for someone who has not read the ticket. Every claim about code has evidence.

```markdown
# Ticket Audit: [<ID>] <Title>

> **TL;DR** — NEEDS YOUR DECISION: <KEEP | UPDATE | CLOSE> and <SINGLE TICKET | EPIC + N SUBTASKS | KEEP STRUCTURE | …>, because <main reason>. **You need to:** <approve the changes in section 7 | nothing>.

| Field | Value |
| :--- | :--- |
| **Ticket** | [<ID> — <Title>](<ID-URL>) |
| **Mode** | Single ticket / Epic with N subtasks / Subtask of [<EPIC-ID>](<url>) |
| **Stage · Priority** | `<stage>` · `<priority>` |
| **ADR** | [<ADR id> — <Title>](<url>) / None |

## 1. What this ticket asks for
<3–5 short sentences: the problem today, who has it (use `{{project.roles}}`), what changes when done.>

## 2. Where the project stands today
<"X of Y items already exist in the code.">

| Ticket item | Status | Evidence |
| :--- | :--- | :--- |

**OpenSpec state:**
| Item | Value |
| :--- | :--- |
| Related living specs | `<spec>` (<n> req.) |
| Active change for this ticket | `<name>` (artifacts n/n, validate: pass) / None |
| Archived change for this ticket | `<date>-<name>` / None |
| Spec conflicts | <spec + requirement> / None |

**Related work:** <tickets / changes in the same area — or "None found (searched: <words>)">.

## 3. Decision 1 — Keep, update, or close
**Verdict: <KEEP | UPDATE | CLOSE>**

Why:
- <reason, with evidence>

<UPDATE: | Part | Now says | Should say | Why |>
<Epic: | Subtask | Verdict | Reason |>

## 4. Decision 2 — One ticket or epic + subtasks
**Verdict: <…>** <or `Skipped — Decision 1 is CLOSE.`>

| Signal | Measured | Limit for one ticket | Result |
| :--- | :--- | :--- | :--- |

Why (context window): <2–3 sentences>

<Epic: | Code | Layer | Summary | Reads ~ | Changes ~ | Specs touched | Depends on | Tested alone? |>

OpenSpec changes this will produce: <N>.

## 5. Consistency checks
| # | Check | Result | Evidence |
| :--- | :--- | :--- | :--- |
<rows 1–10>

## 6. Problems found
**Blockers:** <numbered, or `None.`>
**Warnings:** <numbered, or `None.`>

## 7. Recommendation and next step
Proposed changes (nothing changes until you approve):
1. <exact action>

Next step in the OpenSpec workflow: <from 6.1>

Options:
1. <Apply all changes, then `gsdlc-ticket-kickoff <ID>`> (Recommended)
2. <alternative>
3. Change nothing

**Recommendation:** <option> — <reason>.
```

### 6.1 Next step table

| Audit result | Next step |
| :--- | :--- |
| Open design questions | `gsdlc-grill` (or `openspec-explore` if the idea is still open) |
| KEEP / UPDATE, no change yet | `gsdlc-ticket-kickoff <ID>` |
| UPDATE, active change exists | Update the ticket, then update the change |
| KEEP, active change validates | Continue applying the change |
| Active change, all tasks done | Archive via `gsdlc-pr-review` close-out |
| EPIC + SUBTASKS | Restructure (section 8), then kickoff the first subtask in order |
| CLOSE | Close (section 8). No OpenSpec step |

---

## 8. Apply the approved changes

Only after approval, and only what was approved.

1. **Snapshot first:** save every description and ADR you will edit to a JSON file in the scratchpad.
2. **UPDATE:** write the new description (keep the `gsdlc-ticket-create` structure). If an active change
   exists, update it too and run `openspec validate <change> --strict`.
3. **CLOSE:** follow `gsdlc-ticket-create` section 5 (comment, close, link duplicates).
4. **Single ticket → epic:** create subtasks with `gsdlc-ticket-create`; rename the original to
   `[Epic] <Code> — <Title>` and cut its description to goal, subtask table, and overall DoD.
5. **Epic restructure:** create new tickets; close replaced ones with a comment linking the replacement.
6. **Links:** link subtasks to the epic (tracker adapter).
7. **ADR:** update its ticket table.
8. **Verify:** re-read each changed ticket and report a table: ticket, change, result.
