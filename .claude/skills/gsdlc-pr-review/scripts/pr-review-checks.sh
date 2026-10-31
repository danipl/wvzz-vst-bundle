#!/usr/bin/env bash
# gsdlc static PR review checks (generic, any language).
# Usage: pr-review-checks.sh [repo_root] [base_ref] [out_file] [extra_checks_file]
#   extra_checks_file: optional, one project check per line (from review.static_checks in .agents/sdlc.yaml),
#     TAB-separated (regexes may contain '|'):  name<TAB>search1,search2<TAB>path_regex<TAB>exclude_regex<TAB>SEVERITY
#   Lines starting with # are ignored. An empty regex means "no filter".
# Always exits 0. Output is a Markdown table in out_file (also printed).

set -u

REPO_ROOT="${1:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"
BASE_REF="${2:-origin/main}"
OUT_FILE="${3:-${TMPDIR:-/tmp}/pr-review-checks.md}"
EXTRA="${4:-}"

cd "$REPO_ROOT" || exit 0
mkdir -p "$(dirname "$OUT_FILE")"

TEST_PATH='(_test\.go$|\.test\.|\.spec\.|__tests__/|/tests?/|/testdata/|_test\.py$|test_[^/]*\.py$)'

row() { echo "| $1 | $2 | $3 |" >> "$OUT_FILE"; }
# Files whose diff adds or removes any of the given strings (git pickaxe), excluding deleted files.
pickaxe() {
  local args=()
  for s in "$@"; do args+=(-S "$s"); done
  git diff "${BASE_REF}...HEAD" "${args[@]}" --name-only --diff-filter=d 2>/dev/null || true
}
# Added lines only.
added_lines() { git diff "${BASE_REF}...HEAD" -U0 2>/dev/null | grep '^+[^+]' || true; }
join_files() { tr '\n' ' ' | sed 's/ *$//'; }

{
  echo "# Static PR Review Checks"
  echo
  echo "Base: \`${BASE_REF}\` · Head: \`$(git rev-parse --short HEAD 2>/dev/null)\`"
  echo
  echo "| Category | Status | Details |"
  echo "| :--- | :--- | :--- |"
} > "$OUT_FILE"

CHANGED=$(git diff "${BASE_REF}...HEAD" --name-only --diff-filter=d 2>/dev/null || true)
if [ -z "$CHANGED" ]; then
  row "Changed files" "INFO None" "No changed files against ${BASE_REF}."
  cat "$OUT_FILE"; exit 0
fi

# 1. Debug output left in production code.
DEBUG=$(pickaxe "console.log" "console.debug" "fmt.Println" "fmt.Printf(" "print(" "pprint(" "var_dump(" "dd(" "System.out.println" "debugger;" \
  | grep -vE "$TEST_PATH" | grep -vE '(^|/)(cmd|scripts?|tools?|devtool)/' || true)
if [ -n "$DEBUG" ]; then row "Debug output" "WARN Found" "Check debug prints in: $(echo "$DEBUG" | join_files)"
else row "Debug output" "OK Clean" "No new debug prints in production paths"; fi

# 2. Type-safety escapes (TypeScript / Python / Go).
TYPES=$(pickaxe "as any" "@ts-ignore" "@ts-nocheck" "@ts-expect-error" "# type: ignore" "//nolint" \
  | grep -vE "$TEST_PATH" || true)
if [ -n "$TYPES" ]; then row "Type safety" "WARN Found" "\`as any\` / ignore comments in: $(echo "$TYPES" | join_files)"
else row "Type safety" "OK Clean" "No new type-check escapes"; fi

# 3. TODO / FIXME in added lines.
TODO=$(added_lines | grep -E '\b(TODO|FIXME|XXX|HACK)\b' | head -n 20 || true)
if [ -n "$TODO" ]; then row "TODO / FIXME" "WARN Found" "$(echo "$TODO" | wc -l | tr -d ' ') new markers in the diff"
else row "TODO / FIXME" "OK Clean" "No new TODO/FIXME markers"; fi

# 4. Hard-coded secrets in added lines.
#    Assignment-style secrets: non-test files only. Known key formats: every file.
PROD_FILES=$(echo "$CHANGED" | grep -vE "$TEST_PATH" || true)
added_prod_lines() {
  [ -z "$PROD_FILES" ] && return 0
  echo "$PROD_FILES" | tr '\n' '\0' | xargs -0 git diff "${BASE_REF}...HEAD" -U0 -- 2>/dev/null | grep '^+[^+]' || true
}
SECRETS=$(added_prod_lines | grep -iE '(password|passwd|secret|api[_-]?key|token|bearer|private[_-]?key)["'\'' ]*\s*[:=]\s*["'\''][^"'\'' ]{8,}["'\'']' \
  | grep -viE "[\"'](<[^>]*>|\\\$\{|\{\{|x{4,}|\*{4,}|changeme|change_me|example|placeholder|dummy|your[_-]|test[_-]|fake|mock|sample)" | head -n 5 || true)
KEYS=$(added_lines | grep -E '(AKIA[0-9A-Z]{16}|-----BEGIN [A-Z ]*PRIVATE KEY-----|ghp_[A-Za-z0-9]{36}|sk-[A-Za-z0-9]{20,}|xox[bap]-[A-Za-z0-9-]{10,})' | head -n 5 || true)
if [ -n "$SECRETS$KEYS" ]; then row "Secret scanner" "FAIL Violation" "Possible hard-coded secret or key in added lines"
else row "Secret scanner" "OK Clean" "No plaintext secrets detected"; fi

# 5. Committed env / key files.
ENVF=$(echo "$CHANGED" | grep -E '(^|/)\.env($|\.)|\.(pem|key|p12|pfx)$' | grep -vE '\.env\.(example|sample|template)$' || true)
if [ -n "$ENVF" ]; then row "Sensitive files" "FAIL Violation" "Committed env/key files: $(echo "$ENVF" | join_files)"
else row "Sensitive files" "OK Clean" "No env or key files committed"; fi

# 6. Large files (> 1 MB).
BIG=""
while IFS= read -r f; do
  [ -f "$f" ] || continue
  sz=$(wc -c < "$f" | tr -d ' ')
  [ "$sz" -gt 1048576 ] && BIG="$BIG $f($((sz / 1024))KB)"
done <<< "$CHANGED"
if [ -n "$BIG" ]; then row "Large files" "WARN Found" "Files over 1 MB:$BIG"
else row "Large files" "OK Clean" "No changed file over 1 MB"; fi

# 7. Project checks (review.static_checks).
if [ -n "$EXTRA" ] && [ -f "$EXTRA" ]; then
  while IFS=$'\t' read -r name searches paths exclude severity; do
    [ -z "${name// /}" ] && continue
    case "$name" in \#*) continue ;; esac
    IFS=',' read -r -a terms <<< "$searches"
    hits=$(pickaxe "${terms[@]}")
    [ -n "$paths" ] && hits=$(echo "$hits" | grep -E "$paths" || true)
    [ -n "$exclude" ] && hits=$(echo "$hits" | grep -vE "$exclude" || true)
    if [ -n "$hits" ]; then
      if [ "${severity:-SUGGESTION}" = "BLOCKER" ]; then st="FAIL Violation"; else st="WARN Found"; fi
      row "$name" "$st" "$(echo "$hits" | join_files)"
    else
      row "$name" "OK Clean" "No matches"
    fi
  done < "$EXTRA"
fi

cat "$OUT_FILE"
exit 0
