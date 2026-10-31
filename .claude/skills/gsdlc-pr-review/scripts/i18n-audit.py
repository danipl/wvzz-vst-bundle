#!/usr/bin/env python3
"""gsdlc i18n locale audit (generic).

Resolves every translation key referenced by the changed source files against every locale file
(nested JSON) and reports NEW misses: keys missing now AND missing on the base branch. A key that a
fallback string hides at runtime (t('a.b', 'English')) still counts as a miss.

Usage (values come from review.i18n in .agents/sdlc.yaml):
  i18n-audit.py --base origin/main \
      --locale-file 'frontend/src/i18n/locales/{locale}/translation.json' \
      --locales en,es --source-dir frontend/src \
      [--pattern '<regex with group 1 = key>' ...] [--ext .ts,.tsx,.js,.jsx]

Exit code: 0 when no new misses, 1 otherwise, 2 on setup errors.
Before flagging a miss as introduced by this branch, confirm the REFERENCE is new too:
  git grep -F "<bare key>" <base> -- <source-dir>
(formatters often wrap calls so the key sits alone on its own line; a prefix grep misses it).
"""
import argparse
import json
import os
import re
import subprocess
import sys

DEFAULT_PATTERNS = [
    r"""\bt\(\s*['"]([A-Za-z0-9_.:-]+)['"]""",
    r"""(?:i18nKey|labelKey|titleKey)\s*[:=]\s*['"]([A-Za-z0-9_.:-]+)['"]""",
]


def git(repo, *args):
    return subprocess.run(["git", "-C", repo, *args], capture_output=True, text=True)


def load(repo, path, ref=None):
    if ref:
        res = git(repo, "show", f"{ref}:{path}")
        if res.returncode != 0:
            return {}
        raw = res.stdout
    else:
        full = os.path.join(repo, path)
        if not os.path.exists(full):
            return {}
        with open(full, encoding="utf-8") as fh:
            raw = fh.read()
    try:
        return json.loads(raw)
    except json.JSONDecodeError as exc:
        print(f"ERROR cannot parse {path}{' @ ' + ref if ref else ''}: {exc}", file=sys.stderr)
        sys.exit(2)


def resolve(obj, dotted):
    if isinstance(obj, dict) and dotted in obj:  # flat files
        return obj[dotted]
    cur = obj
    for part in re.split(r"[.:]", dotted):
        if isinstance(cur, dict) and part in cur:
            cur = cur[part]
        else:
            return None
    return cur


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", default=None)
    ap.add_argument("--base", default="origin/main")
    ap.add_argument("--locale-file", required=True)
    ap.add_argument("--locales", required=True)
    ap.add_argument("--source-dir", required=True)
    ap.add_argument("--pattern", action="append")
    ap.add_argument("--ext", default=".ts,.tsx,.js,.jsx,.vue,.svelte")
    a = ap.parse_args()

    repo = a.repo or git(os.getcwd(), "rev-parse", "--show-toplevel").stdout.strip() or os.getcwd()
    exts = tuple(e.strip() for e in a.ext.split(",") if e.strip())
    patterns = [re.compile(p) for p in (a.pattern or DEFAULT_PATTERNS)]

    files = git(repo, "diff", f"{a.base}...HEAD", "--name-only", "--diff-filter=d", "--", a.source_dir).stdout.split()
    keys = set()
    for f in files:
        if not f.endswith(exts):
            continue
        with open(os.path.join(repo, f), encoding="utf-8") as fh:
            src = fh.read()
        for p in patterns:
            keys |= set(p.findall(src))

    print(f"Changed source files: {len(files)} · keys referenced: {len(keys)}")
    total = 0
    for loc in [l.strip() for l in a.locales.split(",") if l.strip()]:
        path = a.locale_file.replace("{locale}", loc)
        now, base = load(repo, path), load(repo, path, a.base)
        new = [k for k in sorted(keys) if resolve(now, k) is None and resolve(base, k) is None]
        regress = [k for k in sorted(keys) if resolve(now, k) is None and resolve(base, k) is not None]
        total += len(new) + len(regress)
        print(f"[{loc}] NEW misses: {len(new)} · regressions (resolved on base, missing now): {len(regress)}")
        for k in new:
            print("   new  ", k)
        for k in regress:
            print("   regr ", k)
    sys.exit(1 if total else 0)


if __name__ == "__main__":
    main()
