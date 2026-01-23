#version 450

layout(std140, binding = 0) uniform UBO {
    mat4 mvp;
    vec4 color;
} u;

layout(location = 0) out vec4 vColor;

void main(void)
{
    vColor = u.color;

    vec2 p;
    if (gl_VertexIndex == 0)
        p = vec2( 0.0,  0.8);
    else if (gl_VertexIndex == 1)
        p = vec2(-0.8, -0.8);
    else
        p = vec2( 0.8, -0.8);

    gl_Position = u.mvp * vec4(p, 0.0, 1.0);
}

// layout(std140, binding = 0) uniform UBO {
//     mat4 mvp;
//     vec4 color;
// } u;

// layout(location = 0) out vec4 vColor;

// vec2 pos(int i) {
//     if (i == 0) return vec2( 0.0,  0.8);
//     if (i == 1) return vec2(-0.8, -0.8);
//     return vec2( 0.8, -0.8);
// }

// void main()
// {
//     vColor = u.color;
//     vec2 p = pos(gl_VertexIndex);
//     gl_Position = u.mvp * vec4(p, 0.0, 1.0);
// }

// layout(location = 0) in vec3 inPos;

// layout(std140, binding = 0) uniform UBO {
//     mat4 mvp;
//     vec4 color;
// } u;

// layout(location = 0) out vec4 vColor;

// void main(void)
// {
//     vColor = u.color;
//     gl_Position = u.mvp * vec4(inPos, 1.0);
// }
