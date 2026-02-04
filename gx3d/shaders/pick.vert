#version 450

layout(location = 0) in vec3 aPos;

layout(std140, binding = 0) uniform PickUBO {
    mat4 uMvp;
    uvec4 uId;
} ubo;

void main(void)
{
    gl_Position = ubo.uMvp * vec4(aPos, 1.0);
}
