#version 450

layout(std140, binding = 0) uniform PickUBO {
    mat4 uMvp;
    uvec4 uId;
} ubo;

layout(location = 0) out vec4 fragColor;

vec4 encodeId(uint id)
{
    // pack 32-bit id into RGBA8 normalized
    float r = float((id >>  0) & 0xFFu) / 255.0;
    float g = float((id >>  8) & 0xFFu) / 255.0;
    float b = float((id >> 16) & 0xFFu) / 255.0;
    float a = float((id >> 24) & 0xFFu) / 255.0;
    return vec4(r, g, b, a);
}

void main(void)
{
    fragColor = encodeId(ubo.uId.x);
}
