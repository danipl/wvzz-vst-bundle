# gsdlc Communication Standard

Default standard for every `gsdlc-*` skill. A project can replace it with its own file
(`communication.standard` in `.agents/sdlc.yaml`). Many readers are not native English speakers:
optimize for fast, unambiguous understanding.

## 1. Structure — the reader understands the result in 10 seconds

- Start every report with a TL;DR line:
  `> **TL;DR** — <STATUS>: <what happened, one sentence>. **You need to:** <one action, or "nothing">.`
- `<STATUS>` is one of `DONE`, `NEEDS YOUR DECISION`, `BLOCKED`, `FAILED`, `IN PROGRESS`.
- Then the skill's fixed sections, in order. Facts go in tables or short lists, not paragraphs.
- Name things exactly: file paths, ticket ids, numbers, commands. Never "some files" or "a few issues".
- End with one clear next step. Never end with a vague offer like "let me know if you need anything".

## 2. Plain English — clear for a non-native reader

- Short sentences (about 20 words or fewer), one idea per sentence, active voice.
- Common words: "use" not "leverage", "check" not "validate the integrity of".
- Explain each project or technical term the first time it appears, in a few words.
  Example: "drift (the code no longer matches what the ticket or spec says)".
- Say what you checked and how: "I ran `go test ./...`: 214 passed, 0 failed", not "tests look good".
- No filler, hype, or idioms. Do not use: delve, leverage, robust, seamless, comprehensive, crucial,
  holistic, streamline, cutting-edge, game-changer, "it's worth noting", "in today's landscape",
  "under the hood", "out of the box", "I hope this helps", "Great question".
- Do not over-hedge. If you verified something, state it plainly. If you did not, say exactly that.

## 3. Every question carries a recommendation and a reason

- Never ask a bare question. Every question, choice, or approval gate ends with:
  `**Recommendation:** <option> — <reason in one sentence, based on evidence>.`
- When offering options, number them and mark one `(Recommended)`.
- If you have no basis to recommend, say why and which information would let you decide.

## 4. Language

- Converse in the user's language.
- Tickets, specs, ADRs, code, commits, PRs, and report files use `{{project.artifact_language}}`
  (default English).
