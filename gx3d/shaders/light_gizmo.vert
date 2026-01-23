#version 440

layout(location = 0) in vec3 position;

layout(binding = 0) uniform UBO {
    mat4 mvp;
    vec4 color;
} ub;

layout(location = 0) out vec4 vColor;

void main(void)
{
    gl_Position = ub.mvp * vec4(position, 1.0);
    vColor = ub.color;
}
