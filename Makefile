# ==============================================================================
# CamelPhat Makefile
# High-level build, test, and installation automation for macOS VST3 / AU
# ==============================================================================

BUILD_DIR      := build
TARGET_VST3    := $(BUILD_DIR)/CamelPhat_artefacts/VST3/CamelPhat.vst3
TARGET_AU      := $(BUILD_DIR)/CamelPhat_artefacts/AU/CamelPhat.component
TARGET_APP     := $(BUILD_DIR)/CamelPhat_artefacts/Standalone/CamelPhat.app

# Destination paths
USER_VST3_DIR  := $(HOME)/Library/Audio/Plug-Ins/VST3
USER_AU_DIR    := $(HOME)/Library/Audio/Plug-Ins/Components
SYS_VST3_DIR   := /Library/Audio/Plug-Ins/VST3
SYS_AU_DIR     := /Library/Audio/Plug-Ins/Components

.PHONY: all config build vst3 au standalone test install install-system clean status help

# Default target: build everything
all: build

# ------------------------------------------------------------------------------
# Help
# ------------------------------------------------------------------------------
help:
	@echo "CamelPhat Build System Commands:"
	@echo "  make config          - Configure CMake project with Ninja"
	@echo "  make build           - Build all targets (VST3, AU, App, Tests)"
	@echo "  make vst3            - Build only the .vst3 bundle"
	@echo "  make au              - Build only the Audio Unit (.component) bundle"
	@echo "  make standalone      - Build Standalone desktop application"
	@echo "  make test            - Run zero-allocation DSP quantitative tests"
	@echo "  make install         - Copy .vst3 & .component to ~/Library/Audio/Plug-Ins/"
	@echo "  make install-vst3    - Copy only .vst3 to ~/Library/Audio/Plug-Ins/VST3/"
	@echo "  make install-system  - Copy .vst3 to /Library/Audio/Plug-Ins/VST3/ (requires sudo)"
	@echo "  make validate-au     - Run macOS 'auval' tool on the installed AU"
	@echo "  make clean           - Remove build directory"

# ------------------------------------------------------------------------------
# CMake Configuration & Build
# ------------------------------------------------------------------------------
config:
	@cmake -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Release

$(BUILD_DIR)/build.ninja:
	@$(MAKE) config

build: $(BUILD_DIR)/build.ninja
	@cmake --build $(BUILD_DIR)

vst3: $(BUILD_DIR)/build.ninja
	@cmake --build $(BUILD_DIR) --target CamelPhat_VST3
	@echo "Built VST3 bundle at: $(TARGET_VST3)"

au: $(BUILD_DIR)/build.ninja
	@cmake --build $(BUILD_DIR) --target CamelPhat_AU
	@echo "Built AU bundle at: $(TARGET_AU)"

standalone: $(BUILD_DIR)/build.ninja
	@cmake --build $(BUILD_DIR) --target CamelPhat_Standalone
	@echo "Built Standalone app at: $(TARGET_APP)"

# ------------------------------------------------------------------------------
# Verification & Tests
# ------------------------------------------------------------------------------
test: $(BUILD_DIR)/build.ninja
	@cmake --build $(BUILD_DIR) --target camelphat_dsp_tests
	@ctest --test-dir $(BUILD_DIR) --output-on-failure

validate-au:
	@killall -9 AudioComponentRegistrar 2>/dev/null || true
	@auval -v aumf Cph3 CamA

# ------------------------------------------------------------------------------
# Installation Targets
# ------------------------------------------------------------------------------
# User installation (~/Library/Audio/Plug-Ins/) - does not require sudo
install: vst3 au
	@mkdir -p $(USER_VST3_DIR) $(USER_AU_DIR)
	@rm -rf $(USER_VST3_DIR)/CamelPhat.vst3
	@rm -rf $(USER_AU_DIR)/CamelPhat.component
	@cp -R $(TARGET_VST3) $(USER_VST3_DIR)/
	@cp -R $(TARGET_AU) $(USER_AU_DIR)/
	@codesign --force --deep -s - $(USER_VST3_DIR)/CamelPhat.vst3
	@codesign --force --deep -s - $(USER_AU_DIR)/CamelPhat.component
	@if [ -w $(SYS_VST3_DIR) ]; then \
		cp -R $(TARGET_VST3) $(SYS_VST3_DIR)/ && \
		codesign --force --deep -s - $(SYS_VST3_DIR)/CamelPhat.vst3 && \
		echo "Synchronized system VST3 at $(SYS_VST3_DIR)/"; \
	fi
	@killall -9 AudioComponentRegistrar 2>/dev/null || true
	@echo "Installed and signed CamelPhat.vst3 to $(USER_VST3_DIR)/"
	@echo "Installed and signed CamelPhat.component to $(USER_AU_DIR)/"

install-vst3: vst3
	@mkdir -p $(USER_VST3_DIR)
	@rm -rf $(USER_VST3_DIR)/CamelPhat.vst3
	@cp -R $(TARGET_VST3) $(USER_VST3_DIR)/
	@codesign --force --deep -s - $(USER_VST3_DIR)/CamelPhat.vst3
	@echo "Installed and signed CamelPhat.vst3 to $(USER_VST3_DIR)/"

# System-wide installation (/Library/Audio/Plug-Ins/VST3)
install-system: vst3
	@if [ "$$(id -u)" -eq 0 ]; then \
		mkdir -p $(SYS_VST3_DIR); \
		rm -rf $(SYS_VST3_DIR)/CamelPhat.vst3; \
		cp -R $(TARGET_VST3) $(SYS_VST3_DIR)/; \
		codesign --force --deep -s - $(SYS_VST3_DIR)/CamelPhat.vst3; \
		echo "Installed and signed CamelPhat.vst3 to $(SYS_VST3_DIR)/"; \
	else \
		echo "Copying to system folder $(SYS_VST3_DIR) requires sudo:"; \
		sudo mkdir -p $(SYS_VST3_DIR); \
		sudo rm -rf $(SYS_VST3_DIR)/CamelPhat.vst3; \
		sudo cp -R $(TARGET_VST3) $(SYS_VST3_DIR)/; \
		sudo codesign --force --deep -s - $(SYS_VST3_DIR)/CamelPhat.vst3; \
		echo "Installed and signed CamelPhat.vst3 to $(SYS_VST3_DIR)/"; \
	fi

# ------------------------------------------------------------------------------
# Clean
# ------------------------------------------------------------------------------
clean:
	@rm -rf $(BUILD_DIR)
	@echo "Cleaned build directory."
