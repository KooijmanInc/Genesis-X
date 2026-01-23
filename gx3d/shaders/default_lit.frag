#version 440

layout(location = 0) in vec3 vWorldPos;
layout(location = 1) in vec3 vWorldN;

// Fragment-stage UBO (binding 1)
layout(binding = 1) uniform FSUBO {
    vec4 baseColor;   // rgba

    vec4 lightPos;    // xyz = world position, w unused
    vec4 lightColor;  // rgb = light color, a = intensity
    vec4 lightParams; // x = range
} fsu;

layout(location = 0) out vec4 fragColor;

void main()
{
    // vec3 N = normalize(vWorldN);

    // vec3 Lvec = fsu.lightPos.xyz - vWorldPos;
    // float dist = length(Lvec);
    // vec3 L = (dist > 1e-5) ? (Lvec / dist) : vec3(0.0, 0.0, 1.0);

    // float ndotl = max(dot(N, L), 0.0);

    // float range = max(fsu.lightParams.x, 1e-4);

    // // nicer falloff (range matters)
    // float x = clamp(1.0 - dist / range, 0.0, 1.0);
    // float att = x * x;

    // // scale big UI intensity values into shader space
    // float intensity = fsu.lightColor.a / 200.0;

    // vec3 ambient = fsu.baseColor.rgb * 0.03;
    // vec3 diffuse = fsu.baseColor.rgb * fsu.lightColor.rgb * (ndotl * att * intensity);

    // vec3 color = ambient + diffuse;
    // color = clamp(color, 0.0, 1.0);

    // fragColor = vec4(color, fsu.baseColor.a);


    // // fragColor = vec4(fsu.lightColor.rgb, 1.0);
    vec3 N = normalize(vWorldN);

    vec3 Lvec = fsu.lightPos.xyz - vWorldPos;
    float dist = length(Lvec);
    vec3 L = (dist > 1e-5) ? (Lvec / dist) : vec3(0.0, 0.0, 1.0);

    float ndotl = max(dot(N, L), 0.0);

    float range = max(fsu.lightParams.x, 1e-4);
    // float att = 1.0 / (1.0 + (dist * dist) / (range * range));
    float x = clamp(1.0 - dist / range, 0.0, 1.0);
    float att = x * x;

    float intensity = fsu.lightColor.a;

    vec3 ambient = fsu.baseColor.rgb * 0.15;
    vec3 diffuse = fsu.baseColor.rgb * fsu.lightColor.rgb * (ndotl * att * intensity);

    vec3 color = ambient + diffuse;
    color = clamp(color, 0.0, 1.0);
    fragColor = vec4(color, fsu.baseColor.a);






    // vec3 N = normalize(vWorldN);

    // // Light vector
    // vec3 Lvec = fsu.lightPos.xyz - vWorldPos;
    // float dist = length(Lvec);

    // // Avoid division by zero
    // vec3 L = (dist > 1e-5) ? (Lvec / dist) : vec3(0.0, 0.0, 1.0);

    // float ndotl = max(dot(N, L), 0.0);

    // // Range + attenuation (cheap, smooth)
    // float range = max(fsu.lightParams.x, 1e-4);
    // float att = 1.0 / (1.0 + (dist * dist) / (range * range));

    // float intensity = fsu.lightColor.a;

    // vec3 ambient = fsu.baseColor.rgb * 0.15;
    // vec3 diffuse = fsu.baseColor.rgb * fsu.lightColor.rgb * (ndotl * att * intensity);

    // Visualize distance to light (debug)

    // float range = max(fsu.lightParams.x, 0.0001);
    // float d = length(vWorldPos - fsu.lightPos.xyz) / range;   // 0 near, 1 at range

    // float g = 1.0 - clamp(d, 0.0, 1.0);
    // fragColor = vec4(vec3(g), 1.0);
    // fragColor = vec4(ambient + diffuse, fsu.baseColor.a);
}
