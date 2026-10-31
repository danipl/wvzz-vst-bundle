# Tasks

## 1. LookAndFeel & Theme Scaffolding

- [x] 1.1 Implement `Source/UI/CamelPhatLookAndFeel.h` and `Source/UI/CamelPhatLookAndFeel.cpp` defining vector drawing routines for rotary knobs (neon arc, glow pointer, value label), toggles, and combo boxes, and verify compilation.
- [x] 1.2 Implement `Source/UI/ModuleCardComponent.h` providing a consistent visual card panel for grouping module controls with title, border, and status indicators.

## 2. Contextual Help System Architecture

- [x] 2.1 Implement `Source/UI/IHelpSystem.h` declaring `IHelpContentProvider` and `IHelpOverlayHost` interface contracts adhering to SOLID principles.
- [x] 2.2 Implement `Source/UI/HelpContentRepository.h` populating structured, jargon-free explanations, signal flow notes, and tips for all modules and parameters.
- [x] 2.3 Implement `Source/UI/HelpButtonComponent.h` ("?" vector button) and `Source/UI/HelpOverlayComponent.h` (floating explanation bubble with auto-positioning and click-outside dismissal).

## 3. Specialized Custom Widgets

- [x] 3.1 Implement `Source/UI/XYPadComponent.h` and `Source/UI/XYPadComponent.cpp` rendering 2D vector coordinate pads with crosshairs, puck dragging, and dual APVTS parameter attachments.
- [x] 3.2 Implement `Source/UI/StereoVUMeterComponent.h` with 60 FPS timer-driven peak/RMS level animation reading atomically from the engine's `IMeterSource`.

## 4. Editor Assembly & Parameter Wiring

- [x] 4.1 Implement `Source/PluginEditor.h` and `Source/PluginEditor.cpp` assembling all module cards (BP Filter, Distortion, MM Filter, Flanger, Magic EQ, Compressor, Master, dual X/Y pads, and help overlays) with `juce::AudioProcessorValueTreeState::SliderAttachment`.
- [x] 4.2 Update `CamelPhatAudioProcessor::createEditor()` to return `CamelPhatAudioProcessorEditor`, configure fixed window sizing and scalability, and verify compilation into VST3, AU, and Standalone.

## 5. Verification & Testing

- [x] 5.1 Run `ctest` / `camelphat_dsp_tests` to ensure zero-allocation DSP invariants remain intact with GUI connected.
- [x] 5.2 Validate macOS Audio Unit compatibility with `auval -v aumf Cph3 CamA` and verify successful GUI Cocoa view registration.

## Workflow follow-up
- Archive the change after the custom LookAndFeel and Help system are fully verified.
