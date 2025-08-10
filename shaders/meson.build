spoopy_shader_sources = files([
    # Add shader files here when they exist
    # 'vertex.slang',
    # 'fragment.slang',
])

compiled_shaders = []

foreach shader : spoopy_shader_sources
    metal_shader = custom_target(
        'metal_' + shader.name(),
        input: shader,
        output: '@BASENAME@.metal',
        command: [
            slangc_prog,
            '@INPUT@',
            '-target', 'metal',
            '-profile', 'metal_2_4',
            '-o', '@OUTPUT@',
        ],
        build_by_default: true,
    )
    compiled_shaders += metal_shader

    spirv_shader = custom_target(
        'spirv_' + shader.name(),
        input: shader,
        output: '@BASENAME@.spv',
        command: [
            slangc_prog,
            '@INPUT@',
            '-target', 'spirv',
            '-profile', 'glsl_450',
            '-o', '@OUTPUT@',
        ],
        build_by_default: true,
    )
    compiled_shaders += spirv_shader

    dxil_shader = custom_target(
        'dxil_' + shader.name(),
        input: shader,
        output: '@BASENAME@.dxil',
        command: [
            slangc_prog,
            '@INPUT@',
            '-target', 'dxil',
            '-profile', 'sm_6_0',
            '-o', '@OUTPUT@',
        ],
        build_by_default: true,
    )
    compiled_shaders += dxil_shader
endforeach
