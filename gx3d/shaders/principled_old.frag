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
    float k = (r * r) / 8.0;
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
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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
    vec4 normalScale;
    vec4 metallicFactor;
    vec4 roughnessFactor;
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
} fe;

layout(location = 0) out vec4 fragColor;

void main(void)
{
    vec3 N = normalize(vWorldN);
    vec3 V = normalize(fl.cameraWorldPos.xyz - vWorldPos);

    vec4 tex = texture(baseColorTex, vUv);
    vec3 albedo = fsu.baseColor.rgb * tex.rgb;

    float metallic  = clamp(fsu.metallicFactor.x, 0.0, 1.0);
    float roughness = clamp(fsu.roughnessFactor.x, 0.04, 1.0);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // bool hasNormalMap = (fsu.normalScale.y > 0.5); // new flag you set in C++

    // if (hasNormalMap && fsu.normalScale.x > 0.0 && length(vWorldT.xyz) > 1e-5) {
    //     vec3 tN = texture(normalTex, vUv).xyz * 2.0 - 1.0;
    //     tN.xy *= fsu.normalScale.x;
    //     tN = normalize(tN);

    //     vec3 T = normalize(vWorldT.xyz);
    //     vec3 B = normalize(cross(N, T)) * vWorldT.w;
    //     mat3 TBN = mat3(T, B, N);

    //     N = normalize(TBN * tN);
    // }

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
    float ambientStrength = fe.ambient_ao.x;

    vec3 ambient = albedo * ambientStrength;
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < lightCount; ++i) {
        GXLightGPU Lg = fl.lights[i];

        vec3 L;
        float att = 1.0;

        if (Lg.pos.w == 2.0) {
            L = normalize(-Lg.dir.xyz);
        } else {
            vec3 Lvec = Lg.pos.xyz - vWorldPos;
            float dist = length(Lvec);
            L = (dist > 1e-5) ? (Lvec / dist) : vec3(0.0, 1.0, 0.0);

            float range = max(Lg.params.x, 1e-4);
            float x = clamp(1.0 - dist / range, 0.0, 1.0);
            att = x * x;
        }

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) continue;

        float spot = 1.0;
        if (Lg.pos.w == 1.0) {
            vec3 D = normalize(Lg.dir.xyz);
            vec3 LtoP = normalize(vWorldPos - Lg.pos.xyz);
            spot = smoothstep(Lg.params.z, Lg.params.y, dot(D, LtoP));
        }

        vec3 radiance = Lg.color.rgb * (Lg.color.a * att * spot);

        vec3 H = normalize(V + L);
        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 specular = (D * G * F) /
                        max(4.0 * max(dot(N, V), 0.0) * NdotL, 1e-6);

        vec3 kD = (1.0 - F) * (1.0 - metallic);
        vec3 diffuse = kD * albedo / PI;

        Lo += (diffuse + specular) * radiance * NdotL;
    }

    vec3 linear = ambient + Lo + (fsu.emission.rgb * fsu.emission.a);

    vec3 R = reflect(-V, N);
    float specStrength = clamp(fe.envSpecularParams.y, 0.0, 1.0);
    float specExtra = fe.envSpecularParams.z;
    int specularSource = int(fe.envSpecularParams.x);

    if (specularSource == GX_SPEC_LIGHTCARD) {
        // ---- Fake environment light-card specular ----
        vec3 skyDir = normalize(fe.envSkyDirStr.xyz);

        float rd = clamp(dot(normalize(R), skyDir), 0.0, 1.0);
        float gloss = 1.0 - roughness;
        float expo = mix(2.0, 10.0, gloss * gloss);

        vec3 skyColor = vec3(1.15, 1.00, 1.20);
        vec3 Fenv = fresnelSchlick(max(dot(N, V), 0.0), F0);
        float metalBoost = mix(0.12, 1.0, metallic);

        // linear += skyColor * pow(rd, expo) * Fenv * metalBoost * specStrength;
        linear += min(vec3(2.0), skyColor * pow(rd, expo) * Fenv * metalBoost * specStrength);

    } else if (specularSource == GX_SPEC_SKYGRADIENT) {
        // gradient factor from reflection direction (up = brighter)
        float t = clamp(R.y * 0.5 + 0.5, 0.0, 1.0);

        // two “sky” colors (TEMP). Later move into UBO (ambientColor/top/bottom/etc)
        vec3 skyTop    = vec3(1.10, 1.05, 1.20);
        vec3 skyBottom = vec3(0.02, 0.02, 0.03);
        vec3 envColor  = mix(skyBottom, skyTop, t);

        // fresnel + roughness response
        float NdotV = max(dot(N, V), 0.0);
        vec3  Fenv  = fresnelSchlick(NdotV, F0);

        // roughness -> strength (rougher = dimmer)
        float envStrength = (1.0 - roughness);
        envStrength *= envStrength;

        float metalBoost = mix(0.12, 1.0, metallic);

        // specExtra can be used as a bias/boost if you want (optional)
        float extra = 1.0 + specExtra;

        linear += envColor * Fenv * envStrength * metalBoost * specStrength * extra;
        // linear += envColor * Fenv * envStrength * metalBoost * specStrength;

        // float rd = max(dot(R, normalize(vec3(0.0, 1.0, 0.0))), 0.0);
        // linear += vec3(10.0) * pow(rd, 32.0);
    }

    // ---- Tonemap + gamma ----
    // vec3 mapped = linear / (linear + vec3(0.75));
    // vec3 mapped = linear / (linear + vec3(1.0));
    vec3 mapped = tonemap_reinhard_luma(linear, 1.0);
    // vec3 color  = pow(clamp(mapped, 0.0, 1.0), vec3(1.0 / 2.2));
    // vec3 color  = pow(clamp(mapped, 0.0, 1.0), vec3(1.0, 1.0, 1.0));
    vec3 color = pow(clamp(mapped, 0.0, 1.0), vec3(1, 1, 1));
    // vec3 color  = clamp(mapped, 0.0, 1.0);

    // fragColor = vec4(fract(vUv.x * 20.0), fract(vUv.y * 20.0), 0.0, 1.0);
    // return;

    // vec2 uv = vUv;

    // // Clamp only for visualization (NOT fract / wrap)
    // uv = clamp(uv, 0.0, 1.0);

    // fragColor = vec4(uv.x, uv.y, 0.0, 1.0);
    // return;


    float alpha = fsu.baseColor.a * tex.a;
    // fragColor = vec4(color * alpha, alpha);
    fragColor = vec4(color, alpha);
}



// #version 440

// #define GX_MAX_LIGHTS 15

// struct GXLightGPU {
//     vec4 pos;
//     vec4 dir;
//     vec4 color;
//     vec4 params;
// };

// const float PI = 3.14159265359;

// float DistributionGGX(vec3 N, vec3 H, float roughness)
// {
//     float a  = roughness * roughness;
//     float a2 = a * a;
//     float NdotH  = max(dot(N, H), 0.0);
//     float NdotH2 = NdotH * NdotH;

//     float denom = (NdotH2 * (a2 - 1.0) + 1.0);
//     return a2 / max(PI * denom * denom, 1e-6);
// }

// float GeometrySchlickGGX(float NdotV, float roughness)
// {
//     float r = roughness + 1.0;
//     float k = (r * r) / 8.0; // direct lighting
//     return NdotV / max(NdotV * (1.0 - k) + k, 1e-6);
// }

// float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
// {
//     float NdotV = max(dot(N, V), 0.0);
//     float NdotL = max(dot(N, L), 0.0);
//     float ggxV = GeometrySchlickGGX(NdotV, roughness);
//     float ggxL = GeometrySchlickGGX(NdotL, roughness);
//     return ggxV * ggxL;
// }

// vec3 fresnelSchlick(float cosTheta, vec3 F0)
// {
//     // F = F0 + (1-F0)(1-cos)^5
//     return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
// }

// layout(location = 0) in vec3 vWorldPos;
// layout(location = 1) in vec3 vWorldN;
// layout(location = 2) in vec2 vUv;
// layout(location = 3) in vec4 vWorldT;

// layout(binding = 1) uniform FSUBO {
//     vec4 baseColor;
//     vec4 emission;
//     vec4 emissionLight;
//     vec4 alphaParams;
//     vec4 gamma;
//     vec4 normalScale;
//     vec4 metallicFactor;
//     vec4 roughnessFactor;
// } fsu;

// layout(binding = 2) uniform FrameLightingUBO {
//     vec4 frameParams;
//     vec4 cameraWorldPos;
//     GXLightGPU lights[GX_MAX_LIGHTS];
// } fl;

// layout(binding = 3) uniform sampler2D baseColorTex;
// layout(binding = 4) uniform sampler2D normalTex;

// layout(location = 0) out vec4 fragColor;

// void main(void)
// {
//     vec3 N = normalize(vWorldN);
//     vec3 V = normalize(fl.cameraWorldPos.xyz - vWorldPos);

//     vec4 tex = texture(baseColorTex, vUv);
//     vec3 albedo = fsu.baseColor.rgb * tex.rgb;

//     float metallic = clamp(fsu.metallicFactor.x, 0.0, 1.0);
//     float roughness = clamp(fsu.roughnessFactor.x, 0.04, 1.0);

//     vec3 F0 = vec3(0.04);
//     F0 = mix(F0, albedo, metallic);

//     if (fsu.normalScale.x > 0.0) {
//         vec3 tN = texture(normalTex, vUv).xyz * 2.0 - 1.0;
//         tN.xy *= fsu.normalScale.x;
//         tN = normalize(tN);

//         vec3 T = normalize(vWorldT.xyz);
//         vec3 B = normalize(cross(N, T)) * vWorldT.w;
//         mat3 TBN = mat3(T, B, N);

//         N = normalize(TBN * tN);
//     }

//     int lightCount = min(int(fl.frameParams.x), GX_MAX_LIGHTS);
//     float ambientStrength = fl.frameParams.y;

//     vec3 ambient = albedo * ambientStrength;

//     vec3 Lo = vec3(0.0);

//     for (int i = 0; i < lightCount; ++i) {
//         GXLightGPU Lg = fl.lights[i];

//         vec3 lightPos = Lg.pos.xyz;
//         float type = Lg.pos.w;

//         vec3 L;
//         float att = 1.0;

//         if (type == 2.0) {
//             // directional light
//             L = normalize(-Lg.dir.xyz);   // light direction *towards* surface
//         } else {
//             // point / spot
//             vec3 Lvec = lightPos - vWorldPos;
//             float dist = length(Lvec);
//             L = (dist > 1e-5) ? (Lvec / dist) : vec3(0, 1, 0);

//             float range = max(Lg.params.x, 1e-4);
//             float x = clamp(1.0 - dist / range, 0.0, 1.0);
//             att = x * x;
//         }

//         float NdotL = max(dot(N, L), 0.0);
//         if (NdotL <= 0) continue;

//         float spot = 1.0;
//         if (type == 1.0) {
//             vec3 D = normalize(Lg.dir.xyz);
//             vec3 LtoP = normalize(vWorldPos - lightPos);

//             float angleCos = dot(D, LtoP);
//             float cIn = Lg.params.y;
//             float cOut = Lg.params.z;
//             spot = smoothstep(cOut, cIn, angleCos);
//         }

//         vec3 radiance = Lg.color.rgb * (Lg.color.a * att * spot);

//         vec3 H = normalize(V + L);
//         float Dg = DistributionGGX(N, H, roughness);
//         float G = GeometrySmith(N, V, L, roughness);
//         vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

//         vec3 specNumerator = Dg * G * F;
//         float specDenom = max(4.0 * max(dot(N, V), 0.0) * NdotL, 1e-6);
//         vec3 specular = specNumerator / specDenom;

//         vec3 kS = F;
//         vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
//         vec3 diffuse = (kD * albedo) / PI;

//         Lo += (diffuse + specular) * radiance * NdotL;
//     }
//     vec3 emissionColor = fsu.emission.rgb * fsu.emission.a;

//     vec3 linear = ambient + Lo + emissionColor;

//     // --- Fake environment "light card" specular (big visible effect) ---
//     vec3 R = reflect(-V, N);

//     // Define a light-card direction in world space (where the bright patch is in the env)
//     // Pick something like upper-left-front so it reads nicely in UI
//     vec3 cardDir = normalize(vec3(-0.4, 0.7, 0.6)); // tweak later

//     // How close is the reflection to that direction?
//     float rd = clamp(dot(normalize(R), cardDir), 0.0, 1.0);

//     // Make the highlight sharper when roughness is low
//     // (roughness -> exponent): low roughness = tight highlight
//     float gloss = 1.0 - roughness;
//     float expo  = mix(2.0, 10.0, gloss * gloss);

//     vec3 cardColor = vec3(1.15, 1.00, 1.20);
//     float NdotV = max(dot(N, V), 0.0);
//     vec3  Fenv  = fresnelSchlick(NdotV, F0);
//     float metalBoost = mix(0.12, 1.0, metallic);

//     float cardLobe = pow(rd, expo);
//     linear += cardColor * cardLobe * Fenv * metalBoost * 0.35;

//     vec3 mapped = linear / (linear + vec3(0.75));
//     vec3 color = pow(clamp(mapped, 0.0, 1.0), vec3(1.0 / 1.6));

//     float alpha = fsu.baseColor.a * tex.a;

//     fragColor = vec4(color * alpha, alpha);
// }
