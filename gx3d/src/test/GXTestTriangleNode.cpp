// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "GXTestTriangleNode.h"

#include <QFile>

using namespace gx::test;

static QShader loadShader(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return {};
    return QShader::fromSerialized(f.readAll());
}

GXTestTriangleNode::GXTestTriangleNode()
{
    m_vs = loadShader(":/gx3d/shaders/testtri.vert.qsb");
    m_fs = loadShader(":/gx3d/shaders/testtri.frag.qsb");
}

GXTestTriangleNode::~GXTestTriangleNode()
{
    destroy();
}

void GXTestTriangleNode::destroy()
{
    if (!m_rhi) return;
    delete m_ps;  m_ps = nullptr;
    delete m_srb; m_srb = nullptr;
    delete m_vbuf; m_vbuf = nullptr;
    delete m_ubuf; m_ubuf = nullptr;
    m_rhi = nullptr;
}

void GXTestTriangleNode::ensure(QRhi *rhi, QRhiRenderTarget *rt)
{
    // qWarning() << "GXTestTriangleNode::ensure rhi=" << rhi << "rt=" << rt
               // << "existing m_rhi=" << m_rhi << "m_ps=" << (void*)m_ps;

    if (m_rhi == rhi && m_ps)
        return;

    destroy();
    m_rhi = rhi;

    // 3D triangle in world space
    struct V { float x,y,z; };
    const V verts[3] = {
        {  0.0f,  0.8f, 0.0f },
        { -0.8f, -0.8f, 0.0f },
        {  0.8f, -0.8f, 0.0f }
    };

    m_vbuf = m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(verts));
    // m_vbuf->create();

    // mat4 (64) + vec4 (16)
    m_ubuf = m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 80);
    // m_ubuf->create();

    if (!m_vbuf->create()) qWarning() << "GXTestTriangleNode: vbuf create failed";
    if (!m_ubuf->create()) qWarning() << "GXTestTriangleNode: ubuf create failed";


    m_srb = m_rhi->newShaderResourceBindings();
    m_srb->setBindings({
        QRhiShaderResourceBinding::uniformBuffer(
            0,
            QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage,
            m_ubuf)
    });
    // m_srb->create();

    if (!m_srb->create())  qWarning() << "GXTestTriangleNode: srb create failed";

    m_ps = m_rhi->newGraphicsPipeline();
    m_ps->setShaderStages({
        { QRhiShaderStage::Vertex, m_vs },
        { QRhiShaderStage::Fragment, m_fs }
    });

    if (!m_vs.isValid() || !m_fs.isValid()) {
        qWarning() << "GXTestTriangleNode: shader invalid"
                   << "vs" << m_vs.isValid()
                   << "fs" << m_fs.isValid();
    }

    QRhiVertexInputLayout inputLayout;
    // inputLayout.setBindings({ { sizeof(V) } });
    // inputLayout.setAttributes({
    //     { 0, 0, QRhiVertexInputAttribute::Float3, 0 }
    // });

    m_ps->setVertexInputLayout(inputLayout);
    m_ps->setShaderResourceBindings(m_srb);
    m_ps->setTopology(QRhiGraphicsPipeline::Triangles);
    m_ps->setRenderPassDescriptor(rt->renderPassDescriptor());
    m_ps->setSampleCount(rt->sampleCount());
    qWarning() << "GXTestTriangleNode::ensure (called) sampleCount=" << rt->sampleCount();
    m_ps->setCullMode(QRhiGraphicsPipeline::None);
    QRhiGraphicsPipeline::TargetBlend blend;
    blend.enable = false;
    m_ps->setTargetBlends({ blend });

    m_ps->setDepthTest(false);
    m_ps->setDepthWrite(false);
    // m_ps->create();

    if (!m_ps->create()) {
        qWarning() << "GXTestTriangleNode: pipeline create FAILED";
        return;
    }


    // Upload verts once
    QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
    u->uploadStaticBuffer(m_vbuf, verts);

    // We cannot call cb->resourceUpdate here (no cb yet). We’ll do it in render().
    // Store u? easiest: upload again in first render frame:
    // We'll upload in render() the first time after pipeline exists.
    // For simplicity, we upload in render() with a local batch.
}

void GXTestTriangleNode::render(const RenderState *state)
{
    Q_UNUSED(state)
    static int c = 0;
    if ((c++ % 120) == 0)
        qWarning() << "GXTestTriangleNode::render() running";

    QRhiCommandBuffer *cb = commandBuffer();
    QRhiRenderTarget *rt = renderTarget();
    if (!cb || !rt)
        return;

    qDebug() << "after QRhiRenderTarget";

    QRhi *rhi = cb->rhi();
    ensure(rhi, rt);

    // Upload vertex buffer if needed (safe to do repeatedly for now; we can optimize later)
    {
        struct V { float x,y,z; };
        const V verts[3] = {
            {  0.0f,  0.8f, 0.0f },
            { -0.8f, -0.8f, 0.0f },
            {  0.8f, -0.8f, 0.0f }
        };
        QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
        u->uploadStaticBuffer(m_vbuf, verts);
        cb->resourceUpdate(u);
    }

    // Update uniforms
    alignas(16) struct U {
        float mvp[16];
        float color[4];
    } ub;

    QMatrix4x4 correctedMvp;
    correctedMvp.setToIdentity();
    correctedMvp = rhi->clipSpaceCorrMatrix() * correctedMvp;

    const float *m = correctedMvp.constData();
    for (int i=0;i<16;++i) ub.mvp[i] = m[i];

    ub.color[0] = float(m_color.redF());
    ub.color[1] = float(m_color.greenF());
    ub.color[2] = float(m_color.blueF());
    ub.color[3] = float(m_color.alphaF());

    {
        QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
        u->updateDynamicBuffer(m_ubuf, 0, sizeof(U), &ub);
        cb->resourceUpdate(u);
    }

    cb->setGraphicsPipeline(m_ps);
    cb->setShaderResources(m_srb);

    const QRhiCommandBuffer::VertexInput vb(m_vbuf, 0);
    // cb->setVertexInput(0, 1, &vb);
    cb->setVertexInput(0, 0, nullptr);

    const QSize ps = renderTarget()->pixelSize();
    cb->setViewport(QRhiViewport(0, 0, float(ps.width()), float(ps.height())));
    cb->setScissor(QRhiScissor(0, 0, ps.width(), ps.height()));

    qWarning() << "draw with viewport" << ps << "color" << m_color;
    qWarning() << "RHI backend:" << int(cb->rhi()->backend());

    cb->draw(3);
}
