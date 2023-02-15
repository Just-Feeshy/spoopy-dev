#include <iostream>
#include <cstdlib>

#include <system/CFFIPointer.h>
#include <core/Log.h>

#ifdef SPOOPY_VULKAN
#include <device/InstanceDevice.h>
#include <device/LogicalDevice.h>
#include <device/PhysicalDevice.h>
#include <device/SurfaceDevice.h>
#endif

#if defined(SPOOPY_VULKAN) || defined(SPOOPY_METAL)
#include <ui/SpoopyWindowSurface.h>
#endif

#ifdef SPOOPY_INCLUDE_EXAMPLE
#include <examples/ExampleWindow.h>
#endif

#include <shaders/CrossShader.h>

namespace lime {
    #ifdef SPOOPY_VULKAN

    void apply_gc_instance_device(value handle) {
        InstanceDevice* instanceDevice = (InstanceDevice*)val_data(handle);
        delete instanceDevice;
    }

    void apply_gc_physical_device(value handle) {
        PhysicalDevice* physicalDevice = (PhysicalDevice*)val_data(handle);
        delete physicalDevice;
    }

    void apply_gc_logical_device(value handle) {
        LogicalDevice* logicalDevice = (LogicalDevice*)val_data(handle);
        delete logicalDevice;
    }

    void apply_gc_surface(value handle) {
        SurfaceDevice* surface = (SurfaceDevice*)val_data(handle);
        delete surface;
    }

    value spoopy_create_instance_device(value window, HxString name, int major, int minor, int patch) {
        const int version[3] = {major, minor, patch};

        SpoopyWindowSurface* targetWindowSurface = (SpoopyWindowSurface*)val_data(window);

        InstanceDevice* instanceDevice = new InstanceDevice(*targetWindowSurface);
        instanceDevice -> createInstance(name.c_str(), version);
        instanceDevice -> createDebugMessenger();

        return CFFIPointer(instanceDevice, apply_gc_instance_device);
    }
    DEFINE_PRIME5(spoopy_create_instance_device);

    value spoopy_create_physical_device(value instance) {
        InstanceDevice* cast_Instance = (InstanceDevice*)val_data(instance);

        PhysicalDevice* physical = new PhysicalDevice(*cast_Instance);
        return CFFIPointer(physical, apply_gc_physical_device);
    }
    DEFINE_PRIME1(spoopy_create_physical_device);

    value spoopy_create_logical_device(value instance, value physical) {
        InstanceDevice* cast_Instance = (InstanceDevice*)val_data(instance);
        PhysicalDevice* cast_Physical = (PhysicalDevice*)val_data(physical);

        LogicalDevice* logical = new LogicalDevice(*cast_Instance, *cast_Physical);
        logical -> initDevice();

        return CFFIPointer(logical, apply_gc_logical_device);
    }
    DEFINE_PRIME2(spoopy_create_logical_device);

    value spoopy_create_surface(value instance_handle, value physical_handle, value logical_handle, value window_handle) {
        InstanceDevice* instance = (InstanceDevice*)val_data(instance_handle);
        PhysicalDevice* physical = (PhysicalDevice*)val_data(physical_handle);
        LogicalDevice* logical = (LogicalDevice*)val_data(logical_handle);
        SpoopyWindowSurface* window = (SpoopyWindowSurface*)val_data(window_handle);

        SurfaceDevice* surface = new SurfaceDevice(*instance, *physical, *logical, *window);
        return CFFIPointer(surface, apply_gc_surface);
    }
    DEFINE_PRIME4(spoopy_create_surface);

    #endif

    #if defined(SPOOPY_VULKAN) || defined(SPOOPY_METAL)

    void apply_gc_window_surface(value handle) {
        SpoopyWindowSurface* windowSurface = (SpoopyWindowSurface*)val_data(handle);
        delete windowSurface;
    }

    value spoopy_create_window_surface(value window_handle) {
        SDLWindow* window = (SDLWindow*)val_data(window_handle);

        SpoopyWindowSurface* windowSurface = createWindowSurface(*window);
        return CFFIPointer(windowSurface, apply_gc_window_surface);
    }
    DEFINE_PRIME1(spoopy_create_window_surface);

    void spoopy_update_window_surface(value window_surface) {
        SpoopyWindowSurface* windowSurface = (SpoopyWindowSurface*)val_data(window_surface);
        windowSurface -> render();
    }
    DEFINE_PRIME1v(spoopy_update_window_surface);

    void spoopy_release_window_surface(value window_surface) {
        SpoopyWindowSurface* windowSurface = (SpoopyWindowSurface*)val_data(window_surface);
        windowSurface -> clear();
    }
    DEFINE_PRIME1v(spoopy_release_window_surface);

    #endif

    #ifdef SPOOPY_INCLUDE_EXAMPLE

    void apply_gc_example_window(value handle) {
        ExampleWindow* window = (ExampleWindow*)val_data(handle);
        delete window;
    }

    value spoopy_create_example_window(HxString title, int width, int height, int flags) {
        ExampleWindow* window = new ExampleWindow(title.c_str(), width, height, flags);
        return CFFIPointer(window, apply_gc_example_window);
    }
    DEFINE_PRIME4(spoopy_create_example_window);

    #endif


    //Other
    

    //Testing Purposes

    bool has_spoopy_wrapper() {
        SPOOPY_LOG_SUCCESS("[lime.ndll] has the spoopy wrapper!");
        return true;
    }
    DEFINE_PRIME0(has_spoopy_wrapper);
}