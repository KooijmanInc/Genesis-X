// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Nodes/GXModelNode.h>
#include <GenesisX/GX3D/Render/Materials/GXDefaultLitMaterial.h>
#include <GenesisX/GX3D/Render/Materials/GXPrincipledMaterial.h>
#include <GenesisX/GX3D/Render/Materials/GXMaterial.h>
#include <GenesisX/GX3D/Render/Meshes/GXConeMesh.h>
#include <GenesisX/GX3D/Render/Meshes/GXCylinderMesh.h>
#include <GenesisX/GX3D/Render/Meshes/GXSphereMesh.h>
#include <GenesisX/GX3D/Render/Meshes/GXTorusMesh.h>
#include <GenesisX/GX3D/Render/Utils/GXGltfLoader.h>

#include <QFile>

using namespace gx::gx3d::render;

static void ensureUbo(QRhi* rhi, QRhiBuffer*& buf, int size)
{
    if (!rhi || size <= 0) {
        qWarning() << "GXModel: ensureUbo invalid args" << "rhi=" << (void*)rhi << "size=" << size;
        return;
    }

    const quint32 usize = quint32(size);

    if (buf && buf->size() == usize) return;

    if (buf) {
        buf->destroy();
        delete buf;
        buf = nullptr;
    }

    buf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, usize);
    if (!buf || !buf->create()) {
        qWarning() << "GXModel: ubuf Uniform buffer failed";
        delete buf;
        buf = nullptr;
    }
}

// static void uploadUbo(QRhi* rhi, QRhiBuffer* buf, const QByteArray& data)
// {
//     if (!buf || data.isEmpty()) return;

//     QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
//     u->updateDynamicBuffer(buf, 0, data.size(), data.constData());
//     rhi->submitResourceUpdates(u);
// }

GXModel::GXModel(QObject *parent)
    : GXRenderableNode{parent}
{
    if (!m_material) setMaterial(new GXDefaultLitMaterial(this));
}

GXModel::~GXModel()
{
    destroyRhiResources();
    delete m_mesh;
    m_mesh = nullptr;
}

void GXModel::setMesh(GXMesh *mesh)
{
    if (m_mesh == mesh) return;
    delete m_mesh;
    m_mesh = mesh;
    m_primitive = None;

    emit meshChanged();
    emit primitiveChanged();
}

void GXModel::setSource(QString src)
{
    if (m_source == src) return;
    m_source = src;
    delete m_mesh;
    m_mesh = nullptr;
    if (src == "#Cone") {
        m_mesh = GXConeMesh::create();
    } else if (src == "#Cube") {
        m_mesh = GXCubeMesh::create();
    } else if (src == "#Cylinder") {
        m_mesh = GXCylinderMesh::create();
    } else if (src == "#Plane") {
        m_mesh = GXPlaneMesh::create();
    } else if (src == "#Sphere") {
        m_mesh = GXSphereMesh::create();
    } else if (src == "#Torus") {
        m_mesh = GXTorusMesh::create();
    } else if (src.endsWith(".glb")) {
        m_mesh = GXGltfLoader::loadMesh(QUrl(src));
    }

    emit sourceChanged();
}

void GXModel::setPrimitive(Primitive p)
{
    if (m_primitive == p) return;
    m_primitive = p;

    // Replace mesh with a built-in one
    // (for now: own it; later: cache/share)
    delete m_mesh;
    m_mesh = nullptr;

    switch (p) {
    case Cube:  m_mesh = GXCubeMesh::create(); break;
    // case Plane: m_mesh = GXPlaneMesh::create(); break;
    // Sphere/Torus later...
    default: break;
    }

    emit primitiveChanged();
    emit meshChanged();
}

void GXModel::ensureResources(QRhi *rhi, QRhiRenderTarget *rt)
{
    if (m_pendingRelease) {
        releaseResources();
        m_pendingRelease = false;
    }
    const bool haveLightingNow = (m_frameLightingUbo != nullptr);
    if (m_boundHadLighting != haveLightingNow || m_boundLightingUbo != m_frameLightingUbo) {
        m_pipelineDirty = true;
    }

    GXRenderableNode::ensureResources(rhi, rt);

    if (m_rhi == rhi && m_ps && m_srb && m_vsUbuf && m_fsUbuf && !m_pipelineDirty && m_boundLightingUbo == m_frameLightingUbo) return;

    destroyPipelineResources();
    m_rhi = rhi;

    if (m_mesh) m_mesh->ensureResources(rhi);

    GXMaterial* mat = material();
    if (!mat) {
        qWarning() << "GXModel: Unable to set vsUbuf and fsUbuf";
        return;
    }

    const int vsSize = mat->vsUboSize();
    const int fsSize = mat->fsUboSize();

    ensureUbo(rhi, m_vsUbuf, vsSize);
    ensureUbo(rhi, m_fsUbuf, fsSize);

    m_srb = m_rhi->newShaderResourceBindings();
    QVector<QRhiShaderResourceBinding> bindings;
    bindings.reserve(3);

    bindings.append(QRhiShaderResourceBinding::uniformBuffer(mat->vsBinding(), QRhiShaderResourceBinding::VertexStage, m_vsUbuf));
    bindings.append(QRhiShaderResourceBinding::uniformBuffer(mat->fsBinding(), QRhiShaderResourceBinding::FragmentStage, m_fsUbuf));

    QRhiBuffer* lightingUbo = m_frameLightingUbo;
    m_boundLightingUbo = lightingUbo;
    m_boundHadLighting = (lightingUbo != nullptr);

    if (m_frameLightingUbo) {
        bindings.append(QRhiShaderResourceBinding::uniformBuffer(2, QRhiShaderResourceBinding::FragmentStage, m_frameLightingUbo));
    }
    m_srb->setBindings(bindings.cbegin(), bindings.cend());

    if (!m_srb->create()) qWarning() << "GXModel: srb Shader Resource Bindings failed";

    m_ps = m_rhi->newGraphicsPipeline();
    GXMaterial* matPtr = mat;
    m_ps->setShaderStages({
        { QRhiShaderStage::Vertex, matPtr->vertexShader() },
        { QRhiShaderStage::Fragment, matPtr->fragmentShader() }
    });

    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({ QRhiVertexInputBinding(sizeof(GXMesh::Vertex)) });
    inputLayout.setAttributes({
        QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float3, 0),
        QRhiVertexInputAttribute(0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float)),
        QRhiVertexInputAttribute(0, 2, QRhiVertexInputAttribute::Float2, 6 * sizeof(float))
    });

    m_ps->setVertexInputLayout(inputLayout);
    m_ps->setShaderResourceBindings(m_srb);
    m_ps->setTopology(QRhiGraphicsPipeline::Triangles);
    m_ps->setRenderPassDescriptor(rt->renderPassDescriptor());
    m_ps->setSampleCount(rt->sampleCount());
    if (mat) {
        mat->applyTo(m_ps);
    } else {
        m_ps->setCullMode(QRhiGraphicsPipeline::Back);
        m_ps->setFrontFace(QRhiGraphicsPipeline::CCW);
        m_ps->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
        m_ps->setDepthTest(true);
        m_ps->setDepthWrite(true);
    }

    if (!m_ps->create()) {
        qWarning() << "GXModel: pipeline create failed";
        return;
    }

    m_pipelineDirty = false;
}

// #define GX_DUMP_UBO_STATE(tag, rhi, cb, vsBuf, fsBuf, vsBytes, fsBytes) \
// do { \
//         qWarning().noquote() \
//         << tag \
//         << "rhi=" << (void*)(rhi) \
//         << "cb=" << (void*)(cb) \
//         << "vsBuf=" << (void*)(vsBuf) << "created=" << ((vsBuf) ? true : false) \
//         << "vsSize=" << ((vsBuf) ? (vsBuf)->size() : -1) << "vsBytes=" << (vsBytes) \
//         << "fsBuf=" << (void*)(fsBuf) << "created=" << ((fsBuf) ? true : false) \
//         << "fsSize=" << ((fsBuf) ? (fsBuf)->size() : -1) << "fsBytes=" << (fsBytes); \
// } while (0)

void GXModel::recordRender(QRhiCommandBuffer *cb, QRhiRenderTarget *rt)
{
    if (!cb || !rt) return;

    QRhi* rhi = cb->rhi();
    ensureResources(rhi, rt);

    if (m_mesh) m_mesh->uploadIfNeeded(rhi, cb);

    if (!m_ps || !m_srb || !m_vsUbuf || !m_fsUbuf) return;

    GXMaterial* mat = material();
    if (!mat) {
        qWarning() << "GXModel::recordRender: no material";
        return;
    }

    const int vsBytes = mat->vsUboSize();
    const int fsBytes = mat->fsUboSize();

    if (!m_vsUbuf || !m_fsUbuf) {
        qWarning() << "GXModel::recordRender: missing UBO buffers"
                   << "vsUbuf=" << (void*)m_vsUbuf
                   << "fsUbuf=" << (void*)m_fsUbuf
                   << "mat=" << mat->metaObject()->className();
        return;
    }

    const int vsBufSize = int(m_vsUbuf->size());
    const int fsBufSize = int(m_fsUbuf->size());

    if (vsBufSize < vsBytes || fsBufSize < fsBytes) {
        qWarning() << "GXModel::recordRender: UBO size mismatch"
                   << "vsBuf=" << vsBufSize << "need" << vsBytes
                   << "fsBuf=" << fsBufSize << "need" << fsBytes
                   << "mat=" << mat->metaObject()->className();
        return;
    }

    QMatrix4x4 model = worldMatrix();

    QMatrix4x4 mvp = rhi->clipSpaceCorrMatrix() * (m_viewProj * model);

    QByteArray vsData(mat->vsUboSize(), Qt::Uninitialized);
    QByteArray fsData(mat->fsUboSize(), Qt::Uninitialized);

    mat->fillVS(vsData.data(), mvp, model);
    mat->fillFS(fsData.data());

    // GX_DUMP_UBO_STATE("[UBO UPDATE]", rhi, cb, m_vsUbuf, m_fsUbuf, vsBytes, fsBytes);

    {
        QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
        u->updateDynamicBuffer(m_vsUbuf, 0, vsData.size(), vsData.constData());
        u->updateDynamicBuffer(m_fsUbuf, 0, fsData.size(), fsData.constData());
        cb->resourceUpdate(u);
    }

    cb->setGraphicsPipeline(m_ps);
    cb->setShaderResources(m_srb);

    if (!m_mesh || !m_mesh->isReady()) {
        qWarning() << "GXModel: mesh or buffers not ready"
                   << "mesh" << (void*)m_mesh
                   << "vbuf" << (m_mesh ? (void*)m_mesh->vertexBuffer() : nullptr)
                   << "ibuf" << (m_mesh ? (void*)m_mesh->indexBuffer() : nullptr);
        return;
    }

    const int icount = m_mesh->indexCount();
    if (icount <= 0) {
        qWarning() << "GXModel: indexCount is 0";
        return;
    }

    const QRhiCommandBuffer::VertexInput vbufBinding(m_mesh->vertexBuffer(), 0);
    cb->setVertexInput(0, 1, &vbufBinding, m_mesh->indexBuffer(), 0, QRhiCommandBuffer::IndexUInt16);

    const QSize ps = rt->pixelSize();
    cb->setViewport(QRhiViewport(0, 0, float(ps.width()), float(ps.height())));
    cb->setScissor(QRhiScissor(0, 0, ps.width(), ps.height()));

    cb->drawIndexed(m_mesh->indexCount());
}

void GXModel::releaseResources()
{
    destroyRhiResources();
    GXRenderableNode::releaseResources();
}

void GXModel::destroyRhiResources()
{
    if (m_ps) { m_ps->destroy(); delete m_ps; m_ps = nullptr; }
    if (m_srb) { m_srb->destroy(); delete m_srb; m_srb = nullptr; }
    if (m_vsUbuf) { m_vsUbuf->destroy(); delete m_vsUbuf; m_vsUbuf = nullptr; }
    if (m_fsUbuf) { m_fsUbuf->destroy(); delete m_fsUbuf; m_fsUbuf = nullptr; }
    m_pipelineDirty = true;
}

void GXModel::destroyPipelineResources()
{
    if (m_ps) { m_ps->destroy(); delete m_ps; m_ps = nullptr; }
    if (m_srb) { m_srb->destroy(); delete m_srb; m_srb = nullptr; }
    if (m_vsUbuf) { m_vsUbuf->destroy(); delete m_vsUbuf; m_vsUbuf = nullptr; }
    if (m_fsUbuf) { m_fsUbuf->destroy(); delete m_fsUbuf; m_fsUbuf = nullptr; }
}
