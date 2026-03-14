#define MAX_STEPS 80
#define MAX_DISTANCE 10
#define TRACER_EPSILON 0.001
#define AA_SAMPLE_COUNT 8

#define R_2A float3(1.10,0.87,1.20)
#define R_3A float3(0.30,0.28,0.28)

struct FragmentInput {
	float4 position : SV_POSITION;
	float2 resolution : TEXCOORD0;
};

float sdfSphere(float3 p, float radius) {
	return length(p) - radius;
}

float sdfEllipsoid(float3 p, float3 c, float3 r) {
	float k1 = length((p-c)/r);
    float k2 = length((p-c)/(r*r));
    return k1*(k1-1.0)/k2;
}

float f_k(float x, float y, float k) {
	float top = max(k - abs(x - y),  0);
	return (top*top*top) / (6*k*k);
}

float sat(float x) {
	return min(1.0, max(0.0, x));
}

float smin(float d_1, float d_2, float k) {
	return min(d_1, d_2) - f_k(d_1, d_2, k);
}

float smax(float d_1, float d_2, float k) {
	return max(d_1, d_2) + f_k(d_1, d_2, k);
}

float sdfSegment(float3 p, float3 a, float3 b) {
	float h = sat(dot(p-a,b-a)/dot(b-a,b-a));
	return length(p-a-(b-a)*h) - 0.55;
}

float3 sabsx(float3 p) {
	const float k = 0.002;
	return float3(sqrt(p.x * p.x + k), p.y, p.z);
}

float3 absx(float3 p) {
	return float3(abs(p.x), p.y, p.z);
}

float mapScene(float3 p) {
	const float3 c_1 = float3(0.67, -0.535, 1.3);
	const float3 c_2 = float3(0.0, -0.95, 0.05);
	const float3 r_2 = R_2A + (float3(0.3,-0.40*p.z,0.1)) * (p.y - c_2.y);
	const float3 c_3a = float3(0.63, 0.445, -0.95);
	const float3 c_3b = float3(-0.63, 0.445, -0.95);
	const float3 r_3 = R_3A + float3(0.07,0.0,0.0) * (p.y - c_2.y);
	const float3 c_4 = float3(-0.38, 0.335, -0.29);

	float d_1 = sdfSphere(p, 1.45);
	float d_2 = sdfEllipsoid(p, c_2, r_2);
	float d_3 = sdfSegment(p, float3(0.0, 0.0, -0.5), float3(0.0, -2.5, -0.67));

	float outline = sdfEllipsoid(sabsx(p), c_1, float3(0.32, 0.28, 0.28));
	float socket  = min(sdfEllipsoid(p+c_3a, float3(0.0), r_3), sdfEllipsoid(p+c_3b, float3(0.0), r_3));

	float eyes    = sdfSphere(absx(p)+c_4, 0.8);
	return smin(smax(smax(smin(smin(d_1, d_2, 0.86), d_3, 0.5), -abs(outline), 1.0), -socket, 0.14), eyes, 0.09);
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

	float3 rayOrigin = float3(0.0, 0.0, 4.5);
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
