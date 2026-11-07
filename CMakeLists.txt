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
# Contains all reusable Core interfaces, UI components, and the 8 audio modules
# ==============================================================================
add_library(WvZzModules STATIC
    # Core Interfaces & DSP
    Source/Core/Interfaces/ProcessContext.h
    Source/Core/Interfaces/IDSPModule.h
    Source/Core/Interfaces/IModuleView.h
    Source/Core/Interfaces/IPluginModule.h
    Source/Core/DSP/AudioVisualizerSource.h

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

    # Audio Modules
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
# Flagship Plugin Target: WvZzDistortion
# ==============================================================================
juce_add_plugin(WvZzDistortion
    COMPANY_NAME "WvZz"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE WvZz
    PLUGIN_CODE WzDs
    FORMATS VST3 Standalone
    PRODUCT_NAME "WvZz Distortion"
)

target_sources(WvZzDistortion
    PRIVATE
        Plugins/WvZzDistortion/PluginProcessor.h
        Plugins/WvZzDistortion/PluginProcessor.cpp
        Plugins/WvZzDistortion/PluginEditor.h
        Plugins/WvZzDistortion/PluginEditor.cpp
        Plugins/WvZzDistortion/Presets/FactoryPresets.h
)

target_include_directories(WvZzDistortion PRIVATE Plugins/WvZzDistortion Source)
target_compile_features(WvZzDistortion PRIVATE cxx_std_20)

target_compile_definitions(WvZzDistortion
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(WvZzDistortion
    PRIVATE
        WvZzModules
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if(APPLE)
    if(TARGET WvZzDistortion_VST3)
        # Re-apply ad-hoc signature after juce_vst3_helper generates moduleinfo.json
        add_custom_command(TARGET WvZzDistortion_VST3 POST_BUILD
            COMMAND codesign --force --deep -s - "$<TARGET_BUNDLE_DIR:WvZzDistortion_VST3>"
            COMMENT "Re-signing VST3 bundle after moduleinfo.json generation"
        )
    endif()
    if(TARGET WvZzDistortion_Standalone)
        add_custom_command(TARGET WvZzDistortion_Standalone POST_BUILD
            COMMAND codesign --force --deep -s - "$<TARGET_BUNDLE_DIR:WvZzDistortion_Standalone>"
            COMMENT "Signing Standalone app bundle"
        )
    endif()
endif()

# ==============================================================================
# Headless DSP Test Target: wvzz_dsp_tests
# ==============================================================================
add_executable(wvzz_dsp_tests
    Tests/DSPTests.cpp
    Plugins/WvZzDistortion/PluginProcessor.cpp
    Plugins/WvZzDistortion/PluginEditor.cpp
)

target_compile_features(wvzz_dsp_tests PRIVATE cxx_std_20)
target_include_directories(wvzz_dsp_tests PRIVATE Source Plugins/WvZzDistortion ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(wvzz_dsp_tests
    PUBLIC
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
