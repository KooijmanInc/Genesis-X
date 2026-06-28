#version 440

layout(binding = 0) uniform samplerCube envCube;  // SOURCE skybox cube

layout(binding = 1) uniform Params {
    float roughness;   // 0..1
    int faceIndex;     // 0..5 (optional if you build direction in VS)
    int sampleCount;   // e.g. 256 desktop, 64 mobile
    int pad;
} p;

layout(location = 0) in vec3 vDir;     // direction for this pixel (world)
layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

// Hammersley
float RadicalInverse_VdC(int bits)
{
    // reinterpret bits as "unsigned-like" via masking in int space
    bits = (bits << 16) | ((bits >> 16) & 0xFFFF);
    bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
    bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
    bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
    bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);

    // Convert to [0,1)
    // 1.0 / 2^32 = 2.3283064365386963e-10
    // But bits is signed int; make it positive-ish by mapping to float with +0.5 trick:
    float f = float(bits);
    // bring into unsigned range by adding 2^32 if negative
    if (f < 0.0) f += 4294967296.0;
    return f * 2.3283064365386963e-10;
}

vec2 Hammersley(int i, int N)
{
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

// GGX importance sample
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
    float a = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(max(0.0, 1.0 - cosTheta*cosTheta));

    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // tangent space -> world (build basis from N)
    vec3 up = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
    vec3 T = normalize(cross(up, N));
    vec3 B = cross(N, T);

    return normalize(T * H.x + B * H.y + N * H.z);
}

void main(void) {
    vec3 N = normalize(vDir);
    vec3 R = N;
    vec3 V = R;

    float rough = clamp(p.roughness, 0.04, 1.0);

    vec3 prefiltered = vec3(0.0);
    float totalWeight = 0.0;

    int Nsamples = int(max(p.sampleCount, 1));

    for (int i = 0; i < Nsamples; ++i) {
        vec2 Xi = Hammersley(i, Nsamples);
        vec3 H = ImportanceSampleGGX(Xi, N, rough);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0) {
            // sample source env
            vec3 c = texture(envCube, L).rgb;

            prefiltered += c * NdotL;
            totalWeight += NdotL;
        }
    }

    prefiltered = prefiltered / max(totalWeight, 1e-6);

    outColor = vec4(prefiltered, 1.0);
}
