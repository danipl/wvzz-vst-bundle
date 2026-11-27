# ==============================================================================
# WvZz Bundle Makefile
# High-level build, test, and installation automation for macOS VST3 / Standalone
# ==============================================================================

BUILD_DIR         := build
DIST_DIR          := dist
VERSION           := $(shell sed -n 's/.*VERSION \([0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\).*/\1/p' CMakeLists.txt | head -n 1)

# ------------------------------------------------------------------------------
# Plugin list
# To add a plugin: add its CMake target to PLUGIN_TARGETS, then define
# KEY_<Target> (short name used in `make <key>-vst3`) and PRODUCT_<Target>
# (bundle name, may contain spaces). Every target below loops over this list.
# ------------------------------------------------------------------------------
PLUGIN_TARGETS         := WvZzDistortion WvZzDynamics WvZzModulation WvZzDelay WvZzReverb

KEY_WvZzDistortion     := distortion
PRODUCT_WvZzDistortion := WvZz Distortion

KEY_WvZzDynamics       := dynamics
PRODUCT_WvZzDynamics   := WvZz Dynamics

KEY_WvZzModulation     := modulation
PRODUCT_WvZzModulation := WvZz Modulation

KEY_WvZzDelay          := delay
PRODUCT_WvZzDelay      := WvZz Delay

KEY_WvZzReverb         := reverb
PRODUCT_WvZzReverb     := WvZz Reverb

PLUGIN_KEYS            := $(foreach t,$(PLUGIN_TARGETS),$(KEY_$(t)))

# Destination paths
USER_VST3_DIR     := $(HOME)/Library/Audio/Plug-Ins/VST3
SYS_VST3_DIR      := /Library/Audio/Plug-Ins/VST3

# Built bundle paths for one target: $(call vst3_path,<Target>) / $(call app_path,<Target>)
vst3_path = $(BUILD_DIR)/$(1)_artefacts/Release/VST3/$(PRODUCT_$(1)).vst3
app_path  = $(BUILD_DIR)/$(1)_artefacts/Release/Standalone/$(PRODUCT_$(1)).app

# Copy one built VST3 to a folder and ad-hoc sign it.
# $(call install_vst3,<Target>,<dest dir>,<command prefix, e.g. sudo>)
define install_vst3
	@$(3) rm -rf "$(2)/$(PRODUCT_$(1)).vst3"
	@$(3) cp -R "$(call vst3_path,$(1))" "$(2)/"
	@$(3) codesign --force --deep -s - "$(2)/$(PRODUCT_$(1)).vst3"
	@echo "Installed and signed $(PRODUCT_$(1)).vst3 to $(2)/"

endef

.PHONY: all config build vst3 standalone test package install install-vst3 install-system clean status help \
        $(foreach k,$(PLUGIN_KEYS),$(k)-vst3 $(k)-standalone)

# Default target: build everything
all: build

# ------------------------------------------------------------------------------
# Help
# ------------------------------------------------------------------------------
help:
	@echo "WvZz Bundle Build System Commands:"
	@echo "  make config                - Configure CMake project with Ninja"
	@echo "  make build                 - Build all targets (WvZzModules, all plugins, tests)"
	@echo "  make vst3                  - Build all VST3 bundles"
	@echo "  make standalone            - Build all Standalone desktop applications"
	@$(foreach t,$(PLUGIN_TARGETS),printf "  make %-22s - Build only $(PRODUCT_$(t)) VST3\n" "$(KEY_$(t))-vst3"; printf "  make %-22s - Build only $(PRODUCT_$(t)) Standalone app\n" "$(KEY_$(t))-standalone";)
	@echo "  make test                  - Run zero-allocation DSP quantitative tests (wvzz_dsp_tests)"
	@echo "  make package               - Package and verify Universal Binary release archives"
	@echo "  make install               - Copy all .vst3 bundles to ~/Library/Audio/Plug-Ins/VST3/"
	@echo "  make install-vst3          - Same as make install"
	@echo "  make install-system        - Copy all .vst3 bundles to /Library/Audio/Plug-Ins/VST3/ (requires sudo)"
	@echo "  make clean                 - Remove build directory"
	@echo "  make status                - Check installed plugins status"

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
	@cmake --build $(BUILD_DIR) --target $(addsuffix _VST3,$(PLUGIN_TARGETS))
	@echo "Built VST3 bundles in $(BUILD_DIR)/*_artefacts/Release/VST3/"

standalone: $(BUILD_DIR)/build.ninja
	@cmake --build $(BUILD_DIR) --target $(addsuffix _Standalone,$(PLUGIN_TARGETS))
	@echo "Built Standalone apps in $(BUILD_DIR)/*_artefacts/Release/Standalone/"

# Per-plugin targets: make <key>-vst3 and make <key>-standalone
define plugin_rules
$(KEY_$(1))-vst3: $$(BUILD_DIR)/build.ninja
	@cmake --build $$(BUILD_DIR) --target $(1)_VST3
	@echo "Built $(PRODUCT_$(1)) VST3 at: $(call vst3_path,$(1))"

$(KEY_$(1))-standalone: $$(BUILD_DIR)/build.ninja
	@cmake --build $$(BUILD_DIR) --target $(1)_Standalone
	@echo "Built $(PRODUCT_$(1)) Standalone app at: $(call app_path,$(1))"

endef
$(foreach t,$(PLUGIN_TARGETS),$(eval $(call plugin_rules,$(t))))

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
	$(foreach t,$(PLUGIN_TARGETS),$(call install_vst3,$(t),$(USER_VST3_DIR),))
	@if [ -w $(SYS_VST3_DIR) ]; then $(MAKE) --no-print-directory sync-system; fi

# Internal: refresh the system folder without sudo when it is writable
.PHONY: sync-system
sync-system:
	$(foreach t,$(PLUGIN_TARGETS),$(call install_vst3,$(t),$(SYS_VST3_DIR),))

install-vst3: install

install-system: vst3
	@sudo mkdir -p $(SYS_VST3_DIR)
	$(foreach t,$(PLUGIN_TARGETS),$(call install_vst3,$(t),$(SYS_VST3_DIR),sudo))

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Cleaned $(BUILD_DIR)"

status:
	@echo "=== Installed WvZz Audio Plug-ins ==="
	@echo "User VST3:"
	@$(foreach t,$(PLUGIN_TARGETS),ls -ld "$(USER_VST3_DIR)/$(PRODUCT_$(t)).vst3" 2>/dev/null || echo "  $(PRODUCT_$(t)): (not found)";)
	@echo "System VST3:"
	@$(foreach t,$(PLUGIN_TARGETS),ls -ld "$(SYS_VST3_DIR)/$(PRODUCT_$(t)).vst3" 2>/dev/null || echo "  $(PRODUCT_$(t)): (not found)";)
