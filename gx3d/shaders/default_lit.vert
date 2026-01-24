#version 440

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 inUv;

// Vertex-stage UBO (binding 0)
layout(binding = 0) uniform VSUBO {
    mat4 mvp;
    mat4 model;
} vsu;

layout(location = 0) out vec3 vWorldPos;
layout(location = 1) out vec3 vWorldN;
layout(location = 2) out vec2 vUv;

void main(void)
{
    gl_Position = vsu.mvp * vec4(position, 1.0);

    vec4 wp = vsu.model * vec4(position, 1.0);
    vWorldPos = wp.xyz;

    // NOTE: for non-uniform scale later you’ll want inverse-transpose normal matrix,
    // but for now this is fine.
    // vWorldN = normalize(mat3(vsu.model) * normal);
    mat3 nrm = transpose(inverse(mat3(vsu.model)));
    vWorldN = normalize(nrm * normal);

}
