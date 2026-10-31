---
name: gsdlc-grill
description: "Generic (any project) read-only stress test of an idea, feature, plan, ticket, or design before it becomes a ticket or an OpenSpec change — e.g. 'grill me', 'grill this', 'stress-test this idea', 'challenge this plan', 'poke holes in this', 'what am I missing', 'machácame', 'cuestiona esto', '/gsdlc-grill'. Walks a decision tree (scope, actors and permissions, data model, API, UI, i18n, failure modes, rollout, verifiability, plus project branches from .agents/sdlc.yaml) one question at a time with grounded recommendations, until every branch is resolved or deferred, then emits a Decision Record and hands off to gsdlc-ticket-create or back to gsdlc-ticket-kickoff. Defers to a project-local sdlc-grill. Not for open brainstorming (openspec-explore) or ticket audits (gsdlc-ticket-audit)."
user-invocable: true
---

# gsdlc Grill

> **Step 0 (mandatory):** follow `~/.agents/skills/gsdlc-config/references/step0.md` with job `grill`.
> A project-local `sdlc-grill` wins.
>
> **Communication contract:** TL;DR first on reports, plain English, every question ends with
> `**Recommendation:** <option> — <reason>`.

Relentless, read-only interrogation of an idea, plan, or ticket until every decision it depends on is
confirmed or deliberately deferred. Output: a **Decision Record** for `gsdlc-ticket-create` (new work)
or `gsdlc-ticket-kickoff` (existing ticket).

> **Grill vs Explore:** `openspec-explore` follows threads and stops when the user feels clear. The grill
> drives a fixed decision tree, pushes back on vague answers, and finishes only when every branch has a
> status. Explore finds the idea; the grill hardens it.

---

## Core principles

1. **Read-only.** Never edit code, write files, run mutating commands, or write to the tracker.
2. **Evidence before questions.** Never ask a fact you can check in the code, specs,
   `openspec/config.yaml`, `.agents/sdlc.yaml`, or the ticket. Ask only what only the user can decide.
3. **One question at a time**, each with why it matters, the decision it unlocks, the evidence, and a
   recommendation with alternatives. Batch only if the user asks.
4. **No premature finish.** Every branch ends `Confirmed`, `Default`, `Deferred`, or `N/A`. "Sounds
   good" to a vague question is not an answer: restate the concrete decision and ask again.
5. **Deferral is explicit**, with a reason and a deadline (`before kickoff | implementation | release`).
6. **Never shortcut the SDLC.** Do not create OpenSpec changes here; kickoff Phase 4 does that.

---

## Phase 1: Ground the subject

1. **Subject:** free-form idea, pasted plan, or a ticket id (read it and its ADR via the tracker adapter).
2. **Project constraints:** `openspec/config.yaml` (`context`, `rules`), `{{rules.*}}`, `AGENTS.md`.
3. **Living specs:** `openspec list --specs`, then read every spec the subject touches in full.
4. **Changes in progress:** `openspec list --json`; read the `proposal.md` of any overlapping change. If
   one already covers the subject, grill against it and hand off to an OpenSpec update instead of a new
   ticket.
5. **Code:** find the modules, endpoints, screens, and migrations the subject would touch.
6. **Grounding summary** (5 lines max): what exists, what would change, specs in play, overlapping work.

---

## Phase 2: Build the decision tree

Use the checklist below. Keep the branches that matter; mark the rest `N/A` with a reason. Order by
dependency: outcome and scope → data model → API → UI.

```text
| #  | Branch               | Status    | Decision / Note                    |
|----|----------------------|-----------|------------------------------------|
| 1  | Outcome & scope      | Confirmed | Users can export their own records |
| 2  | Actors & permissions | Open      |                                    |
| 3  | Data model           | Default   | New table, soft delete             |
| 4  | Rollout              | Deferred  | Before release: feature flag?      |
```

Statuses: `Open`, `Confirmed` (user decided), `Default` (accepted your recommendation), `Deferred`,
`N/A`.

### Grill checklist

1. **Outcome & scope:** What user-visible outcome proves it is done? What is out of scope? One ticket or
   an epic?
2. **Actors & permissions:** Which roles (`{{project.roles}}`) act? Which permissions are new or changed?
   Can a role see or do something it must not?
3. **Data isolation & sensitive data:** Is data scoped to the right owner / tenant? Which fields are
   personal or secret? Does anything export, log, or cache them?
4. **Data model & migrations:** New tables or columns? Indexes, backfill for existing rows, delete
   strategy? Check `{{rules.data_model}}`.
5. **API contract:** Endpoints, request/response shapes, pagination, status codes, error format.
6. **UI & navigation:** Which screens and routes? Where does it live in the navigation? Check
   `{{rules.ui}}`.
7. **i18n:** Every user-facing string in every locale (`{{review.i18n.locales}}` when enabled)?
   Locale-sensitive dates, numbers, currency?
8. **Failure modes:** Concurrency, idempotency, partial failure, retries, empty and error states,
   limits — and what the user sees for each.
9. **Rollout & compatibility:** Existing data and clients, feature flags, order against other tickets,
   reversibility.
10. **Configuration & tooling:** New settings or environment variables, and where they are read
    (`{{rules.invariants}}`).
11. **Verifiability:** How is each DoD item tested? Which WHEN/THEN scenarios will the delta spec need?
12. **Project branches:** one branch per item in `{{rules.grill_branches}}`.

---

## Phase 3: The grill loop

Ask about the first `Open` branch whose dependencies are settled:

```text
[Branch 2 - Actors & permissions]
Question: Can staff export a user's records, or only the user?
Why it matters: decides whether we add a new permission or reuse records:read.
Evidence: the permissions spec gives records:read to staff and owner (spec.md:41); no export permission.
Recommendation: user-only, with a new records:export permission. Alternative: reuse records:read
          (simpler, but widens staff access).
```

Answers: concrete → `Confirmed`; "go with your recommendation" → `Default`; "later" → `Deferred` with
reason and deadline; vague → restate 2–3 concrete options and ask again; contradiction with an earlier
decision or a spec → say so with evidence and reopen the affected branches; upstream change → revisit
every dependent branch. Refresh the status table every few questions.

---

## Phase 4: Decision Record

When no branch is `Open`, emit exactly:

````markdown
# Decision Record: <Subject>

> **TL;DR** — DONE: <one sentence>. **You need to:** <one action, or "nothing">.

## Summary
<2–3 sentences: the hardened idea and its boundaries.>

## Confirmed Decisions
| # | Branch | Decision | Evidence / Rationale |
|---|--------|----------|----------------------|

## Accepted Defaults
| # | Branch | Default | Why it fits |
|---|--------|---------|-------------|

## Deferred Questions
| # | Branch | Question | Reason deferred | Answer before |
|---|--------|----------|-----------------|---------------|

## Out of Scope
- <item>

## Risks
- <risk and mitigation>

## Proposed Ticket Shape
- **Type:** Task / Bug / Epic with subtasks
- **Specs affected:** <capabilities, or skip_specs: true>
- **Definition of Done (draft):**
  - <criterion>
````

Empty sections contain `N/A`.

---

## Phase 5: Hand-off (ask first, never silently)

- **No ticket yet:** offer `gsdlc-ticket-create`. Summary + Confirmed Decisions → Problem/Goal/Scope;
  Out of Scope → Out of Scope; draft DoD → DoD; Deferred Questions → `## Open Questions`.
- **Called from `gsdlc-ticket-kickoff`:** return to its Phase 4. Decisions shape the OpenSpec files;
  deferred questions go to `design.md` Open Questions and report section 6.
- **A change already exists:** offer to update it (`openspec-update-change` or edit + validate).

End with `**Recommendation:** <target> — <reason>`.

The Decision Record and derived artifacts use `{{project.artifact_language}}`; talk in the user's language.
