#include "Instance.h"

#include <sstream>
#include <string>

#ifndef VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    #define VK_EXT_DEBUG_UTILS_EXTENSION_NAME "VK_EXT_debug_utils"
#endif

namespace lime {
    #if VK_HEADER_VERSION > 101
        const std::vector<const char*> Instance::ValidationLayers = {"VK_LAYER_KHRONOS_validation", "VK_KHR_portability_subset"};
    #else
        const std::vector<const char*> Instance::ValidationLayers = {"VK_LAYER_LUNARG_standard_validation", "VK_KHR_portability_subset"};
    #endif

    #if SPOOPY_DEBUG_MESSENGER
    VKAPI_ATTR VkBool32 VKAPI_CALL CallbackDebug(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            SPOOPY_LOG_WARN(pCallbackData->pMessage);
        }else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            SPOOPY_LOG_INFO(pCallbackData->pMessage);
        }else {
            SPOOPY_LOG_ERROR(pCallbackData->pMessage);
        }

        return VK_FALSE;
    }
    #else
    VKAPI_ATTR VkBool32 VKAPI_CALL CallbackDebug(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode,
        const char *pLayerPrefix, const char *pMessage, void *pUserData) {

        if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
            SPOOPY_LOG_WARN(pMessage);
        }else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
            SPOOPY_LOG_INFO(pMessage);
        }else {
            SPOOPY_LOG_ERROR(pMessage);
        }

        return VK_FALSE;
    }
    #endif

    Instance::Instance(SDL_Window* window): m_window(window) {
        #ifdef SPOOPY_DEBUG
        enableValidationLayers = true;
        #endif

        CreateInstance();
        CreateDebugMessenger();
    }

    Instance::~Instance() {
        #ifdef SPOOPY_DEBUG_MESSENGER
        FvkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        #else
        FvkDestroyDebugReportCallbackEXT(instance, debugMessenger, nullptr);
        #endif

        vkDestroyInstance(instance, nullptr);
        m_window = nullptr;
    }

    void Instance::CreateInstance() {
        checkVulkan(volkInitialize());

        VkApplicationInfo applicationInfo = {};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pEngineName = "Spoopy";
        applicationInfo.engineVersion = SPOOPY_ENGINE_VERSION;
        applicationInfo.apiVersion = volkGetInstanceVersion() >= VK_API_VERSION_1_1 ? VK_API_VERSION_1_1 : VK_MAKE_VERSION(1, 0, 57);

        if(enableValidationLayers && !CheckValidationLayerSupport()) {
            SPOOPY_LOG_ERROR("Vulkan validation layers requested, but not available!");
            enableValidationLayers = false;
        }

        auto extensions = GetExtensions();

        VkInstanceCreateInfo instanceCreateInfo = {};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

        #ifdef SPOOPY_DEBUG_MESSENGER
            VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {};
        #endif

        if (enableValidationLayers) {
            #ifdef SPOOPY_DEBUG_MESSENGER
                debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                debugUtilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                debugUtilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                debugUtilsMessengerCreateInfo.pfnUserCallback = &CallbackDebug;
                instanceCreateInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT *>(&debugUtilsMessengerCreateInfo);
            #endif

            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
            instanceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
        }

        checkVulkan(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));

        #if VOLK_HEADER_VERSION >= 131
            volkLoadInstanceOnly(instance);
        #else
            volkLoadInstance(instance);
        #endif
    }

    void Instance::CreateDebugMessenger() {
        if(!enableValidationLayers) {
            return;
        }

        #ifdef SPOOPY_DEBUG_MESSENGER
                VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {};
            debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugUtilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugUtilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugUtilsMessengerCreateInfo.pfnUserCallback = &CallbackDebug;

            checkVulkan(FvkCreateDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCreateInfo, nullptr, &debugMessenger));
        #else
            VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo = {};
            debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            debugReportCallbackCreateInfo.pNext = nullptr;
            debugReportCallbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            debugReportCallbackCreateInfo.pfnCallback = &CallbackDebug;
            debugReportCallbackCreateInfo.pUserData = nullptr;
            auto debugReportResult = FvkCreateDebugReportCallbackEXT(instance, &debugReportCallbackCreateInfo, nullptr, &debugMessenger);

            if (debugReportResult == VK_ERROR_EXTENSION_NOT_PRESENT) {
                enableValidationLayers = false;
                SPOOPY_LOG_ERROR("Extension vkCreateDebugReportCallbackEXT not present!");
            } else {
                checkVulkan(debugReportResult);
            }
        #endif
    }

    bool Instance::CheckValidationLayerSupport() const {
        uint32_t count;
        vkEnumerateInstanceLayerProperties(&count, nullptr);

        std::vector<VkLayerProperties> instanceLayerProperties(count);
        vkEnumerateInstanceLayerProperties(&count, instanceLayerProperties.data());

        for(const char* layerName: ValidationLayers) {
            bool layerFound = false;

            for(const auto& layerProperties: instanceLayerProperties) {
                if(strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if(!layerFound) {
                std::ostringstream msg;
                msg << "Vulkan validation layer not found: \"" << layerName << "\"\n";

                SPOOPY_LOG_ERROR(msg.str());
                return false;
            }
        }

        return true;
    }

    std::vector<const char *> Instance::GetExtensions() const {
        #ifdef SPOOPY_SDL
                unsigned int extensionCount = 0;
                SDL_Vulkan_GetInstanceExtensions(m_window, &extensionCount, nullptr);
                std::vector<const char*> extensions(extensionCount);
                SDL_Vulkan_GetInstanceExtensions(m_window, &extensionCount, extensions.data());

                if(enableValidationLayers) {
                    extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                    extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
                }

                extensions.emplace_back("VK_KHR_portability_enumeration");
        #else
                std::vector<const char*> extensions;
        #endif

        return extensions;
    }
}