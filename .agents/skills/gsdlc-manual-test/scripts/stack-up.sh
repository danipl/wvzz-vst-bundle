#!/usr/bin/env bash
# gsdlc-manual-test: start (or rebuild) the local stack in the background and wait until it answers.
# Never follows logs; always returns control.
#   START_CMD     default "docker compose up -d --build" (extra args to this script are appended)
#   HEALTH        see lib.sh; empty = only wait for START_CMD to finish
#   WAIT_SECONDS  default 300
# Output goes to <git-dir>/manual-checks/stack-up.log. Exit 1 on failure with the last log lines.
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(git rev-parse --show-toplevel)" || exit 1
cd "$ROOT" || exit 1
# shellcheck source=lib.sh
. "$HERE/lib.sh"
START_CMD="${START_CMD:-docker compose up -d --build}"
LOG_DIR="$(git rev-parse --absolute-git-dir)/manual-checks"
mkdir -p "$LOG_DIR"
LOG="$LOG_DIR/stack-up.log"

echo "Starting the stack in the background: $START_CMD $*"
echo "Log: $LOG"
START=$(date +%s)
if ! bash -c "$START_CMD $*" >"$LOG" 2>&1; then
  echo "FAIL  start command failed. Last 30 log lines:"
  tail -n 30 "$LOG"
  exit 1
fi
echo "Start command finished after $(( $(date +%s) - START ))s."

if [ -z "$HEALTH" ]; then
  echo "OK    no HEALTH checks configured; not waiting."
  exit 0
fi
echo "Waiting for health checks (max ${WAIT_SECONDS}s)."
while :; do
  out="$(check_all)" && break
  elapsed=$(( $(date +%s) - START ))
  if [ "$elapsed" -ge "$WAIT_SECONDS" ]; then
    echo "FAIL  not ready after ${elapsed}s:"; echo "$out"
    if has_compose; then
      docker compose ps -a --format 'table {{.Service}}\t{{.State}}\t{{.Health}}\t{{.Status}}' 2>/dev/null
      for svc in $(docker compose ps -a --format '{{.Service}}' 2>/dev/null); do
        echo "--- last 20 log lines: $svc"
        docker compose logs "$svc" --tail 20 --no-color 2>&1 | cut -c1-300
      done
    fi
    exit 1
  fi
  if has_compose; then
    dead="$(docker compose ps -a --format '{{.Service}} {{.State}} {{.ExitCode}}' 2>/dev/null | awk '$2=="exited" && $3!="0" {print $1}' | tr '\n' ' ')"
    if [ -n "${dead// /}" ]; then
      echo "FAIL  container(s) exited with an error: $dead"
      for svc in $dead; do
        echo "--- last 30 log lines: $svc"
        docker compose logs "$svc" --tail 30 --no-color 2>&1 | cut -c1-300
      done
      exit 1
    fi
  fi
  sleep 5
done

echo "OK    ready after $(( $(date +%s) - START ))s."
echo "$out"
has_compose && docker compose ps --format 'table {{.Service}}\t{{.State}}\t{{.Health}}' 2>/dev/null
exit 0
