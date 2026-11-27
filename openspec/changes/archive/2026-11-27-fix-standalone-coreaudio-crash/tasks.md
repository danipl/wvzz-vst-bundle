# Tasks

## 1. Dependency

- [x] 1.1 Set `GIT_TAG 8.0.15` in `CMakeLists.txt` and the cache key in `.github/workflows/pr.yml`; verify `git -C build/_deps/juce-src describe --tags` prints `8.0.15` and the fetched `juce_CoreAudio_mac.cpp` reads the sample count from the incoming buffers (`mDataByteSize`)

## 2. Source compatibility

- [x] 2.1 Replace the 3 removed `Font::getStringWidth*` calls and the base-class `createEditor()` call in the test; verify a clean `make build` has 0 errors

## 3. Integration check

- [x] 3.1 Run `make clean && make package`; verify all suites pass and 5 VST3 + 5 Standalone are Universal Binary with valid signatures
- [x] 3.2 Launch each of the 5 Standalone apps twice; verify 10 of 10 stay up for 8 seconds and no new crash report appears
- [x] 3.3 Build this branch together with `fix/hidden-test-failures-and-preset-state`; verify all 42 suites pass with real checks
- [x] 3.4 Run `openspec validate --all`; verify it passes

## Workflow follow-up

- Archive this change before the PR merges.
- PR title: `fix(standalone): update JUCE to 8.0.15 to fix the CoreAudio buffer overflow crash`.
- A person should listen to one plugin in a host before the next release.
