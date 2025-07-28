# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System and Commands

This project uses Meson as its build system with SDL3 as a dependency.

### Build Commands
```bash
# Configure build directory
meson setup builddir

# Build the project
meson compile -C builddir

# Debug build (default)
meson setup builddir --buildtype=debug

# Release build
meson setup builddir --buildtype=release

# Clean build
rm -rf builddir && meson setup builddir
```

### Test Commands
```bash
# Enable tests during setup
meson setup builddir -Denable_tests=true

# Run tests
meson test -C builddir
```

### Configuration Options
- `enable_tests`: Enable/disable test building (default: false)
- `deprecation_warnings`: Control deprecation warning behavior (error/no-error/ignore/default)
- `debug`: Enable debug mode (affects optimization and symbols)

## Architecture Overview

Spoopy is a lightweight C game framework inspired by the Taisei Project. The codebase is organized into core modules with platform-specific abstractions.

### Core Structure
- **Memory Management**: Custom allocators for heap and stack allocation with platform-specific optimizations
- **Threading**: SDL3-based threading abstraction layer  
- **Logging**: Colored console logging system with multiple severity levels
- **Cross-Platform Support**: Comprehensive compiler and platform detection (MSVC, GCC, Clang, Windows, macOS, Linux)

### Key Directories
- `src/core/`: Core framework implementation
- `include/`: Public headers with clean C API
- `include/pch/`: Precompiled headers for build optimization
- `subprojects/`: External dependencies (SDL3 wrapper)
- `test/`: Test suite (optional build)

### Platform Abstractions
The framework provides extensive cross-platform support:
- Memory allocation (aligned_alloc, _aligned_free, alloca variants)
- Compiler-specific warning configurations
- Dynamic linking support (DLL export/import on Windows)
- POSIX feature detection

### Memory System
The memory subsystem (`spoopy_memory.h/c`) provides:
- `spoopy_heap_alloc()`: Standard heap allocation with error handling
- `spoopy_stack_alloc()`: Stack allocation using alloca variants
- `spoopy_heap_free()`: Platform-aware deallocation
- Future: Custom allocator support with type-tagged pointers

### Threading Model
Threading is handled through SDL3 abstraction (`spoopy_sdl_thread.c`) providing cross-platform thread management.

### Build Configuration
The build system automatically:
- Detects compiler capabilities and applies appropriate warning flags
- Configures platform-specific features (POSIX, aligned allocation)
- Handles debug/release optimizations
- Manages SDL3 integration with minimal feature set (threads only)
- Sets up precompiled headers for faster compilation

### Development Notes
- Uses C99 standard with platform extensions
- Extensive use of compiler attributes for optimization hints
- Error handling through logging macros with colored output
- Static analysis friendly with comprehensive warning configurations