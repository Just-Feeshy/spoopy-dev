#pragma once

#include <spoopy.h>

/*
 * A component class for the Haxe frontend command pool
 * to be able to use as a handle.
 */

namespace lime { namespace spoopy {
    class GraphicsVulkan;
    class LogicalDevice;

    class CommandPoolVulkan {
        public:
            explicit CommandPoolVulkan(const LogicalDevice &device, uint32_t queueFamily);
            ~CommandPoolVulkan();

            operator const VkCommandPool &() const { return commandPool; }
            const VkCommandPool &GetCommandPool() const { return commandPool; }

        private:
            VkCommandPool commandPool = VK_NULL_HANDLE;
    };
}}