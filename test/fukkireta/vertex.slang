struct VertexInput {
	float2 pos : POSITION;
	float2 resolution : TEXCOORD0;
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
	output.resolution = input.resolution;
	return output;
}
