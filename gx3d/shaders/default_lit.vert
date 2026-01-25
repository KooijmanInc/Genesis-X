#version 440

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;

layout(binding = 0) uniform VSUBO {
    mat4 mvp;
    mat4 model;
} vsu;

layout(location = 0) out vec3 vWorldPos;
layout(location = 1) out vec3 vWorldN;
layout(location = 2) out vec2 vUv;

void main(void)
{
    gl_Position = vsu.mvp * vec4(inPosition, 1.0);

    vec4 wp = vsu.model * vec4(inPosition, 1.0);
    vWorldPos = wp.xyz;

    mat3 nrm = transpose(inverse(mat3(vsu.model)));
    vWorldN = normalize(nrm * inNormal);

    vUv = inUv;
}
