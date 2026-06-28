#version 440

layout(location = 0) out vec3 vDir;

layout(binding = 1) uniform Params {
    float roughness;
    int faceIndex;
    int sampleCount;
    int pad;
} p;

// Convert fullscreen triangle to cube direction
vec3 getDir(int face, vec2 uv)
{
    vec2 a = uv * 2.0 - 1.0; // [-1..1]

    if (face == 0) return normalize(vec3( 1.0, -a.y, -a.x));
    if (face == 1) return normalize(vec3(-1.0, -a.y,  a.x));
    if (face == 2) return normalize(vec3( a.x,  1.0,  a.y));
    if (face == 3) return normalize(vec3( a.x, -1.0, -a.y));
    if (face == 4) return normalize(vec3( a.x, -a.y,  1.0));
    return normalize(vec3(-a.x, -a.y, -1.0));
}

void main(void)
{
    vec2 pos = vec2(
        (gl_VertexIndex == 1) ? 3.0 : -1.0,
        (gl_VertexIndex == 2) ? 3.0 : -1.0
    );

    gl_Position = vec4(pos, 0.0, 1.0);

    vec2 uv = (pos + 1.0) * 0.5;
    vDir = getDir(p.faceIndex, uv);
}
