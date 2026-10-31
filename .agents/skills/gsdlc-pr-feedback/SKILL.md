---
name: gsdlc-pr-feedback
description: "Generic (any project) handler for review feedback on session work or a pull request — e.g. 'review feedback', 'PR feedback', 'the reviewer said', 'address the review', 'revisa el feedback', '/gsdlc-pr-feedback', or pasted reviewer comments. Checks each point against the code, the living specs, and the ticket's OpenSpec change (scope, design decisions, scenarios), gives each one a verdict (Accept / Reject / Partial / Defer) with a short technical reason, and waits for approval; then updates and validates the OpenSpec change, applies the fixes, verifies, commits, and pushes. Defers to a project-local sdlc-pr-feedback."
user-invocable: true
---

# gsdlc PR Feedback

> **Step 0 (mandatory):** follow `~/.agents/skills/gsdlc-config/references/step0.md` with job
> `pr-feedback`. A project-local `sdlc-pr-feedback` wins.
>
> **Communication contract:** TL;DR first, plain English, every question ends with
> `**Recommendation:** <option> — <reason>`.

```text
Feedback -> 1 split into points -> 2 verify vs code + plan -> 3 report -> STOP for approval
         -> 5 update OpenSpec change -> fix -> verify -> commit + push
```

## Core principles

1. **Guilty until proven innocent.** Never accept a suggestion blindly. Check it against the code, types,
   call sites, living specs, and the ticket's OpenSpec change.
2. **The OpenSpec change is the approved plan** (`proposal.md` scope, `design.md` decisions and contracts,
   delta specs behavior, `tasks.md` work). Feedback against the plan is never a silent ACCEPT: it is DEFER
   (out of scope) or a decision for the user. When a fix changes the plan, update the plan first.
3. **No changes while analyzing.** No edits, mutating commands, or commits until the user approves.

## Verdicts

| Verdict | Meaning | In the plan? |
| :--- | :--- | :--- |
| **ACCEPT** | Correct, finds a real problem, fits the architecture | Yes, as suggested |
| **REJECT** | Wrong, based on false assumptions, causes regressions, or breaks project rules / specs | No, with a concrete reason |
| **PARTIAL / DEVIATE** | The concern is valid, the proposed fix is not | Yes, with a corrected fix |
| **DEFER** | Valid but out of scope (including Out of Scope in `proposal.md`) | No; offer a follow-up ticket |

A point that is right but **changes a decision confirmed in `design.md`** keeps ACCEPT/PARTIAL and is
marked `Plan change: needs your decision`. Never apply it silently.

---

## Phase 1: Split
Number the points (`Point 1`, `Point 2`, …) with the files, lines, and concern of each. For a PR, read the
threads with `gh pr view <n> --repo {{repo.slug}} --comments` and
`gh api repos/{{repo.slug}}/pulls/<n>/comments`.

## Phase 2: Verify
1. **Load the change:** `ls -d openspec/changes/*/ | grep -i "<id>-"`; read all four artifacts. None →
   `OpenSpec Change: N/A — <reason>` and check against living specs only.
2. **Against the plan:** in scope? agrees with `design.md`? changes a scenario (then the spec changes too)?
3. **Against the code and rules:** contracts in living specs, `{{rules.invariants}}`, `{{rules.ui}}`,
   `{{rules.data_model}}`, `AGENTS.md`.
4. Assign the verdict.

## Phase 3: Report

```markdown
# Review Feedback Analysis: [<ID>] <Title>

> **TL;DR** — NEEDS YOUR DECISION: <one sentence>. **You need to:** <one action>.

## Overview
<1–2 sentences: source of the review and overall assessment.>

| Field | Value |
| :--- | :--- |
| **Pull Request** | [#<n> — <Title>](<url>) / N/A |
| **Ticket** | [<ID>](<ID-URL>) |
| **Branch** | `<branch>` |
| **OpenSpec Change** | `openspec/changes/<change>/` / N/A — <reason> |

| # | Topic | File / Area | Verdict | Impact |
|---|---|---|---|---|
| 1 | <summary> | `path` | ACCEPT / REJECT / PARTIAL / DEFER | Low / Medium / High / Critical |

---

## Point by point

### Point 1: <title>
- **Reviewer suggestion:** …
- **Code today:** … (with file links)
- **Plan says:** <proposal / design / scenario, with link> / Not covered by the plan
- **Plan change:** None / Updates <file> / Needs your decision — <decision>
- **Assessment:** why it makes sense or not; trade-offs and risks
- **Verdict:** …
- **Resolution:** exact planned action, or why no action

---

## Implementation plan (ACCEPT and PARTIAL only)
- [MODIFY] `path` · [NEW] `path`
1. <step>

## Verification plan
- <tests and checks from `{{commands.*}}`>

## OpenSpec plan updates
| File | Change |
| :--- | :--- |
| `tasks.md` | `<N>` new tasks (<numbers>) |
| `design.md` / `specs/` / `proposal.md` | <change or "No change"> |

**Decisions needed:** <one line each with a recommendation, or "None">.

---

## Awaiting your approval
1. **Approve full plan** — update the change, then apply all accepted points.
2. **Adjust / cherry-pick** — say which points to include, change, or drop.

**Recommendation:** <1 | 2> — <reason>.
```

## Phase 4: STOP until the user approves or cherry-picks.

## Phase 5: Update the plan, fix, verify (after approval)

1. **Update the OpenSpec change first** (`openspec-update-change` when the project has it): add one
   `- [ ]` task per accepted point under `## <next>. Review feedback (round <k>)` with its verification;
   update `design.md` / delta specs / `proposal.md` only where an approved point changes them.
2. **Validate:** `openspec validate "<change>" --strict`. Fix format yourself; ask if a fix goes beyond
   what was approved.
3. **Apply** layer by layer and tick each new task. Delegate to `{{delegates}}` like
   `gsdlc-ticket-kickoff` Phase 5.5 step 4 when the harness supports sub-agents; small fixes directly.
4. **Verify:** `{{commands.format}}`, `{{commands.lint}}`, `{{commands.test}}`; `tasks.md` has no open tasks.
5. **Follow-ups:** for each DEFER, offer a ticket via `gsdlc-ticket-create`.
6. **Commit and push:** subject from `{{repo.commit_pattern}}` with "Address PR review feedback".
   Optionally reply on each GitHub thread with the fix commit (ask first: it posts publicly).

```markdown
# Review Feedback Resolved: [<ID>] <Title>

> **TL;DR** — DONE: <one sentence>. **You need to:** <one action>.

| Field | Value |
| :--- | :--- |
| **Pull Request** | [#<n>](<url>) |
| **Fix Commit** | `<sha>` — `<subject>` |
| **Applied** | `<N>` (ACCEPT `<a>`, PARTIAL `<p>`) |
| **Rejected / Deferred** | REJECT `<r>`, DEFER `<d>` (follow-up: [<ID>](url) / N/A) |
| **OpenSpec** | `<change>`: <files updated>, `<N>` new tasks done, validate `--strict` PASS |
| **Verification** | format / lint / tests: PASS (<counts>) |

**Next step:** ask for a re-review with `gsdlc-pr-review`.
```
