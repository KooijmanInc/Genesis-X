#version 440

#define GX_MAX_LIGHTS 15

struct GXLightGPU {
    vec4 pos;
    vec4 dir;
    vec4 color;
    vec4 params;
};

layout(location = 0) in vec3 vWorldPos;
layout(location = 1) in vec3 vWorldN;
layout(location = 2) in vec2 vUv;
layout(location = 3) in vec4 vWorldT;

layout(binding = 1) uniform FSUBO {
    vec4 baseColor;
    vec4 emission;
    vec4 emissionLight;
    vec4 alphaParams;
    vec4 gamma;
    vec4 normalScale;
} fsu;

layout(binding = 2) uniform FrameLightingUBO {
    vec4 frameParams;
    GXLightGPU lights[GX_MAX_LIGHTS];
} fl;

layout(binding = 3) uniform sampler2D baseColorTex;
layout(binding = 4) uniform sampler2D normalTex;

layout(location = 0) out vec4 fragColor;

void main(void)
{
    vec3 N = normalize(vWorldN);

    vec4 tex = texture(baseColorTex, vUv);
    vec3 albedo = fsu.baseColor.rgb * tex.rgb;

    if (fsu.normalScale.x > 0.0) {
        vec3 tN = texture(normalTex, vUv).xyz * 2.0 - 1.0;
        tN.xy *= fsu.normalScale.x;
        tN = normalize(tN);

        vec3 T = normalize(vWorldT.xyz);
        vec3 B = normalize(cross(N, T)) * vWorldT.w;
        mat3 TBN = mat3(T, B, N);

        N = normalize(TBN * tN);
    }

    int lightCount = min(int(fl.frameParams.x), GX_MAX_LIGHTS);
    float ambientStrength = fl.frameParams.y;

    vec3 ambient = albedo * ambientStrength;

    vec3 diffuseSum = vec3(0.0);

    for (int i = 0; i < lightCount; ++i) {
        GXLightGPU Lg = fl.lights[i];

        vec3 lightPos = Lg.pos.xyz;
        float type = Lg.pos.w;

        vec3 Lvec = lightPos.xyz - vWorldPos;
        float dist = length(Lvec);
        vec3 L = (dist >1e-5) ? (Lvec / dist) : vec3(0, 1, 0);

        float ndot1 = max(dot(N, L), 0.0);

        float range = max(Lg.params.x, 1e-4);
        float x = clamp(1.0 - dist / range, 0.0, 1.0);
        float att = x * x;

        float spot = 1.0;
        if (type > 0.5) {
            vec3 D = normalize(Lg.dir.xyz);
            vec3 V = normalize(vWorldPos - lightPos);
            float angleCos = dot(D, V);
            float cIn = Lg.params.y;
            float cOut = Lg.params.z;
            spot = smoothstep(cOut, cIn, angleCos);
        }

        diffuseSum += albedo * Lg.color.rgb * (ndot1 * att * Lg.color.a * spot);
    }
    vec3 emissionColor = fsu.emission.rgb * fsu.emission.a;

    vec3 linear = ambient + diffuseSum + emissionColor;
    linear = min(linear, vec3(4.0));

    vec3 color = pow(clamp(linear, 0.0, 1.0), fsu.gamma.xyz);

    float alpha = fsu.baseColor.a * tex.a;

    fragColor = vec4(color * alpha, alpha);
}
