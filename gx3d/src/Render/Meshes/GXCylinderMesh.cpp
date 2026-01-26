// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Meshes/GXCylinderMesh.h>

#include <QtMath>
#include <QVector3D>

using namespace gx::gx3d::render;

static inline GXMesh::Vertex makeV(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
{
    return GXMesh::Vertex{ px, py, pz, nx, ny, nz, u, v };
}

GXMesh *GXCylinderMesh::create()
{
    auto *mesh = new GXMesh();

    // Blender-ish defaults: diameter 2, height 2
    const float radius = 1.0f;
    const float height = 2.0f;

    const int sectors = 32; // keep modest for quint16

    QVector<GXMesh::Vertex> verts;
    QVector<quint16> indices;

    // Layout:
    // Side vertices: bottom ring (sectors) + top ring (sectors)
    // Cap vertices:  bottom center (1) + bottom ring (sectors)
    //               top center (1) + top ring (sectors)
    //
    // Total verts = 2*sectors + 1 + sectors + 1 + sectors = 4*sectors + 2
    verts.reserve(4 * sectors + 2);

    // Indices:
    // Side: sectors quads => 2 tris each => sectors * 6
    // Caps: bottom fan => sectors * 3, top fan => sectors * 3
    indices.reserve(sectors * 6 + sectors * 3 + sectors * 3);

    const float y0 = -height * 0.5f;
    const float y1 = height * 0.5f;

    const int sideBottomStart = 0;
    const int sideTopStart    = sideBottomStart + sectors;

    const int bottomCenterIdx = sideTopStart + sectors;
    const int bottomRingStart = bottomCenterIdx + 1;

    const int topCenterIdx    = bottomRingStart + sectors;
    const int topRingStart    = topCenterIdx + 1;

    // --- Side bottom ring ---
    for (int s = 0; s < sectors; ++s) {
        const float u = float(s) / float(sectors);
        const float a = float(2.0 * M_PI) * u;

        const float x = radius * qCos(a);
        const float z = radius * qSin(a);

        QVector3D n(x, 0.0f, z);
        n.normalize();

        verts << makeV(x, y0, z, n.x(), n.y(), n.z(), u, 0.0f);
    }

    // --- Side top ring ---
    for (int s = 0; s < sectors; ++s) {
        const float u = float(s) / float(sectors);
        const float a = float(2.0 * M_PI) * u;

        const float x = radius * qCos(a);
        const float z = radius * qSin(a);

        QVector3D n(x, 0.0f, z);
        n.normalize();

        verts << makeV(x, y1, z, n.x(), n.y(), n.z(), u, 1.0f);
    }

    // --- Bottom cap center ---
    verts << makeV(0.0f, y0, 0.0f, 0.0f, -1.0f, 0.0f, 0.5f, 0.5f);

    // --- Bottom cap ring (separate normals) ---
    for (int s = 0; s < sectors; ++s) {
        const float u = float(s) / float(sectors);
        const float a = float(2.0 * M_PI) * u;

        const float x = radius * qCos(a);
        const float z = radius * qSin(a);

        const float capU = (x / (2.0f * radius)) + 0.5f;
        const float capV = (z / (2.0f * radius)) + 0.5f;

        verts << makeV(x, y0, z, 0.0f, -1.0f, 0.0f, capU, capV);
    }

    // --- Top cap center ---
    verts << makeV(0.0f, y1, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f);

    // --- Top cap ring (separate normals) ---
    for (int s = 0; s < sectors; ++s) {
        const float u = float(s) / float(sectors);
        const float a = float(2.0 * M_PI) * u;

        const float x = radius * qCos(a);
        const float z = radius * qSin(a);

        const float capU = (x / (2.0f * radius)) + 0.5f;
        const float capV = (z / (2.0f * radius)) + 0.5f;

        verts << makeV(x, y1, z, 0.0f, 1.0f, 0.0f, capU, capV);
    }

    // --- Side indices (quads) ---
    // CCW when viewed from outside.
    for (int s = 0; s < sectors; ++s) {
        const int s0 = sideBottomStart + s;
        const int s1 = sideBottomStart + ((s + 1) % sectors);
        const int t0 = sideTopStart + s;
        const int t1 = sideTopStart + ((s + 1) % sectors);

        // triangle 1: bottom s0 -> bottom s1 -> top t1
        indices << quint16(s1) << quint16(s0) << quint16(t1);
        // triangle 2: bottom s0 -> top t1 -> top t0
        indices << quint16(s0) << quint16(t0) << quint16(t1);
    }

    // --- Bottom cap indices ---
    // Bottom faces downward (-Y). For outward-facing bottom, winding is CCW when seen from below.
    for (int s = 0; s < sectors; ++s) {
        const int b0 = bottomRingStart + s;
        const int b1 = bottomRingStart + ((s + 1) % sectors);

        indices << quint16(bottomCenterIdx) << quint16(b0) << quint16(b1);
    }

    // --- Top cap indices ---
    // Top faces upward (+Y). Winding is CCW when seen from above.
    for (int s = 0; s < sectors; ++s) {
        const int t0 = topRingStart + s;
        const int t1 = topRingStart + ((s + 1) % sectors);

        indices << quint16(topCenterIdx) << quint16(t1) << quint16(t0);
    }

    mesh->setGeometry(verts, indices);
    return mesh;
}
