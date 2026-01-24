// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Nodes/GXCubeNode.h>
#include <GenesisX/GX3D/Render/Materials/GXMaterial.h>

#include <QFile>
#include <QTimer>

using namespace gx::gx3d::render;

struct Vertex {
    float px, py, pz;
    float nx, ny, nz;
};

static const Vertex kCubeVerts[24] = {
    // +Z (front)
    { -0.5f,-0.5f, 0.5f,   0,0,1 },
    {  0.5f,-0.5f, 0.5f,   0,0,1 },
    {  0.5f, 0.5f, 0.5f,   0,0,1 },
    { -0.5f, 0.5f, 0.5f,   0,0,1 },

    // -Z (back)
    {  0.5f,-0.5f,-0.5f,   0,0,-1 },
    { -0.5f,-0.5f,-0.5f,   0,0,-1 },
    { -0.5f, 0.5f,-0.5f,   0,0,-1 },
    {  0.5f, 0.5f,-0.5f,   0,0,-1 },

    // -X (left)
    { -0.5f,-0.5f,-0.5f,  -1,0,0 },
    { -0.5f,-0.5f, 0.5f,  -1,0,0 },
    { -0.5f, 0.5f, 0.5f,  -1,0,0 },
    { -0.5f, 0.5f,-0.5f,  -1,0,0 },

    // +X (right)
    {  0.5f,-0.5f, 0.5f,   1,0,0 },
    {  0.5f,-0.5f,-0.5f,   1,0,0 },
    {  0.5f, 0.5f,-0.5f,   1,0,0 },
    {  0.5f, 0.5f, 0.5f,   1,0,0 },

    // +Y (top)
    { -0.5f, 0.5f, 0.5f,   0,1,0 },
    {  0.5f, 0.5f, 0.5f,   0,1,0 },
    {  0.5f, 0.5f,-0.5f,   0,1,0 },
    { -0.5f, 0.5f,-0.5f,   0,1,0 },

    // -Y (bottom)
    { -0.5f,-0.5f,-0.5f,   0,-1,0 },
    {  0.5f,-0.5f,-0.5f,   0,-1,0 },
    {  0.5f,-0.5f, 0.5f,   0,-1,0 },
    { -0.5f,-0.5f, 0.5f,   0,-1,0 },
};

static const quint16 kCubeIndices[36] = {
    // +Z
    0, 1, 2,   0, 2, 3,
    // -Z
    4, 5, 6,   4, 6, 7,
    // -X
    8, 9,10,   8,10,11,
    // +X
    12,13,14,  12,14,15,
    // +Y
    16,17,18,  16,18,19,
    // -Y
    20,21,22,  20,22,23
};

static QShader loadShader(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open shader:" << path;
        return {};
    }
    const QByteArray data = f.readAll();
    QShader s = QShader::fromSerialized(data);

    if (!s.isValid()) qWarning() << "Invalid .qsb shader:" << path;

    return s;
}

GXCubeNode::GXCubeNode(QObject *parent)
    : GXRenderableNode{parent}
{
    m_vs = loadShader(":/gx3d/shaders/cube.vert.qsb");
    m_fs = loadShader(":/gx3d/shaders/cube.frag.qsb");

    m_mesh = new GXMesh();

    QVector<GXMesh::Vertex> verts;
    verts.reserve(24);
    float u = 0.0f, uv = 0.0f;
    for (const auto &v : kCubeVerts)
        verts << GXMesh::Vertex{ v.px,v.py,v.pz, v.nx,v.ny,v.nz, u, uv };

    QVector<quint16> idx;
    idx.reserve(36);
    for (quint16 i : kCubeIndices)
        idx << i;

    m_mesh->setGeometry(verts, idx);
}

GXCubeNode::~GXCubeNode()
{
    destroyRhiResources();
    delete m_mesh;
    m_mesh = nullptr;
}

void GXCubeNode::setColor(const QColor &c)
{
    if (m_color == c) return;
    m_color = c;

    emit colorChanged();
}

void GXCubeNode::setMvp(const QMatrix4x4 &mvp)
{
    m_mvp = mvp;
}

void GXCubeNode::setPosition(const QVector3D &pos)
{
    if (m_position == pos) return;
    m_position = pos;

    emit positionChanged();
}

void GXCubeNode::setRotation(const QQuaternion &rot)
{
    if (m_rotation == rot) return;
    m_rotation = rot;

    emit rotationChanged();
}

void GXCubeNode::setScale(const QVector3D &s)
{
    if (m_scale == s) return;
    m_scale = s;

    emit scaleChanged();
}

void GXCubeNode::ensureResources(QRhi *rhi, QRhiRenderTarget *rt)
{
    GXRenderableNode::ensureResources(rhi, rt);

    if (m_rhi == rhi && m_ps && !m_pipelineDirty) return;

    releaseResources();
    m_rhi = rhi;

    // m_vbuf = rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(kCubeVerts));
    // if (!m_vbuf->create()) qWarning() << "GXCubeNode: vbuf Vertices buffer failed";

    // m_ibuf = rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::IndexBuffer, sizeof(kCubeIndices));
    // if (!m_ibuf->create()) qWarning() << "GXCubeNode: ibuf Indices buffer failed";

    if (m_mesh) m_mesh->ensureResources(rhi);

    const int ubufSize = 160;
    m_ubuf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubufSize);
    if (!m_ubuf->create()) qWarning() << "GXCubeNode: ubuf Uniform buffer failed";

    m_srb = m_rhi->newShaderResourceBindings();
    m_srb->setBindings({
        QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, m_ubuf)
    });
    if (!m_srb->create()) qWarning() << "GXCubeNode: srb Shader Resource Bindings failed";

    m_ps = m_rhi->newGraphicsPipeline();
    m_ps->setShaderStages({
        { QRhiShaderStage::Vertex, m_vs },
        { QRhiShaderStage::Fragment, m_fs }
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
    if (material()) {
        material()->applyTo(m_ps);
    }
    else {
        m_ps->setCullMode(QRhiGraphicsPipeline::Back);
        m_ps->setFrontFace(QRhiGraphicsPipeline::CCW);
        m_ps->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
        m_ps->setDepthTest(true);
        m_ps->setDepthWrite(true);
    }

    // QRhiGraphicsPipeline::TargetBlend blend;
    // blend.enable = false;
    // m_ps->setTargetBlends({ blend });

    if (!m_ps->create()) {
        qWarning() << "GXCubeNode: pipeline create failed";
        return;
    }

    QTimer::singleShot(2000, this, [this] {
        material()->setDoubleSided(true);
    });

    m_indexCount = 36;
    m_uploaded = false;
    m_pipelineDirty = false;
}

void GXCubeNode::recordRender(QRhiCommandBuffer *cb, QRhiRenderTarget *rt)
{
    if (!cb || !rt) return;

    QRhi* rhi = cb->rhi();
    ensureResources(rhi, rt);

    if (m_mesh) m_mesh->uploadIfNeeded(rhi, cb);

    if (!m_ps) return;

    alignas(16) struct U {
        float mvp[16];
        float model[16];
        float color[4];
        float lightDir[4];
    } ub;

    if (m_mvp.isIdentity()) {
        QMatrix4x4 view, proj, model;
        view.lookAt({0,0,5}, {0,0,0}, {0,1,0});
        const float aspect = (rt->pixelSize().height() > 0)
            ? float(rt->pixelSize().width()) / float(rt->pixelSize().height())
            : 1.0f;
        proj.perspective(60.0f, aspect, 0.1f, 1000.0f);
        model.setToIdentity();
        m_mvp = proj * view * model;
    }

    static float a = 0.0f;
    a += 1.0f;

    QMatrix4x4 model;
    model.rotate(a, 0, 1, 0);
    model.rotate(a * 0.7f, 1, 0, 0);

    QMatrix4x4 view, proj;
    view.lookAt({0,0,5}, {0,0,0}, {0,1,0});
    const QSize prs = rt->pixelSize();
    const float aspect = prs.height() > 0 ? float(prs.width()) / float(prs.height()) : 1.0f;
    proj.perspective(60.0f, aspect, 0.1f, 1000.0f);

    // m_mvp = proj * view * model;
    m_mvp = m_viewProj * model;

    QMatrix4x4 correctedMvp = rhi->clipSpaceCorrMatrix() * m_mvp;

    memcpy(ub.mvp, correctedMvp.constData(), 16 * sizeof(float));
    memcpy(ub.model, model.constData(), 16 * sizeof(float));

    ub.color[0] = float(m_color.redF());
    ub.color[1] = float(m_color.greenF());
    ub.color[2] = float(m_color.blueF());
    ub.color[3] = float(m_color.alphaF());

    // light direction (world space), pick something nice
    QVector3D ld = QVector3D(0.4f, 1.0f, 0.2f).normalized();
    ub.lightDir[0] = ld.x();
    ub.lightDir[1] = ld.y();
    ub.lightDir[2] = ld.z();
    ub.lightDir[3] = 0.0f;


    // QMatrix4x4 correctedMvp = rhi->clipSpaceCorrMatrix() * m_mvp;

    // const float *m = correctedMvp.constData();
    // for (int i = 0; i < 16; ++i) ub.mvp[i] = m[i];

    // ub.color[0] = float(m_color.redF());
    // ub.color[1] = float(m_color.greenF());
    // ub.color[2] = float(m_color.blueF());
    // ub.color[3] = float(m_color.alphaF());

    {
        QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
        u->updateDynamicBuffer(m_ubuf, 0, sizeof(U), &ub);
        cb->resourceUpdate(u);
    }

    cb->setGraphicsPipeline(m_ps);
    cb->setShaderResources(m_srb);

    if (!m_mesh || !m_mesh->isReady()) {
        qWarning() << "GXCubeNode: mesh or buffers not ready"
                   << "mesh" << (void*)m_mesh
                   << "vbuf" << (m_mesh ? (void*)m_mesh->vertexBuffer() : nullptr)
                   << "ibuf" << (m_mesh ? (void*)m_mesh->indexBuffer() : nullptr);
        return;
    }

    const int icount = m_mesh->indexCount();
    if (icount <= 0) {
        qWarning() << "GXCubeNode: indexCount is 0";
        return;
    }


    const QRhiCommandBuffer::VertexInput vbufBinding(m_mesh->vertexBuffer(), 0);
    cb->setVertexInput(0, 1, &vbufBinding, m_mesh->indexBuffer(), 0, QRhiCommandBuffer::IndexUInt16);

    const QSize ps = rt->pixelSize();
    cb->setViewport(QRhiViewport(0, 0, float(ps.width()), float(ps.height())));
    cb->setScissor(QRhiScissor(0, 0, ps.width(), ps.height()));

    cb->drawIndexed(m_mesh->indexCount());
}

void GXCubeNode::releaseResources()
{
    destroyRhiResources();
    GXRenderableNode::releaseResources();
}

void GXCubeNode::destroyRhiResources()
{
    if (m_ps) { m_ps->destroy(); delete m_ps; m_ps = nullptr; }
    if (m_srb) { m_srb->destroy(); delete m_srb; m_srb = nullptr; }
    if (m_ubuf) { m_ubuf->destroy(); delete m_ubuf; m_ubuf = nullptr; }
    m_pipelineDirty = true;
}
