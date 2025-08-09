.PHONY: setup build clean install rust-visuals rust-clean

# Detect OS
UNAME_S := $(shell uname -s)

# Default target
all: setup build

# Setup build directory with platform-specific configuration
setup:
ifeq ($(UNAME_S),Darwin)
	meson setup build/ --native-file misc/cross/arm64-darwin.ini --native-file misc/cross/darwin-crappy-sdl3-hack.ini
else
	meson setup build/
endif

# Build the Rust visuals library
rust-visuals:
	cd src/visuals && cargo-cbuild cbuild --manifest-path ./Cargo.toml --release
	mkdir -p build/src/visuals
	cp src/visuals/target/aarch64-apple-darwin/release/libspoopy_visuals.a build/src/visuals/

# Build the project
build: rust-visuals
	meson compile -C build/

# Clean Rust build artifacts
rust-clean:
	cd src/visuals && cargo clean

# Clean build directory
clean: rust-clean
	rm -rf build/

# Install the project
install:
	meson install -C build/

# Reconfigure (useful when meson files change)
reconfigure:
ifeq ($(UNAME_S),Darwin)
	meson setup build/ --reconfigure --native-file misc/cross/arm64-darwin.ini --native-file misc/cross/darwin-crappy-sdl3-hack.ini
else
	meson setup build/ --reconfigure
endif

# Fresh build (clean + setup + build)
fresh: clean setup build
