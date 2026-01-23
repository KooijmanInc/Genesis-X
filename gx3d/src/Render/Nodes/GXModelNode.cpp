// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Nodes/GXModelNode.h>
#include <GenesisX/GX3D/Render/Materials/GXDefaultLitMaterial.h>
#include <GenesisX/GX3D/Render/Materials/GXMaterial.h>
#include <GenesisX/GX3D/Render/Meshes/GXConeMesh.h>
#include <GenesisX/GX3D/Render/Meshes/GXCylinderMesh.h>
#include <GenesisX/GX3D/Render/Meshes/GXSphereMesh.h>
#include <GenesisX/GX3D/Render/Meshes/GXTorusMesh.h>
#include <GenesisX/GX3D/Render/Utils/GXGltfLoader.h>

#include <QFile>

using namespace gx::gx3d::render;

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
    GXRenderableNode::ensureResources(rhi, rt);

    if (m_rhi == rhi && m_ps && m_srb && m_vsUbuf && m_fsUbuf && !m_pipelineDirty) return;

    destroyPipelineResources();
    m_rhi = rhi;

    if (m_mesh) m_mesh->ensureResources(rhi);

    GXMaterial* m = material();
    if (!m) {
        qWarning() << "GXModel: has no material";
        return;
    }

    // const int vsUbufSize = sizeof(VSUniforms);
    auto* mat = qobject_cast<GXDefaultLitMaterial*>(m);
    if (!mat) {
        qWarning() << "GXModel: Unable to set vsUbuf and fsUbuf";
        return;
    }
    m_vsUbuf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, mat->vsUboSize());
    if (!m_vsUbuf->create()) qWarning() << "GXModel: ubuf Uniform buffer failed";

    qDebug() << "GXModel UBO size =" << m_vsUbuf->size()
             << "Uniforms =" << sizeof(VSUniforms);

    // const int fsUbufSize = sizeof(VSUniforms);
    m_fsUbuf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, mat->fsUboSize());
    if (!m_fsUbuf->create()) qWarning() << "GXModel: ubuf Uniform buffer failed";

    m_srb = m_rhi->newShaderResourceBindings();
    m_srb->setBindings({
        QRhiShaderResourceBinding::uniformBuffer(mat->vsBinding(), QRhiShaderResourceBinding::VertexStage, m_vsUbuf),
        QRhiShaderResourceBinding::uniformBuffer(mat->fsBinding(), QRhiShaderResourceBinding::FragmentStage, m_fsUbuf)
    });
    if (!m_srb->create()) qWarning() << "GXModel: srb Shader Resource Bindings failed";

    m_ps = m_rhi->newGraphicsPipeline();
    m_ps->setShaderStages({
        { QRhiShaderStage::Vertex, m->vertexShader() },
        { QRhiShaderStage::Fragment, m->fragmentShader() }
    });

    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({ QRhiVertexInputBinding(sizeof(GXMesh::Vertex)) });
    inputLayout.setAttributes({
        QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float3, 0),
        QRhiVertexInputAttribute(0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float))
    });

    m_ps->setVertexInputLayout(inputLayout);
    m_ps->setShaderResourceBindings(m_srb);
    m_ps->setTopology(QRhiGraphicsPipeline::Triangles);
    m_ps->setRenderPassDescriptor(rt->renderPassDescriptor());
    m_ps->setSampleCount(rt->sampleCount());
    if (m) {
        m->applyTo(m_ps);
    } else {
        m_ps->setCullMode(QRhiGraphicsPipeline::None);
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

void GXModel::recordRender(QRhiCommandBuffer *cb, QRhiRenderTarget *rt)
{
    if (!cb || !rt) return;

    QRhi* rhi = cb->rhi();
    ensureResources(rhi, rt);

    if (m_mesh) m_mesh->uploadIfNeeded(rhi, cb);

    if (!m_ps) return;

    auto* mat = qobject_cast<GXDefaultLitMaterial*>(material());
    if (!mat) return;

    DefaultLitVSUBO vsu{};
    DefaultLitFSUBO fsu{};

    QMatrix4x4 model = worldMatrix();

    QMatrix4x4 mvp = rhi->clipSpaceCorrMatrix() * (m_viewProj * model);

    mvp = rhi->clipSpaceCorrMatrix() * mvp;

    mat->fillVS(vsu, mvp, model);
    mat->fillFS(fsu, m_pointLight);

    {
        QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
        u->updateDynamicBuffer(m_vsUbuf, 0, sizeof(vsu), &vsu);
        u->updateDynamicBuffer(m_fsUbuf, 0, sizeof(fsu), &fsu);
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
