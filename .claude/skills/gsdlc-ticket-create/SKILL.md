---
name: gsdlc-ticket-create
description: "Generic (any project) ticket creator for YouTrack or GitHub Issues: use when the user wants to create, log, or track a task, bug, epic, or subtask — e.g. 'create ticket', 'create task', 'create epic', 'log bug', 'crear ticket/tarea/épica', '/gsdlc-ticket-create'. Writes Problem/Goal/Scope/Out of Scope/DoD + a single-fenced AI Agent Prompt, sets fields from .agents/sdlc.yaml, keeps tickets unassigned, runs a quality gate, and supports epic + subtask hierarchies. Defers to a project-local sdlc-ticket-create skill when one exists. Not for auditing existing tickets (gsdlc-ticket-audit)."
user-invocable: true
---

# gsdlc Ticket Create

> **Step 0 (mandatory):** follow `~/.agents/skills/gsdlc-config/references/step0.md` with job
> `ticket-create`. A project-local `sdlc-ticket-create` wins. Values like `{{tracker.key_prefix}}` come
> from `.agents/sdlc.yaml`; tracker calls come from the adapter Step 0 loads.
>
> **Communication contract:** every report starts with the TL;DR line, uses plain English, and every
> question or gate ends with `**Recommendation:** <option> — <reason>`.

Standard workflow to create tickets, epics, and subtasks for `{{project.name}}`.

> Checking an existing ticket (still needed? right size? links and ADR coverage?) belongs to
> `gsdlc-ticket-audit`. When you choose between one ticket and an epic, use its sizing rules (section 4).

---

## 1. Before creating

0. **Grill first when undecided:** if the request still has open design decisions, run `gsdlc-grill`
   first and build the ticket from its Decision Record.
1. **Ground in specs:** when OpenSpec is on, run `openspec list --specs` and read the related
   `openspec/specs/<capability>/spec.md`.
2. **Check project rules:** `{{rules.invariants}}`, `{{rules.data_model}}`, `{{rules.ui}}`, and the
   `context:` of `openspec/config.yaml`. Name any the ticket touches in Constraints.
3. **Check duplicates:** search the tracker with 2–3 key words from the summary.
4. **Propose all fields at once** with your value for each, then end with
   `**Recommendation:** create as proposed — <reason>`:
   - **Summary:** short title naming the problem and scope.
   - **Description:** the structure in section 3.
   - **Type:** Task / Feature / Bug / Epic.
   - **Priority / Stage / other fields:** from the tracker config defaults (`fields.*.default`,
     `stages.backlog`, `priority_labels`) unless the user asked otherwise.

---

## 2. Epic and subtask naming

### 2.1 Epic: `[Epic] <Code> — <Title>`

- `<Code>` = a domain letter plus a number (`A1`, `C2`, `D1`).
- **Mandatory:** list existing epics first (YouTrack `search_issues "project: <P> [Epic]"`; GitHub
  `gh issue list --label {{tracker.github.epic_label}}`) and pick an unused code.

### 2.2 Subtask: `<EpicCode>.<n> <LAYER>: <Summary>`

- `<n>` starts at 1.
- `<LAYER>` is a `code` from `{{layers}}` (defaults: `DB`, `BE`, `FE`, `INFRA`, `TEST`, `DOCS`).
- Examples: `C2.1 BE: Schema — appointment status + negotiation message`,
  `C2.6 FE: Proposal chat — timeline + counter form`, `D1.7 TEST: Multi-node integration tests`.

---

## 3. Description structure

```markdown
## Problem

<what is wrong / what is needed>

## Goal

<what the ticket intends to achieve>

## Scope

- <in scope item>

## Out of Scope

- <not covered item>

## Open Questions

- <deferred question> — answer before <kickoff | implementation | release>
  (only when a gsdlc-grill Decision Record has deferred questions)

## Dependencies

- <ticket id> — <why> (or "None")

## Definition of Done

- <testable criterion>

---

## AI Agent Prompt

\`\`\`markdown
### Task: <Title>

### Package Structure
    <path>/
    <path>/

### Step-by-Step
1. <step>
2. <step>

### Constraints
- <constraint, including project rules this ticket touches>

### Expected Outputs
1. `<file>` — <what changes>
\`\`\`
```

### AI Agent Prompt format rules (critical)

1. The whole prompt is ONE ` ```markdown ` fenced block.
2. No nested fences: code inside uses **4-space indentation**.
3. A `---` line comes before `## AI Agent Prompt`.
4. It is always the last part of the description.

### Fields

- Set every configured field explicitly (YouTrack: stage, priority, `fields.extra`; GitHub: type,
  priority, and stage labels / project status).
- **Never set an assignee** when `keep_unassigned` is true (default). If the tracker auto-assigns the
  creator, unassign at once.

---

## 4. Quality gate (mandatory, right after creation)

1. The tracker returned a valid id (`<ID>`) and URL.
2. All configured fields / labels are set; nothing failed to update.
3. The ticket is unassigned.
4. The description has Problem, Goal, Scope, Out of Scope, Definition of Done, and AI Agent Prompt.
5. The AI Agent Prompt follows the format rules.

If a check fails, patch the ticket (update fields or description) and check again.

---

## 5. Closing a ticket (authoritative procedure)

Other gsdlc skills (`gsdlc-pr-review` close-out, `gsdlc-ticket-audit` CLOSE) call this section.

- **Done after merge:** set the `done` stage (YouTrack) or close as completed (GitHub). Verify the field
  changed.
- **Closed without work:** first comment `Closed without implementation: <reason>. Evidence: <links>.`,
  then close (YouTrack: `done` stage; GitHub: `--reason "not planned"`). For a duplicate, also link it
  to the other ticket.

---

## 6. Epic with subtasks

1. Classify subtasks by layer.
2. Create the epic, then each subtask (sub-agents may create subtasks in parallel when the harness
   supports them).
3. Link each subtask to the epic (YouTrack `subtask_link`; GitHub sub-issues or task list).
4. Verify link counts, then offer `gsdlc-ticket-audit` on the epic.

---

## 7. Reports (always this format)

### Single ticket

```markdown
# Ticket Created: [<ID>] <Summary>

> **TL;DR** — DONE: <what happened, one sentence>. **You need to:** <one action, or "nothing">.

| Field | Value |
| :--- | :--- |
| **Ticket** | [<ID> — <Summary>](<ID-URL>) |
| **Type** | Task / Feature / Bug |
| **Stage · Priority** | `<stage>` · `<priority>` |
| **Other fields** | `<name>: <value>` / N/A |
| **Assignee** | `Unassigned` (verified) |
| **Quality Gate** | `PASS` (5/5 checks) |

## Scope & Objective
- **Problem & Goal:** <1–2 sentences>
- **Key Deliverables:** <2–3 bullets from the DoD>

**Next step:** `gsdlc-ticket-audit <ID>` or `gsdlc-ticket-kickoff <ID>`.
```

### Epic + subtasks

```markdown
# Epic Created: [Epic] <Code> — <Title> [<ID>]

> **TL;DR** — DONE: <what happened>. **You need to:** <one action, or "nothing">.

| Field | Value |
| :--- | :--- |
| **Epic** | [<ID> — [Epic] <Code> — <Title>](<ID-URL>) |
| **Epic Code** | `<Code>` |
| **Subtasks** | `<N>` created, links verified (`parent for: <N>`) |
| **Quality Gate** | `PASS` (all unassigned, fields set, prompts formatted) |

| Code | Ticket | Layer | Summary | Link |
| :--- | :--- | :--- | :--- | :--- |
| `<Code>.1` | [<ID>](<ID-URL>) | `BE` | <Summary> | PASS |

**Next step:** audit the epic with `gsdlc-ticket-audit`, then `gsdlc-ticket-kickoff` on subtask 1.
```

## 8. Language

Ticket text uses `{{project.artifact_language}}` (default English). Talk to the user in their language.
When the request came through a remote channel (`{{communication.channels}}`), send a short
confirmation with the ticket link and one-line summary instead of the full report.
