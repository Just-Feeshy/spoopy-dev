#include "kore2.h"

#include <spoopy_types.h>
#include <spoopy_log.h>

#include <string.h>

static kinc_g5_texture_addressing_t to_kinc_address(spoopy_texture_wrap_mode_t wrap) {
    switch (wrap) {
        case SPOOPY_TEXTURE_WRAP_REPEAT:
            return KINC_G5_TEXTURE_ADDRESSING_REPEAT;
        case SPOOPY_TEXTURE_WRAP_MIRROR:
            return KINC_G5_TEXTURE_ADDRESSING_MIRROR;
        case SPOOPY_TEXTURE_WRAP_CLAMP:
        default:
            return KINC_G5_TEXTURE_ADDRESSING_CLAMP;
    }
}

static kinc_g5_texture_filter_t to_kinc_filter(spoopy_texture_filter_mode_t filter) {
    switch (filter) {
        case SPOOPY_TEXTURE_FILTER_LINEAR:
        case SPOOPY_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
        case SPOOPY_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
            return KINC_G5_TEXTURE_FILTER_LINEAR;
        default:
            return KINC_G5_TEXTURE_FILTER_POINT;
    }
}

static kinc_g5_mipmap_filter_t to_kinc_mip(spoopy_texture_filter_mode_t filter) {
    switch (filter) {
        case SPOOPY_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
        case SPOOPY_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
            return KINC_G5_MIPMAP_FILTER_LINEAR;
        case SPOOPY_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
        case SPOOPY_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
            return KINC_G5_MIPMAP_FILTER_POINT;
        default:
            return KINC_G5_MIPMAP_FILTER_NONE;
    }
}

static enum kinc_image_format to_kinc_format(spoopy_image_format_t fmt) {
    switch (fmt) {
        case SPOOPY_IMAGE_FORMAT_RGBA32:
            return KINC_IMAGE_FORMAT_RGBA32;
        case SPOOPY_IMAGE_FORMAT_GREY8:
            return KINC_IMAGE_FORMAT_GREY8;
        case SPOOPY_IMAGE_FORMAT_RGB24:
            return KINC_IMAGE_FORMAT_RGB24;
        case SPOOPY_IMAGE_FORMAT_RGBA128:
            return KINC_IMAGE_FORMAT_RGBA128;
        case SPOOPY_IMAGE_FORMAT_RGBA64:
            return KINC_IMAGE_FORMAT_RGBA64;
        case SPOOPY_IMAGE_FORMAT_A32:
            return KINC_IMAGE_FORMAT_A32;
        case SPOOPY_IMAGE_FORMAT_BGRA32:
            return KINC_IMAGE_FORMAT_BGRA32;
        case SPOOPY_IMAGE_FORMAT_A16:
            return KINC_IMAGE_FORMAT_A16;
        default:
            SPOOPY_LOG_WARN("Unsupported image format %d, defaulting to RGBA32", (int)fmt);
            return KINC_IMAGE_FORMAT_RGBA32;
    }
}

static bool get_mip_size(const kinc_g4_texture_t* texture, uint32_t mipmap, uint32_t* width, uint32_t* height) {
    const uint32_t max_mips = 32;
    if (mipmap >= max_mips) {
        return false;
    }

    uint32_t w = (uint32_t)texture->tex_width;
    uint32_t h = (uint32_t)texture->tex_height;

    if (mipmap > 0) {
        w = w >> mipmap;
        h = h >> mipmap;
        if (w == 0) {
            w = 1;
        }
        if (h == 0) {
            h = 1;
        }
    }

    if (width != NULL) {
        *width = w;
    }

    if (height != NULL) {
        *height = h;
    }

    return true;
}

void spoopy_kinc_texture_create(spoopy_texture_t* tex, const spoopy_texture_params_t* params) {
    if (tex == NULL || params == NULL) {
        SPOOPY_LOG_ERROR("Cannot create texture: invalid arguments (tex=%p, params=%p)", (void*)tex, (void*)params);
        return;
    }

    if (params->width == 0 || params->height == 0) {
        SPOOPY_LOG_ERROR("Cannot create texture with zero dimensions (%ux%u)", params->width, params->height);
        return;
    }

    tex->sampler_initialized = false;

    enum kinc_image_format img_fmt = to_kinc_format(params->format);
    kinc_g4_texture_init(&tex->raw, (int)params->width, (int)params->height, img_fmt);

    kinc_g5_sampler_options_t opts;
    kinc_g5_sampler_options_set_defaults(&opts);
    opts.u_addressing = to_kinc_address(params->wrap.u);
    opts.v_addressing = to_kinc_address(params->wrap.v);
    opts.minification_filter = to_kinc_filter(params->filter.min);
    opts.magnification_filter = to_kinc_filter(params->filter.mag);
    opts.mipmap_filter = to_kinc_mip(params->filter.min);

    kinc_g5_sampler_init(&tex->sampler, &opts);
    tex->sampler_initialized = true;

	return tex;
}

void spoopy_kinc_texture_fill(spoopy_texture_t* tex, uint32_t mipmap, uint32_t layer, const spoopy_image_t* img) {
    if (tex == NULL || img == NULL) {
        SPOOPY_LOG_ERROR("Cannot fill texture: invalid arguments (tex=%p, img=%p)", (void*)tex, (void*)img);
        return;
    }

    if (img->pixels.raw_data == NULL) {
        SPOOPY_LOG_ERROR("Cannot fill texture: image has no pixel data");
        return;
    }

    if (layer != 0) {
        SPOOPY_LOG_ERROR("Layered textures are not supported yet (layer=%u)", layer);
        return;
    }

    uint32_t expected_width = 0;
    uint32_t expected_height = 0;
    if (!get_mip_size(&tex->raw, mipmap, &expected_width, &expected_height)) {
        SPOOPY_LOG_ERROR("Invalid mip level %u for texture fill", mipmap);
        return;
    }

    if (img->width != expected_width || img->height != expected_height) {
        SPOOPY_LOG_ERROR(
            "Image dimensions (%ux%u) do not match expected mip size (%ux%u)",
            img->width,
            img->height,
            expected_width,
            expected_height);
        return;
    }

    enum kinc_image_format kinc_fmt = to_kinc_format(img->format);

    const size_t bytes_per_pixel = (size_t)kinc_image_format_sizeof(kinc_fmt);
    const size_t expected_size = (size_t)expected_width * expected_height * bytes_per_pixel;
    if (img->data_size < expected_size) {
        SPOOPY_LOG_ERROR(
            "Image data is smaller (%zu bytes) than expected for mip (%zu bytes)",
            (size_t)img->data_size,
            expected_size);
        return;
    }

    kinc_image_t source_image;
    memset(&source_image, 0, sizeof(source_image));
    kinc_image_init_from_bytes(
        &source_image,
        img->pixels.raw_data,
        (int)expected_width,
        (int)expected_height,
        kinc_fmt);

    kinc_g4_texture_set_mipmap(&tex->raw, &source_image, (int)mipmap);
}

void spoopy_kinc_texture_destroy(spoopy_texture_t* tex) {
    if (tex == NULL) {
        SPOOPY_LOG_WARN("Attempted to destroy NULL texture");
        return;
    }

    if (tex->sampler_initialized) {
        kinc_g5_sampler_destroy(&tex->sampler);
        tex->sampler_initialized = false;
    }

    kinc_g4_texture_destroy(&tex->raw);
	spoopy_heap_free(tex);
}
