#version 440

layout(location = 0) in vec3 vWorldPos;
layout(location = 1) in vec3 vWorldN;
layout(location = 2) in vec2 vUv;

layout(binding = 1) uniform FSUBO {
    vec4 baseColor;
    vec4 emissive;
    vec4 emissiveLight;
} fsu;

layout(binding = 2) uniform FrameLightingUBO {
    vec4 lightPos;
    vec4 lightColor;
    vec4 lightParams;
} fl;

layout(location = 0) out vec4 fragColor;

void main(void)
{
    vec3 N = normalize(vWorldN);
    vec4 base = fsu.baseColor;
    vec4 emissive = fsu.emissive;

    vec3 emissiveColor = emissive.rgb * emissive.a;

    vec3 Lvec = fl.lightPos.xyz - vWorldPos;
    float dist = length(Lvec);
    vec3 L = (dist >1e-5) ? (Lvec / dist) : vec3(0.0, 0.0, 1.0);
    float ndot1 = max(dot(N, L), 0.0);

    float range = max(fl.lightParams.x, 1e-4);
    float x = clamp(1.0 - dist / range, 0.0, 1.0);
    float att = x * x;

    float intensity = fl.lightColor.a;

    vec3 diffuse = base.rgb * fl.lightColor.rgb * (ndot1 * att * intensity);
    vec3 ambient = base.rgb * 0.15;

    vec3 color = ambient + diffuse + emissiveColor;

    fragColor = vec4(clamp(color, 0.0, 1.0), base.a);
}
