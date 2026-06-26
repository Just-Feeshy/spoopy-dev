.PHONY: setup reconfigure build clean install fresh macos-deployment-target

# Detect OS
UNAME_S := $(shell uname -s)
SUPPORT_OPUS_FILES ?= auto
DARWIN_DEPLOYMENT_FILE := build/macos-deployment-target.ini

# Default target
all: setup build

macos-deployment-target:
	mkdir -p $(dir $(DARWIN_DEPLOYMENT_FILE))
	node scripts/macos_deployment_target.js --support-opus-files $(SUPPORT_OPUS_FILES) > $(DARWIN_DEPLOYMENT_FILE)

# Setup build directory with platform-specific configuration
ifeq ($(UNAME_S),Darwin)
setup: macos-deployment-target
	meson setup build/ \
		--native-file misc/cross/common.ini \
		--native-file misc/cross/darwin-crappy-sdl3-hack.ini \
		--native-file misc/cross/arm64-darwin.ini \
		--native-file $(DARWIN_DEPLOYMENT_FILE) \
		-Dsupport_opus_files=$(SUPPORT_OPUS_FILES)
else
setup:
	meson setup build/ \
		--native-file misc/cross/common.ini
endif

ifeq ($(UNAME_S),Darwin)
reconfigure: macos-deployment-target
	meson setup build/ --reconfigure \
		--native-file misc/cross/common.ini \
		--native-file misc/cross/darwin-crappy-sdl3-hack.ini \
		--native-file misc/cross/arm64-darwin.ini \
		--native-file $(DARWIN_DEPLOYMENT_FILE) \
		-Dsupport_opus_files=$(SUPPORT_OPUS_FILES)
else
reconfigure:
	meson setup build/ --reconfigure \
		--native-file misc/cross/common.ini
endif

# Build the project
build:
	meson compile -C build/

# Clean build directory
clean:
	rm -rf build/

# Install the project
install:
	meson install -C build/

# Fresh build (clean + setup + build)
fresh: clean setup build
