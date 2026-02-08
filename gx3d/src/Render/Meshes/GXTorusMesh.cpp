// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Meshes/GXTorusMesh.h>

#include <QtMath>
#include <QVector3D>

using namespace gx::gx3d::render;

static inline GXMesh::Vertex makeV(float px, float py, float pz, float nx, float ny, float nz, float u, float v, float tx, float ty, float tz, float tw)
{
    return GXMesh::Vertex{ px, py, pz, nx, ny, nz, u, v, tx, ty, tz, tw };
}

GXMesh *GXTorusMesh::create()
{
    auto *mesh = new GXMesh();

    // Blender-ish feel: torus is "about 2 units across" if R=1
    const float R = 1.0f;   // major radius
    const float r = 0.35f;  // minor radius (tube)

    // Segment counts (keep safe for quint16):
    // verts = (majorSeg+1)*(minorSeg+1)
    // indices = majorSeg*minorSeg*6
    const int majorSeg = 48;
    const int minorSeg = 24;

    const int vertCount  = (majorSeg + 1) * (minorSeg + 1);
    const int indexCount = majorSeg * minorSeg * 6;

    QVector<GXMesh::Vertex> verts;
    QVector<quint16> indices;
    verts.reserve(vertCount);
    indices.reserve(indexCount);

    // Parametric torus (u around Y axis, v around tube)
    // Position:
    //   x = (R + r cos v) cos u
    //   y = r sin v
    //   z = (R + r cos v) sin u
    // Normal:
    //   center of tube circle at (R cos u, 0, R sin u)
    //   normal = normalize(pos - center)
    for (int i = 0; i <= majorSeg; ++i) {
        const float u = float(i) / float(majorSeg);
        const float a = float(2.0 * M_PI) * u;

        const float cu = qCos(a);
        const float su = qSin(a);

        const QVector3D ringCenter(R * cu, 0.0f, R * su);

        for (int j = 0; j <= minorSeg; ++j) {
            const float v = float(j) / float(minorSeg);
            const float b = float(2.0 * M_PI) * v;

            const float cv = qCos(b);
            const float sv = qSin(b);

            const float x = (R + r * cv) * cu;
            const float y = (r * sv);
            const float z = (R + r * cv) * su;

            QVector3D pos(x, y, z);
            QVector3D n = (pos - ringCenter);
            n.normalize();

            const float uu = u;
            const float vv = 1.0f - v;

            float tx = 1.0f, ty = 0.0f, tz = 0.0f, tw = 1.0f;

            verts << makeV(pos.x(), pos.y(), pos.z(), n.x(), n.y(), n.z(), uu, vv, tx, ty, tz, tw);
        }
    }

    // Indices (quads -> two triangles), CCW outward
    const int stride = minorSeg + 1;
    for (int i = 0; i < majorSeg; ++i) {
        for (int j = 0; j < minorSeg; ++j) {
            const int i0 = i * stride + j;
            const int i1 = (i + 1) * stride + j;
            const int i2 = (i + 1) * stride + (j + 1);
            const int i3 = i * stride + (j + 1);

            indices << quint16(i0) << quint16(i2) << quint16(i1);
            indices << quint16(i0) << quint16(i3) << quint16(i2);
        }
    }

    mesh->setGeometry(verts, indices);
    return mesh;
}
