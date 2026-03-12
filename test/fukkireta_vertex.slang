struct VertexInput {
	float2 pos : POSITION;
};

struct VertexOutput {
	// Pass clip-space position straight through so the quad covers the whole viewport.
	float4 position : SV_POSITION;
	float2 resolution : TEXCOORD0;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
	VertexOutput output;
	output.position = float4(input.pos.x, input.pos.y, 0.0, 1.0);
	output.resolution = float2(1280.0, 720.0);
	return output;
}
