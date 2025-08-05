.PHONY: setup build clean install

# Detect OS
UNAME_S := $(shell uname -s)

# Default target
all: setup build

# Setup build directory with platform-specific configuration
setup:
ifeq ($(UNAME_S),Darwin)
	meson setup build/ --native-file misc/cross/arm64-darwin.ini
else
	meson setup build/
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

# Reconfigure (useful when meson files change)
reconfigure:
ifeq ($(UNAME_S),Darwin)
	meson setup build/ --reconfigure --native-file misc/cross/arm64-darwin.ini
else
	meson setup build/ --reconfigure
endif

# Fresh build (clean + setup + build)
fresh: clean setup build