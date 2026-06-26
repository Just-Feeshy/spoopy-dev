struct VSParams {
    float2 disp_size;
};

[[vk::binding(0, 0)]]
ConstantBuffer<VSParams> params : register(b0);

struct VSInput {
    uint2 packed : TEXCOORD0;
};

struct VSOutput {
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
    float4 color    : COLOR0;
};

float unpack12(uint v) {
    return float(v & 0xFFFu) / 4095.0;
}

float4 unpackRGBA4444(uint packed1) {
    float r = float((packed1 >> 16) & 0xFu) / 15.0;
    float g = float((packed1 >> 20) & 0xFu) / 15.0;
    float b = float((packed1 >> 24) & 0xFu) / 15.0;
    float a = float((packed1 >> 28) & 0xFu) / 15.0;

    return float4(r, g, b, a);
}

[shader("vertex")]
VSOutput vertexMain(VSInput input) {
    VSOutput output;

    uint p0 = input.packed.x;
    uint p1 = input.packed.y;

    uint pos_x_q =  p0        & 0xFFFu;
    uint pos_y_q = (p0 >> 12) & 0xFFFu;

    uint uv_x_q = ((p0 >> 24) & 0xFFu) |
                  ((p1 & 0xFu) << 8);

    uint uv_y_q = (p1 >> 4) & 0xFFFu;

    float2 pos01 = float2(
        float(pos_x_q) / 4095.0,
        float(pos_y_q) / 4095.0
    );

    float2 uv = float2(
        float(uv_x_q) / 4095.0,
        float(uv_y_q) / 4095.0
    );

    float2 clip = (pos01 - float2(0.5, 0.5)) * float2(2.0, -2.0);
    output.position = float4(clip, 0.5, 1.0);
    output.uv = uv;
    output.color = unpackRGBA4444(p1);

    return output;
}
