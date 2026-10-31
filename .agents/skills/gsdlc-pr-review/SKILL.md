---
name: gsdlc-pr-review
description: "Generic (any project) strict review of a GitHub pull request or a pushed branch with no PR — e.g. 'review PR', 'review the PR for <ID>', 'revisa la PR', 'review branch', '/gsdlc-pr-review' — and the close-out after approval ('close it', 'merge it', 'cierrala', 'mergeala', 'squash to main'). Binding severity rules, static checks, optional i18n audit, DoD and OpenSpec plan reconciliation, CI gate, approve / request-changes posted to GitHub with an AI Agent Prompt, re-review of fix commits, then OpenSpec archive + merge + ticket close. Project rules come from .agents/sdlc.yaml. Defers to a project-local sdlc-pr-review."
user-invocable: true
---

# gsdlc PR Review — strict review with fixed output

> **Step 0 (mandatory):** follow `~/.agents/skills/gsdlc-config/references/step0.md` with job
> `pr-review`. A project-local `sdlc-pr-review` wins.
>
> **Communication contract:** TL;DR first (local reports), plain English, every question ends with
> `**Recommendation:** <option> — <reason>`.

`R` = `{{repo.slug}}`, `<BASE>` = `{{repo.remote}}/{{repo.default_branch}}`. Scratch files go in the
session scratchpad (`$TMPDIR` when there is none), written below as `$SCRATCH`.
Scripts are in `~/.agents/skills/gsdlc-pr-review/scripts/`.

---

## 1. Severity (binding)

### BLOCKER — request changes, must fix before merge

- **Security:** authorization gaps, IDOR, injection, secrets, error responses that leak internals,
  sensitive data leaked through DTOs or logs.
- **Architecture:** circular dependencies, wrong layer, violations of living specs, ADRs, or
  `{{rules.invariants}}`.
- **Data loss:** destructive migration, soft-delete bypass, unhandled cascade.
- **Tests:** new behavior without tests, or deleted coverage without replacement.
- **Scope creep:** unrelated refactors, seeds, or features bundled into the PR.
- **Error handling:** swallowed errors on live paths, missing validation at trust boundaries.
- **Incomplete fix:** one site fixed, sibling sites left (grep the same value repo-wide); new fields not
  wired end to end.
- **DoD / tasks not wired:** a ticket-claimed or `- [x]` feature that never reaches the live path (API
  wrapper with zero callers, a required field missing from the submit payload, authorization claimed but
  not enforced).
- **OpenSpec plan not met:** open `- [ ]` tasks; `openspec validate --strict` fails; code differs from a
  `design.md` contract (route, payload, status code, permission, i18n key) without a plan update; diff adds
  work that is Out of Scope or not in the plan.
- **Missing OpenSpec change:** a behavior-changing ticket branch with no change (`N/A` only for docs, CI,
  tooling; say why).
- **Client↔server contract drift:** code reads a field the DTO does not expose, or sends values the
  server rejects.
- **PR description drift:** claimed reason codes, event names, status values must exist in code.
- **Build break:** type errors, missing imports/exports.
- **Formatting:** `{{commands.format_check}}` fails on changed files.
- **Project checks** with severity BLOCKER in `{{review.static_checks}}`.

### SUGGESTION — should fix, does not block

- Maintainability (duplication, small helpers), clarity (naming, smaller functions, comments for
  non-obvious logic).
- A delta-spec `#### Scenario` without a test. Raise to BLOCKER when the scenario is about
  authorization, data isolation, or sensitive data.

---

## 2. Review workflow

### Step 1: Target and safe checkout

1. Find the PR: `gh pr list --repo R --state open` (match `<ID>` in title or branch). No PR → find the
   branch (`git branch -a | grep -i <ID>`); a pushed branch is a valid target (branch-only review).
2. **Current branch is the target** → review in place. **Otherwise** (other branch or dirty tree) → use a
   detached worktree:
   ```bash
   git fetch {{repo.remote}} <branch>
   git worktree add "$SCRATCH/pr<n>-review" {{repo.remote}}/<branch> --detach
   # after the review:
   git worktree remove --force "$SCRATCH/pr<n>-review" && git worktree prune
   ```

### Step 2: Diff and static checks

1. `git diff <BASE>...HEAD --stat` and the full diff.
2. Static checks. Write the project checks from `{{review.static_checks}}` to a file, one per line,
   **tab-separated** (regexes may contain `|`): `name<TAB>search1,search2<TAB>path_regex<TAB>exclude_regex<TAB>SEVERITY`
   (e.g. with `printf '%s\t%s\t%s\t%s\t%s\n' …`), then:
   ```bash
   bash ~/.agents/skills/gsdlc-pr-review/scripts/pr-review-checks.sh \
     "$(git rev-parse --show-toplevel)" <BASE> "$SCRATCH/pr-checks.md" "$SCRATCH/project-checks.txt"
   ```
   Built-in checks: debug prints, type-safety escapes, TODO/FIXME, secrets, committed env/key files,
   large files. Every hit needs your judgment (a debug print in a CLI is fine).
3. Formatting (BLOCKER if dirty): run each `{{commands.format_check}}` with `{files}` = changed files of
   that area.
4. i18n (when `{{review.i18n.enabled}}`):
   ```bash
   python3 ~/.agents/skills/gsdlc-pr-review/scripts/i18n-audit.py --base <BASE> \
     --locale-file '{{review.i18n.locale_file}}' --locales <comma list> \
     --source-dir {{review.i18n.source_dir}} [--pattern '<each key_patterns entry>']
   ```
   A NEW miss in any locale is a BLOCKER, but first confirm the reference itself is new
   (`git grep -F "<bare key>" <BASE> -- <source_dir>`). A renamed key prefix makes every reference new.

### Step 3: CI gate

`gh pr checks <n> --repo R` (branch-only: `gh run list --repo R --branch <branch> --limit 5`).

- **Case A — failed only because of quota / billing / offline runner** (and `{{review.ci_quota_fallback}}`
  is true): do not ask. Run `{{commands.lint}}` and `{{commands.test}}` on the review checkout at the PR
  head, and say in one line `CI blocked by quota — ran local suites instead.` All pass → record
  `CI: N/A — quota; verified locally (<sha>)`. Any fail → BLOCKER. Toolchain missing → stop and explain.
- **Case B — still running:** ask once:
  > CI is still running. If it fails: fix before continuing (1), or keep reviewing and handle it later (2)?
  > **Recommendation:** 1 — a red check found now is cheaper than one found at merge time.
  Never merge known-red CI.
- **Case C — genuine failure:** BLOCKER, with the failing job and log lines.

---

## 3. Checklist

### Build & formatting
- [ ] Formatting checks pass. New imports exist; test mocks match real types.

### Tests
- [ ] New behavior has tests that cover failure modes and wire contracts, not only happy-path mocks.

### OpenSpec reconciliation (both directions)
- [ ] **Find the change:** `ls -d openspec/changes/*/ | grep -i "<id>-"`. None → "Missing OpenSpec change".
- [ ] `openspec validate "<change>" --strict` passes.
- [ ] Read `proposal.md`, `design.md`, delta `specs/`, `tasks.md` first.
- [ ] **Plan → code:** `grep -n -- '- \[ \]' <change>/tasks.md` is empty; each `- [x]` reaches the live
  path; contracts match `design.md`; each `#### Scenario` has a test (list the ones without).
- [ ] **Code → plan:** each changed area maps to a task or design section; nothing Out of Scope;
  deliberate drift from the ticket is written in `design.md`.
- [ ] Delta specs describe the behavior the diff really adds, changes, or removes.

### Project invariants
- [ ] Every rule in `{{rules.invariants}}`, `{{rules.ui}}`, `{{rules.data_model}}` that the diff touches.

### DoD
- [ ] Each Definition of Done item of the ticket is met (tracker adapter to read it).

---

## 4. Submit the review and report (both mandatory every round)

### Template 1 — GitHub review body

Write it to `$SCRATCH/review.md` (`review-round<k>.md` for re-reviews), using **Schema A**. When any
BLOCKER or SUGGESTION exists, it MUST end with the AI Agent Prompt
(`references/ai-agent-prompt.md` rules: one fenced block, 4-space indent inside, `---` before the heading).

Quality gate before posting:
```bash
if grep -qE '\[(BLOCKER|SUGGESTION)-' "$SCRATCH/review.md" && ! grep -q '## AI Agent Prompt' "$SCRATCH/review.md"; then
  echo "QUALITY GATE VIOLATION: findings without AI Agent Prompt" >&2
fi
```

Post:
1. `gh pr review <n> --repo R --request-changes|--approve --body-file "$SCRATCH/review.md"`
2. If GitHub rejects it (authors cannot review their own PR):
   `gh pr comment <n> --repo R --body-file "$SCRATCH/review.md"`
3. Verify delivery and keep the URL for the chat report.

Branch-only review: no GitHub post; give Schema A-Local and keep the body file path.

### Template 2 — chat report by channel

| Channel | Output |
| :--- | :--- |
| GitHub (always) | **Schema A**, full |
| Local agent console (default) | **Schema A-Local** |
| A remote channel from `{{communication.channels}}` (e.g. Telegram bridge) | **Schema A-Remote** |

#### Schema A — full (GitHub body)

```markdown
# PR Review: [<ID>] <Title>

## Verdict: `APPROVED` | `APPROVED WITH SUGGESTIONS` | `REQUEST CHANGES`

<1–2 sentence summary.>

| Field | Value |
| :--- | :--- |
| **Review Round** | Round `<k>` (Initial / Re-review) |
| **Pull Request** | [#<n> — <Title>](<pr url>) _(or `Branch-only: <branch>`)_ |
| **Ticket** | [<ID> — <Summary>](<ID-URL>) |
| **Branch** | `<head>` → `{{repo.default_branch}}` (`<N>` files, `+<a>` / `-<d>`) |

---

## 1. PR Summary & Scope
<1–2 paragraphs: what the PR does across layers.>

## 2. Automated & Architectural Checks
| Check | Status | Details |
| :--- | :--- | :--- |
| Static checks | PASS / FAIL | <hits> |
| Formatting | PASS / FAIL / N/A | <N> files |
| i18n | PASS / FAIL / N/A | new misses per locale |
| OpenSpec reconciliation | PASS / FAIL / N/A — <reason> | validate, `<done>/<total>` tasks, `<covered>/<total>` scenarios tested, scope match |
| Project invariants | PASS / FAIL | <rules checked> |
| Test coverage | PASS / FAIL | |
| CI | PASS / FAIL / N/A — quota (local PASS) / PENDING | |

## 3. Findings

### Blockers
_(`None — Ready to merge.` if zero)_
1. **[BLOCKER-1] <Title>** — `path/to/file.ext#L10-L20`
   - **Issue:** <what and why it blocks>
   - **Required Fix:** <exact fix and test assertion>

### Suggestions
_(`None.` if zero)_
1. **[SUGGESTION-1] <Title>** — `path#L30`
   - **Observation:** … **Recommendation:** …

### Previous Round Resolution _(re-reviews only)_
| Finding | Status | Evidence |
| :--- | :--- | :--- |
| `[BLOCKER-1] <Title>` | FIXED / STILL OPEN | `<sha>` + `TestXxx` |

---

## AI Agent Prompt
_(Mandatory when findings > 0.)_
<one fenced markdown block: Task: Fix PR #<n> (or Cleanup PR #<n>) · Context · Mission · Step-by-Step
(4-space indented commands from `{{commands.*}}`) · Constraints · Expected Outputs>
```

#### Schema A-Local — console subset (never paste the AI Agent Prompt here)

```markdown
# PR Review: [<ID>] <Title> — <Verdict> (Round <k>)

> **TL;DR** — <DONE | NEEDS YOUR DECISION | BLOCKED>: <one sentence>. **You need to:** <one action>.

**PR:** [#<n>](<url>) · **Ticket:** [<ID>](<ID-URL>) · **GitHub review:** [full feedback + AI Agent Prompt](<url>)
**Branch:** `<head>` → `{{repo.default_branch}}` (<N> files, +<a>/-<d>)

**Checks:** Static <…> · Format <…> · i18n <…> · OpenSpec <…> · Invariants <…> · Tests <…> · CI <…>

**Blockers:** <N>
1. <title> — `path#L` (why)

**Suggestions:** <N>
1. <title> — `path#L` (what)

**Next:** <one step, e.g. "fix BLOCKER-1, push, ask for a re-review" or "say 'close it' to archive + merge">
```

#### Schema A-Remote — micro summary

Max ~10 short lines, no tables, no code fences, no file links, no AI Agent Prompt. Emojis allowed here only.

```text
🔍 PR Review — [<ID>] <Title>
📌 Round <k> · <N> files (+<a>/-<d>)
✅/🟡/🔴 Verdict: <…>
📝 <one sentence>
⛔ Blockers: <N or "0 — ready to merge">
💡 Suggestions: <N>
🧪 Checks: <green | issues: …>
🔗 PR: <url> · 🎫 Ticket: <url> · 📄 Review: <url>
👉 <next action>
```

---

## 5. Re-review

1. `git log --oneline <BASE>..HEAD`; inspect only the fix commits (`git show <sha>`).
2. Each previous BLOCKER is fixed and has a regression test.
3. Re-run static checks and the i18n audit.
4. Re-check OpenSpec: validate again; fix tasks ticked; `design.md` / specs updated when a contract changed.
5. Post round `<k>` (both mandatory actions of section 4) with Previous Round Resolution.

---

## 6. Close-out: archive, merge, close ticket

Only when the PR is approved and the user says "close it", "merge it", "cierrala", "mergeala", or similar.

1. **Archive the OpenSpec change on the PR branch** (when active): follow `openspec-archive-change` if the
   project has it, else `openspec archive "<change>" --yes`, then `openspec validate --specs`. Commit
   (`… Archive OpenSpec change and sync specs`) and push the branch.
2. **Merge:** `gh pr ready <n>` if draft, then
   `gh pr merge <n> --repo R --{{repo.merge_method}} [--delete-branch when {{repo.delete_branch_on_merge}}]`.
   Verify `gh pr view <n> --json state,mergedAt` → `MERGED`.
3. **Close the ticket:** `gsdlc-ticket-create` section 5 (done stage / closed as completed). Verify.
4. **Report** (remote channel: compress to the micro format):

```markdown
# PR Merged & Closed: [<ID>] <Title>

> **TL;DR** — DONE: <one sentence>. **You need to:** <one action, or "nothing">.

| Field | Value |
| :--- | :--- |
| **Pull Request** | [#<n> — <Title>](<url>) |
| **Merge** | `MERGED` (<method> to `{{repo.default_branch}}` @ `<sha>`, branch deleted: yes/no) |
| **Merged At** | `<ISO-8601>` |
| **Ticket** | [<ID>](<ID-URL>) — `<stage>` → `<done>` (verified) |
| **OpenSpec Archive** | `openspec/changes/archive/<date>-<change>/` / N/A — no change |
| **Synced Specs** | `<capabilities>` (`openspec validate --specs`: PASS) |

## What Was Delivered
- <1–2 sentences>
```
