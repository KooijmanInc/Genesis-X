// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Meshes/GXPlaneMesh.h>

using namespace gx::gx3d::render;

struct Vertex {
    float px, py, pz;
    float nx, ny, nz;
    float u, v;
};

static const Vertex kPlaneVerts[4] = {
    { -1.0f,0.0f,-1.0f,   0,1,0,    0.0f, 0.0f },
    {  1.0f,0.0f,-1.0f,   0,1,0,    1.0f, 0.0f },
    {  1.0f,0.0f, 1.0f,   0,1,0,    1.0f, 1.0f },
    { -1.0f,0.0f, 1.0f,   0,1,0,    0.0f, 1.0f }
};

static const quint16 kPlaneIndices[6] = {
    0, 2, 1,   0, 3, 2
};

GXMesh *GXPlaneMesh::create()
{
    auto* mesh = new GXMesh();

    QVector<GXMesh::Vertex> verts;
    QVector<quint16> indices;

    // float u = 0.0f, uv = 0.0f;
    float tx = 1.0f, ty = 0.0f, tz = 0.0f, tw = 1.0f;

    verts.reserve(4);
    for (const auto &v : kPlaneVerts)  verts << GXMesh::Vertex{ v.px,v.py,v.pz, v.nx,v.ny,v.nz, v.u, v.v, tx, ty, tz, tw };
    indices.reserve(6);
    for (quint16 i : kPlaneIndices) indices << i;

    mesh->setGeometry(verts, indices);

    return mesh;
}
