#define MAX_STEPS 80
#define MAX_DISTANCE 10
#define TRACER_EPSILON 0.001
#define AA_SAMPLE_COUNT 8

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

float2 makeUv(float2 fragCoord, float2 resolution) {
	float2 uv = (fragCoord / resolution) * 2.0 - 1.0;
	uv.y = -uv.y;
	uv.x *= resolution.x / resolution.y;
	return uv;
}

float4 traceSample(float2 fragCoord, float2 resolution) {
	float2 uv = makeUv(fragCoord, resolution);

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

[shader("fragment")]
float4 fragmentMain(FragmentInput input) : SV_Target {
	float2 fragCoord = input.position.xy;
	float4 color = 0.0;

	color += traceSample(fragCoord + float2(-0.5625, -0.4375), input.resolution);
	color += traceSample(fragCoord + float2(-0.0625, -0.1875), input.resolution);
	color += traceSample(fragCoord + float2( 0.4375, -0.3125), input.resolution);
	color += traceSample(fragCoord + float2( 0.1875,  0.0625), input.resolution);
	color += traceSample(fragCoord + float2(-0.3125,  0.1875), input.resolution);
	color += traceSample(fragCoord + float2(-0.4375,  0.5625), input.resolution);
	color += traceSample(fragCoord + float2( 0.0625,  0.3125), input.resolution);
	color += traceSample(fragCoord + float2( 0.5625,  0.4375), input.resolution);

	return color / AA_SAMPLE_COUNT;
}
