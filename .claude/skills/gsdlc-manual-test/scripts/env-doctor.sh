#!/usr/bin/env bash
# gsdlc-manual-test: read-only diagnosis of the local stack before a manual test.
# Changes nothing. Never follows logs, never prompts, needs no TTY.
# Each line: OK | WARN | FAIL, the area, a message; a "FIX:" line names a command that fixes it.
# Settings: see lib.sh (ENV_FILE, ENV_EXAMPLE, HEALTH). Exit 0 when there is no FAIL.
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(git rev-parse --show-toplevel 2>/dev/null)" || { echo "FAIL  repo         not inside a git repository"; exit 1; }
cd "$ROOT" || exit 1
# shellcheck source=lib.sh
. "$HERE/lib.sh"

FAILS=0
ok()   { printf 'OK    %-12s %s\n' "$1" "$2"; }
warn() { printf 'WARN  %-12s %s\n' "$1" "$2"; }
fail() { printf 'FAIL  %-12s %s\n' "$1" "$2"; FAILS=$((FAILS + 1)); }
fix()  { printf '      FIX: %s\n' "$1"; }

# --- Checkout ---------------------------------------------------------------
GIT_DIR="$(git rev-parse --absolute-git-dir)"
COMMON_DIR="$(cd "$(git rev-parse --git-common-dir)" && pwd)"
if [ "$GIT_DIR" != "$COMMON_DIR" ]; then
  MAIN_DIR="$(git worktree list --porcelain | awk 'NR==1 && $1=="worktree" {print $2}')"
  ok checkout "worktree $ROOT (main checkout: $MAIN_DIR)"
else
  MAIN_DIR="$ROOT"; ok checkout "main checkout $ROOT"
fi

# --- Env file ---------------------------------------------------------------
if [ -f "$ENV_EXAMPLE" ]; then
  if [ ! -f "$ENV_FILE" ]; then
    fail env "$ENV_FILE is missing"
    if [ "$ROOT" != "$MAIN_DIR" ] && [ -f "$MAIN_DIR/$ENV_FILE" ]; then fix "cp '$MAIN_DIR/$ENV_FILE' '$ENV_FILE'   (or the project's worktree setup command)"
    else fix "cp $ENV_EXAMPLE $ENV_FILE"; fi
  else
    MISSING="$(comm -23 <(grep -oE '^[A-Za-z0-9_]+=' "$ENV_EXAMPLE" | tr -d = | sort -u) \
                        <(grep -oE '^[A-Za-z0-9_]+=' "$ENV_FILE" | tr -d = | sort -u) | tr '\n' ' ')"
    if [ -n "${MISSING// /}" ]; then
      fail env "$ENV_FILE misses keys from $ENV_EXAMPLE: $MISSING"
      fix "$HERE/env-backfill.sh   (adds only the missing keys with $ENV_EXAMPLE values)"
    else
      ok env "$ENV_FILE has every key from $ENV_EXAMPLE"
    fi
  fi
else
  ok env "no $ENV_EXAMPLE in this repo (nothing to compare)"
fi

# --- Docker / Compose -------------------------------------------------------
if has_compose; then
  if ! docker info >/dev/null 2>&1; then
    fail docker "Docker is not running"
    fix "open -a Docker (macOS) or start the Docker service, then wait until 'docker info' works"
    echo "SUMMARY $FAILS problem(s). Docker checks skipped."
    exit 1
  fi
  ok docker "Docker is running"
  PROJECT="$(docker compose config --format json 2>/dev/null | python3 -c 'import json,sys; print(json.load(sys.stdin)["name"])' 2>/dev/null)"
  PROJECT="${PROJECT:-$(basename "$ROOT")}"
  ok project "compose project: $PROJECT"

  # Fixed container_name values are shared by every checkout of the repo.
  NAMES="$(docker compose config --format json 2>/dev/null | python3 -c '
import json,sys
d=json.load(sys.stdin)
print("\n".join(s["container_name"] for s in d.get("services",{}).values() if s.get("container_name")))' 2>/dev/null)"
  CLASH=""; STOPPED=""
  for n in $NAMES; do
    owner="$(docker inspect -f '{{index .Config.Labels "com.docker.compose.project"}} {{.State.Status}}' "$n" 2>/dev/null || true)"
    [ -z "$owner" ] && continue
    p="${owner%% *}"; st="${owner#* }"
    [ "$p" = "$PROJECT" ] && continue
    if [ "$st" = "running" ]; then CLASH="$CLASH $p"; else STOPPED="$STOPPED $n"; fi
  done
  for p in $(printf '%s\n' $CLASH | sort -u); do
    dir="$(docker ps --filter "label=com.docker.compose.project=$p" --format '{{.Label "com.docker.compose.project.working_dir"}}' | head -1)"
    fail stack-clash "another checkout's stack holds the shared container names (project '$p', folder ${dir:-unknown})"
    fix "ASK THE DEVELOPER FIRST: docker compose -p $p down   (data volumes stay)"
  done
  [ -n "$STOPPED" ] && { fail containers "stopped containers of another checkout keep shared names:$STOPPED"; fix "docker rm$STOPPED   (stopped containers only; volumes stay)"; }
  [ -z "$CLASH$STOPPED" ] && ok stack-clash "no other checkout holds the container names"

  STATUS="$(docker compose ps -a --format '{{.Service}}={{.State}}/{{.Health}}' 2>/dev/null | tr '\n' ' ')"
  if [ -z "${STATUS// /}" ]; then warn stack "not started for this checkout"; fix "$HERE/stack-up.sh"
  else ok stack "$STATUS"; fi
else
  ok docker "no compose file; Docker checks skipped"
fi

# --- Health -----------------------------------------------------------------
if [ -n "$HEALTH" ]; then
  out="$(check_all)"; rc=$?
  while IFS= read -r l; do
    case "$l" in OK*) ok health "${l#OK   }" ;; *) warn health "${l#FAIL }" ;; esac
  done <<< "$out"
  [ $rc -ne 0 ] && fix "$HERE/stack-up.sh   (or read the service logs, bounded: docker compose logs <svc> --tail 40 --no-color)"
fi

echo "SUMMARY $FAILS problem(s)."
[ "$FAILS" -eq 0 ]
