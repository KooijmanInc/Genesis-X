#version 440

#define GX_MAX_LIGHTS 15

struct GXLightGPU {
    vec4 pos;
    vec4 dir;
    vec4 color;
    vec4 params;
};

const float PI = 3.14159265359;
const int GX_SPEC_NONE = 0;
const int GX_SPEC_LIGHTCARD = 1;
const int GX_SPEC_SKYGRADIENT = 2;
const int GX_SPEC_SKYBOX = 3;
const int GX_SPEC_IBL = 4;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / max(PI * denom * denom, 1e-6);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // direct lighting
    return NdotV / max(NdotV * (1.0 - k) + k, 1e-6);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggxV = GeometrySchlickGGX(NdotV, roughness);
    float ggxL = GeometrySchlickGGX(NdotL, roughness);
    return ggxV * ggxL;
}

float luma(vec3 c) {
    return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

vec3 tonemap_reinhard_luma(vec3 c, float k)
{
    float Y = luma(c);
    float Yt = Y / (Y + k);
    return (Y > 1e-6) ? c * (Yt / Y) : vec3(0.0);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    // F = F0 + (1-F0)(1-cos)^5
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 srgbToLinear(vec3 c)
{
    // return pow(c, vec3(2.2));
    bvec3 cutoff = lessThanEqual(c, vec3(0.04045));
    vec3 lower = c / 12.92;
    vec3 upper = pow((c + 0.055) / 1.055, vec3(2.4));
    return mix(upper, lower, vec3(cutoff));
}

vec3 linearToSrgb(vec3 c, vec4 gamma)
{
    bvec3 cutoff = lessThanEqual(c, vec3(0.0031308));
    vec3 lower = c * 12.92;
    vec3 upper = 1.055 * pow(max(c, vec3(0.0)), gamma.xyz) - 0.055;
    return mix(upper, lower, vec3(cutoff));
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float fresnelUser(float ndotv, vec4 fresnelParams)
{
    // params: x=bias, y=power, z=scale
    float bias  = fresnelParams.x;
    float power = max(fresnelParams.y, 0.0);
    float scale = fresnelParams.z;

    // Classic rim curve: (1 - N·V)^power
    float f = pow(clamp(1.0 - ndotv, 0.0, 1.0), power);
    return clamp(bias + scale * f, 0.0, 1.0);
}

float pickChannel(vec4 rgba, int ch)
{
    // 0=R,1=G,2=B,3=A
    if (ch == 1) return rgba.g;
    if (ch == 2) return rgba.b;
    if (ch == 3) return rgba.a;
    return rgba.r;
}

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
    vec4 metallicFactor;
    vec4 roughnessFactor;
    vec4 fresnel;
    vec4 specular;
} fsu;

layout(binding = 2) uniform FrameLightingUBO {
    vec4 frameParams;
    vec4 cameraWorldPos;
    GXLightGPU lights[GX_MAX_LIGHTS];
} fl;

layout(binding = 3) uniform sampler2D baseColorTex;
layout(binding = 4) uniform sampler2D normalTex;

layout(binding = 5) uniform FrameEnvironmentUBO {
    vec4 ambient_ao;
    vec4 aoParams;
    vec4 envSkyDirStr;
    vec4 envSpecularParams;
    vec4 brdfLutInvSize;
} fe;

layout(binding = 6) uniform sampler2D brdfLUT;
layout(binding = 7) uniform samplerCube envCube;
layout(binding = 8) uniform samplerCube prefilterSpecCube;
layout(binding = 9) uniform samplerCube irradianceCube;

layout(location = 0) out vec4 fragColor;

void main(void)
{
    vec3 N = normalize(vWorldN);
    vec3 V = normalize(fl.cameraWorldPos.xyz - vWorldPos);

    float NdotV = max(dot(N, V), 0.0);

    float userF = fresnelUser(NdotV, fsu.fresnel);   // 0..1

    float specAmount = clamp(fsu.specular.x, 0.0, 1.0);
    int   specCh     = int(fsu.specular.y + 0.5);    // enum -> int

    float specUser = specAmount;

    vec3 base = srgbToLinear(fsu.baseColor.rgb);
    // vec2 eps = vec2(0.0009803922, 0.0015723270);
    vec2 eps = vec2(0.0012, 0.0019);
    vec2 uv = vUv;
    uv = clamp(uv, eps, vec2(1.0) - eps);
    vec4 tex = texture(baseColorTex, uv);
    vec3 albedo = base * tex.rgb;

    float metallic  = clamp(fsu.metallicFactor.x, 0.0, 1.0);
    float roughness = clamp(fsu.roughnessFactor.x, 0.04, 1.0);
    float perceptualRoughness = clamp(roughness, 0.045, 1.0);
    float alphaR = perceptualRoughness * perceptualRoughness;

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    if (fsu.normalScale.x > 0.0) {
        vec3 tN = texture(normalTex, vUv).xyz * 2.0 - 1.0;
        tN.xy *= fsu.normalScale.x;
        tN = normalize(tN);

        // vec3 T = normalize(vWorldT.xyz);
        // vec3 B = normalize(cross(N, T)) * vWorldT.w;
        // mat3 TBN = mat3(T, B, N);
        vec3 T = normalize(vWorldT.xyz);

        // re-orthonormalize T to N (critical!)
        T = normalize(T - N * dot(N, T));

        vec3 B = cross(N, T) * vWorldT.w;   // cross already orthogonal; normalize optional
        mat3 TBN = mat3(T, B, N);

        N = normalize(TBN * tN);
    }

    int lightCount = min(int(fl.frameParams.x), GX_MAX_LIGHTS);
    float ambientStrength = fe.ambient_ao.x;

    vec3 ambient = albedo * ambientStrength;
    // vec3 ambient = vec3(0.0);
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < lightCount; ++i) {
        GXLightGPU Lg = fl.lights[i];

        vec3 lightPos = Lg.pos.xyz;
        vec3 L;
        float att = 1.0;

        if (Lg.pos.w == 2.0) {
            L = normalize(-Lg.dir.xyz);
            att = 1.0;
        } else {
            vec3 Lvec = lightPos.xyz - vWorldPos;
            float dist = length(Lvec);
            L = (dist >1e-5) ? (Lvec / dist) : vec3(0, 1, 0);

            float range = max(Lg.params.x, 1e-4);
            float x = clamp(1.0 - dist / range, 0.0, 1.0);
            att = x * x;
        }

        // float NdotL = max(dot(N, L), 0.0);
        // if (NdotL <= 0.0) continue;
        float NdotL = dot(N, L);
        float soft = smoothstep(-0.1, 0.1, NdotL);
        NdotL = max(NdotL, 0.0);

        float spot = 1.0;
        if (Lg.pos.w == 1.0) {
            vec3 D = normalize(Lg.dir.xyz);
            // vec3 LtoP = normalize(vWorldPos - lightPos);
            vec3 lightToFrag = -L;
            float cosAng = dot(D, lightToFrag);
            spot = smoothstep(Lg.params.z, Lg.params.y, cosAng);
            // float angleCos = dot(D, V);
            // float cIn = Lg.params.y;
            // float cOut = Lg.params.z;
            // spot = smoothstep(cOut, cIn, angleCos);
        }

        vec3 radiance = Lg.color.rgb * (Lg.color.a * att * spot);

        vec3 H = normalize(V + L);
        float D = DistributionGGX(N, H, perceptualRoughness);
        float G = GeometrySmith(N, V, L, perceptualRoughness);
        vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 specular = (D * G * F) / max(4.0 * max(dot(N, V), 0.0) * NdotL, 1e-6);

        specular *= specUser;

        vec3 kD = (1.0 - F) * (1.0 - metallic);
        vec3 diffuse = kD * albedo / PI;

        Lo += (diffuse + specular) * radiance * NdotL * soft;
        // Lo += albedo * Lg.color.rgb * (NdotL * att * Lg.color.a * spot);
    }
    vec3 emissionColor = fsu.emission.rgb * fsu.emission.a;
    vec3 linear = ambient + Lo + emissionColor;

    float rim = fresnelUser(NdotV, fsu.fresnel);
    linear += rim * 0.05;

    vec3 R = reflect(-V, N);
    float specStrength = clamp(fe.envSpecularParams.y, 0.0, 1.0);
    float specExtra = fe.envSpecularParams.z;
    int specularSource = int(fe.envSpecularParams.x);

    if (specularSource == GX_SPEC_LIGHTCARD) {
        // ---- Fake environment light-card specular ----
        vec3 skyDir = normalize(fe.envSkyDirStr.xyz);

        float rd = clamp(dot(normalize(R), skyDir), 0.0, 1.0);
        float gloss = 1.0 - perceptualRoughness;
        float expo = mix(2.0, 10.0, gloss * gloss);

        vec3 skyColor = vec3(1.15, 1.00, 1.20);
        vec3 Fenv = fresnelSchlick(max(dot(N, V), 0.0), F0);
        Fenv *= specUser;
        float metalBoost = mix(0.12, 1.0, metallic);

        linear += min(vec3(2.0), skyColor * pow(rd, expo) * Fenv * metalBoost * specStrength);
    } else if (specularSource == GX_SPEC_SKYGRADIENT) {
        float t = clamp(R.y * 0.5 + 0.5, 0.0, 1.0);

        float r = clamp(perceptualRoughness, 0.0, 1.0);
        float maxMip = max(fe.envSpecularParams.w, 0.0);
        float lod = r * maxMip;

        vec3 prefiltered = textureLod(prefilterSpecCube, R, lod).rgb;

        float NdotV = max(dot(N, V), 0.0);

        vec2 invSize = fe.brdfLutInvSize.xy;

        // BRDF LUT lookup (your PNG stores A in .r and B in .a)
        vec2 uv = vec2(NdotV, r);
        uv = clamp(uv, 0.5 * invSize, 1.0 - 0.5 * invSize);

        // vec4 brdfTex = texture(brdfLUT, uv);
        vec4 brdfTex = texture(brdfLUT, uv);

        float A = brdfTex.r;
        float B = brdfTex.a;

        // Fresnel at view angle
        vec3 F = fresnelSchlickRoughness(NdotV, F0, r);

        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - metallic);

        // // Split-sum specular IBL (prefiltered env missing -> envColor is a stand-in)
        vec3 specIBL = prefiltered * (F * A + B);

        specIBL *= specUser;

        float irrLod = maxMip;
        // vec3 irradiance = texture(irradianceCube, N, irrLod).rgb;
        vec3 irradiance = texture(irradianceCube, N).rgb;
        vec3 diffuseIBL = irradiance * albedo;

        linear += diffuseIBL * kD * fe.envSkyDirStr.w;

        // Optional: keep your existing intensity knobs
        float extra = 1.0 + specExtra;
        linear += specIBL * (specStrength * extra);
    } else if (specularSource == GX_SPEC_SKYBOX) {
        // vec4 norm = texture(normalTex, vUv);
        // fragColor = vec4(norm.rgb, 1.0);
        // return;
    }

    vec3 mapped = tonemap_reinhard_luma(linear, 1.0);

    vec3 color = clamp(mapped, 0.0, 1.0);

    color = linearToSrgb(color, fsu.gamma);

    // float z = gl_FragCoord.z;
    // fragColor = vec4(fsu.baseColor.rgb, 1.0);
    // return;
    float alpha = fsu.baseColor.a * tex.a;
    fragColor = vec4(color, alpha);
}
