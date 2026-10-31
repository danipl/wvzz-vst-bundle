#!/usr/bin/env bash
# Shared helpers for the gsdlc-manual-test scripts. Source it; do not run it.
#
# Settings (environment variables, filled by the agent from manual_test.* in .agents/sdlc.yaml):
#   ENV_FILE      default .env
#   ENV_EXAMPLE   default .env.example
#   HEALTH        health checks separated by ';', each "url|expect".
#                 expect = "HTTP <code>" for a status code, otherwise a text the body must contain.
#                 Default: none.
#   START_CMD     command that starts the stack in the background (default: docker compose up -d --build)
#   WAIT_SECONDS  max wait for health (default 300)

ENV_FILE="${ENV_FILE:-.env}"
ENV_EXAMPLE="${ENV_EXAMPLE:-.env.example}"
HEALTH="${HEALTH:-}"
WAIT_SECONDS="${WAIT_SECONDS:-300}"

# check_one "url|expect" -> prints "OK|FAIL <url> -> <observed>", returns 0 on OK
check_one() {
  local url="${1%%|*}" expect="${1#*|}"
  [ "$expect" = "$1" ] && expect="HTTP 200"
  if [[ "$expect" == HTTP\ * ]]; then
    local want="${expect#HTTP }" got
    got="$(curl -s -m 3 -o /dev/null -w '%{http_code}' "$url" 2>/dev/null || true)"
    if [ "$got" = "$want" ]; then echo "OK   $url -> HTTP $got"; return 0; fi
    echo "FAIL $url -> HTTP ${got:-000} (want $want)"; return 1
  else
    local body
    body="$(curl -s -m 3 "$url" 2>/dev/null | head -c 300 || true)"
    if printf '%s' "$body" | grep -qF -- "$expect"; then echo "OK   $url -> $body"; return 0; fi
    echo "FAIL $url -> ${body:-no answer} (want text: $expect)"; return 1
  fi
}

# check_all -> 0 when every HEALTH entry is OK (or HEALTH is empty)
check_all() {
  local rc=0 item
  [ -z "$HEALTH" ] && return 0
  IFS=';' read -r -a items <<< "$HEALTH"
  for item in "${items[@]}"; do
    item="$(echo "$item" | sed 's/^ *//; s/ *$//')"
    [ -z "$item" ] && continue
    check_one "$item" || rc=1
  done
  return $rc
}

has_compose() {
  local f
  for f in docker-compose.yml docker-compose.yaml compose.yml compose.yaml; do
    [ -f "$f" ] && return 0
  done
  return 1
}
