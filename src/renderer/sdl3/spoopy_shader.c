#include <spoopy_shader.h>

bool spoopy_shader_language_supported(const spoopy_shader_info_t* info) {
    const spoopy_shader_language_t desired_langs = spoopy_shader_get_supported_languages();

    if(info->language == desired_langs) {
        return true;
    }
}
