// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Resources/GXMesh.h>
#include <GenesisX/GX3D/Render/Utils/GXMeshData.h>

#include <rhi/qrhi.h>
#include <QtGlobal>

using namespace gx::gx3d::render;

GXMesh::~GXMesh()
{
    destroyRhiResources();
}

void GXMesh::setGeometry(const QVector<Vertex> &v, const QVector<quint16> &i)
{
    m_vertices = v;
    m_indices16 = i;
    m_indices32.clear();
    m_indexType = IndexUInt16;
    m_dirty = true;
}

void GXMesh::setGeometry(const QVector<Vertex> &v, const QVector<quint32> &i)
{
    m_vertices = v;
    m_indices32 = i;
    m_indices16.clear();
    m_indexType = IndexUInt32;
    m_dirty = true;
}

void GXMesh::setCpuData(const GXMeshData &data)
{
    m_cpu = data;
    m_uploadDirty = true;
    m_dirty = true;
}

void GXMesh::ensureResources(QRhi *rhi)
{
    if (!rhi) return;

    syncFromCpuIfNeeded();

    // qDebug() << "[GXMesh] (ensureResources) synced"
    //          << "verts" << m_vertices.size()
    //          << "idx" << indexCount()
    //          << "subs" << m_subMeshes.size()
    //          << "indexType" << (m_indexType == IndexUInt32 ? "u32" : "u16");

    const int vsize = m_vertices.size() * int(sizeof(Vertex));
    const int isize = (m_indexType == IndexUInt32)
        ?  (m_indices32.size() * int(sizeof(quint32)))
        :  (m_indices16.size() * int(sizeof(quint16)));

    if (vsize <= 0 || isize <= 0) {
        qWarning() << "GXMesh::ensureResources called with empty geometry";
        return;
    }

    const bool needRecreate =
        (m_rhi != rhi) ||
        (!m_vbuf || !m_ibuf) ||
        (m_vbufSize != vsize) ||
        (m_ibufSize != isize);

    // Recreate when QRhi backend/device changes
    if (!needRecreate) return;

    destroyRhiResources();
    m_rhi = rhi;

    m_vbuf = rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, vsize);
    if (!m_vbuf->create()) {
        qWarning() << "GXMesh: vertex buffer create failed";
        delete m_vbuf;
        m_vbuf = nullptr;
        return;
    }

    m_ibuf = rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::IndexBuffer, isize);
    if (!m_ibuf->create()) {
        qWarning() << "GXMesh: index buffer create failed";
        delete m_ibuf;
        m_ibuf = nullptr;
        return;
    }

    m_vbufSize = vsize;
    m_ibufSize = isize;
    m_dirty = true;
}

void GXMesh::uploadIfNeeded(QRhi *rhi, QRhiCommandBuffer *cb)
{
    if (!rhi || !cb || !m_vbuf || !m_ibuf)
        return;

    syncFromCpuIfNeeded();

    // qDebug() << "[GXMesh] (uploadIfNeeded) synced"
    //          << "verts" << m_vertices.size()
    //          << "idx" << indexCount()
    //          << "subs" << m_subMeshes.size()
    //          << "indexType" << (m_indexType == IndexUInt32 ? "u32" : "u16");


    if (!m_dirty)
        return;

    QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();

    // Upload raw bytes from QVector storage
    u->uploadStaticBuffer(m_vbuf, m_vertices.constData());

    if (m_indexType == IndexUInt32) {
        u->uploadStaticBuffer(m_ibuf, m_indices32.constData());
    } else {
        u->uploadStaticBuffer(m_ibuf, m_indices16.constData());
    }

    cb->resourceUpdate(u);
    m_dirty = false;
}

void GXMesh::destroyRhiResources()
{
    if (m_ibuf) { m_ibuf->destroy(); delete m_ibuf; m_ibuf = nullptr; }
    if (m_vbuf) { m_vbuf->destroy(); delete m_vbuf; m_vbuf = nullptr; }
    m_rhi = nullptr;
    m_vbufSize = 0;
    m_ibufSize = 0;
    m_dirty = true;
}

void GXMesh::syncFromCpuIfNeeded()
{
    if (!m_uploadDirty) return;

    // Convert GXMeshData -> GXMesh runtime arrays
    // 1) vertices
    m_vertices.clear();
    m_vertices.resize(m_cpu.vertices.size());

    for (int i = 0; i < m_cpu.vertices.size(); ++i) {
        const auto &v = m_cpu.vertices[i];
        auto &o = m_vertices[i];

        o.px = float(v.position.x());
        o.py = float(v.position.y());
        o.pz = float(v.position.z());

        o.nx = float(v.normal.x());
        o.ny = float(v.normal.y());
        o.nz = float(v.normal.z());

        o.u  = float(v.uv0.x());
        o.v  = float(v.uv0.y());
    }

    // 2) indices (choose 16 vs 32)
    const int vCount = m_vertices.size();
    bool canUseU16 = (vCount <= 65535);

    if (canUseU16) {
        // also ensure indices fit
        for (quint32 idx : m_cpu.indices) {
            if (idx > 65535u) { canUseU16 = false; break; }
        }
    }

    if (canUseU16) {
        m_indices16.clear();
        m_indices16.reserve(m_cpu.indices.size());
        for (quint32 idx : m_cpu.indices)
            m_indices16.push_back(quint16(idx));
        m_indices32.clear();
        m_indexType = IndexUInt16;
    } else {
        m_indices32 = m_cpu.indices;
        m_indices16.clear();
        m_indexType = IndexUInt32;
    }

    // 3) submeshes: copy into GXSubMesh list (your GXSubMesh class/struct)
    m_subMeshes.clear();
    m_subMeshes.reserve(m_cpu.subMeshes.size());
    for (const auto &sm : m_cpu.subMeshes) {
        GXSubMesh out;
        out.indexOffset    = sm.firstIndex;
        out.indexCount     = sm.indexCount;
        out.materialSlot   = int(sm.materialIndex);
        out.baseVertex     = 0;
        m_subMeshes.push_back(out);
    }

    // Mark geometry dirty (buffers need rebuild/upload)
    m_dirty = true;
    m_uploadDirty = false;
}
