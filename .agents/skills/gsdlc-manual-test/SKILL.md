---
name: gsdlc-manual-test
description: "Generic (any project) guided manual test of a finished branch before the pull request — e.g. 'walk me through testing this', 'guide me through the checks', 'manual test <ID>', 'smoke test my branch', 'let me check it works', 'guíame para probar esto', 'prueba manual', '/gsdlc-manual-test'. Input: a ticket id, an OpenSpec change, and/or free text. Builds checks from the git diff, the ticket Definition of Done, and the OpenSpec scenarios; diagnoses, fixes, seeds, and starts the local stack itself with commands that always return control; guides the developer one check and one step at a time with expected results it pre-checked read-only; records every answer in a report; separates problems this branch did not cause; then proposes one decision per bug (fix now, follow-up task, separate ticket, not a bug) and applies only what the developer approves. Stack details come from .agents/sdlc.yaml. Defers to a project-local sdlc-manual-test. Not for code review (gsdlc-pr-review) or writing automated tests."
user-invocable: true
argument-hint: "[<ticket id>] [openspec/changes/<change>/] [<base branch or commit>] [free-text scope]"
---

# gsdlc Manual Test

> **Step 0 (mandatory):** follow `~/.agents/skills/gsdlc-config/references/step0.md` with job
> `manual-test`. A project-local `sdlc-manual-test` wins.
>
> **Communication contract:** plain English; every question ends with `**Recommendation:** …`.
> Check-loop messages (Phase 3) are short step messages without a TL;DR; the Phase 7 report has one.

The developer, not the agent, decides whether each change works. The agent prepares and starts the
environment, prepares each check, gives exact steps and expected results, asks for the result after every
step, helps when the developer is stuck, and records the answer. At the end it proposes one decision per
bug, and the developer approves or changes each one.

`M` = `manual_test` in `.agents/sdlc.yaml`. Scripts: `~/.agents/skills/gsdlc-manual-test/scripts/`
(written `$S/` below). They read these environment variables, which the agent sets from `M`:

| Variable | From | Example |
| :--- | :--- | :--- |
| `ENV_FILE`, `ENV_EXAMPLE` | `M.env_file`, `M.env_example` | `.env`, `.env.example` |
| `HEALTH` | `M.health` joined as `url\|expect;url\|expect` | `http://localhost:8080/health\|"status":"ok";http://localhost:8082\|HTTP 200` |
| `START_CMD` | `M.start` | `docker compose up -d --build` |
| `WAIT_SECONDS` | — | `300` |

```text
Trigger -> 0 preflight -> 1 inventory (developer confirms) -> 2 environment (agent runs it) -> card
        -> 3 check loop (one step per message) -> 4 gate -> [5 decision panel -> 6 apply] -> 7 final report
```

---

## Core principles

1. **The developer decides.** Never mark a check passed for them, even when output looks right.
2. **No code changes during the check loop.** Fixes happen only in Phase 6, after approval.
3. **No ticket, task, or commit without approval of the exact draft.**
4. **The agent runs the environment; the developer runs the checks.**
   - Phase 2: the agent runs every setup command, saying in one line before each what it does and why.
   - Phase 3: the developer does the steps under test (clicks, state-changing requests, scripts). The
     agent runs only read-only commands (files, grep, `GET`, logs, read-only DB queries) unless the
     developer asks it to run a step.
   - **Never without an explicit yes:** stop a stack another checkout runs, stop a non-Docker process,
     change an existing `.env` key, touch another checkout's files. **Never at all:** commands in
     `M.forbidden_commands`, `docker compose down -v`, deleting volumes, `git clean` outside generated code.
5. **Every command returns control** (see Terminal rules). Long commands run in the background and write to
   a file.
6. **One step per message, an answer after every step.**
7. **Save progress after every answer** in the report file.
8. **Development data only.** Seed users and the local stack; never real customer data or production
   credentials.

---

## Input

- A ticket id → read it with the tracker adapter; it is also the parent for follow-up subtasks.
- A path under `openspec/changes/` → the change directory.
- A branch or SHA → the base.
- Other text → scope guidance ("only the admin screens"). With no ticket and no diff, it is the full scope.

---

## Phase 0: Preflight

1. Read `AGENTS.md` and `M.dev_doc` (how to start the stack, seed data, log in).
2. **Base:** from the input, else `{{repo.remote}}/{{repo.default_branch}}` (fetch first). On the default
   branch itself, ask what to test.
3. `git status --short` — include uncommitted changes and say so.
4. **Change directory:** from input, else the folder in `openspec/changes/` starting with the ticket id, or
   the one this branch changed. Optional.
5. **Report path** (always names the ticket; `<key>` = lowercase ticket id from the branch, else the branch
   name with `/` → `-`):
   - with a change: `openspec/changes/<change>/manual-checks-<key>.md` (committed and archived with it);
   - without: `$(git rev-parse --git-dir)/manual-checks/<key>.md` (outside the working tree).
6. Report exists → show progress and ask: resume or start again. **Recommendation:** resume — earlier
   answers stay valid unless the code changed (compare the report's head commit with `HEAD`).

---

## Phase 1: Inventory of checks

1. Read `git log --oneline <base>..HEAD`, `git diff --stat <base>...HEAD`, and the diff.
2. Group changes by **what the developer can observe**: a screen (name the role, `{{project.roles}}`), an
   endpoint, a permission rule, a status change, an event / email / message, a migration or seed change, a
   log line or metric, a translation.
3. Link each group to its source: ticket DoD items, `#### Scenario:` blocks in the change's delta specs,
   task ids in `tasks.md`. Report every DoD item or scenario with no change, and every change with no source.
4. For each permission or data-isolation change, add a **negative check** (a role or tenant that must NOT
   see or do it).
5. Changes with no observable effect go into one last check: "run the automated tests that cover them",
   with the exact command (from `{{commands.test}}`, narrowed). The agent runs it in the background.
6. Show the numbered list (title, what changed, source). The developer confirms, removes, or adds. Write
   the confirmed list to the report.

---

## Phase 2: Environment (the agent runs it)

Goal: a healthy local stack with the data the checks need, with no setup command typed by the developer.

Start with one message: the plan as short titles (diagnose, fix, seed, start, verify) and "You do not need
to run anything; I will tell you what I run and why." Record every command and result in the report's
**Environment** section.

| Script | What it does | Changes state? |
| :--- | :--- | :--- |
| `$S/env-doctor.sh` | Checks env file vs example, Docker, container-name clashes with other checkouts, stack status, health URLs. Prints `OK` / `WARN` / `FAIL` with `FIX:` lines | No |
| `$S/env-backfill.sh` | Adds only missing keys to the env file, with example values. Prints key names only | env file only |
| `$S/stack-up.sh` | Runs `START_CMD` with output to a log file, waits for `HEALTH` (max `WAIT_SECONDS`), prints status or the failing logs | Yes |

When the project has no Docker stack (for example a CLI or a library), skip what does not apply and say so.

### E1. Diagnose
Run `env-doctor.sh` and each `M.doctor_extra` command. Summarize `FAIL` / `WARN` in plain words.

### E2. Fix what the doctor found (only that), then run the doctor again

| Finding | Agent runs | Who decides |
| :--- | :--- | :--- |
| Docker not running | `open -a Docker` (macOS), then a bounded wait loop on `docker info` | Agent |
| Env file missing | copy from the main checkout (worktree) or from the example | Agent |
| Env keys missing | `env-backfill.sh`; name the keys added; warn when a check needs a real value | Agent |
| `M.doctor_extra` finding | its `fix` command (dry run first when it deletes generated files) | Agent |
| Stopped containers hold shared names | `docker rm <names>` (volumes stay) | Agent |
| Another checkout's stack is running | ask first; **Recommendation:** stop it (fixed names/ports cannot run twice; volumes stay) | Developer |
| A non-Docker process uses a port | ask first, with process name and PID | Developer |

A setup problem that also exists on the base branch is **not caused by this branch**: record it under
"Observed, not caused by this change".

### E3. Feature flags
Find the flags each check needs in the project's config code and env example. Changing an existing env key
needs a yes (name key, current value, new value); record the old value and offer to restore it in Phase 7.

### E4. Seed data
When a check needs clean data, or the branch adds a migration, run `M.seed` **before** E5. Otherwise skip
and say why.

### E5. Start
Run `stack-up.sh` in the background (or with a 10-minute timeout). `OK` → E6. `FAIL` → read the printed
logs, find the cause read-only, fix when it is in the E2 table, else explain and ask. Retry at most twice per
cause. After any code change (Phase 6), run it again.

### E6. Verify the data
Read-only: seed users (`M.seed_users`), DB rows (read-only client, e.g. `sqlite3 -readonly`,
`psql -c` with a SELECT), startup log lines (`docker compose logs <svc> --since 10m --no-color | grep -c …`).

### E7. Environment card, then check 1 in the same message

````markdown
### Environment ready

| What | Value |
| :--- | :--- |
| <each M.urls entry> | <url> |
| Health | <each M.health url> → <answer seen in E5> |
| Logs (last 10 min) | `docker compose logs <svc> --since 10m --no-color \| tail -n 80` |
| Restart after a code change | `~/.agents/skills/gsdlc-manual-test/scripts/stack-up.sh` |
| Stop the stack | `{{manual_test.stop}}` (data stays) |

**Seed users for these checks:** <one row per role: exact login entry, user, role, tenant>.
**Not available locally:** <e.g. real email delivery, metrics>.

You can ask for `help` at any time: I can check the logs, the database, or the API for you.
````

Use real ports and answers. Test any token/login command once with a read-only request first. Use only the
seed users from `M.seed_users`; never invent or use real credentials. If none fits, ask.

### Terminal rules

| Do not use | Use instead |
| :--- | :--- |
| `docker compose up` without `-d`, `… watch`, any foreground dev server | `stack-up.sh` |
| `docker compose logs -f`, `tail -f` | `docker compose logs <svc> --since 10m --no-color \| tail -n 80` |
| interactive tools (`-it`, TUIs, pagers) | non-interactive flags (`-T`, `--no-pager`, `| cat`) |
| test runners that may watch | CI mode (`CI=true`, `--ci`, `--run`) |
| `timeout <n> <cmd>` (missing on macOS) | `for i in $(seq 1 <n>); do <check> && break; sleep 5; done` |
| `curl` without `-m`; DB clients that can write | `curl -s -m 10 …`; read-only DB modes |
| long `sleep` | a bounded loop on the condition |

To capture events or logs **while** the developer clicks: start a background capture to a file before the
step (note the PID), read the file after, then `kill <PID>`.

---

## Phase 3: Check loop

### Prepare
For each check, write all steps for yourself first: exact click path / URL / command / request body, the
seed user, the input data, and the expected result of each step (including what must NOT happen). Run every
read-only command yourself before giving it and fix the command or the expectation when they differ. Never
run state-changing check steps yourself. When output text depends on tool versions, describe what it must
show, not exact words.

When the change runs where the developer cannot see it (a subscriber, job, outbox, email/message sender),
add a step that confirms the effect: the log line, event, or cache key, with the exact command, time range,
and expected count. When the change adds a log line, event, or metric, finding it is the check.

### Show one step at a time
- **First message of a check:** `Check <i> of <n>: <title>`, what changed and why it matters (1–2
  sentences), the seed user, the step titles (no commands), a **Help** line (logs / events / DB command that
  shows this check's effect, or "answer help"), then **step 1 only** with its expected result before its
  command or click path, then the step answer line.
- **After each step:** check the result read-only when you can, compare in a few lines, then give only the
  next step (repeat commands; never "see above").
- **After the last step:** say per step whether it matched, propose the check outcome, ask the check answer.

### Answers (same four for steps and checks)

| Answer | Meaning |
| :--- | :--- |
| `pass` | Matches the expected result |
| `fail` | The expected result did not happen |
| `bug` | The main result happened, but something else is wrong (text, translation, layout, side effect, speed) |
| `help` | Stuck or unsure; the agent helps, then asks again |

`skip: <reason>` is also accepted. When the harness has a button question tool (Claude Code:
`AskUserQuestion`), use it with `pass`, `fail`, `bug`, `help` in that order. Otherwise end with:

`Step 1 of 2 — your answer: pass · fail: <what happened> · bug: <what is wrong> · help · skip: <reason>`

and for the check: `Check 3 of 7 — I propose: pass (both steps matched). Your answer: pass · fail · bug · help · skip`

Handling: `pass` → next step / next check. `fail` (step) → ask for detail, check logs, ask whether to go on
(**Recommendation:** continue when later steps do not depend on it). `bug` (step) → record expected vs
observed, next step; a check with a bug step and no fail step proposes `bug`. `fail`/`bug` (check) → record,
no fix yet, next check. `help` → offer concrete help (logs, repeat command, explain, query DB), do it, no
code edits, then ask the same step again. `skip` → record; it stays open. Accept answers in the developer's
words ("works", "sí, va bien" = pass). Record evidence with each answer; if the developer gives none, ask
once, else note "output not in the session". Update the report after every answer.

### Problems this branch did not cause
Record under the check as **"Observed, not caused by this change"**: what (exact message, time, service),
evidence it is not from this branch (same on base; `git diff --stat <base>...HEAD -- <path>` empty), and the
project it belongs to when it is upstream. No evidence → say so; do not guess.

---

## Phase 4: Gate
All passed and nothing "not caused by this change" → Phase 7. Otherwise → Phase 5.

## Phase 5: Decision panel

Per failed check and bug:

| Decision | When |
| :--- | :--- |
| **Fix now** | In scope, cause clear, fix small |
| **Follow-up task in this change** | In scope, fix too large or risky before the PR |
| **Separate ticket** | Existed before this branch (check base) or out of scope |
| **Not a bug** | Behavior matches ticket and specs; the expectation was wrong |

Per skipped check: run now, accept the risk, or add a follow-up task.

Per problem not caused by this branch: **Separate ticket** (affects users/devs, enough evidence) · **Keep in
the report only** (local-only or too small) · **Not a problem** (expected in this setup).

The developer accepts or changes each. End with `**Recommendation:**`. Record the decisions.

## Phase 6: Apply (in this order)

1. **Fix now:** not on the default branch; failing test first when possible, then the fix (load the project
   skills in `{{rules.related_skills}}`); commit each fix (`{{repo.commit_pattern}}`, "Fix …"); run
   `stack-up.sh` again; re-run every affected check with the Phase 3 loop.
2. **Follow-up task:** add it to `tasks.md` (update the change, validate). Too big for this PR → draft a
   subtask with `gsdlc-ticket-create`.
3. **Separate ticket:** draft with `gsdlc-ticket-create` (type Bug) with steps, expected, observed.
4. **Not a bug:** fix the expectation in the report; propose a wording fix to the ticket or spec if it misled.
5. **Not caused by this branch:** draft tickets as in 3; record "keep" / "not a problem" with date and who
   decided.

Show every ticket draft; create it only after approval. Write ticket ids and SHAs into the report.

## Phase 7: Final report

```markdown
# Manual Test: [<ID>] <Title or scope>

> **TL;DR** — <DONE | NEEDS YOUR DECISION | BLOCKED>: <N>/<M> checks passed, <F> failed, <B> bugs, <S> skipped. **You need to:** <one action>.

| Field | Value |
| :--- | :--- |
| Base → Head | `<base>` @ `<sha>` → `<branch>` @ `<sha>` |
| Report | `<path>` |

| # | Check | Outcome | Decision / Link |
| :--- | :--- | :--- | :--- |

**Observed, not caused by this change:**
| Check | Problem | Decision / Link |
| :--- | :--- | :--- |

**Open items:** <skipped checks and follow-ups, or "none">.

**Next step:** <back to gsdlc-ticket-kickoff Phase 6 push gate | open the PR | what blocks it>.
```

Write `none` in empty tables. Then ask once: keep the stack running or stop it (**Recommendation:** keep it
when a review or fix may need it soon). On stop, run `M.stop`. Offer to restore env values changed in E3.

When called from `gsdlc-ticket-kickoff`, return to its Phase 6 push gate with `<passed>/<total>` and the
report path.
