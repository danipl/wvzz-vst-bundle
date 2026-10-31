#!/usr/bin/env bash
# gsdlc-manual-test: add to ENV_FILE only the keys ENV_EXAMPLE has and ENV_FILE does not, with the example values.
# Never changes or removes an existing key. Prints the key names it added (never the values).
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(git rev-parse --show-toplevel)"
cd "$ROOT"
# shellcheck source=lib.sh
. "$HERE/lib.sh"
[ -f "$ENV_EXAMPLE" ] || { echo "FAIL  $ENV_EXAMPLE not found"; exit 1; }
[ -f "$ENV_FILE" ] || { echo "FAIL  $ENV_FILE not found (cp $ENV_EXAMPLE $ENV_FILE)"; exit 1; }

added=()
while IFS= read -r line; do
  key="${line%%=*}"
  grep -qE "^${key}=" "$ENV_FILE" || added+=("$line")
done < <(grep -E '^[A-Za-z0-9_]+=' "$ENV_EXAMPLE")

if [ "${#added[@]}" -eq 0 ]; then
  echo "OK    $ENV_FILE already has every key from $ENV_EXAMPLE. Nothing changed."
  exit 0
fi
{
  echo ""
  echo "# Added by gsdlc-manual-test env-backfill.sh on $(date +%Y-%m-%d): defaults from $ENV_EXAMPLE"
  printf '%s\n' "${added[@]}"
} >> "$ENV_FILE"
echo "OK    added ${#added[@]} key(s) to $ENV_FILE: $(printf '%s\n' "${added[@]}" | cut -d= -f1 | tr '\n' ' ')"
