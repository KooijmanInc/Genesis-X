// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Meshes/GXSphereMesh.h>

#include <QtMath>        // qSin, qCos, qDegreesToRadians
#include <QVector3D>

using namespace gx::gx3d::render;

struct Vertex {
    float px, py, pz;
    float nx, ny, nz;
};

static inline Vertex makeSphereVertex(float x, float y, float z)
{
    // Unit sphere: position == normal (normalized)
    const float len = std::sqrt(x*x + y*y + z*z);
    const float inv = (len > 1e-8f) ? (1.0f / len) : 0.0f;

    Vertex v{};
    v.px = x; v.py = y; v.pz = z;
    v.nx = x * inv; v.ny = y * inv; v.nz = z * inv;
    return v;
}

GXMesh *GXSphereMesh::create()
{
    auto *mesh = new GXMesh();

    // Blender-default-ish: radius 1.0 => diameter 2.0
    const float radius = 1.0f;

    // Reasonable defaults. Keep these modest since indices are quint16.
    const int rings   = 16; // latitude (excluding poles)
    const int sectors = 32; // longitude

    // Vertex count: (rings+1) * (sectors+1)  (duplicate seam for correct normals)
    const int vertCount = (rings + 1) * (sectors + 1);
    // Index count: rings * sectors * 6
    const int indexCount = rings * sectors * 6;

    QVector<GXMesh::Vertex> verts;
    QVector<quint16> indices;
    verts.reserve(vertCount);
    indices.reserve(indexCount);

    // Generate vertices (latitude from 0..pi, longitude 0..2pi)
    for (int r = 0; r <= rings; ++r) {
        const float v = float(r) / float(rings);          // 0..1
        const float phi = float(M_PI) * v;                // 0..pi

        const float y = qCos(phi);                        // -1..1? actually cos(0)=1, cos(pi)=-1
        const float sinPhi = qSin(phi);

        for (int s = 0; s <= sectors; ++s) {
            const float u = float(s) / float(sectors);    // 0..1
            const float theta = float(2.0 * M_PI) * u;    // 0..2pi

            const float x = sinPhi * qCos(theta);
            const float z = sinPhi * qSin(theta);

            const Vertex vv = makeSphereVertex(x * radius, y * radius, z * radius);
            float uv = 0.0f, v = 0.0f;
            verts << GXMesh::Vertex{ vv.px, vv.py, vv.pz, vv.nx, vv.ny, vv.nz, uv, v };
        }
    }

    // Generate indices
    // Each quad on the sphere surface -> two triangles
    const int stride = sectors + 1;
    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < sectors; ++s) {
            const int i0 = r * stride + s;
            const int i1 = (r + 1) * stride + s;
            const int i2 = (r + 1) * stride + (s + 1);
            const int i3 = r * stride + (s + 1);

            // CCW winding for outward normals (right-handed)
            indices << quint16(i0) << quint16(i2) << quint16(i1);
            indices << quint16(i0) << quint16(i3) << quint16(i2);
        }
    }

    mesh->setGeometry(verts, indices);
    return mesh;
}
