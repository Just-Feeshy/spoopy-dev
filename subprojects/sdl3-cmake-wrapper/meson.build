project('sdl3-cmake-wrapper')

cmake = import('cmake')
sdl3_options = cmake.subproject_options()
sdl3_options.set_install(false)
sdl3_options.add_cmake_defines({
    'SDL_DISABLE_INSTALL': true,        # Don't generate install targets
    'SDL_DISABLE_UNINSTALL': true,      # Don't generate uninstall targets
    'SDL_SHARED': false,                # Don't build shared library (.dll/.so)
    'SDL_STATIC': true,                 # Build static library (.lib/.a)
    'SDL_EXAMPLES': false,              # Don't build example programs
    'SDL_TESTS': false,                 # Don't build test suite
    'SDL_RENDER': false,                # No 2D rendering API

    'SDL_AUDIO': false,                 # No audio playback/recording
    'SDL_VIDEO': false,                 # No windows/graphics (headless)
    'SDL_POWER': false,                 # No battery/power management
    'SDL_HAPTIC': false,                # No force feedback/rumble
    'SDL_SENSOR': false,                # No accelerometer/gyroscope
    'SDL_JOYSTICK': false,              # No joystick/controller input
    'SDL_GAMEPAD': false,               # No high-level gamepad support
    'SDL_HIDAPI': false,                # No USB HID device access
    'SDL_LOCALE': false,                # No system locale detection
    'SDL_MISC': true,                   # No platform utils/clipboard/URLs
})

passthrough_toggles = [
    'pthreads',
    'win32_threads',
]

foreach toggle : passthrough_toggles
    opt = get_option(toggle)

    if not opt.auto()
        sdl3_options.add_cmake_defines({ 'SDL_@0@'.format(toggle.to_upper()) : opt.enabled() })
    endif
endforeach

if host_machine.system() == 'emscripten'
    sdl3_options.add_cmake_defines({ 'EMSCRIPTEN' : true })
endif

sdl3_subproject = cmake.subproject('sdl3', options : sdl3_options)
sdl3_dep = sdl3_subproject.dependency('SDL3-static')
