cmake_minimum_required(VERSION 3.22)

project(CamelPhat VERSION 3.0.0 LANGUAGES C CXX)

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

juce_add_plugin(CamelPhat
    COMPANY_NAME "WvZ"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE CamA
    PLUGIN_CODE Cph3
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "CamelPhat"
)

target_sources(CamelPhat
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/Parameters.h
        Source/DSP/IDSPModule.h
        Source/DSP/BPFilter.h
        Source/DSP/DistortionModule.h
        Source/DSP/MMFilter.h
        Source/DSP/FlangerModule.h
        Source/DSP/MagicEQ.h
        Source/DSP/CompressorModule.h
        Source/DSP/MasterSection.h
        Source/UI/CamelPhatLookAndFeel.cpp
        Source/UI/CamelPhatLookAndFeel.h
        Source/UI/XYPadComponent.cpp
        Source/UI/XYPadComponent.h
)

target_include_directories(CamelPhat PRIVATE Source)

target_compile_features(CamelPhat PRIVATE cxx_std_20)

target_compile_definitions(CamelPhat
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(CamelPhat
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Test target for offline headless DSP verification
add_executable(camelphat_dsp_tests
    Tests/DSPTests.cpp
)

target_compile_features(camelphat_dsp_tests PRIVATE cxx_std_20)
target_include_directories(camelphat_dsp_tests PRIVATE Source)

target_compile_definitions(camelphat_dsp_tests
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(camelphat_dsp_tests
    PRIVATE
        CamelPhat
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_audio_utils
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

enable_testing()
add_test(NAME CamelPhatDSPTests COMMAND camelphat_dsp_tests)

if(APPLE AND TARGET CamelPhat_VST3)
    # Re-apply ad-hoc signature after juce_vst3_helper generates moduleinfo.json
    add_custom_command(TARGET CamelPhat_VST3 POST_BUILD
        COMMAND codesign --force --deep -s - "$<TARGET_BUNDLE_DIR:CamelPhat_VST3>"
        COMMENT "Re-signing VST3 bundle after moduleinfo.json generation"
    )
endif()
