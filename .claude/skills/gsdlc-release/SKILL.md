---
name: gsdlc-release
description: "Generic (any project) product release: use when creating, bumping, or publishing a release — e.g. 'cut a release', 'create release', 'bump version', 'publish release', 'prepare release', '/gsdlc-release'. Checks the default branch is clean and in sync, finds unreleased commits since the last semver tag, recommends a major/minor/patch bump, drafts a changelog grouped by type with ticket links, waits for approval, then creates the tag (dispatched workflow, local tag, or gh release, per .agents/sdlc.yaml), publishes the GitHub release, and checks the post-release runs. Defers to a project-local sdlc-release."
user-invocable: true
---

# gsdlc Release & Semver

> **Step 0 (mandatory):** follow `~/.agents/skills/gsdlc-config/references/step0.md` with job `release`.
> A project-local `sdlc-release` wins.
>
> **Communication contract:** TL;DR first, plain English, every gate ends with
> `**Recommendation:** <option> — <reason>`.

`REL` = `release` in `.agents/sdlc.yaml`. `R` = `{{repo.slug}}`. `B` = `{{repo.default_branch}}`,
`REMOTE` = `{{repo.remote}}`. Tag prefix `P` = `v` when `REL.tag_pattern: vsemver`, else empty.

Read the project's release docs and ADRs first when they exist (search `AGENTS.md`, `docs/`, ADRs for
"release" / "tag"), and follow any rule they add.

```text
1 pre-flight -> 2 semver -> 3 changelog -> 4 APPROVAL GATE -> 5 create tag -> 6 GitHub release -> 7 post-release checks
```

---

## Phase 1: Pre-flight

```bash
git status --porcelain                      # must be empty
git branch --show-current                   # must be B
git fetch REMOTE B --tags
[ "$(git rev-parse HEAD)" = "$(git rev-parse REMOTE/B)" ] || echo "Local B is not aligned with REMOTE/B"

LATEST_TAG=$(git tag -l | grep -E "^${P}[0-9]+\.[0-9]+\.[0-9]+$" | sort -V | tail -n 1)
RANGE="${LATEST_TAG:+$LATEST_TAG..}REMOTE/B"
git log "$RANGE" --oneline --no-merges
```

- Dirty tree, wrong branch, or not aligned → stop and say what to do.
- No commits in the range → `DONE: nothing to release since <tag>.`
- No tag yet → the first release; recommend `0.1.0` (or `1.0.0` if the user says it is production).

## Phase 2: Semver

Read each commit (and its PR / ticket when the subject is unclear):

| Bump | Signals |
| :--- | :--- |
| **major** | `BREAKING CHANGE:` / `!:` in commits, branches from the `break` type, removed or incompatible API, schema removals |
| **minor** | new user-facing features, new modules, branches from the `feature` type, `feat:` |
| **patch** | fixes, performance, docs, tooling, refactors, CI |

The highest signal wins. Before `1.0.0`, a breaking change may be a minor bump: ask.
`X.Y.Z` → major `(X+1).0.0`, minor `X.(Y+1).0`, patch `X.Y.(Z+1)`.

## Phase 3: Changelog

Group commits with `REL.changelog_sections` (first `match` regex that fits the subject or branch wins).
Extract ticket ids (`{{tracker.key_prefix}}-<n>`) and PR numbers; link them (`{{tracker.issue_url}}`,
`https://github.com/R/pull/<n>`). Commits with no ticket keep their subject. Drop empty sections.

```markdown
## Release <NEW_TAG> (<YYYY-MM-DD>)

### Features
- [<ID>](<url>) <subject> (PR [#<n>](<url>))

### Bug Fixes
- …
```

## Phase 4: Approval gate (mandatory STOP)

```text
TL;DR — NEEDS YOUR DECISION: release <NEW_TAG> (<bump>) is ready. You need to: approve or reject.

Current tag: <LATEST_TAG or none>   Target tag: <NEW_TAG>
Bump: <bump> — <reason from the commits>
Branch: B @ <sha>   Commits: <N>   Tag method: <REL.mode>

<changelog>

Recommendation: approve — <reason>.
```

Do not continue without an explicit yes.

## Phase 5: Create the tag

| `REL.mode` | Steps |
| :--- | :--- |
| `workflow` | `gh workflow run {{release.workflow}} --repo R -f {{release.workflow_input}}=<bump>`; find the run (`gh run list --workflow={{release.workflow}} --repo R --limit 1`), wait with `gh run watch <id> --exit-status` (background when long); then `git fetch --tags REMOTE` and confirm `<NEW_TAG>` exists and points at the expected commit. If the workflow computes a different tag than you showed, stop and tell the user. |
| `local` | `git tag -a <NEW_TAG> -m "Release <NEW_TAG>" REMOTE/B && git push REMOTE <NEW_TAG>` |
| `gh` | Skip; Phase 6 creates the tag with `--target B` |

## Phase 6: GitHub release

```bash
if gh release view "<NEW_TAG>" --repo R >/dev/null 2>&1; then
  gh release edit "<NEW_TAG>" --repo R --title "Release <NEW_TAG>" --notes-file "$SCRATCH/changelog.md"
else
  gh release create "<NEW_TAG>" --repo R --title "Release <NEW_TAG>" --notes-file "$SCRATCH/changelog.md" --target B
fi
gh release view "<NEW_TAG>" --repo R
```

## Phase 7: Post-release checks

Run `REL.post_release_checks` (default: `gh run list --repo R --event push --limit 5` to find runs started by
the tag). Report each run's status; do not wait forever (bounded wait or background). Check that
`REL.artifacts` (with `{tag}` filled) exist when a command can tell (e.g. `docker manifest inspect`).

```markdown
# Release Published: <NEW_TAG>

> **TL;DR** — DONE: published <NEW_TAG> (<bump>). **You need to:** <nothing | watch run X>.

| Field | Value |
| :--- | :--- |
| **GitHub Release** | https://github.com/R/releases/tag/<NEW_TAG> |
| **Tag** | `<NEW_TAG>` → `<sha>` (via <mode>) |
| **Post-release runs** | <name: status> |
| **Artifacts** | <list with status> / N/A |
```
