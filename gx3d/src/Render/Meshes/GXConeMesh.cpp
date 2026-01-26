// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Meshes/GXConeMesh.h>

#include <QtMath>
#include <QVector3D>

using namespace gx::gx3d::render;

static inline GXMesh::Vertex makeV(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
{
    return GXMesh::Vertex{ px, py, pz, nx, ny, nz, u, v };
}

GXMesh *GXConeMesh::create()
{
    auto *mesh = new GXMesh();

    // Blender-ish defaults
    const float radius = 1.0f;
    const float height = 2.0f;

    const float yBase = -height * 0.5f;
    const float yTip  =  height * 0.5f;

    // Segments around the cone
    const int sectors = 32; // keep modest for quint16

    // Geometry layout:
    // - side ring: sectors vertices
    // - tip: 1 vertex
    // - base center: 1 vertex
    // - base ring: sectors vertices (separate so base normals can be (0,-1,0))
    const int sideRingStart = 0;
    const int tipIndex      = sectors;
    const int baseCenter    = sectors + 1;
    const int baseRingStart = sectors + 2;

    QVector<GXMesh::Vertex> verts;
    QVector<quint16> indices;

    verts.reserve(sectors + 1 + 1 + sectors);
    // sides: sectors triangles + base cap: sectors triangles
    indices.reserve(sectors * 3 + sectors * 3);

    // Precompute slope normal factor for the cone side.
    // For a right cone, side normal direction is proportional to (x, radius/height, z).
    // Normalize with y component = radius/height.
    const float nyUn = radius / height;

    // --- Side ring vertices (y = 0) ---
    for (int s = 0; s < sectors; ++s) {
        const float u = float(s) / float(sectors);
        const float a = float(2.0 * M_PI) * u;

        const float x = radius * qCos(a);
        const float z = radius * qSin(a);

        QVector3D n(x, nyUn, z);
        n.normalize();

        verts << makeV(x, yBase, z, n.x(), n.y(), n.z(), u, 0.0f);
    }

    // --- Tip vertex (y = height) ---
    // Normal at the tip is undefined; give it a reasonable up-ish normal.
    verts << makeV(0.0f, yTip, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);

    // --- Base center vertex (cap) ---
    verts << makeV(0.0f, yBase, 0.0f, 0.0f, -1.0f, 0.0f, 0.5f, 0.5f);

    // --- Base ring vertices (cap) ---
    for (int s = 0; s < sectors; ++s) {
        const float u = float(s) / float(sectors);
        const float a = float(2.0 * M_PI) * u;

        const float x = radius * qCos(a);
        const float z = radius * qSin(a);

        const float capU = (x / (2.0f * radius)) + 0.5f;
        const float capV = (z / (2.0f * radius)) + 0.5f;

        verts << makeV(x, yBase, z, 0.0f, -1.0f, 0.0f, capU, capV);
    }

    // --- Side triangles ---
    // Winding CCW when viewed from outside.
    for (int s = 0; s < sectors; ++s) {
        const int s0 = sideRingStart + s;
        const int s1 = sideRingStart + ((s + 1) % sectors);

        indices << quint16(s1) << quint16(s0) << quint16(tipIndex);
    }

    // --- Base cap triangles ---
    // We want the base to face downward (-Y), so when viewed from below it should be CCW.
    // Equivalently: from above it will look CW, which is fine.
    for (int s = 0; s < sectors; ++s) {
        const int b0 = baseRingStart + s;
        const int b1 = baseRingStart + ((s + 1) % sectors);

        indices << quint16(baseCenter) << quint16(b0) << quint16(b1);
    }

    mesh->setGeometry(verts, indices);
    return mesh;
}
