struct FSInput {
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
    float4 color    : COLOR0;
};

[[vk::binding(1, 0)]]
Texture2D<float> font_tex : register(t0);

[[vk::binding(2, 0)]]
SamplerState font_smp : register(s0);

[shader("fragment")]
float4 fragmentMain(FSInput input) : SV_Target {
    float alpha = font_tex.Sample(font_smp, input.uv);

    return float4(
        input.color.rgb,
        input.color.a * alpha
    );
}
