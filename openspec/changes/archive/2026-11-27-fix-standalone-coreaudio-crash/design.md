# Design

## Context

See proposal.md - Why. The AddressSanitizer report from GH-15:

    heap-buffer-overflow, WRITE of size 4
    #0 juce::CoreAudioClasses::CoreAudioInternal::audioCallback  juce_CoreAudio_mac.cpp:795
    0x... is located 0 bytes after 1296-byte region allocated by CoreAudioInternal::allocateTempBuffers  juce_CoreAudio_mac.cpp:361

In 8.0.4 the callback copies `bufferSize` samples per channel into buffers sized at open time and trusts that the device delivers exactly that many.

## Goals / Non-Goals

**Goals:**
- Remove the out-of-bounds write in the Standalone audio callback.
- Keep the build, the tests, and the UI layout unchanged.

**Non-Goals:**
- Patching JUCE in this repository.

## Decisions

1. **Update JUCE instead of patching it.** The upstream fix reads the real sample count from the incoming buffers and splits a larger buffer into chunks that fit. Carrying a private patch would have to be re-applied on every update. Alternative: pin 8.0.9, the first release with the fix; rejected in favor of 8.0.15, which also has the later CoreAudio fixes of the 8.0 line. The `-security` tags (8.0.16, 8.0.17) are back-ports made after JUCE 9; they can follow in a separate update.
2. **Text width API.** `juce::GlyphArrangement::getStringWidthInt(font, text)` replaces `Font::getStringWidth` (same whole-pixel result), `getStringWidth(font, text)` replaces `getStringWidthFloat`.
3. **No spec change.** The specs do not name a JUCE version.

## Risks / Trade-offs

- [The crash could not be triggered on demand] → After 08:09 the same 8.0.4 builds ran 10 of 10 launches without a crash, so the trigger depends on the audio device state. The fix is judged by the matching root cause (sanitizer stack and upstream commit message), not by a before/after crash count. Stated in the PR.
- [A newer JUCE changes behavior somewhere else] → Clean build, all test suites, `make package` for 5 VST3 + 5 Standalone, and 10 launches of the Standalone apps. Listening in a host is left to a person.
