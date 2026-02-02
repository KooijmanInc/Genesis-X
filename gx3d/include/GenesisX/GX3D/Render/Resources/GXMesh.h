// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXMESH_H
#define GXMESH_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Render/Resources/GXSubMesh.h>
#include <GenesisX/GX3D/Render/Utils/GXMeshData.h>

#include <QVector>
#include <QDebug>

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

    enum IndexType {
        IndexUInt16,
        IndexUInt32
    };

    GXMesh() = default;
    virtual ~GXMesh();

    bool isReady() const {
        if (!m_vbuf || !m_ibuf) return false;
        if (m_vertices.isEmpty()) return false;

        if (m_indexType == IndexUInt32) return !m_indices32.isEmpty();
        return !m_indices16.isEmpty();
    }

    // CPU data
    const QVector<Vertex>& vertices() const { return m_vertices; }

    const QVector<quint16>& indices() const { return m_indices16; }

    int vertexCount() const { return m_vertices.size(); }
    int indexCount() const {
        return (m_indexType == IndexUInt32) ? m_indices32.size()
                                            : m_indices16.size();
    }

    IndexType indexType() const { return m_indexType; }

    void setGeometry(const QVector<Vertex>& v, const QVector<quint16>& i);
    void setGeometry(const QVector<Vertex>& v, const QVector<quint32>& i);

    // GPU resources
    void setCpuData(const GXMeshData& data);
    bool hasCpuData() const { return !m_cpu.vertices.isEmpty(); }

    void ensureResources(QRhi* rhi);
    void uploadIfNeeded(QRhi* rhi, QRhiCommandBuffer* cb);

    QRhiBuffer* vertexBuffer() const { return m_vbuf; }
    QRhiBuffer* indexBuffer() const { return m_ibuf; }

    const QVector<GXSubMesh>& subMeshes() const { return m_subMeshes; }
    void addSubMesh(const GXSubMesh& sm) { return m_subMeshes.append(sm); }

    void destroyRhiResources();

protected:
    QVector<Vertex> m_vertices;
    QVector<quint16> m_indices16;
    QVector<quint32> m_indices32;

    QRhi* m_rhi = nullptr;
    QRhiBuffer* m_vbuf = nullptr;
    QRhiBuffer* m_ibuf = nullptr;
    int m_vbufSize = 0;
    int m_ibufSize = 0;
    bool m_dirty = true;

    IndexType m_indexType = IndexUInt16;

private:
    QVector<GXSubMesh> m_subMeshes;

    GXMeshData m_cpu;
    bool m_uploadDirty = false;
    void syncFromCpuIfNeeded();
};

}

#endif // GXMESH_H
