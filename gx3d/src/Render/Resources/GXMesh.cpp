// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Resources/GXMesh.h>

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

void GXMesh::ensureResources(QRhi *rhi)
{
    if (!rhi) return;

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
