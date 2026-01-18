#version 450

layout(location = 0) in vec2 inPos;

layout(std140, binding = 0) uniform UBO {
    mat4 mvp;
    vec4 color;
} u;

layout(location = 0) out vec4 vColor;

void main(void)
{
    vColor = u.color;
    gl_Position = u.mvp * vec4(inPos.xy, 0.0, 1.0);
}
