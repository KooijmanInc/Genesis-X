// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Meshes/GXPlaneMesh.h>

using namespace gx::gx3d::render;

struct Vertex {
    float px, py, pz;
    float nx, ny, nz;
};

static const Vertex kPlaneVerts[4] = {
    { -1.0f,0.0f,-1.0f,   0,1,0 },
    {  1.0f,0.0f,-1.0f,   0,1,0 },
    {  1.0f,0.0f, 1.0f,   0,1,0 },
    { -1.0f,0.0f, 1.0f,   0,1,0 }
};

static const quint16 kPlaneIndices[6] = {
    0, 2, 1,   0, 3, 2
};

GXMesh *GXPlaneMesh::create()
{
    auto* mesh = new GXMesh();

    QVector<GXMesh::Vertex> verts;
    QVector<quint16> indices;

    verts.reserve(4);
    for (const auto &v : kPlaneVerts)  verts << GXMesh::Vertex{ v.px,v.py,v.pz, v.nx,v.ny,v.nz };
    indices.reserve(6);
    for (quint16 i : kPlaneIndices) indices << i;

    mesh->setGeometry(verts, indices);

    return mesh;
}
