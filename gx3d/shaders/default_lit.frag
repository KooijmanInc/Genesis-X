#version 440

layout(location = 0) in vec3 vWorldPos;
layout(location = 1) in vec3 vWorldN;
layout(location = 2) in vec2 vUv;

layout(binding = 1) uniform FSUBO {
    vec4 baseColor;
} fsu;

layout(binding = 2) uniform FrameLightingUBO {
    vec4 lightPos;
    vec4 lightDir;
    vec4 lightColor;
    vec4 lightParams;
    float cosInner;
    float cosOuter;
} fl;

layout(binding = 3) uniform sampler2D baseColorTex;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 N = normalize(vWorldN);

    vec3 D = normalize(fl.lightDir.xyz);
    vec3 V = normalize(vWorldPos - fl.lightPos.xyz);
    float angleCos = dot(D, V);

    float spot = 1.0;

    if (fl.cosOuter > -0.5) {
        spot = smoothstep(fl.cosOuter, fl.cosInner, angleCos);
    }

    vec3 Lvec = fl.lightPos.xyz - vWorldPos;
    float dist = length(Lvec);
    vec3 L = (dist > 1e-5) ? (Lvec / dist) : vec3(0.0, 0.0, 1.0);

    float ndot1 = max(dot(N, L), 0.0);

    float range = max(fl.lightParams.x, 1e-4);
    // float att = 1.0 / (1.0 + (dist * dist) / (range * range));
    float x = clamp(1.0 - dist / range, 0.0, 1.0);
    float att = x * x;

    float intensity = fl.lightColor.a;

    vec4 tex = texture(baseColorTex, vUv);
    vec3 albedo = fsu.baseColor.rgb * tex.rgb;

    vec3 ambient = albedo * fl.lightParams.y;
    vec3 diffuse = albedo * fl.lightColor.rgb * (ndot1 * att * intensity * spot);

    vec3 color = clamp(ambient + diffuse, 0.0, 1.0);
    color = pow(color, vec3(1.0/2.2));
    fragColor = vec4(color, fsu.baseColor.a * tex.a);
    // vec3 ambient = fsu.baseColor.rgb * 0.15;
    // vec3 diffuse = fsu.baseColor.rgb * fl.lightColor.rgb * (ndotl * att * intensity);

    // vec3 color = ambient + diffuse;
    // color = clamp(color, 0.0, 1.0);
    // fragColor = vec4(fract(vUv), 0.0, 1.0);
    // fragColor = vec4(vUv, 0.0, 1.0);
    // fragColor = vec4(color, fsu.baseColor.a);
    // fragColor = tex;
    // return;
}
