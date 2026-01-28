#version 440

layout(location = 0) in vec3 vWorldPos;
layout(location = 1) in vec3 vWorldN;
layout(location = 2) in vec2 vUv;

layout(binding = 1) uniform FSUBO {
    vec4 baseColor;
    vec4 emission;
    vec4 emissionLight;
} fsu;

layout(binding = 2) uniform FrameLightingUBO {
    vec4 lightPos;
    vec4 lightColor;
    vec4 lightParams;
} fl;

layout(binding = 3) uniform sampler2D baseColorTex;

layout(location = 0) out vec4 fragColor;

void main(void)
{
    vec3 N = normalize(vWorldN);
    vec4 base = fsu.baseColor;
    vec4 emission = fsu.emission;

    vec3 emissionColor = emission.rgb * emission.a;

    vec3 Lvec = fl.lightPos.xyz - vWorldPos;
    float dist = length(Lvec);
    vec3 L = (dist >1e-5) ? (Lvec / dist) : vec3(0.0, 0.0, 1.0);

    float ndot1 = max(dot(N, L), 0.0);

    float range = max(fl.lightParams.x, 1e-4);
    float x = clamp(1.0 - dist / range, 0.0, 1.0);
    float att = x * x;

    float intensity = fl.lightColor.a;

    vec4 tex = texture(baseColorTex, vUv);
    vec3 albedo = base.rgb * tex.rgb;

    // vec3 ambient = albedo * 0.15;
    vec3 debugAmbient = albedo * 0.00;
    // vec3 diffuse = albedo * fl.lightColor.rgb * (ndot1 * att * intensity);
    vec3 diffuse = vec3(0.0);

    // vec3 ambient = base.rgb * 0.15;
    // vec3 diffuse = base.rgb * fl.lightColor.rgb * (ndot1 * att * intensity);

    // vec3 color = clamp(ambient + diffuse + emissionColor, 0.0, 1.0);
    vec3 linear = debugAmbient + diffuse + emissionColor;
    linear = min(linear, vec3(4.0));

    // vec3 color = pow(clamp(linear / 4.0, 0.0, 1.0), vec3(1.0/2.2)); // base hdr settings
    // color *= 4.0; // base hdr settings

    // vec3 color = pow(clamp(linear, 0.0, 4.0) / 4.0, vec3(1.0/2.2)); // low emission
    vec3 color = pow(clamp(linear, 0.0, 1.0), vec3(1.0/2.2)); // what I had

    float alpha = base.a * tex.a; // respect alpha
    // float alpha = max(base.a * tex.a, clamp(emission.a, 0.0, 1.0)); // boost alpha
    fragColor = vec4(color, alpha);

    // vec3 emissionColor = emission.rgb * emission.a;
    // vec3 color = pow(clamp(emissionColor, 0.0, 1.0), vec3(1.0/2.2));
    // fragColor = vec4(color, 1.0);


    // fragColor = vec4(pow(clamp(emissionColor, 0.0, 1.0), vec3(1.0/2.2)), 1.0);
    // return;

    // vec3 linear = ambient + diffuse;
    // fragColor = vec4(pow(clamp(linear, 0.0, 1.0), vec3(1.0/2.2)), 1.0);
    // return;


    // color = pow(color, vec3(1.0/2.2));
    // fragColor = vec4(color, base.a * tex.a);
    // vec3 color = ambient + diffuse + emissiveColor;

    // fragColor = vec4(vUv, 0.0, 1.0);
    // fragColor = vec4(clamp(color, 0.0, 1.0), base.a);
}
