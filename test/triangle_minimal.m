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

static sg_swapchain get_swapchain(void) {
    id<CAMetalDrawable> drawable = [metal_layer nextDrawable];
    CGSize size = metal_layer.drawableSize;
    return (sg_swapchain){
        .width = (int)size.width,
        .height = (int)size.height,
        .sample_count = 1,
        .color_format = SG_PIXELFORMAT_BGRA8,
        .depth_format = SG_PIXELFORMAT_NONE,
        .metal = {
            .current_drawable = (__bridge const void*)drawable,
        }
    };
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

    // Create window
    SDL_Window* window = SDL_CreateWindow("Minimal Triangle", 800, 600, SDL_WINDOW_METAL);
    if (!window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    // Get Metal layer from SDL
    SDL_MetalView view = SDL_Metal_CreateView(window);
    metal_layer = (__bridge CAMetalLayer*)SDL_Metal_GetLayer(view);
    metal_layer.device = metal_device;
    metal_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;

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
    sg_shader shd = sg_make_shader(&(sg_shader_desc){
        .vertex_func.source =
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
            "}\n",
        .vertex_func.entry = "vertexMain",
        .fragment_func.source =
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
            "    pixelOutput_0 _S1 = { float4(0.0, 1.0, 0.0, 1.0) };\n"
            "    return _S1;\n"
            "}\n",
        .fragment_func.entry = "fragmentMain"
    });
    printf("Shader id=%u\n", shd.id);

    // Pipeline - position only (float3), with index buffer
    sg_pipeline pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3,
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16
    });
    printf("Pipeline id=%u\n", pip.id);

    // Bindings
    sg_bindings bindings = {
        .vertex_buffers[0] = vbuf,
        .index_buffer = ibuf
    };

    // Pass action
    sg_pass_action pass_action = {
        .colors[0] = { .load_action = SG_LOADACTION_CLEAR, .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f } }
    };

    printf("Starting render loop...\n");

    // Render loop
    bool running = true;
    int frame = 0;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        @autoreleasepool {
            sg_swapchain swapchain = get_swapchain();
            if (frame < 3) {
                printf("Frame %d: swapchain %dx%d, drawable=%p\n",
                    frame, swapchain.width, swapchain.height,
                    swapchain.metal.current_drawable);
            }

            sg_begin_pass(&(sg_pass){ .action = pass_action, .swapchain = swapchain });
            sg_apply_pipeline(pip);
            sg_apply_bindings(&bindings);
            sg_draw(0, 3, 1);  // 3 indices
            sg_end_pass();
            sg_commit();
        }
        frame++;
    }

    sg_shutdown();
    SDL_Metal_DestroyView(view);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
