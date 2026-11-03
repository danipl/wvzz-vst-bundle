# ==============================================================================
# WvZz Bundle Makefile
# High-level build, test, and installation automation for macOS VST3 / Standalone
# ==============================================================================

BUILD_DIR      := build
DIST_DIR       := dist
VERSION        := $(shell sed -n 's/.*VERSION \([0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\).*/\1/p' CMakeLists.txt | head -n 1)
TARGET_VST3    := $(BUILD_DIR)/WvZzDistortion_artefacts/Release/VST3/WvZz\ Distortion.vst3
TARGET_APP     := $(BUILD_DIR)/WvZzDistortion_artefacts/Release/Standalone/WvZz\ Distortion.app

# Destination paths
USER_VST3_DIR  := $(HOME)/Library/Audio/Plug-Ins/VST3
SYS_VST3_DIR   := /Library/Audio/Plug-Ins/VST3

.PHONY: all config build vst3 standalone test package install install-vst3 install-system clean status help

# Default target: build everything
all: build

# ------------------------------------------------------------------------------
# Help
# ------------------------------------------------------------------------------
help:
	@echo "WvZz Bundle Build System Commands:"
	@echo "  make config          - Configure CMake project with Ninja"
	@echo "  make build           - Build all targets (WvZzModules, WvZzDistortion VST3 & App, Tests)"
	@echo "  make vst3            - Build only the WvZz Distortion VST3 bundle"
	@echo "  make standalone      - Build WvZz Distortion Standalone desktop application"
	@echo "  make test            - Run zero-allocation DSP quantitative tests (wvzz_dsp_tests)"
	@echo "  make install         - Copy .vst3 to ~/Library/Audio/Plug-Ins/VST3/"
	@echo "  make install-vst3    - Copy only .vst3 to ~/Library/Audio/Plug-Ins/VST3/"
	@echo "  make install-system  - Copy .vst3 to /Library/Audio/Plug-Ins/VST3/ (requires sudo)"
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
	@cmake --build $(BUILD_DIR) --target WvZzDistortion_VST3
	@echo "Built VST3 bundle at: $(TARGET_VST3)"

standalone: $(BUILD_DIR)/build.ninja
	@cmake --build $(BUILD_DIR) --target WvZzDistortion_Standalone
	@echo "Built Standalone app at: $(TARGET_APP)"

# ------------------------------------------------------------------------------
# Verification & Tests
# ------------------------------------------------------------------------------
test: $(BUILD_DIR)/build.ninja
	@cmake --build $(BUILD_DIR) --target wvzz_dsp_tests
	@ctest --test-dir $(BUILD_DIR) --output-on-failure

# ------------------------------------------------------------------------------
# Multi-Plugin Release Packaging
# ------------------------------------------------------------------------------
package: build test
	@echo "Packaging WvZz multi-plugin release artifacts for v$(VERSION)..."
	@rm -rf $(DIST_DIR)
	@mkdir -p $(DIST_DIR)/staging/VST3 $(DIST_DIR)/staging/Standalone
	@# Collect all VST3 bundles across build/*_artefacts/Release/
	@find $(BUILD_DIR) -type d -path '*/Release/VST3/*.vst3' | while read -r vst3; do \
		name=$$(basename "$$vst3"); \
		echo "Found VST3 bundle: $$vst3"; \
		codesign --verify --deep --strict "$$vst3" || { echo "Codesign verification failed for $$vst3"; exit 1; }; \
		binary="$$vst3/Contents/MacOS/$${name%.vst3}"; \
		if [ -f "$$binary" ]; then \
			archs=$$(lipo -archs "$$binary"); \
			echo "Architecture check for $$binary: $$archs"; \
			echo "$$archs" | grep -q "arm64" || { echo "Missing arm64 in $$binary"; exit 1; }; \
			echo "$$archs" | grep -q "x86_64" || { echo "Missing x86_64 in $$binary"; exit 1; }; \
		fi; \
		cp -R "$$vst3" $(DIST_DIR)/staging/VST3/; \
	done
	@# Collect all Standalone apps across build/*_artefacts/Release/
	@find $(BUILD_DIR) -type d -path '*/Release/Standalone/*.app' | while read -r app; do \
		name=$$(basename "$$app"); \
		echo "Found Standalone app: $$app"; \
		codesign --force --deep -s - "$$app"; \
		codesign --verify --deep --strict "$$app" || { echo "Codesign verification failed for $$app"; exit 1; }; \
		binary="$$app/Contents/MacOS/$${name%.app}"; \
		if [ -f "$$binary" ]; then \
			archs=$$(lipo -archs "$$binary"); \
			echo "Architecture check for $$binary: $$archs"; \
			echo "$$archs" | grep -q "arm64" || { echo "Missing arm64 in $$binary"; exit 1; }; \
			echo "$$archs" | grep -q "x86_64" || { echo "Missing x86_64 in $$binary"; exit 1; }; \
		fi; \
		cp -R "$$app" $(DIST_DIR)/staging/Standalone/; \
	done
	@# Create format-split universal archives using ditto (preserves bundle metadata and code signature)
	@cd $(DIST_DIR)/staging && ditto -c -k --keepParent VST3 ../WvZz-VST3-v$(VERSION)-macOS-universal.zip
	@cd $(DIST_DIR)/staging && ditto -c -k --keepParent Standalone ../WvZz-Standalone-v$(VERSION)-macOS-universal.zip
	@rm -rf $(DIST_DIR)/staging
	@echo "Packaged release archives in $(DIST_DIR):"
	@ls -lh $(DIST_DIR)/*.zip

# ------------------------------------------------------------------------------
# Installation Targets
# ------------------------------------------------------------------------------
install: vst3
	@mkdir -p $(USER_VST3_DIR)
	@rm -rf $(USER_VST3_DIR)/"WvZz Distortion.vst3"
	@cp -R $(TARGET_VST3) $(USER_VST3_DIR)/
	@codesign --force --deep -s - $(USER_VST3_DIR)/"WvZz Distortion.vst3"
	@if [ -w $(SYS_VST3_DIR) ]; then \
		cp -R $(TARGET_VST3) $(SYS_VST3_DIR)/ && \
		codesign --force --deep -s - $(SYS_VST3_DIR)/"WvZz Distortion.vst3" && \
		echo "Synchronized system VST3 at $(SYS_VST3_DIR)/"; \
	fi
	@echo "Installed and signed WvZz Distortion.vst3 to $(USER_VST3_DIR)/"

install-vst3: install

install-system: vst3
	@sudo mkdir -p $(SYS_VST3_DIR)
	@sudo rm -rf $(SYS_VST3_DIR)/"WvZz Distortion.vst3"
	@sudo cp -R $(TARGET_VST3) $(SYS_VST3_DIR)/
	@sudo codesign --force --deep -s - $(SYS_VST3_DIR)/"WvZz Distortion.vst3"
	@echo "Installed system WvZz Distortion.vst3 to $(SYS_VST3_DIR)/"

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Cleaned $(BUILD_DIR)"

status:
	@echo "=== Installed WvZz Audio Plug-ins ==="
	@echo "User VST3:"
	@ls -ld $(USER_VST3_DIR)/"WvZz Distortion.vst3" 2>/dev/null || echo "  (not found)"
	@echo "System VST3:"
	@ls -ld $(SYS_VST3_DIR)/"WvZz Distortion.vst3" 2>/dev/null || echo "  (not found)"
