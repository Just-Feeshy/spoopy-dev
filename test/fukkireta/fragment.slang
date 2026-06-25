#define MAX_STEPS 256
#define MAX_DISTANCE 8
#define TRACER_EPSILON 0.001
#define DOT_EPSILON 0.0001
#define SMOOTH_EYE 0.3
#define WORLD_ROT_Y 0.0
#define INV_N_SPIRAL 0.0
#define HAIR_ROT 0.21
#define B_SPIRAL 0.0318309886184
#define PC_PI 3.14159265358979323846
#define INV_PC_PI 0.31830988618379067154
#define Y_CYL_RAD 2.4

#define AA_SAMPLE_COUNT 1

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

// Credit to HaxeFlixel for fastSin and fastCos
// https://github.com/HaxeFlixel/flixel/blob/dev/flixel/math/FlxMath.hx

float fastSin(in float n) {
	n *= 0.3183098862;

	if(n > 1.0) {
		n -= ((int)ceil(n) >> 1) << 1;
	}else if(n < -1) {
		n += ((int)ceil(-n) >> 1) << 1;
	}

	return n > 0.0 ? n * (3.1 + n * (0.5 + n * (-7.2 + n * 3.6)))
				: n * (3.1 - n * (0.5 + n * (7.2 + n * 3.6)));
}

float fastCos(in float n) {
	return fastSin(n + 1.570796327);
}

float4 opElongate(in float3 p, in float3 h) {
	float3 q = abs(p) - h;
	return float4(max(q, 0.0), min(max(q.x, max(q.y, q.z)), 0.0));
}

float2 pc_convert_to_polar(float2 p) {
    return float2(sqrt(dot(p, p)), atan2(p.y, p.x));
}

float pc_euclidean_distance(float2 p, float2 q) {
    return sqrt(p.x * p.x + q.x * q.x - 2.0 * p.x * q.x * cos(p.y - q.y));
}

float2 shear(float2 p, float k) {
	float2x2 I2 = float2x2(1.0, 0.0, 0.0, 1.0);
	I2[0][1] = k;
	return mul(I2, p);
}

float3 rotY(float3 p, float an) {
	p.xz = rot2(p.xz, an);
	return p;
}

float2 iCylinderY(float3 ro, float3 rd, float rad) {
	float3 oc = ro;
	float a = dot(rd.xz, rd.xz);
	float b = dot(oc.xz, rd.xz);
	float c = dot(oc.xz, oc.xz) - rad * rad;
	float h = b * b - a * c;
	if(h < 0.0) {
		return float2(-1.0, -1.0);
	}
	h = sqrt(h);
	return float2(-b - h, -b + h) / a;
}

float bnoise( in float x ) {
    float i = floor(x);
    float f = fract(x);
    float s = sign(fract(x/2.0)-0.5);
    float k = 0.5+0.5*sin(i);
    return s*f*(f-1.0)*((16.0*k-4.0)*f*(f-1.0)-1.0);
}

float det2(float2 a, float2 b) {
	return a.x * b.y - a.y * b.x;
}

float3 calcNormal(float3 pos) {
	const float eps = 0.001;

    float3 n = float3(
        mapD(pos + float3(eps, 0.0, 0.0)).x -
        mapD(pos - float3(eps, 0.0, 0.0)).x,

        mapD(pos + float3(0.0, eps, 0.0)).x -
        mapD(pos - float3(0.0, eps, 0.0)).x,

        mapD(pos + float3(0.0, 0.0, eps)).x -
        mapD(pos - float3(0.0, 0.0, eps)).x
	);

    return normalize(n);
}

float calcSoftShadow(in float3 ro, in float3 rd, in float mint, in float tmax, float k) {
	float2 sph = iCylinderY(ro, rd, Y_CYL_RAD);
	tmax = min(tmax, sph.y);

	float res = 1.0;
	float t = mint;

    [loop]
	for(int i = 0; i < 128; i++) {
		float kk;
		float3 kk2;
		float h = mapScene(ro + rd * t, kk, kk2).x;
		res = min(res, k*h/t);
		t += clamp(h, 0.005, 0.1);

		if(res<0.002 || t>tmax) {
			break;
		}
	}

	return max(res, 0.0);
}

float calcOcclusion(float3 pos, float3 nor) {
    const float4 H4 = float4(0.1031, 0.11369, 0.13787, 0.09987);
    float occlusion = 0.0;
	float kk = 0.0;
	float3 uvw = float3(0.0);
    float4 k = float4(0.7012912, 0.3941462, 0.8294585, 0.109841);

    [loop]
    for(int i = 0; i < 16; i++) {
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

float sdfHorCylinder( float3 p, float h, float r ) {
	float2 d = abs(float2(length(p.xy),p.z)) - float2(r,h);
	return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float sdfBox(float3 p, float3 b) {
	float3 q = abs(p) - b;
	return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sdfTorus(float3 p, float2 t) {
	float2 q = float2(length(p.xz)-t.x, p.y);
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

float fakeBottomHighlight(float2 p, float xCenter, float xInset, float yCenter) {
	float2 q = float2(abs(abs(p.x) - xCenter) - xInset, p.y - yCenter);
	q = rot2(q, -0.56);
	q.x *= 1.75;
	return length(q);
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

float2 sdfSegment(float3 p, float3 a, float3 b) {
	float h = sat(dot(p-a,b-a)/dot(b-a,b-a));
	return float2(length(p-a-(b-a)*h), h);
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

float sabs(float x, float k) {
	return sqrt(x * x + k);
}

float3 sabsx(float3 p, float k) {
	return float3(sabs(p.x, k), p.y, p.z);
}

float bWave(float x, float b) {
	return smax(1.0 - sabs(fmod(2.0 * b * x * INV_PC_PI, 2.0) - 1.0, 0.01), 0.0, 0.5);
	// return sabs(fastSin(b * x), 0.002);
}

float eyeSockets(float3 p) {
	static const float3 R_3A = float3(0.31,0.3,0.31);
	static const float3 c_3  = float3(-0.55, 0.38, -0.91);

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

	float t = clamp(0.5 + 0.5 * (det2(z - q.xy, m + n)) / mn, 0.0, 1.0);

	float2 cp = m * (1.0 - t) * (1.0 - t) + n * t * t - q.xy;

	float d2 = dot(cp, cp);
	return float4(sqrt(d2 + q.z * q.z), t, q.z, -sign(f) * sqrt(d2));
}

float drillProfile(float y, float c1) {
	return max(0.0, min(2.0, y * y)) * c1;
}

float sdfSpiralTurnDrillCompact(
	float3 p,
	float2 polar,
	float a,
	float bRadius,
	float bRange,
	float bHeight,
	float turn,
	float thickness,
	float heightScale,
	float c1
) {
	// q_by(p) = theta + 2π turn
	float qby = polar.y + turn * 2.0 * PC_PI;

	// Radial spiral equation:
	// r = a + bRadius * theta
	float qbx = a + bRadius * qby;

	float2 q = float2(qbx, qby);

	// Spiral ribbon thickness
	float d_spiral = pc_euclidean_distance(polar, q) - thickness;

	// Turn/range limiter.
	// This is now independent from radial spacing.
	float d_range = abs(qby * bRange) - 1.0;

	// Helical base height.
	// This is also independent from radial spacing.
	float yBase = heightScale * bHeight * qby;

	float h = drillProfile(yBase, c1);
	h = max(h, 0.025);

	// Height band
	float d_lower = yBase - p.y;
	float d_upper = p.y - (yBase + h);

	// Intersection of constraints for one turn
	float d = max(d_spiral, d_range);
	d = max(d, d_lower);
	d = max(d, d_upper);

	return d;
}

float4 sdfDrills(in float3 pos, float3 qos) {
	static const float a = 0.0;
	static const float A = 0.05;

	// Controls how many visible turns/segments you keep.
	static const float activeTurns = 11.0;

	// Range normalization:
	// qby is allowed roughly between -activeTurns*pi and +activeTurns*pi.
	static const float bRange = 1.0 / (activeTurns * PC_PI);

	// Smaller = tighter radial spacing between turns.
	static const float radialCompact = 1.72;
	static const float bRadius = bRange / radialCompact;

	// Keep height independent from radial compactness.
	static const float bHeight = bRange;

	static const float thickness = 0.02;
	static const float heightScale = 2.0;
	static const float c1 = 0.275;

	float3 mos = qos - float3(2.05, -1.45, 0.15);
	mos.xz *= A * fastSin(15.0 * mos.y) + 1 - A;
	mos.xy = rot2(mos.xy, -HAIR_ROT);

	// Spiral lies in xz plane, y is vertical.
	float2 polar = pc_convert_to_polar(mos.xz);

	// baseTurn must use bRadius because bRadius controls the actual spiral radius.
	float baseTurn = floor((polar.x - a - polar.y * bRadius) / (2.0 * bRadius * PC_PI));

	float d = 1e20;

	[unroll]
	for(int i = -2; i <= 2; ++i) {
		float turn = baseTurn + float(i);

		float di = sdfSpiralTurnDrillCompact(
			mos,
			polar,
			a,
			bRadius,
			bRange,
			bHeight,
			turn,
			thickness,
			heightScale,
			c1
		);

		// Union across nearby turns
		d = min(d, di);
	}

	d *= 0.35;

	return float4(d, pos);
}

float4 sdfClothes(in float3 pos, float3 qos, float2 se, out float matID, float m) {
	matID = 4.0;
	float3 opos = pos;

	float d = se.x - 0.49 * (1.0 - 0.3 * sqrt(se.y));
	d = max(d,  abs(pos.x) - 1.15);

	opos -= float3(0.0, -2.3, -0.3);
	opos.x *= 0.05 * fastSin(8.0 * opos.y) + (1 - 0.05);
	opos.z += 0.03 * fastSin(9.0 * opos.y);
	opos.y += 0.38 * log(0.5 * opos.z + 1.0);
	float4 w = opElongate(opos, float3(0.0, 0.45, 0.0));
	float d_2 = w.w + sdfTorus(float3(w.xy, -w.z), float2(0.54, 0.02));
	d = smin(d, d_2, 0.07);

	// Collar
	{
		float3 mpos = opos - float3(0.0, -1.36, 0.0);
		float radii = 0.84 + 0.1 * log(2.0 * saturate(opos.z) + 1.0);
		mpos.y = exp(-length(mpos.xz) + 1.17) - mpos.y;
		float d_3 = sdfBox(mpos, float3(radii, 0.01, radii));
		d_3 = max(d_3, -length(opos.xz) + 0.56);
		d_3 = max(d_3, length(opos.xz) - radii);
		mpos.x *= 0.2;
		float cut = -exp(7.5 * abs(mpos.x)) * abs(mpos.x) - 0.52;
		d_3 = max(d_3, mpos.z + cut);
		d_3 *= 0.55;
		d = min(d, d_3);

		float d_4 = d;
		d_3 = max(d_3 - 0.005, abs(length(opos.xz) - radii + 0.07) - 0.02);
		d_3 = max(d_3, mpos.z + cut + 0.005);
		d   = smin(d, d_3, 0.02);

		if(d_3 < d_4) {
			matID = 4.1;
		}
	}

	// Strap
	{
		float d_3 = d;
		d_2 = max(d_2 - 0.005, abs(pos.x) - 0.17);
		d   = smin(d, d_2, 0.02);

		if(d_2 < d_3) {
			matID = 4.1;
		}
	}

	return float4(d, pos);
}


// TODO (MATH): Write this out in LaTeX

float3 quadBezierPoint(float3 p0, float3 p1, float3 p2, float t) {
    float u = 1.0 - t;
    return u*u*p0 + 2.0*u*t*p1 + t*t*p2;
}

float3 quadBezierTangent(float3 p0, float3 p1, float3 p2, float t) {
    return normalize(2.0*(1.0 - t)*(p1 - p0) + 2.0*t*(p2 - p1));
}

float3 cubicBezierPoint(float3 p0, float3 p1, float3 p2, float3 p3, float t) {
	float u = 1.0 - t;
	return u*u*u*p0
	     + 3.0*u*u*t*p1
	     + 3.0*u*t*t*p2
	     + t*t*t*p3;
}

float3 cubicBezierD1(float3 p0, float3 p1, float3 p2, float3 p3, float t) {
	float u = 1.0 - t;
	return 3.0*u*u*(p1 - p0)
	     + 6.0*u*t*(p2 - p1)
	     + 3.0*t*t*(p3 - p2);
}

float3 cubicBezierD2(float3 p0, float3 p1, float3 p2, float3 p3, float t) {
	float u = 1.0 - t;
	return 6.0*u*(p2 - 2.0*p1 + p0)
	     + 6.0*t*(p3 - 2.0*p2 + p1);
}

float3 cubicBezierTangent(float3 p0, float3 p1, float3 p2, float3 p3, float t) {
	return normalize(cubicBezierD1(p0, p1, p2, p3, t));
}

float segmentProjection01(float3 p, float3 a, float3 b) {
	float3 v = b - a;
	float vv = dot(v, v);
	return vv < 1e-8 ? 0.0 : saturate(dot(p - a, v) / vv);
}

float initialTControlPolygon(float3 p, float3 p0, float3 p1, float3 p2, float3 p3) {
	float h0 = segmentProjection01(p, p0, p1);
	float h1 = segmentProjection01(p, p1, p2);
	float h2 = segmentProjection01(p, p2, p3);

	float3 c0 = lerp(p0, p1, h0);
	float3 c1 = lerp(p1, p2, h1);
	float3 c2 = lerp(p2, p3, h2);

	float d0 = dot(c0 - p, c0 - p);
	float d1 = dot(c1 - p, c1 - p);
	float d2 = dot(c2 - p, c2 - p);

	float t0 = h0 / 3.0;
	float t1 = (1.0 + h1) / 3.0;
	float t2 = (2.0 + h2) / 3.0;

	float bestT = t0;
	float bestD = d0;

	if(d1 < bestD) {
		bestD = d1;
		bestT = t1;
	}

	if(d2 < bestD) {
		bestT = t2;
	}

	return bestT;
}

float cubicBezierDist2(
	float3 p,
	float3 p0, float3 p1, float3 p2, float3 p3,
	float t
) {
	float3 q = cubicBezierPoint(p0, p1, p2, p3, t);
	float3 r = q - p;
	return dot(r, r);
}

// So it's inspired off of https://www.shadertoy.com/view/MdKBWt
// However, this iBezier takes wayyy too much unnecessary performance so
// this cubic bezier is 5x less expensive and barely any drop in quality
// Sorry iq, I just can't stand working at 2 FPS for me :(
float2 sdfCubicBezier(float3 p, float3 p0, float3 p1, float3 p2, float3 p3) {
	float t = initialTControlPolygon(p, p0, p1, p2, p3);

	float bestT = t;
	float bestD2 = cubicBezierDist2(p, p0, p1, p2, p3, t);

	[unroll]
	for(int i = 0; i < 3; ++i) {
		float3 q  = cubicBezierPoint(p0, p1, p2, p3, t);
		float3 q1 = cubicBezierD1   (p0, p1, p2, p3, t);
		float3 q2 = cubicBezierD2   (p0, p1, p2, p3, t);

		float3 r = q - p;

		float numerator   = dot(r, q1);
		float denominator = dot(q1, q1) + dot(r, q2);

		if(abs(denominator) < DOT_EPSILON) {
			break;
		}

		float step = numerator / denominator;

		// Prevent Newton from teleporting across the curve.
		step = clamp(step, -0.25, 0.25);

		float newT = saturate(t - step);
		float newD2 = cubicBezierDist2(p, p0, p1, p2, p3, newT);

		// Only accept if it improved.
		if(newD2 < bestD2) {
			t = newT;
			bestT = newT;
			bestD2 = newD2;
		} else {
			break;
		}
	}

	// Endpoint fallback is cheap and important.
	float d20 = cubicBezierDist2(p, p0, p1, p2, p3, 0.0);
	float d21 = cubicBezierDist2(p, p0, p1, p2, p3, 1.0);

	if(d20 < bestD2) {
		bestD2 = d20;
		bestT = 0.0;
	}

	if(d21 < bestD2) {
		bestD2 = d21;
		bestT = 1.0;
	}

	return float2(sqrt(max(bestD2, 0.0)), bestT);
}

float sdfHairStrand(float4x3 mp, in float3 p, float flatten, float r1, float angle, float b, float a) {
	float2 be = sdfCubicBezier(p, mp[0], mp[1], mp[2], mp[3]);
	float t = saturate(be.y);

	float3 c = cubicBezierPoint  (mp[0], mp[1], mp[2], mp[3], t);
	float3 T = cubicBezierTangent(mp[0], mp[1], mp[2], mp[3], t);

	float3 Braw = cross(mp[1] - mp[0], mp[2] - mp[0]);
	float3 B = dot(Braw, Braw) < DOT_EPSILON
		? float3(0.0, 0.0, 1.0)
		: normalize(Braw);

	float3 N = normalize(cross(B, T));

	// Rotate the flattening frame around the tangent
	float ca = fastCos(angle);
	float sa = fastSin(angle);

	float3 Nrot = ca * N + sa * B;
	float3 Brot = -sa * N + ca * B;

	float3 mos = p;
	float2 Bos = normalize(float2(B.x * 0.75, B.y * 0.25));
	float2 pol = float2(atan2(p.z, p.x), -mos.y);
	mos.xz *= 1.0 - a * bWave(dot(pol, Bos), b);

	float t2 = smax(4.0 * t * t - 3.0, 0.0, 0.1);
	float3 q = lerp(mos, p, t2) - c;

	float axialDist = dot(q, T);
	float profile = saturate(4.0 * t * (1.0 - t));
	float radius = r1 * profile;

	float capDist = 0.0;

	if(t <= DOT_EPSILON) {
		capDist = max(-axialDist, 0.0);
	}

	if(t >= 1.0 - DOT_EPSILON) {
		capDist = max(axialDist, 0.0);
	}

	float qN = dot(q, Nrot);
	float qB = dot(q, Brot);

	// Flatten along Nrot
	float d = (length(float3(qN / flatten, capDist, qB)) - radius) * flatten;

	return d;
}

float4 sdfHair(in float3 pos, float3 qos) {
	float3 opos = pos;
	float d_1 = sdfEllipsoid(qos, float3(0.22, 0.05, -0.03), float3(1.25, 1.43, 1.45));
	float d_2 = dot(pos - float3(0.0, 0.0, 0.7), float3(0.0, 0.0, 1.0));
	float d   = max(d_1, d_2);

	{
		float2 be = sdfCubicBezier(qos - float3(0.05, 0.05, 0.0),
			float3(-0.3, 1.25, 0.7),
			float3(0.4, 1.15, 0.65),
			float3(1.9, 0.7, 0.6),
			float3(0.84, -1.32, 0.75)
		);

		float h = saturate(be.y);
		float profile = pow(saturate(4.0 * h * (1.0 - h)), 1.25);
		float radius = 0.20 * profile;
		float d_be = be.x - radius;
		d = smin(d, d_be, 0.02);
	}

	{
		float2 be = sdfCubicBezier(qos - float3(0.05, 0.05, 0.0),
			float3(-0.3, 1.25, 0.7),
			float3(0.5, 0.95, 1.05),
			float3(1.77, 0.7, 0.8),
			float3(0.84, -1.02, 0.85)
		);

		float h = saturate(be.y);
		float profile = saturate(6.0 * h * (1.0 - h));
		float radius = 0.095 * profile;
		float d_be = be.x - radius;
		d = min(d, d_be);
	}

	{
		float3 p0 = float3(-0.1, 1.29, 0.7);
		float3 p1 = float3(0.77, 0.9, 1.55);
		float3 p2 = float3(1.01, -0.1, 0.9);
		float3 p3 = float3(1.36, -0.48, 0.91);

		float s = sdfHairStrand(float4x3(p0, p1, p2, p3), qos, 0.24, 0.41, 0.04, 16.0, 0.017);
		d = min(d, s);
	}

	{
		float3 p0 = float3(0.0, 1.29, 0.7);
		float3 p1 = float3(0.17, 0.85, 1.525);
		float3 p2 = float3(-0.15, -0.07, 1.435);
		float3 p3 = float3(-0.24, -0.51, 1.535);

		float s = sdfHairStrand(float4x3(p0, p1, p2, p3), opos, 0.23, 0.38, 0.42, 20.0, 0.029);
		d_2 = s;
	}


	// Attention to Detail

	{
		float3 p0 = float3(-0.06, 1.295, 0.7);
		float3 p1 = float3(0.515, 0.73, 1.505);
		float3 p2 = float3(0.4, -0.065, 1.27);
		float3 p3 = float3(-0.02, -0.47, 1.565);
		float2 be = sdfCubicBezier(pos, p0, p1, p2, p3);

		float h = saturate(be.y);
		float profile = saturate(3.4 * h * (1.0 - h));
		float radius = 0.095 * profile;
		float d_be = be.x - radius;
		d_2 = min(d_2, d_be);
	}

	{
		float3 p0 = float3(-0.06, 1.295, 0.7);
		float3 p1 = float3(0.375, 0.79, 1.565);
		float3 p2 = float3(0.545, -0.06, 1.31);
		float3 p3 = float3(0.69, -0.325, 1.625);
		float2 be = sdfCubicBezier(pos, p0, p1, p2, p3);

		float h = saturate(be.y);
		float profile = pow(saturate(3.0 * h * (1.0 - h)), 1.25);
		float radius = 0.1 * profile;
		float d_be = be.x - radius;
		d = smin(d, d_be, 0.02);
	}

	{
		float3 p0 = float3(0.0, 1.285, 0.7);
		float3 p1 = float3(0.285, 1.56, 0.72);
		float3 p2 = float3(-0.05, 1.72, 0.68);
		float4 be = sdfBezier(pos, p0, p1, p2);

		float profile = saturate(be.y * (1.0 - be.y));
		float radius = 0.17 * profile;
		float d_be = be.x - radius;
		d_2 = min(d_2, d_be);
	}

	{
		float3 p0 = float3(0.35, 1.0, 0.65);
		float3 p1 = float3(0.205, 0.8, 1.87);
		float3 p2 = float3(0.815, -0.40, 1.17);
		float3 p3 = float3(1.165, -0.495, 1.09);
		float2 be = sdfCubicBezier(qos, p0, p1, p2, p3);

		float profile = saturate(be.y * (1.0 - be.y));
		float radius = 0.18 * profile;
		float d_be = be.x - radius;
		d = smin(d, d_be, 0.02);
	}

	d = min(d, d_2);
	return float4(d, pos);
}

float mapScene(float3 p, out float outMat, out float3 uvw) {
	outMat = 1.0;
	float3 opos = p;
	p.x *= 1.045;
	p.y *= 1.005;
	uvw = p;

	static const float2 jawZ = float2(0.125, 0.03);

	static const float3 R_2A = float3(1.05,0.61,1.20);
	static const float3 c_1  = float3(0.7, -0.52, 1.38);
	static const float3 c_2  = float3(0.0, -0.885, 0.25 - jawZ.x);
	static const float3 c_4  = float3(-0.385, 0.24, -0.5);

	const float3 qos = float3(abs(p.x), p.yz);
	const float3 tos = float3(p.xy, abs(p.z));
	float3 sos = sabsx(p, 0.002);

	float d = sdfSphere(p, 1.45);

	// Jaw
	float3 mos = p - c_2;
	mos.yz = rot2(mos.yz, 0.1);
	const float3 r_2 = R_2A + (float3(0.3,-0.4*p.z,0.1)) * 1.15 * (p.y + 0.81);
	float d_2 = sdfEllipsoid(mos, float3(0.0, 0.0, 0.05), r_2);
	d = smin(d, d_2, 0.4);

	// Mouth
	const float mouth = sdfEllipsoid(mos, float3(0.0, -0.24, 1.15 + jawZ.y), float3(0.13, 0.045, 0.2) * 1.2);

	float eyelid_plane =
		dot(qos, float3(-0.05, 1.0, 0.0)) + 0.74;
	float eyelid_plane_2 =
		dot(qos, float3(-0.05, 1.0, 0.0)) + 0.74;

	// Eye Socket
	d_2 = sdfEllipsoid(sos, c_1, float3(0.29, 0.24, 0.28));
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
	d_2 = sdfSphere(ebos, 0.67);
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
				float3(0.0, -0.035, 0.015),
				float3(0.0, -0.265, -0.01));
	float ra = 0.02 + 0.01 * (1.0 - be.y);
	d_2 = be.x - ra;
	eos.x = sqrt(eos.x*eos.x + 0.001);
	d = smin(d, d_2, 0.08 - 0.02 * h);

	// Upper Lip
	float3 cos = mos-float3(0.0, 0.015, 1.1 + jawZ.y);
	cos.yz = rot2(cos.yz, 0.1);
	d_2 = sdfCone(cos, float2(0.08, -0.16)) - 0.01;
	d = smin(d, d_2, 0.1);

	mos = sabsx(mos, 0.005) + float3(0.13, 0.0, 0.0);
	float3 lipEos = float3(mos.x, (mos.y + 0.04) / 1.05, mos.z - jawZ.y);
	float3 uos = lipEos - float3(0.0, 0.17, 0.04);
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
	lip = sdfBezier(lipEos,
	 	float3(0.0, -0.264, 1.105),
	 	float3(0.333, -0.269, 1.105),
	 	float3(0.391, -0.02, 0.9205));
	lipRa = 0.021 * (1.0 - lip.y * lip.y) * smoothstep(1.0, 0.4, lip.y);
	d_2 = lip.x - lipRa;
	d_2 = max(d_2, -mouth);
	d = smin(d, d_2, 0.05);

	// Cheeks
	d = smin(d, sdfSphere(qos - float3(0.53, -0.82, 0.71 + jawZ.x), 0.1), 0.3);


	// Neck
	const float2 se_1 = float2(-0.65, -0.23);
	const float2 se_2 = float2(-2.60, -0.35);
	float2 se = sdfSegment(p, float3(0.0, se_1), float3(0.0, se_2));
	d_2 = se.x - 0.53;

	// Shoulders
	sos.y *= 1.03;
	se = sdfSegment(sos, float3(0.0, -2.7, -0.25), float3(1.83, -2.98, -0.17));
	d_2 = smin(d_2, se.x - 0.34, 0.1);
	d = smin(d, d_2, 0.4);

	// Hair
	float4 tmp = sdfHair(p, qos);
	if(tmp.x < d) {
		d = tmp.x;
		outMat = 3.0;
		uvw = tmp.yzw;
	}

	// Drills
	tmp = sdfDrills(p, qos);
	if(tmp.x < d) {
		d = tmp.x;
		outMat = 3.1;
		uvw = tmp.yzw;
	}

	// Clothes
	float tempMat = 4.0;
	tmp = sdfClothes(opos, qos, se, tempMat, (se_2.y - se_1.y) / (se_2.x - se_1.x));
	if(tmp.x < d) {
		d = tmp.x;
		outMat = tempMat;
		uvw = tmp.yzw;
	}

	return d;
}

float4 mapD(in float3 pos) {
	float matID;
	float3 uvw;
	float4 h = mapScene(pos, matID, uvw);

	return h;
}

float3 sdif3(float ndl, float ir) {
    float pndl = clamp( ndl, 0.0, 1.0 );
    float nndl = clamp(-ndl, 0.0, 1.0 );
    return float3(pndl) + float3(1.0,0.1,0.01)*0.7*pow(clamp(ir*0.75-nndl,0.0,1.0),2.0);
}

float sdif(float d, float b) {
	return lerp(0.5 + 0.5 * smoothstep(-b, b, d), sdif3(d, b).x, 0.2);
}


// TODO (Swapchain): Have resolution be a uniform and be iResolution like shadertoy

float2 makeUv(float2 fragCoord, float2 resolution) {
	float2 uv = (fragCoord / resolution) * 2.0 - 1.0;
	uv.y = -uv.y;
	uv.x *= resolution.x / resolution.y;
	return uv;
}

float2 traceSample(in float3 rayOrigin, in float3 rayDir, out float3 uvw) {
	uvw = float3(0.0);
	float matID = -1.0;
	float t = 1.0;
	float tmax = MAX_DISTANCE;

	float2 cyl = iCylinderY(rayOrigin, rayDir, Y_CYL_RAD);
	if(cyl.y < 0.0) {
		return float2(-1.0, -1.0);
	}

	tmax = min(tmax, cyl.y);
	t = max(1.0, cyl.x);

	[loop]
	for(int i = 0; i < MAX_STEPS; ++i) {
		float tmp;
		float3 pos = rayOrigin + rayDir * t;
		float dist = mapScene(pos, tmp, uvw);

		if(dist < TRACER_EPSILON) {
			matID = tmp;
			break;
		}

		t += dist * 0.95;
		if(t > tmax) {
			break;
		}
	}

	return float2(t, matID);
}

float4 renderTeto(float2 fragCoord, float2 resolution) {
	float2 uv = makeUv(fragCoord, resolution);
	const float3 cameraTarget = float3(0.0, -0.25, -0.6);
	const float3 cameraBaseOffset = float3(0.0, 0.0, 4.5);
	const float cameraFocalLength = 1.5;
	float3 rayOrigin = cameraTarget + rotY(cameraBaseOffset, -WORLD_ROT_Y);
	float3 cameraForward = normalize(cameraTarget - rayOrigin);
	float3 cameraRight = normalize(cross(float3(0.0, 1.0, 0.0), cameraForward));
	float3 cameraUp = cross(cameraForward, cameraRight);
	float3 rayDir = normalize(cameraForward * cameraFocalLength + cameraRight * uv.x + cameraUp * uv.y);
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
			float4 be = sdfBezier(float3(qos.xy + float2(0.05, 0.05), 0.0),
						float3(0.41, -0.225, 0.0),
						float3(0.77, 0.05, 0.0),
						float3(0.92, -0.19, 0.0));
			float4 be2 = sdfBezier(float3(qos.xy + float2(0.05, 0.05), 0.0),
						float3(0.95, -0.21, 0.0),
						float3(0.945, -0.465, 0.0),
						float3(0.82, -0.54, 0.0));

			float ra = 0.01 + 0.05 * sqrt(be.y * (1.0 - be.y));
			float ra2 = 0.003 + 0.025 * sqrt(be2.y * (1.0 - be2.y));

			float d = smin(be.x - ra, be2.x - ra2, 0.24);
			float mask = 1.0-smoothstep(-0.006, 0.012, d);
			color = mix(color, float3(0.04, 0.02, 0.02), mask);
		}

		// Eyebrows (Tiny)
		{
			float4 be = sdfBezier(float3(qos.xy + float2(0.1, 0.1), 0.0),
						float3(0.445, -0.065, 0.0),
						float3(0.52, 0.015, 0.0),
						float3(0.615, 0.035, 0.0));

			float ra = 0.0001 + 0.01 * sqrt(be.y * (1.0 - be.y));

			float d = be.x - ra;
			float mask = 1.0-smoothstep(-0.006, 0.011, d);
			color = mix(color, float3(0.4, 0.2, 0.2), mask * 0.67);
		}

		// Eyebrows
		{
			float4 be = sdfBezier(float3(qos.xy, 0.0),
						float3(0.315, 0.04, 0.0),
						float3(0.59, 0.16, 0.0),
						float3(0.895, 0.10, 0.0));

			float ra = 0.025 * sqrt(be.y * (1.0 - be.y));

			float d = be.x - ra;
			float mask = 1.0-smoothstep(-0.006, 0.012, d);
			color = mix(color, float3(1.0, 0.0, 0.271), mask);
		}
	}else if(tm.y < 2.5) {
		sss = 1.0;

		const float3 eyeMiddleValue = float3(0.0);
		const float3 eyeBottomValue = float3(-0.05);
		const float eyeMiddleMin = 0.35;
		const float eyeMiddleMax = 0.55;
		const float3 eyeTopValue = float3(0.05, 0.75, 0.55);

		float3 qos = float3(abs(uvw.x), uvw.yz);
		float ss = sign(uvw.x);
		qos.xz = rot2(qos.xz, -0.001 * ss - 0.002);
		float3 eos = qos - float3(0.57, -0.39, 0.45);
		eos.x *= 1.2;

		float r = length(eos.xy)+0.005;
		float3 iris = float3(0.87, 0.175, 0.35);
		float ringGradient = 1.0 - saturate((eos.y + 0.31) / 0.62);
		color = float3(0.94);

		// Gradient of the Eye
		float eyeGradientT = 1.0 - saturate((eos.y + 0.31) / 0.62);
		float3 eyeGradientValue = (eyeGradientT > eyeMiddleMax)
			? lerp(eyeMiddleValue, eyeTopValue, (eyeGradientT - eyeMiddleMax) / (1.0 - eyeMiddleMax))
			: ((eyeGradientT < eyeMiddleMin)
			? lerp(eyeBottomValue, eyeMiddleValue, eyeGradientT / eyeMiddleMin)
			: eyeMiddleValue);
		iris += float3(eyeGradientValue);
		float irisRing = smoothstep(0.162, 0.172, r) - smoothstep(0.184, 0.187, r);
		iris *= 1.0 - 0.1 * saturate(irisRing);

		// Final Iris Color
		color = mix(iris * ringGradient, color, smoothstep(0.27, 0.31, r));
		color = mix(color, iris, 1.0-smoothstep(0.24, 0.31, r));
		color = mix(color, float3(0.35, 0.0, 0.15), 1.0 - smoothstep(0.055, 0.11, r));
		color = mix(color, float3(0.89, 0.57, 0.57), 1.0 - smoothstep(0.002, 0.033, r));

		// Fake Bottom Highlight
		{
			color += 1.0 - smoothstep(0.0, 0.035, fakeBottomHighlight(qos.xy, 0.58, 0.13, -0.56));
		}

		// Fake Highlight
		qos = float3(abs(uvw.x), uvw.yz);
		color += 1.0 - smoothstep(0.01, 0.04, length(qos.xy - float2(0.59 - 0.08 * ss, -0.27)));

		// Fake Occlusion Backed In
		float edis = length((float2(abs(uvw.x),uvw.y)-float2(0.68,-0.51))/float2(4.2,3.3));
		color *= mix(float3(1.0), float3(0.2, 0.1, 0.1), linearstep(0.07, 0.16, edis)) * 1.1;
	}else if(tm.y > 3.5 && tm.y < 4.5) {
		sss = 1.0;

		if(tm.y == 4.0) {
			color = float3(0.08, 0.2, 0.47);
		}else {
			color = float3(0.83, 0.1, 0.35);
		}
	}

	float fre = clamp(1.0 + dot(normal, rayDir), 0.0, 1.0);
	float occ = focc * calcOcclusion(pos, normal);

	if(tm.y >= 3.0 && tm.y < 3.5) {
		sss = 1.0;
		color = float3(0.9, 0.1, 0.3);

		if(tm.y == 3.0) {
			occ = lerp(1.0, occ, 0.68);

			// Fake Shadow
			color *= 1.0 - max(abs(uvw.y - 0.2) * 0.3, 0.0);

			// Fake Hair Highlight
			color = max(
				color + 0.25 * float3(min(1.8 - abs((12.8 + bnoise(dot(uvw.xz, uvw.xz))
					* 10.0 * abs(fastSin(20.0 * uvw.x)))
					* (uvw.y - 0.15 - 0.045 * fastSin(3.0 * uvw.x) - 0.3)), 1.5) * 0.5)
				, color);

			// Pinkish bottom fake highlight
			static const float ph = 0.48;
			color *= 1.0 + smax(saturate(abs(uvw.y - ph) - ph), 0.0, 0.1) * occ;
		}else if(tm.y == 3.1) {
			occ = lerp(1.0, occ, 0.66);

			float3 mos = float3(abs(uvw.x) - 1.84, uvw.yz);
			mos.xy = rot2(mos.xy, -HAIR_ROT);

			color = mix(min(color + float3(0.1), float3(1.0)), color, 1.0 - saturate(mos.y));

			color = max(
				color + 0.25 * float3(min(1.8 - abs((6.0 + bnoise(dot(mos.yz, mos.zx))
					* 7.0 * abs(fastCos(20.0 * uvw.y)))
					* mos.x), 1.5) * 0.4)
				, color);

			color = max(
				color + 0.25 * float3(min(1.4 - abs((12.0 + bnoise(dot(mos.yz, mos.zx))
					* 14.0 * abs(fastCos(40.0 * uvw.y)))
					* mos.x), 1.5) * 0.5)
				, color);
		}
	}

	float3 lig = float3(0.57, 0.46, 0.68);
	float3 light = color * (1.25 * float3(0.988, 0.757, 0.733) * sdif(dot(normal, lig), 0.67));
	color = mix(color, light, 0.4);

	// float bou = clamp(0.4 - 0.6 * normal.x, 0.0, 1.0);
	float3 lin = float3(0.0);
	float sha = calcSoftShadow(pos + normal * 0.002, lig, 0.0001, 2.0, 5.0);
	float3 amb = occ * (0.55 + 0.45 * normal.y) * 1.15;
	lig = normalize(float3(-1.87, -0.86, -0.6));
	lin += occ - amb * 0.01;
	lin += float3(1.0, 0.3, 0.1) * sss * fre * 0.2 * (0.5+0.5 * saturate(dot(normal, lig)) * sha * amb) * (0.1 + 0.9 * focc);

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

	[loop]
	for(int m = 0; m < AA_SAMPLE_COUNT; ++m) {
		[loop]
		for(int n = 0; n < AA_SAMPLE_COUNT; ++n) {
			float2 offset = (float2((float)m, (float)n) / (float)AA_SAMPLE_COUNT) - 0.5;
			sample = renderTeto(fragCoord + offset, input.resolution);
			accum += sample.rgb * sample.a;
			coverage += sample.a;
		}
	}

	float sampleWeight = 1.0 / ((float)AA_SAMPLE_COUNT * (float)AA_SAMPLE_COUNT);
	float alpha = coverage * sampleWeight;
	float3 color = accum * sampleWeight + background * (1.0 - alpha);
	color = pow(color, float3(0.9090));
	return float4(color, 1.0);
}
