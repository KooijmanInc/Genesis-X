// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXMESH_H
#define GXMESH_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <QVector>

class QRhi;
class QRhiBuffer;
class QRhiCommandBuffer;

namespace gx::gx3d::render {

class GENESISX_GX3D_EXPORT GXMesh
{
public:
    struct Vertex {
        float px, py, pz;
        float nx, ny, nz;
        float u, v;
    };

    GXMesh() = default;
    virtual ~GXMesh();

    bool isReady() const { return m_vbuf && m_ibuf && !m_vertices.isEmpty() && !m_indices.isEmpty(); }

    // CPU data
    const QVector<Vertex>& vertices() const { return m_vertices; }
    const QVector<quint16>& indices() const { return m_indices; }

    int vertexCount() const { return m_vertices.size(); }
    int indexCount() const { return m_indices.size(); }

    void setGeometry(const QVector<Vertex>& v, const QVector<quint16>& i);

    // GPU resources
    void ensureResources(QRhi* rhi);
    void uploadIfNeeded(QRhi* rhi, QRhiCommandBuffer* cb);

    QRhiBuffer* vertexBuffer() const { return m_vbuf; }
    QRhiBuffer* indexBuffer() const { return m_ibuf; }

    void destroyRhiResources();

protected:
    QVector<Vertex> m_vertices;
    QVector<quint16> m_indices;

    QRhi* m_rhi = nullptr;
    QRhiBuffer* m_vbuf = nullptr;
    QRhiBuffer* m_ibuf = nullptr;
    int m_vbufSize = 0;
    int m_ibufSize = 0;
    bool m_dirty = true;
};

}

#endif // GXMESH_H
