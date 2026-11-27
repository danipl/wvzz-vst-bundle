cmake_minimum_required(VERSION 3.22)

project(WvZzBundle VERSION 0.4.0 LANGUAGES C CXX) # x-release-please-version

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Target architectures" FORCE)
endif()

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# ==============================================================================
# Static Library Target: WvZzModules
# Contains all reusable Core interfaces, UI components, and the audio modules
# ==============================================================================
add_library(WvZzModules STATIC
    # Core Interfaces & DSP
    Source/Core/Interfaces/ProcessContext.h
    Source/Core/Interfaces/IDSPModule.h
    Source/Core/Interfaces/IModuleView.h
    Source/Core/Interfaces/IPluginModule.h
    Source/Core/DSP/AudioVisualizerSource.h
    Source/Core/DSP/LfoPhaseTap.h

    # Core UI
    Source/Core/UI/WvZzLookAndFeel.h
    Source/Core/UI/WvZzLookAndFeel.cpp
    Source/Core/UI/WvZzLogoComponent.h
    Source/Core/UI/IHelpSystem.h
    Source/Core/UI/HelpContentRepository.h
    Source/Core/UI/HelpButtonComponent.h
    Source/Core/UI/HelpOverlayComponent.h
    Source/Core/UI/ModuleCardComponent.h
    Source/Core/UI/XYPadComponent.h
    Source/Core/UI/XYPadComponent.cpp
    Source/Core/UI/StereoVUMeterComponent.h
    Source/Core/UI/SignalComparisonComponent.h
    Source/Core/UI/SignalComparisonComponent.cpp
    Source/Core/UI/ModuleViewControls.h
    Source/Core/UI/LfoPhaseScopeComponent.h
    Source/Core/UI/LfoPhaseScopeComponent.cpp
    Source/Core/UI/DelayTapDisplayComponent.h
    Source/Core/UI/DelayTapDisplayComponent.cpp
    Source/Core/UI/ReverbDecayDisplayComponent.h
    Source/Core/UI/ReverbDecayDisplayComponent.cpp

    # Audio Modules
    Source/Modules/DynDetector/DynDetectorDSP.h
    Source/Modules/DynDetector/DynDetectorDSP.cpp
    Source/Modules/DynDetector/DynDetectorView.h
    Source/Modules/DynDetector/DynDetectorView.cpp
    Source/Modules/DynDetector/DynDetectorModule.h

    Source/Modules/TransientShaper/TransientShaperDSP.h
    Source/Modules/TransientShaper/TransientShaperDSP.cpp
    Source/Modules/TransientShaper/TransientShaperView.h
    Source/Modules/TransientShaper/TransientShaperView.cpp
    Source/Modules/TransientShaper/TransientShaperModule.h

    Source/Modules/MultibandOTT/MultibandOTTDSP.h
    Source/Modules/MultibandOTT/MultibandOTTDSP.cpp
    Source/Modules/MultibandOTT/MultibandOTTView.h
    Source/Modules/MultibandOTT/MultibandOTTView.cpp
    Source/Modules/MultibandOTT/MultibandOTTModule.h

    Source/Modules/RhythmicPumper/RhythmicPumperDSP.h
    Source/Modules/RhythmicPumper/RhythmicPumperDSP.cpp
    Source/Modules/RhythmicPumper/RhythmicPumperView.h
    Source/Modules/RhythmicPumper/RhythmicPumperView.cpp
    Source/Modules/RhythmicPumper/RhythmicPumperModule.h

    Source/Modules/MaximizerClipper/MaximizerClipperDSP.h
    Source/Modules/MaximizerClipper/MaximizerClipperDSP.cpp
    Source/Modules/MaximizerClipper/MaximizerClipperView.h
    Source/Modules/MaximizerClipper/MaximizerClipperView.cpp
    Source/Modules/MaximizerClipper/MaximizerClipperModule.h
    Source/Modules/BPFilter/BPFilterDSP.h
    Source/Modules/BPFilter/BPFilterDSP.cpp
    Source/Modules/BPFilter/BPFilterView.h
    Source/Modules/BPFilter/BPFilterView.cpp
    Source/Modules/BPFilter/BPFilterModule.h

    Source/Modules/Distortion/DistortionDSP.h
    Source/Modules/Distortion/DistortionDSP.cpp
    Source/Modules/Distortion/DistortionView.h
    Source/Modules/Distortion/DistortionView.cpp
    Source/Modules/Distortion/DistortionModule.h

    Source/Modules/MMFilter/EnvelopeFollower.h
    Source/Modules/MMFilter/MMFilterDSP.h
    Source/Modules/MMFilter/MMFilterDSP.cpp
    Source/Modules/MMFilter/MMFilterView.h
    Source/Modules/MMFilter/MMFilterView.cpp
    Source/Modules/MMFilter/MMFilterModule.h

    Source/Modules/Flanger/FlangerDSP.h
    Source/Modules/Flanger/FlangerDSP.cpp
    Source/Modules/Flanger/FlangerView.h
    Source/Modules/Flanger/FlangerView.cpp
    Source/Modules/Flanger/FlangerModule.h

    Source/Modules/Chorus/ChorusDSP.h
    Source/Modules/Chorus/ChorusDSP.cpp
    Source/Modules/Chorus/ChorusView.h
    Source/Modules/Chorus/ChorusView.cpp
    Source/Modules/Chorus/ChorusModule.h

    Source/Modules/Phaser/PhaserDSP.h
    Source/Modules/Phaser/PhaserDSP.cpp
    Source/Modules/Phaser/PhaserView.h
    Source/Modules/Phaser/PhaserView.cpp
    Source/Modules/Phaser/PhaserModule.h

    Source/Modules/Tremolo/TremoloDSP.h
    Source/Modules/Tremolo/TremoloDSP.cpp
    Source/Modules/Tremolo/TremoloView.h
    Source/Modules/Tremolo/TremoloView.cpp
    Source/Modules/Tremolo/TremoloModule.h

    Source/Modules/Delay/DelayLine.h
    Source/Modules/Delay/DelayTapState.h
    Source/Modules/Delay/DelayDSP.h
    Source/Modules/Delay/DelayDSP.cpp
    Source/Modules/Delay/DelayView.h
    Source/Modules/Delay/DelayView.cpp
    Source/Modules/Delay/DelayModule.h

    Source/Modules/Grain/GrainEngine.h
    Source/Modules/Grain/GrainEngine.cpp
    Source/Modules/Grain/GrainDSP.h
    Source/Modules/Grain/GrainDSP.cpp
    Source/Modules/Grain/GrainView.h
    Source/Modules/Grain/GrainView.cpp
    Source/Modules/Grain/GrainModule.h

    Source/Modules/EarlyReflections/EarlyReflectionsDSP.h
    Source/Modules/EarlyReflections/EarlyReflectionsDSP.cpp
    Source/Modules/EarlyReflections/EarlyReflectionsView.h
    Source/Modules/EarlyReflections/EarlyReflectionsView.cpp
    Source/Modules/EarlyReflections/EarlyReflectionsModule.h

    Source/Modules/Reverb/ReverbDecayState.h
    Source/Modules/Reverb/ReverbDSP.h
    Source/Modules/Reverb/ReverbDSP.cpp
    Source/Modules/Reverb/ReverbView.h
    Source/Modules/Reverb/ReverbView.cpp
    Source/Modules/Reverb/ReverbModule.h

    Source/Modules/MagicEQ/MagicEQDSP.h
    Source/Modules/MagicEQ/MagicEQDSP.cpp
    Source/Modules/MagicEQ/MagicEQView.h
    Source/Modules/MagicEQ/MagicEQView.cpp
    Source/Modules/MagicEQ/MagicEQModule.h

    Source/Modules/Compressor/CompressorDSP.h
    Source/Modules/Compressor/CompressorDSP.cpp
    Source/Modules/Compressor/CompressorView.h
    Source/Modules/Compressor/CompressorView.cpp
    Source/Modules/Compressor/CompressorModule.h

    Source/Modules/Modulation/ModulationDSP.h
    Source/Modules/Modulation/ModulationDSP.cpp
    Source/Modules/Modulation/ModulationView.h
    Source/Modules/Modulation/ModulationView.cpp
    Source/Modules/Modulation/ModulationModule.h

    Source/Modules/Master/MasterDSP.h
    Source/Modules/Master/MasterDSP.cpp
    Source/Modules/Master/MasterView.h
    Source/Modules/Master/MasterView.cpp
    Source/Modules/Master/MasterModule.h
)

target_include_directories(WvZzModules PUBLIC Source)
target_compile_features(WvZzModules PUBLIC cxx_std_20)

target_compile_definitions(WvZzModules
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(WvZzModules
    PUBLIC
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ==============================================================================
# Plugin Targets
# wvzz_add_plugin(<Target> <PluginCode> <ProductName> [extra sources...])
# Builds Plugins/<Target>/ as a VST3 + Standalone plugin linked to WvZzModules.
# To add a plugin: create Plugins/<Target>/ with PluginProcessor.{h,cpp},
# PluginEditor.{h,cpp}, and Presets/FactoryPresets.h, add one call below, and add
# the target to PLUGIN_TARGETS in the Makefile.
# ==============================================================================
function(wvzz_add_plugin TARGET CODE PRODUCT_NAME)
    juce_add_plugin(${TARGET}
        COMPANY_NAME "WvZz"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        PLUGIN_MANUFACTURER_CODE WvZz
        PLUGIN_CODE ${CODE}
        FORMATS VST3 Standalone
        PRODUCT_NAME "${PRODUCT_NAME}"
    )

    target_sources(${TARGET}
        PRIVATE
            Plugins/${TARGET}/PluginProcessor.h
            Plugins/${TARGET}/PluginProcessor.cpp
            Plugins/${TARGET}/PluginEditor.h
            Plugins/${TARGET}/PluginEditor.cpp
            Plugins/${TARGET}/Presets/FactoryPresets.h
            ${ARGN}
    )

    target_include_directories(${TARGET} PRIVATE Plugins/${TARGET} Source)
    target_compile_features(${TARGET} PRIVATE cxx_std_20)

    target_compile_definitions(${TARGET}
        PUBLIC
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_VST3_CAN_REPLACE_VST2=0
    )

    target_link_libraries(${TARGET}
        PRIVATE
            WvZzModules
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )

    if(APPLE)
        if(TARGET ${TARGET}_VST3)
            # Re-apply ad-hoc signature after juce_vst3_helper generates moduleinfo.json
            add_custom_command(TARGET ${TARGET}_VST3 POST_BUILD
                COMMAND codesign --force --deep -s - "$<TARGET_BUNDLE_DIR:${TARGET}_VST3>"
                COMMENT "Re-signing ${PRODUCT_NAME} VST3 bundle after moduleinfo.json generation"
            )
        endif()
        if(TARGET ${TARGET}_Standalone)
            add_custom_command(TARGET ${TARGET}_Standalone POST_BUILD
                COMMAND codesign --force --deep -s - "$<TARGET_BUNDLE_DIR:${TARGET}_Standalone>"
                COMMENT "Signing ${PRODUCT_NAME} Standalone app bundle"
            )
        endif()
    endif()
endfunction()

wvzz_add_plugin(WvZzDistortion WzDs "WvZz Distortion")
wvzz_add_plugin(WvZzDynamics   WzDy "WvZz Dynamics")
wvzz_add_plugin(WvZzModulation WzMd "WvZz Modulation")
wvzz_add_plugin(WvZzDelay      WzDl "WvZz Delay")
wvzz_add_plugin(WvZzReverb     WzRv "WvZz Reverb")

# ==============================================================================
# Headless DSP Test Target: wvzz_dsp_tests
# ==============================================================================
add_executable(wvzz_dsp_tests
    Tests/DSPTests.cpp
    Plugins/WvZzDistortion/PluginProcessor.cpp
    Plugins/WvZzDistortion/PluginEditor.cpp
    Plugins/WvZzDynamics/PluginProcessor.cpp
    Plugins/WvZzDynamics/PluginEditor.cpp
    Plugins/WvZzModulation/PluginProcessor.cpp
    Plugins/WvZzModulation/PluginEditor.cpp
    Plugins/WvZzDelay/PluginProcessor.cpp
    Plugins/WvZzDelay/PluginEditor.cpp
    Plugins/WvZzReverb/PluginProcessor.cpp
    Plugins/WvZzReverb/PluginEditor.cpp
)

target_compile_features(wvzz_dsp_tests PRIVATE cxx_std_20)
target_include_directories(wvzz_dsp_tests PRIVATE Source Plugins/WvZzDistortion Plugins/WvZzDynamics ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(wvzz_dsp_tests
    PUBLIC
        WVZZ_NO_CREATE_PLUGIN_FILTER=1
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(wvzz_dsp_tests
    PRIVATE
        WvZzModules
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

enable_testing()
add_test(NAME WvZzDSPTests COMMAND wvzz_dsp_tests)
