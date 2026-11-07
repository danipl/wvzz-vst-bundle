# Changelog

## [0.4.0](https://github.com/danipl/wvzz-vst-bundle/compare/v0.3.0...v0.4.0) (2026-11-07)


### Features

* **compressor:** add studio controls, optical ballistics, adaa saturation, and horizontal metering ([#11](https://github.com/danipl/wvzz-vst-bundle/issues/11)) ([02077e9](https://github.com/danipl/wvzz-vst-bundle/commit/02077e9c0d753c270d80052d2dd5b2237b095e6b))
* **ui:** modernize module design with luminous dark glass styling ([#9](https://github.com/danipl/wvzz-vst-bundle/issues/9)) ([e6c28a7](https://github.com/danipl/wvzz-vst-bundle/commit/e6c28a779b9e6292a1495011d50db9258390b02c))

## [0.3.0](https://github.com/danipl/wvzz-vst-bundle/compare/v0.2.0...v0.3.0) (2026-11-07)


### Features

* **distortion:** add hard clip, wavefold, kick tok pre-eq, mono bass and beat-synced lfo ([#7](https://github.com/danipl/wvzz-vst-bundle/issues/7)) ([13cacdd](https://github.com/danipl/wvzz-vst-bundle/commit/13cacdd229d7e6934e93f0afd7ed7b4714095f2b))

## [0.2.0](https://github.com/danipl/wvzz-vst-bundle/compare/v0.1.0...v0.2.0) (2026-11-03)


### Features

* add dual-trace signal comparison visualizer (oscilloscope, spectrum, delta) ([3574123](https://github.com/danipl/wvzz-vst-bundle/commit/3574123f8fbe48c3bb914ab4d95ea08bcd9193cb))
* **arch:** WvZz Modular Audio Plugin Bundle & Flagship Distortion ([#2](https://github.com/danipl/wvzz-vst-bundle/issues/2)) ([a2888e6](https://github.com/danipl/wvzz-vst-bundle/commit/a2888e67ca24c0da9165f560bd597d3a45f97f0d))
* **ci:** add CI/CD release pipeline with release-please and multi-plugin bundling ([#4](https://github.com/danipl/wvzz-vst-bundle/issues/4)) ([59a582e](https://github.com/danipl/wvzz-vst-bundle/commit/59a582e3d8817105cf697c8850a69f68efb675ad))
* **dsp,ui:** implement complete CamelPhat 3 specification and archive OpenSpec ([4b2c85a](https://github.com/danipl/wvzz-vst-bundle/commit/4b2c85a0a8e38742db393903a52411ccc0b55cdd))
* **dsp:** implement CamelPhat 3 DSP engine and JUCE 8 plugin architecture ([5496230](https://github.com/danipl/wvzz-vst-bundle/commit/54962307bae94b021ff931c7d5265903295a352e))
* **ui:** display signal monitor by default in center column ([#3](https://github.com/danipl/wvzz-vst-bundle/issues/3)) ([a007a7a](https://github.com/danipl/wvzz-vst-bundle/commit/a007a7a9a8d49af277bd29784c0824f0f9805636))
* **ui:** implement modern vector LookAndFeel, dual XY pads, VU meters, and in-plugin help system ([ce2f051](https://github.com/danipl/wvzz-vst-bundle/commit/ce2f0515b733ae0229c815497d7869f7eb624629))
* **ui:** initialize Filter X/Y and Drive X/Y pucks at the center of the axes ([95b29b4](https://github.com/danipl/wvzz-vst-bundle/commit/95b29b4e74684f81edd7d0ea5bb235edb1e60a3d))
* **ui:** responsive 1080x680 redesign, header toolbar, custom visualizers, and install sync ([e0a48d4](https://github.com/danipl/wvzz-vst-bundle/commit/e0a48d498e681da691f7c54e1e7e5e7770937fba))


### Bug Fixes

* **build:** re-sign VST3 bundle after moduleinfo.json generation to satisfy macOS codesign ([cdb7e03](https://github.com/danipl/wvzz-vst-bundle/commit/cdb7e03b0b8d78e2297f6a71bdcf12d8a4374780))
* resolve BP filter crossover, activate modulation routing, and add dual I/O meters ([1ea5ef0](https://github.com/danipl/wvzz-vst-bundle/commit/1ea5ef02a56ee72feb21a404b9c8c736153e3bbb))
* **ui:** adjust knob layout margins, dial scaling, and initialize XY crosshairs at top-left ([3789379](https://github.com/danipl/wvzz-vst-bundle/commit/37893795b0528a1f743401ffb329be86b46794b5))
* **ui:** pass valid dashLengths pattern to drawDashedLine to prevent null pointer crash ([26fcb04](https://github.com/danipl/wvzz-vst-bundle/commit/26fcb04c8311d907ec79d7b07d9bfebce2a5d796))
* **ui:** use getBounds() instead of getLocalBounds() for card content placement ([f4f2029](https://github.com/danipl/wvzz-vst-bundle/commit/f4f20292057ce59ff3fbab97243e0aff74fa4a7a))
