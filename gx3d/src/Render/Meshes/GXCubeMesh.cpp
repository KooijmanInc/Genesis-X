// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Meshes/GXCubeMesh.h>

using namespace gx::gx3d::render;

struct Vertex {
    float px, py, pz;
    float nx, ny, nz;
    float u, v;
};

static const Vertex kCubeVerts[24] = {
    // +Z (front)
    { -1.0f,-1.0f, 1.0f,   0,0,1,    0.0f, 0.0f },
    {  1.0f,-1.0f, 1.0f,   0,0,1,    1.0f, 0.0f },
    {  1.0f, 1.0f, 1.0f,   0,0,1,    1.0f, 1.0f },
    { -1.0f, 1.0f, 1.0f,   0,0,1,    0.0f, 1.0f },

    // -Z (back)
    {  1.0f,-1.0f,-1.0f,   0,0,-1,    0.0f, 0.0f },
    { -1.0f,-1.0f,-1.0f,   0,0,-1,    1.0f, 0.0f },
    { -1.0f, 1.0f,-1.0f,   0,0,-1,    1.0f, 1.0f },
    {  1.0f, 1.0f,-1.0f,   0,0,-1,    0.0f, 1.0f },

    // -X (left)
    { -1.0f,-1.0f,-1.0f,  -1,0,0,    0.0f, 0.0f },
    { -1.0f,-1.0f, 1.0f,  -1,0,0,    1.0f, 0.0f },
    { -1.0f, 1.0f, 1.0f,  -1,0,0,    1.0f, 1.0f },
    { -1.0f, 1.0f,-1.0f,  -1,0,0,    0.0f, 1.0f },

    // +X (right)
    {  1.0f,-1.0f, 1.0f,   1,0,0,    0.0f, 0.0f },
    {  1.0f,-1.0f,-1.0f,   1,0,0,    1.0f, 0.0f },
    {  1.0f, 1.0f,-1.0f,   1,0,0,    1.0f, 1.0f },
    {  1.0f, 1.0f, 1.0f,   1,0,0,    0.0f, 1.0f },

    // +Y (top)
    { -1.0f, 1.0f, 1.0f,   0,1,0,    0.0f, 0.0f },
    {  1.0f, 1.0f, 1.0f,   0,1,0,    1.0f, 0.0f },
    {  1.0f, 1.0f,-1.0f,   0,1,0,    1.0f, 1.0f },
    { -1.0f, 1.0f,-1.0f,   0,1,0,    0.0f, 1.0f },

    // -Y (bottom)
    { -1.0f,-1.0f,-1.0f,   0,-1,0,    0.0f, 0.0f },
    {  1.0f,-1.0f,-1.0f,   0,-1,0,    1.0f, 0.0f },
    {  1.0f,-1.0f, 1.0f,   0,-1,0,    1.0f, 1.0f },
    { -1.0f,-1.0f, 1.0f,   0,-1,0,    0.0f, 1.0f },
};

static const quint16 kCubeIndices[36] = {
    // +Z
    0, 1, 2,   0, 2, 3,
    // -Z
    4, 5, 6,   4, 6, 7,
    // -X
    8, 9,10,   8,10,11,
    // +X
    12,13,14,  12,14,15,
    // +Y
    16,17,18,  16,18,19,
    // -Y
    20,21,22,  20,22,23
};

GXMesh *GXCubeMesh::create()
{
    auto* mesh = new GXMesh();

    QVector<GXMesh::Vertex> verts;
    QVector<quint16> indices;

    // float u = 0.0f, uv = 0.0f;
    float tx = 1.0f, ty = 0.0f, tz = 0.0f, tw = 1.0f;

    verts.reserve(24);
    for (const auto &v : kCubeVerts)  verts << GXMesh::Vertex{ v.px,v.py,v.pz, v.nx,v.ny,v.nz, v.u,v.v, tx,ty,tz,tw };
    indices.reserve(36);
    for (quint16 i : kCubeIndices) indices << i;

    mesh->setGeometry(verts, indices);

    return mesh;
}
