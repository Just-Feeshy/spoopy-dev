#define MAX_STEPS 80
#define MAX_DISTANCE 10
#define TRACER_EPSILON 0.001

struct FragmentInput {
	float4 position : SV_POSITION;
	float2 resolution : TEXCOORD0;
};

float sdSphere(float3 p, float radius) {
	return length(p) - radius;
}

float mapScene(float3 p) {
	return sdSphere(p, 0.75);
}

float3 calcNormal(float3 p) {
	float2 e = float2(TRACER_EPSILON, 0.0);
	return normalize(float3(
		mapScene(p + float3(e.x, e.y, e.y)) - mapScene(p - float3(e.x, e.y, e.y)),
		mapScene(p + float3(e.y, e.x, e.y)) - mapScene(p - float3(e.y, e.x, e.y)),
		mapScene(p + float3(e.y, e.y, e.x)) - mapScene(p - float3(e.y, e.y, e.x))
	));
}

[shader("fragment")]
float4 fragmentMain(FragmentInput input) : SV_Target {
	float2 uv = (input.position.xy / input.resolution) * 2.0 - 1.0;
	uv.x *= input.resolution.x / input.resolution.y;
	uv.y = -uv.y;

	float3 rayOrigin = float3(0.0, 0.0, 2.25);
	float3 rayDir = normalize(float3(uv, -1.5));
	float t = 0.0;
	bool hit = false;
	float3 hitPos = rayOrigin;

	[loop]
	for(int i = 0; i < MAX_STEPS; ++i) {
		hitPos = rayOrigin + rayDir * t;
		float dist = mapScene(hitPos);

		if(dist < TRACER_EPSILON) {
			hit = true;
			break;
		}

		t += dist;
		if(t > MAX_DISTANCE) {
			break;
		}
	}

	if(!hit) {
		return float4(1.0, 1.0, 1.0, 1.0);
	}

	float3 normal = calcNormal(hitPos);
	float3 color = normal * 0.5 + 0.5;
	return float4(color, 1.0);
}
