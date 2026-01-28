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

layout(binding = 1) uniform FSUBO {
    vec4 baseColor;
    vec4 emission;
    vec4 emissionLight;
} fsu;

layout(binding = 2) uniform FrameLightingUBO {
    vec4 frameParams;
    GXLightGPU lights[GX_MAX_LIGHTS];
    // vec4 lightPos;
    // vec4 lightDir;
    // vec4 lightColor;
    // vec4 lightParams;
    // float cosInner;
    // float cosOuter;
} fl;

layout(binding = 3) uniform sampler2D baseColorTex;

layout(location = 0) out vec4 fragColor;

void main(void)
{
    vec3 N = normalize(vWorldN);

    vec4 tex = texture(baseColorTex, vUv);
    vec3 albedo = fsu.baseColor.rgb * tex.rgb;

    int lightCount = min(int(fl.frameParams.x), GX_MAX_LIGHTS);
    float ambientStrength = fl.frameParams.y;
    int debugMode = int(fl.frameParams.z + 0.5);

    // Debug views (return early)
    // 1: show lightCount normalized (0..15)
    // 2: show light0 position (xyz mapped to 0..1-ish)
    // 3: show light0 range (x = 1 - dist/range grayscale)
    // 4: show light0 type (point=blue, spot=red)
    // 5: show light0 spot factor (grayscale)
    // 6: show light0 attenuation (grayscale)

    if (debugMode != 0) {
        if (debugMode == 1) {
            float t = clamp(float(lightCount) / 15.0, 0.0, 1.0);
            fragColor = vec4(t, t, t, 1.0);
            return;
        }

        if (lightCount <= 0) {
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }

        GXLightGPU Lg = fl.lights[0];
        vec3 lightPos = Lg.pos.xyz;
        float type = Lg.pos.w; // 0=point, 1=spot (your convention)
        float range = max(Lg.params.x, 1e-4);

        if (debugMode == 2) {
            // position debug (just to see it's not NaN/zero)
            fragColor = vec4(clamp(lightPos * 0.1 + 0.5, 0.0, 1.0), 1.0);
            return;
        }

        float dist = length(lightPos - vWorldPos);
        float x = clamp(1.0 - dist / range, 0.0, 1.0);

        if (debugMode == 3) {
            fragColor = vec4(x, x, x, 1.0);
            return;
        }

        if (debugMode == 4) {
            // point=blue, spot=red
            float isSpot = (type > 0.5) ? 1.0 : 0.0;
            fragColor = vec4(isSpot, 0.0, 1.0 - isSpot, 1.0);
            return;
        }

        float spot = 1.0;
        if (type > 0.5) {
            vec3 D = normalize(Lg.dir.xyz);
            vec3 V = normalize(vWorldPos - lightPos); // IMPORTANT (fixed)
            float angleCos = dot(D, V);
            float cIn = Lg.params.y;
            float cOut = Lg.params.z;
            spot = smoothstep(cOut, cIn, angleCos);
        }

        if (debugMode == 5) {
            fragColor = vec4(spot, spot, spot, 1.0);
            return;
        }

        float att = x * x;
        if (debugMode == 6) {
            fragColor = vec4(att, att, att, 1.0);
            return;
        }

        fragColor = vec4(1.0, 0.0, 1.0, 1.0); // unknown mode -> magenta
        return;
    }

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
    vec3 color = pow(clamp(linear, 0.0, 1.0), vec3(1.0/2.2));

    float alpha = fsu.baseColor.a * tex.a; // respect alpha
    fragColor = vec4(color, alpha);

    // vec3 N = normalize(vWorldN);

    // // light vector (fragment-facing)
    // vec3 Lvec = fl.lightPos.xyz - vWorldPos;
    // float dist = length(Lvec);
    // vec3 L = (dist >1e-5) ? (Lvec / dist) : vec3(0.0, 0.0, 1.0);

    // float ndot1 = max(dot(N, L), 0.0);

    // // Attenuation
    // float range = max(fl.lightParams.x, 1e-4);
    // float x = clamp(1.0 - dist / range, 0.0, 1.0);
    // float att = x * x;
    // // float att = 1.0;

    // // Intensity
    // float intensity = fl.lightColor.a;
    // // float intensity = 1.0;

    // // Spotlight cone
    // float spot = 1.0;
    // if (fl.cosOuter > -0.5) { // spotlight enabled
    //     vec3 D = (length(fl.lightDir.xyz) > 1e-6) ? normalize(fl.lightDir.xyz) : vec3(0, 0, -1);
    //     vec3 V = normalize(vWorldPos - fl.lightPos.xyz);

    //     float a0 = dot(D, V);
    //     float a1 = dot(-D, V);
    //     float angleCos = max(a0, a1);

    //     float cIn = max(fl.cosInner, fl.cosOuter);
    //     float cOut = min(fl.cosInner, fl.cosOuter);

    //     spot = smoothstep(cOut, cIn, dot(D, V));
    // }

    // // Material
    // vec4 tex = texture(baseColorTex, vUv);
    // vec3 albedo = fsu.baseColor.rgb * tex.rgb;

    // // Ambient + emission
    // vec3 ambient = albedo * fl.lightParams.y;
    // vec3 emissionColor = fsu.emission.rgb * fsu.emission.a;

    // // Diffuse
    // vec3 diffuse = albedo * fl.lightColor.rgb * (ndot1 * att * intensity * spot);

    // // HDR-ish clamp + gamma
    // vec3 linear = ambient + diffuse + emissionColor;
    // linear = min(linear, vec3(4.0));
    // vec3 color = pow(clamp(linear, 0.0, 1.0), vec3(1.0/2.2));
    // // vec3 color = clamp(ambient + diffuse + emissionColor, 0.0, 1.0);
    // // vec3 color = pow(clamp(linear / 4.0, 0.0, 1.0), vec3(1.0/2.2)); // base hdr settings
    // // color *= 4.0; // base hdr settings

    // // vec3 color = pow(clamp(linear, 0.0, 4.0) / 4.0, vec3(1.0/2.2)); // low emission

    // float alpha = fsu.baseColor.a * tex.a; // respect alpha
    // // float alpha = max(base.a * tex.a, clamp(emission.a, 0.0, 1.0)); // boost alpha
    // fragColor = vec4(color, alpha);
}
