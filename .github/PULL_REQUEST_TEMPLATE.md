<!-- 
  ==============================================================================
  PULL REQUEST AUTHORING GUIDELINES:
  1. Plain English: Write short, direct, human-friendly sentences explaining
     what changed and why.
  2. Strict NO AI SLOP rule: Avoid AI buzzwords and filler phrases (e.g. delve,
     leverage, robust, seamless, comprehensive, crucial, holistic, streamline,
     cutting-edge, game-changer, "under the hood", "in today's landscape").
  3. Conventional Commits Title: Your PR title drives release-please semver bumps
     and MUST follow: ^(fix|feat|docs|refactor|test|ci|chore|revert)(\([^)]+\))?!?: .+
  4. OpenSpec Gate: All work in openspec/changes/ must be archived via
     `openspec archive <change-id>` before merging to main.
  ==============================================================================
-->

## Description

<!-- 
  Plain-English summary of what this PR does and why it is needed.
  If this change corresponds to an OpenSpec change, reference its archived path:
  e.g., `openspec/changes/archive/2026-11-03-add-agents-guide-and-pr-template/`
-->

Fixes #[issue number or ticket]

## Type of Change

- [ ] Bug fix (non-breaking change that fixes an audio glitch, crash, or defect)
- [ ] New feature (non-breaking change adding DSP module, UI feature, or capability)
- [ ] Breaking change (fix or feature modifying interfaces, APVTS IDs, or host contracts)
- [ ] Documentation update (specs, user manuals, guides)
- [ ] Refactoring (internal cleanup with zero audio/behavioral changes)
- [ ] CI/CD or Build system change

## Changes Made

<!-- 
  High-level bulleted summary of functional and behavioral changes.
  Keep descriptions concrete and human-friendly (no filler).
-->
- 

## Key Files Changed

<!-- 
  Table or list highlighting important files touched and their concrete responsibility.
-->

| File Path | Description of Changes |
| :--- | :--- |
| `Source/...` |  |
| `Tests/...` |  |

## Architecture / Signal Flow Diagram (if applicable)

<!-- 
  Include a GitHub-compatible Mermaid (```mermaid ... ```) or plain ASCII diagram 
  if this PR introduces or changes signal routing, module topologies, state machines,
  or workflow lifecycles.
  Tip: Quote Mermaid node labels containing parentheses: A["BPFilter (Cytomic ZDF)"]
-->

```text
[ Input ] ---> [ Module A ] ---> [ Module B ] ---> [ Output ]
```

## Testing

Describe the tests and verification steps you performed:
- [ ] Unit & DSP tests pass (`make test`) — zero audio-thread heap allocations
- [ ] Multi-plugin universal packaging verified (`make package`) — `arm64` + `x86_64` Mach-O architectures and codesign valid
- [ ] Manual testing performed in Standalone or DAW (`make install-vst3`)

**Test Configuration:**
- OS: macOS (Apple Silicon / Intel)
- Host/DAW: (e.g. Ableton Live 12, Reaper 7, Bitwig, or WvZz Standalone)
- Build System: CMake + Ninja (Release)

## Checklist

- [ ] **Conventional PR Title:** PR title matches `^(fix|feat|docs|refactor|test|ci|chore|revert)(\([^)]+\))?!?: .+`
- [ ] **OpenSpec Archived:** Any `openspec/changes/` work is archived (`openspec archive <id>`) and `openspec validate --all` passes
- [ ] **Real-Time Safety:** Strictly zero dynamic memory allocations (`new`, `malloc`, `push_back`), zero locks, and `noexcept` inside audio rendering callbacks
- [ ] **SOLID Separation:** DSP logic (`IDSPModule`) decoupled from GUI views (`IModuleView`); no GUI includes in DSP headers
- [ ] **Parameter Prefixing:** All APVTS parameters respect `instancePrefix` for multi-instance chaining
- [ ] **No AI Slop:** Description is written in clear, natural human English without AI clichés
- [ ] **Documentation Updated:** Corresponding updates made to `README.md`, `MANUAL.md`, or plugin user guides if behavior changed

## Screenshots / Visuals (if applicable)

<!-- 
  Add screenshots, UI mockups, or analyzer traces if this PR affects GUI layout or visualizers.
-->

## Related Issues

<!-- 
  Link related issues or PRs: Closes #12, Relates to #4
-->

## Additional Notes

<!-- 
  Any edge cases, trade-offs, or follow-up tasks reviewers should be aware of.
-->
