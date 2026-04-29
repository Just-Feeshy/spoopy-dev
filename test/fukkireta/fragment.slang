#define MAX_STEPS 256
#define MAX_DISTANCE 10
#define TRACER_EPSILON 0.001
#define AA_SAMPLE_COUNT 8
#define PI 3.14159265359
#define HASHSCALE1 .1031
#define SMOOTH_EYE 0.3

struct FragmentInput {
	float4 position : SV_POSITION;
	float2 resolution : TEXCOORD0;
};

float2 rot2(float2 p, float an) {
	float cc = cos(an);
	float ss = sin(an);
	float2x2 m = float2x2(cc, -ss, ss, cc);
	return mul(m, p);
}

float2 shear(float2 p, float k) {
	float2x2 I2 = float2x2(1.0, 0.0, 0.0, 1.0);
	I2[0][1] = k;
	return mul(I2, p);
}

float hash(float p) {
	float3 p3  = frac(float3(p) * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return frac((p3.x + p3.y) * p3.z);
}

float det2(float2 a, float2 b) {
	return a.x * b.y - a.y * b.x;
}

float calcOcclusion(float3 pos, float3 nor) {
    const float4 H4 = float4(0.1031, 0.11369, 0.13787, 0.09987);
    float occlusion = 0.0;
	float kk = 0.0;
	float3 uvw = float3(0.0);
    float4 k = float4(0.7012912, 0.3941462, 0.8294585, 0.109841);

    [loop]
    for(int i = 0; i < 16; ++i) {
        k = frac(k + H4);
        float3 ap = normalize(-1.0 + 2.0 * k.xyz);
        float h = k.w * 0.1;
        ap = (nor + ap) * h;
        float d = mapScene(pos + ap, kk, uvw);
        occlusion += max(0.0, h - d);

        if(occlusion > 16.0) {
            break;
        }
    }

    occlusion /= 16.0;
    return clamp(1.0 - occlusion * 12.0, 0.0, 1.0);
}

float sdfTorus(float3 p, float2 t) {
  float2 q = float2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

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

float3 mix(float3 x, float3 y, float3 a) {
	return x * (float3(1.0) - a) + y * a;
}

float linearstep(float a, float b, in float x) {
    return clamp( (x-a)/(b-a), 0.0, 1.0 );
}

float sat(float x) {
	return min(1.0, max(0.0, x));
}

float smin(float d_1, float d_2, float k) {
    float h = max(k-abs(d_1-d_2),0.0);
    return min(d_1, d_2) - h*h*0.25/k;
}

float smin3(float d_1, float d_2, float k) {
	return min(d_1, d_2) - f_k(d_1, d_2, k);
}

float smax(float d_1, float d_2, float k) {
	return max(d_1, d_2) + f_k(d_1, d_2, k);
}

float sdfSegment(float3 p, float3 a, float3 b) {
	float h = sat(dot(p-a,b-a)/dot(b-a,b-a));
	return length(p-a-(b-a)*h);
}

float sdfCone( in float3 p, in float2 c ) {
	float2 q = float2( length(p.xz), p.y );

	float2 a = q - c*clamp( (q.x*c.x+q.y*c.y)/dot(c,c), 0.0, 1.0 );
	float2 b = q - c*float2( clamp( q.x/c.x, 0.0, 1.0 ), 1.0 );

	float s = -sign( c.y );
	float2 d = min( float2( dot( a, a ), s*(q.x*c.y-q.y*c.x) ),
			    float2( dot( b, b ), s*(q.y-c.y)  ));
	return -sqrt(d.x)*sign(d.y);
}

float3 sabsx(float3 p) {
	const float k = 0.002;
	return float3(sqrt(p.x * p.x + k), p.y, p.z);
}

float eyeSockets(float3 p) {
	static const float3 R_3A = float3(0.31,0.32,0.3);
	static const float3 c_3  = float3(-0.61, 0.38, -0.91);

	const float3 r_3 = R_3A + float3(0.03,0.0,0.0) * (p.y + 0.41);
	float d_2 = sdfEllipsoid(p+float3(abs(c_3.x), c_3.yz), float3(0.0), r_3);
	d_2 = min(d_2, sdfEllipsoid(p+c_3, float3(0.0), r_3));
	return d_2;
}

// http://research.microsoft.com/en-us/um/people/hoppe/ravg.pdf
float4 sdfBezier(float3 p, float3 va, float3 vb, float3 vc) {
	float3 w = normalize(cross(vc - vb, va - vb));
	float3 u = normalize(vc - vb);
	float3 v = cross(w, u);

	float2 m = float2(dot(va - vb, u), dot(va - vb, v));
	float2 n = float2(dot(vc - vb, u), dot(vc - vb, v));
	float3 q = float3(dot(p - vb, u), dot(p - vb, v), dot(p - vb, w));

	float mn = det2(m, n);
	float mq = det2(m, q.xy);
	float nq = det2(n, q.xy);

	float2 g = (nq + mq + mn) * n + (nq + mq - mn) * m;
	float  f = (nq - mq + mn) * (nq - mq + mn) + 4.0 * mq * nq;

	float2 z = 0.5 * f * float2(-g.y, g.x) / dot(g, g);

	// float t = clamp(0.5 + 0.5 * (det2(z, m + n) + mq + nq) / mn, 0.0, 1.0);
	float t = clamp(0.5 + 0.5 * (det2(z - q.xy, m + n)) / mn, 0.0, 1.0);

	float2 cp = m * (1.0 - t) * (1.0 - t) + n * t * t - q.xy;

	float d2 = dot(cp, cp);
	return float4(sqrt(d2 + q.z * q.z), t, q.z, -sign(f) * sqrt(d2));
}


float mapScene(float3 p, out float outMat, out float3 uvw) {
	outMat = 1.0;
	p.x *= 1.03;
	p.y *= 1.01;
	uvw = p;

	static const float3 R_2A = float3(1.05,0.61,1.20);
	static const float3 c_1  = float3(0.7, -0.52, 1.38);
	static const float3 c_2  = float3(0.0, -0.87, 0.25);
	static const float3 c_4  = float3(-0.45, 0.24, -0.455);

	const float3 qos = float3(abs(p.x), p.yz);
	const float3 tos = float3(p.xy, abs(p.z));
	const float3 sos = sabsx(p);

	float d = sdfSphere(p, 1.45);

	// Jaw
	float3 mos = p - c_2;
	mos.yz = rot2(mos.yz, 0.1);
	const float3 r_2 = R_2A + (float3(0.3,-0.35*p.z,0.1)) * 1.15 * (p.y + 0.81);
	float d_2 = sdfEllipsoid(mos, float3(0.0, 0.0, 0.05), r_2);
	d = smin(d, d_2, 0.4);

	// Mouth
	const float mouth = sdfEllipsoid(mos, float3(0.0, -0.24, 1.15), float3(0.13, 0.045, 0.2) * 1.2);

	float eyelid_plane =
		dot(qos, float3(-0.05, 1.0, 0.0)) + 0.74;
	float eyelid_plane_2 =
		dot(qos, float3(-0.05, 1.0, 0.0)) + 0.74;

	// Eye Socket
	d_2 = sdfEllipsoid(sos, c_1, float3(0.32, 0.24, 0.28));
	d = smax(d, -d_2, 1.0);

	d_2 = eyeSockets(p);
	d_2 = smax(-eyelid_plane, d_2, SMOOTH_EYE);
	d = smax(d, -d_2, 0.15);

	// Eyelids
	float3 ebos = qos + c_4;
	ebos.xy = shear(ebos.xy, -0.12);
	float3 plane_n = normalize(float3(-0.1, 1.0, 0.0));
	float d_3 = dot(qos + float3(0.0, 0.56, 0.0), plane_n) - 0.5;
	d_2 = smax(-d_3, d_2, 0.01);
	d_2 = smax(-eyelid_plane_2, d_2, SMOOTH_EYE / 1.7);
	d = smin(d, d_2, 0.02);

	// Eyeballs
	d_2 = sdfSphere(ebos, 0.69);
	if(d_2 < d) {
		outMat = 2.0;
	}
	d_2 = smax(-eyelid_plane_2, d_2, SMOOTH_EYE / 1.7);
	d = min(d, d_2);

	// Nose
	float3 eos = p - float3(0.0, -0.445, 1.03);
	eos.z *= 0.35;
	eos.yz = rot2(eos.yz, 0.465);
	float h = smoothstep(0.0, 0.26, -eos.y);
	float4 be = sdfBezier(eos,
				float3(0.0, 0.105, 0.045),
				float3(0.0, -0.035, 0.04),
				float3(0.0, -0.265, 0.005));
	float ra = 0.02 + 0.01 * (1.0 - be.y);
	d_2 = be.x - ra;
	eos.x = sqrt(eos.x*eos.x + 0.001);
	d = smin(d, d_2, 0.07 - 0.015 * h);

	// Upper Lip
	float3 cos = mos-float3(0.0, 0.015, 1.1);
	cos.yz = rot2(cos.yz, 0.1);
	d_2 = sdfCone(cos, float2(0.08, -0.16)) - 0.01;
	d = smin(d, d_2, 0.1);

	mos = sabsx(mos) + float3(0.13, 0.0, 0.0);
	eos = float3(mos.x, (mos.y + 0.04) / 1.05, mos.z);
	float3 uos = eos - float3(0.0, 0.17, 0.04);
	float4 lip = sdfBezier(uos,
	  	float3(0.0, -0.264, 1.17),
	  	float3(0.333, -0.269, 1.17),
	  	float3(0.381, -0.4, 0.8));
	float lipRa = 0.019 * (1.0 - lip.y * lip.y) * smoothstep(1.0, 0.4, lip.y);
	d_2 = lip.x - lipRa;
	d = smin(d, d_2, 0.05);

	// Mouth
	d = smax(d, -mouth, 0.01);

	// Bottom Lip
	lip = sdfBezier(eos,
	 	float3(0.0, -0.264, 1.105),
	 	float3(0.333, -0.269, 1.105),
	 	float3(0.391, -0.02, 0.9205));
	lipRa = 0.021 * (1.0 - lip.y * lip.y) * smoothstep(1.0, 0.4, lip.y);
	d_2 = lip.x - lipRa;
	d_2 = max(d_2, -mouth);
	d = smin(d, d_2, 0.05);

	// Cheeks
	d = smin(d, sdfSphere(qos - float3(0.52, -0.8, 0.71), 0.1), 0.4);

	d = smin3(d, sdfSegment(p, float3(0.0, 0.0, -0.5), float3(0.0, -2.5, -0.67)) - 0.55, 0.5);
	return d;
}

float3 calcNormal(float3 p) {
	float kk = 0.0;
	float3 uvw = float3(0.0);
	float2 e = float2(TRACER_EPSILON, 0.0);
	return normalize(float3(
		mapScene(p + float3(e.x, e.y, e.y), kk, uvw) - mapScene(p - float3(e.x, e.y, e.y), kk, uvw),
		mapScene(p + float3(e.y, e.x, e.y), kk, uvw) - mapScene(p - float3(e.y, e.x, e.y), kk, uvw),
		mapScene(p + float3(e.y, e.y, e.x), kk, uvw) - mapScene(p - float3(e.y, e.y, e.x), kk, uvw)
	));
}

float sdif(float d, float b) {
	return 0.5 + 0.5 * smoothstep(-b, b, d);
}

float calcSoftshadow(float3 ro, float3 rd, float mint, float maxt, float k) {
	float res = 1.0;
	float t = mint;
	float kk = 0.0;
	float3 uvw = float3(0.0);

	[loop]
	for(int i = 0; i < 32; ++i) {
		float h = mapScene(ro + rd * t, kk, uvw);
		res = min(res, k * h / t);
		t += clamp(h, 0.01, 0.1);
		if(res < 0.001 || t > maxt) break;
	}
	return clamp(res, 0.0, 1.0);
}


// TODO (Swapchain): Have resolution be a uniform and be iResolution like shadertoy

float2 makeUv(float2 fragCoord, float2 resolution) {
	float2 uv = (fragCoord / resolution) * 2.0 - 1.0;
	uv.y = -uv.y;
	uv.x *= resolution.x / resolution.y;
	return uv;
}

float2 traceSample(in float3 rayOrigin, in float3 rayDir, out float3 uvw) {
	float t = 0.0;
	uvw = float3(0.0);
	float3 hitPos = rayOrigin;
	float3 closestPos = rayOrigin;
	float closestDist = MAX_DISTANCE;
	float matID = -1.0;

	[loop]
	for(int i = 0; i < MAX_STEPS; ++i) {
		float tmp;
		hitPos = rayOrigin + rayDir * t;
		float dist = mapScene(hitPos, tmp, uvw);

		if(dist < closestDist) {
			closestDist = dist;
			closestPos = hitPos;
		}

		if(dist < TRACER_EPSILON) {
			matID = tmp;
			break;
		}

		t += dist;
		if(t > MAX_DISTANCE) {
			break;
		}
	}

	return float2(t,matID);
}

float4 renderTeto(float2 fragCoord, float2 resolution) {
	float2 uv = makeUv(fragCoord, resolution);
	float3 rayOrigin = float3(0.0, 0.0, 4.5);
	float3 rayDir = normalize(float3(uv, -1.5));
	float3 color = float3(1.0);
	float3 uvw = float3(0.0);
	float2 tm = traceSample(rayOrigin, rayDir, uvw);
	float focc = 1.0;
	float sss = 0.0;

	if(tm.y < 0.0) {
		return float4(color, 1.0);
	}

	float3 pos = rayOrigin + tm.x * rayDir;
	float3 normal = calcNormal(rayOrigin + tm.x * rayDir);

	if(tm.y < 1.5) {
		sss = 1.0;

		float3 qos = float3(abs(uvw.x), uvw.yz);
		color = mix(float3(0.992, 0.918, 0.859),
					float3(1.0, 0.768, 0.705),
					smoothstep(0.5, 0.0, length(qos.xy - float2(0.67, -0.69)))
		);

		// Eyeliner - follows upper eyelid edge
		{
			float4 be = sdfBezier(float3(qos.xy + float2(0.0, 0.05), 0.0),
						float3(0.39, -0.215, 0.0),
						float3(0.77, 0.1, 0.0),
						float3(0.99, -0.19, 0.0));
			float4 be2 = sdfBezier(float3(qos.xy + float2(0.0, 0.05), 0.0),
						float3(1.05, -0.19, 0.0),
						float3(0.935, -0.565, 0.0),
						float3(0.7, -0.59, 0.0));

			float ra = 0.01 + 0.05 * sqrt(be.y * (1.0 - be.y));
			float ra2 = 0.003 + 0.025 * sqrt(be2.y * (1.0 - be2.y));

			float d = smin(be.x - ra, be2.x - ra2, 0.25);
			float mask = 1.0-smoothstep(-0.006, 0.012, d);
			color = mix(color, float3(0.04, 0.02, 0.02), mask);
		}

		// Eyebrows (Tiny)
		{
			float4 be = sdfBezier(float3(qos.xy + float2(0.05, 0.1), 0.0),
						float3(0.445, -0.065, 0.0),
						float3(0.52, 0.025, 0.0),
						float3(0.675, 0.065, 0.0));

			float ra = 0.0002 + 0.01 * sqrt(be.y * (1.0 - be.y));

			float d = be.x - ra;
			float mask = 1.0-smoothstep(-0.006, 0.012, d);
			color = mix(color, float3(0.4, 0.4, 0.4), mask * 0.67);
		}

		// Eyebrows
		{
			float4 be = sdfBezier(float3(qos.xy, 0.0),
						float3(0.345, 0.145, 0.0),
						float3(0.62, 0.265, 0.0),
						float3(0.925, 0.205, 0.0));

			float ra = 0.0002 + 0.035 * sqrt(be.y * (1.0 - be.y));

			float d = be.x - ra;
			float mask = 1.0-smoothstep(-0.006, 0.012, d);
			color = mix(color, float3(1.0, 0.0, 0.271), mask);
		}
	}else if(tm.y < 2.5) {
		sss = 1.0;

		float3 qos = float3(abs(uvw.x), uvw.yz);
		float3 eos = qos - float3(0.63, -0.39, 0.45);
		eos.x *= 1.2;

		float r = length(eos.xy)+0.005;
		float3 iris = float3(0.9, 0.15, 0.3);
		color = float3(0.94);
		color *= 0.1 + 0.9 * smoothstep(0.26, 0.3, r);
		color = mix(color, iris, 1.0-smoothstep(0.24, 0.3, r));
		float edis = length((float2(abs(uvw.x),uvw.y)-float2(0.68,-0.49))/float2(4.2,3.3));
		color *= mix(float3(1.0), float3(0.4, 0.2, 0.2), linearstep(0.07, 0.16, edis)) * 1.1;
	}

	float fre = clamp(1.0 + dot(normal, rayDir), 0.0, 1.0);
	float occ = focc * calcOcclusion(pos, normal);

	float3 lig = float3(0.57, 0.46, 0.68);
	float3 light = color * (1.25 * float3(0.988, 0.757, 0.733) * sdif(dot(normal, lig), 0.67));
	color = mix(color, light, 0.4);

	// float bou = clamp(0.4 - 0.6 * normal.x, 0.0, 1.0);
	float3 lin = float3(0.0);
	lin += occ;
	lig = normalize(float3(-1.87, -0.86, -0.6));
	lin += float3(1.0, 0.3, 0.1) * sss * fre * 0.2 * (0.5+0.5 * dot(normal, lig)) * (0.1 + 0.9 * focc);

	color = color*lin + fre*fre*fre*0.07*occ;
	// color = normal * 0.5 + 0.5;
	color *= 1.1;
	return float4(color, 1.0);
}

[shader("fragment")]
float4 fragmentMain(FragmentInput input) : SV_Target {
	float2 fragCoord = input.position.xy;
	float4 sample = 0.0;
	float3 accum = 0.0;
	float coverage = 0.0;
	const float3 background = float3(1.0, 1.0, 1.0);

	sample = renderTeto(fragCoord + float2(-0.5625, -0.4375), input.resolution);
	accum += sample.rgb * sample.a;
	coverage += sample.a;

	sample = renderTeto(fragCoord + float2(-0.0625, -0.1875), input.resolution);
	accum += sample.rgb * sample.a;
	coverage += sample.a;

	sample = renderTeto(fragCoord + float2( 0.4375, -0.3125), input.resolution);
	accum += sample.rgb * sample.a;
	coverage += sample.a;

	sample = renderTeto(fragCoord + float2( 0.1875,  0.0625), input.resolution);
	accum += sample.rgb * sample.a;
	coverage += sample.a;

	sample = renderTeto(fragCoord + float2(-0.3125,  0.1875), input.resolution);
	accum += sample.rgb * sample.a;
	coverage += sample.a;

	sample = renderTeto(fragCoord + float2(-0.4375,  0.5625), input.resolution);
	accum += sample.rgb * sample.a;
	coverage += sample.a;

	sample = renderTeto(fragCoord + float2( 0.0625,  0.3125), input.resolution);
	accum += sample.rgb * sample.a;
	coverage += sample.a;

	sample = renderTeto(fragCoord + float2( 0.5625,  0.4375), input.resolution);
	accum += sample.rgb * sample.a;
	coverage += sample.a;

	float sampleWeight = 1.0 / AA_SAMPLE_COUNT;
	float alpha = coverage * sampleWeight;
	float3 color = accum * sampleWeight + background * (1.0 - alpha);
	color = pow(color, float3(0.9090));
	return float4(color, 1.0);
}
