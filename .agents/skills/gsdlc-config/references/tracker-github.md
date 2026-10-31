# Tracker adapter — GitHub Issues

Used when `tracker.type: github`. Values come from `.agents/sdlc.yaml`:
`G = tracker.github`, repo `R = {{G.repo}}` (default `{{repo.slug}}`), prefix `P = {{tracker.key_prefix}}`
(default `GH`).

## Ticket ids

A ticket id is `P-<n>` (for example `GH-45`). It means issue `#45` in `R`. Use the full id in branch
names, commits, change names, and reports, and link it as `https://github.com/R/issues/<n>`.
Accept `#45` from the user as the same ticket.

## Tools

Use the `gh` CLI (or a GitHub MCP server when it is loaded). Always pass `--repo R`.

| Operation | Command |
| :--- | :--- |
| Read issue | `gh issue view <n> --repo R --json number,title,body,labels,state,assignees,milestone,url` |
| Comments | `gh issue view <n> --repo R --comments` |
| Search | `gh issue list --repo R --state all --search "<words>" --json number,title,state,labels` |
| Epics | `gh issue list --repo R --label "{{G.epic_label}}" --state all --json number,title` |
| Create | `gh issue create --repo R --title "<t>" --body-file <file> [--label …]` (prints the URL) |
| Edit body / title | `gh issue edit <n> --repo R --body-file <file> [--title …]` |
| Labels | `gh issue edit <n> --repo R --add-label X --remove-label Y` |
| Comment | `gh issue comment <n> --repo R --body-file <file>` |
| Close (done) | `gh issue close <n> --repo R --reason completed` |
| Close (won't do / duplicate) | `gh issue close <n> --repo R --reason "not planned"` (comment `Duplicate of #m` first for duplicates) |
| Unassign | `gh issue edit <n> --repo R --remove-assignee @me` |

### Sub-issues (epic → subtask), when `use_sub_issues: true`

```bash
CHILD_ID=$(gh api repos/R/issues/<child-n> --jq .id)          # numeric id, not the number
gh api -X POST repos/R/issues/<epic-n>/sub_issues -F sub_issue_id="$CHILD_ID"
gh api repos/R/issues/<epic-n>/sub_issues --jq '.[].number'    # verify
```

If sub-issues are not available, keep a task list in the epic body (`- [ ] #<n> <title>`) and add
`Part of #<epic-n>` to each subtask body.

## Fields

GitHub has no custom fields. Map them like this:

| Concept | Mapping |
| :--- | :--- |
| Type (bug/feature/task) | label from `type_labels` |
| Priority | label from `priority_labels` (skip when empty) |
| Epic | label `epic_label`, title `[Epic] <Code> — <Title>` |
| Stage | `stage_mode: labels` → swap the labels in `stage_labels` (empty = no label). `stage_mode: project` → Projects v2 Status (below). `none` → open/closed only |
| ADR | `adr.location: repo` → Markdown file under `{{adr.path}}`; `tracker` is not supported on GitHub |

### Projects v2 status (`stage_mode: project`)

```bash
gh project item-list <number> --owner <owner> --format json --limit 500 \
  | jq '.items[] | select(.content.number==<n>) | .id'
gh project field-list <number> --owner <owner> --format json   # find Status field id + option ids
gh project item-edit --id <item-id> --project-id <project-node-id> \
  --field-id <status-field-id> --single-select-option-id <option-id>
```

## Stage moments

| Moment | Action |
| :--- | :--- |
| New ticket | backlog label / status (or nothing) |
| Kickoff approved | in_progress label / status |
| PR opened | review label / status; put `Closes #<n>` in the PR body |
| PR merged | done: the `Closes #<n>` keyword closes it; verify with `gh issue view` and close by hand if needed |
