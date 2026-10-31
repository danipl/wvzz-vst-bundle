# Tracker adapter — YouTrack

Used when `tracker.type: youtrack`. All names come from `.agents/sdlc.yaml`:
`Y = tracker.youtrack`, `{{Y.base_url}}`, `{{Y.project}}`, `{{Y.mcp_server}}`, `{{Y.token_env}}`.

## Resolve the tools (3 tiers, first that works)

1. **Native MCP.** The harness has the `{{Y.mcp_server}}` server (or any `youtrack*` server) loaded.
   Call its tools directly: `get_issue`, `get_issue_comments`, `search_issues`, `create_issue`,
   `update_issue`, `add_issue_comment`, `link_issues`, `change_issue_assignee`, `get_article`.
2. **`mcporter` CLI** with a portable config search:
   ```bash
   MCPORTER_CFG="${MCPORTER_CONFIG:-}"
   if [ -z "$MCPORTER_CFG" ]; then
     for c in "$HOME/.config/mcporter.json" "$HOME/config/mcporter.json"; do
       [ -f "$c" ] && MCPORTER_CFG="$c" && break
     done
   fi
   mcporter call {{Y.mcp_server}}.get_issue issueId="<ID>" --config "$MCPORTER_CFG"
   ```
3. **REST** with a Bearer token:
   ```bash
   TOKEN="${<token_env>:-$(jq -r '.mcpServers["{{Y.mcp_server}}"].headers.Authorization // empty' "$MCPORTER_CFG" 2>/dev/null | sed 's/^Bearer //')}"
   curl -s -H "Authorization: Bearer $TOKEN" \
     "{{Y.base_url}}/api/issues/<ID>?fields=idReadable,summary,description,customFields(name,value(name))"
   ```
   Use plain field lists; nested `$type` queries return HTTP 400.

Never print the token.

## Operations

| Operation | MCP / mcporter | REST |
| :--- | :--- | :--- |
| Read issue | `get_issue {issueId}` | `GET /api/issues/<ID>?fields=idReadable,summary,description,customFields(name,value(name))` |
| Comments | `get_issue_comments` | `GET /api/issues/<ID>/comments?fields=text,author(login),created` |
| Search | `search_issues {query}` (e.g. `project: {{Y.project}} [Epic]`) | `GET /api/issues?query=<q>&fields=idReadable,summary` |
| Create | `create_issue project={{Y.project}} summary=… description=… customFields='{…}'` | `POST /api/issues` |
| Update fields / description | `update_issue issueId=<ID> customFields='{"<stage field>":"<value>"}'` | `POST /api/issues/<ID>` |
| Comment | `add_issue_comment` | `POST /api/issues/<ID>/comments {"text": …}` |
| Link subtask → epic | `link_issues` linkType `{{Y.subtask_link}}` (target = subtask, issueToLink = epic) | — |
| Duplicate | `link_issues` linkType `{{Y.duplicate_link}}` (fallback `relates to`) | — |
| Unassign | `change_issue_assignee issueId=<ID>` (no login) | — |
| ADR article | `get_article articleId=<ADR id>` | `GET /api/articles/<ID>?fields=idReadable,summary,content`; update `POST /api/articles/<ID> {"content": …}` |

## Facts to trust

- Link counts: trust `get_issue` → `linkedIssueCounts` (`subtask of: 1`, `parent for: N`). REST
  `subtaskOf` / `child` can read empty even when the link exists.
- Create result: check `createdIssue` (id + URL), `updatedFields`, and that `failedToUpdateFields` is empty.
- If YouTrack auto-assigns the creator and `{{Y.keep_unassigned}}` is true, unassign at once.
- YouTrack usually has no "Won't do" stage: to close without work, comment the reason, then set the
  `done` stage.

## Stage moments

| Moment | Value |
| :--- | :--- |
| New ticket | `{{Y.stages.backlog}}` |
| Kickoff approved | `{{Y.stages.in_progress}}` |
| PR opened | `{{Y.stages.review}}` |
| PR merged / closed | `{{Y.stages.done}}` |
