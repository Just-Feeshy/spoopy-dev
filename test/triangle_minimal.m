// Minimal Metal + Sokol test - bypasses all spoopy abstractions
// This is to verify sokol works correctly on this system

#define SOKOL_IMPL
#define SOKOL_METAL
#include <sokol_gfx.h>
#include <sokol_log.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_metal.h>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

static CAMetalLayer* metal_layer;
static id<MTLDevice> metal_device;
static id<CAMetalDrawable> active_drawable;
static sg_swapchain swapchain;

static void update_present(void) {
    const CGSize bounds = metal_layer.bounds.size;
    const CGFloat scale = metal_layer.contentsScale;
    metal_layer.drawableSize = (CGSize) {
        .width = bounds.width * scale,
        .height = bounds.height * scale
    };
    active_drawable = [metal_layer nextDrawable];
    swapchain.width = (int)metal_layer.drawableSize.width;
    swapchain.height = (int)metal_layer.drawableSize.height;
    swapchain.sample_count = 1;
    swapchain.color_format = SG_PIXELFORMAT_BGRA8;
    swapchain.depth_format = SG_PIXELFORMAT_NONE;
    swapchain.metal.current_drawable = (__bridge const void*)active_drawable;
}

static sg_swapchain get_swapchain(void) {
    return swapchain;
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    // Create Metal device
    metal_device = MTLCreateSystemDefaultDevice();
    if (!metal_device) {
        printf("Failed to create Metal device\n");
        return 1;
    }
    printf("Metal device: %s\n", [[metal_device name] UTF8String]);

    // Init SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // Create window (match Spoopy's properties path)
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Minimal Triangle");
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_METAL_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, false);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, false);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, 800);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 600);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);

    SDL_Window* window = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);

    if (!window) {
        printf("SDL_CreateWindowWithProperties failed: %s\n", SDL_GetError());
        return 1;
    }

    // Get Metal layer from SDL
    SDL_MetalView view = SDL_Metal_CreateView(window);
    metal_layer = (__bridge CAMetalLayer*)SDL_Metal_GetLayer(view);
    metal_layer.device = metal_device;
    metal_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metal_layer.framebufferOnly = YES;

    printf("Metal layer drawable size: %.0fx%.0f\n",
        metal_layer.drawableSize.width, metal_layer.drawableSize.height);

    // Setup sokol
    sg_setup(&(sg_desc){
        .environment = {
            .defaults.depth_format = SG_PIXELFORMAT_NONE,
            .metal.device = (__bridge const void*)metal_device,
        },
        .logger.func = slog_func,
    });
    printf("Sokol initialized, valid=%d\n", sg_isvalid());

    // Vertex buffer - position only (float3), matching spoopy test
    float vertices[] = {
        -0.75f, -0.75f, 0.0f,
         0.75f, -0.75f, 0.0f,
         0.0f,   0.75f, 0.0f
    };
    sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(vertices)
    });
    printf("Vertex buffer id=%u\n", vbuf.id);

    // Index buffer
    uint16_t indices[] = { 0, 1, 2 };
    sg_buffer ibuf = sg_make_buffer(&(sg_buffer_desc){
        .usage.index_buffer = true,
        .data = SG_RANGE(indices)
    });
    printf("Index buffer id=%u\n", ibuf.id);

    // Shader - using Slang-transpiled Metal source (with custom entry points)
    const char* vs_src_literal =
        "#include <metal_stdlib>\n"
        "#include <metal_math>\n"
        "#include <metal_texture>\n"
        "using namespace metal;\n"
        "struct vertexMain_Result_0\n"
        "{\n"
        "    float4 position_0 [[position]];\n"
        "};\n"
        "\n"
        "struct vertexInput_0\n"
        "{\n"
        "    float3 pos_0 [[attribute(0)]];\n"
        "};\n"
        "\n"
        "struct VertexOutput_0\n"
        "{\n"
        "    float4 position_1;\n"
        "};\n"
        "\n"
        "[[vertex]] vertexMain_Result_0 vertexMain(vertexInput_0 _S1 [[stage_in]])\n"
        "{\n"
        "    thread VertexOutput_0 output_0;\n"
        "    (&output_0)->position_1 = float4(_S1.pos_0, 1.0);\n"
        "    thread vertexMain_Result_0 _S2;\n"
        "    (&_S2)->position_0 = output_0.position_1;\n"
        "    return _S2;\n"
        "}\n";

    const char* fs_src_literal =
        "#include <metal_stdlib>\n"
        "#include <metal_math>\n"
        "#include <metal_texture>\n"
        "using namespace metal;\n"
        "struct pixelOutput_0\n"
        "{\n"
        "    float4 output_0 [[color(0)]];\n"
        "};\n"
        "\n"
        "[[fragment]] pixelOutput_0 fragmentMain()\n"
        "{\n"
        "    pixelOutput_0 _S1 = { float4(1.0, 0.0, 0.0, 1.0) };\n"
        "    return _S1;\n"
        "}\n";

    char* vs_src = strdup(vs_src_literal);
    char* fs_src = strdup(fs_src_literal);

    sg_shader shd = sg_make_shader(&(sg_shader_desc){
        .vertex_func.source = vs_src,
        .vertex_func.entry = "vertexMain",
        .fragment_func.source = fs_src,
        .fragment_func.entry = "fragmentMain"
    });

    free(vs_src);
    free(fs_src);
    printf("Shader id=%u\n", shd.id);

    // Pipeline - position only (float3), with index buffer
    sg_pipeline_desc pip_desc = {0};
    pip_desc.shader = shd;
    pip_desc.index_type = SG_INDEXTYPE_UINT16;
    pip_desc.depth.pixel_format = SG_PIXELFORMAT_NONE;
    // match Spoopy-style: only set attrs[i].format
    pip_desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    sg_pipeline pip = sg_make_pipeline(&pip_desc);
    printf("Pipeline id=%u\n", pip.id);

    // Bindings (recreated per draw, like Spoopy does)
    sg_bindings bindings = {0};

    // Pass action
    sg_pass_action pass_action = {
        .colors[0] = { .load_action = SG_LOADACTION_CLEAR, .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f } }
    };

    printf("Starting render loop...\n");

    // Render loop
    bool running = true;
    int frame = 0;
    while (running) {
        // Spoopy-style: update drawable + grab swapchain at frame start
        update_present();
        sg_swapchain swapchain = get_swapchain();
        if (frame < 3) {
            printf("Frame %d: swapchain %dx%d, drawable=%p\n",
                frame, swapchain.width, swapchain.height,
                swapchain.metal.current_drawable);
        }

        SDL_PumpEvents();
        SDL_Event events[8];
        int n_events = SDL_PeepEvents(events, 8, SDL_GETEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST);
        if (n_events < 0) {
            printf("SDL_PeepEvents failed: %s\n", SDL_GetError());
        }
        for (SDL_Event* e = events; e < events + n_events; ++e) {
            if (e->type == SDL_EVENT_QUIT) {
                printf("SDL_EVENT_QUIT received\n");
                running = false;
            }
        }

            sg_begin_pass(&(sg_pass){ .action = pass_action, .swapchain = swapchain });
            sg_apply_pipeline(pip);
            bindings = (sg_bindings){0};
            bindings.vertex_buffers[0] = vbuf;
            bindings.index_buffer = ibuf;
            sg_apply_bindings(&bindings);
            sg_draw(0, 3, 1);  // 3 indices
            sg_end_pass();
            sg_commit();
        frame++;
    }

    sg_shutdown();
    SDL_Metal_DestroyView(view);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
