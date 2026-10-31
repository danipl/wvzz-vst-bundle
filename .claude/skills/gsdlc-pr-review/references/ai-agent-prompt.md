# AI Agent Prompt

Append to the GitHub review body (or PR-comment fallback) whenever the verdict is REQUEST CHANGES **or** APPROVED-with-SUGGESTIONs, so the assignee can delegate the fix or cleanup. If there are zero findings, no prompt block is required.

```markdown
# Task: Fix PR #[number] — <Title>

## Context
Brief paragraph about the PR and what the review identified.

## Mission
1. First objective

## Step-by-Step
1. Specific file/line edit
2. Using 4-space indent for any commands/examples

## Constraints
- English only
- No unrelated changes

## Expected Outputs
1. path/to/file — what changes
```

## CRITICAL — Formatting Rules (same as gsdlc-ticket-create)
1. Single ```markdown block, NO nested ``` inside.
2. Sub-blocks of code (git commands, examples) use **4-space indentation**.
3. `---` separator before the heading. Position: always at the very end.
