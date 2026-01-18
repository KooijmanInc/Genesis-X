// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "GXClearNode.h"

#include <QFile>

using namespace gx::render;

static QShader loadShader(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};

    return QShader::fromSerialized(f.readAll());
}

GXClearNode::GXClearNode()
{
    m_vs = loadShader(QStringLiteral(":/gx3d/shaders/solidcolor.vert.qsb"));
    m_fs = loadShader(QStringLiteral(":/gx3d/shaders/solidcolor.frag.qsb"));
}

GXClearNode::~GXClearNode()
{
    destroyRhiResources();
}

void GXClearNode::prepare()
{
}

void GXClearNode::render(const RenderState *state)
{
    QRhiCommandBuffer* cb = commandBuffer();
    QRhiRenderTarget* rt = renderTarget();
    if (!cb || !rt) return;

    QRhi* rhi = cb->rhi();
    ensureRhiResources(rhi);

    if (!m_ps->renderPassDescriptor()) {
        m_ps->setRenderPassDescriptor(rt->renderPassDescriptor());
        m_ps->create();

        QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
        struct V { float x, y; };
        const V verts[4] = { {0,0}, {1,0}, {0,1}, {1,1} };
        u->uploadStaticBuffer(m_vbuf, verts);
        cb->resourceUpdate(u);
    }

    QMatrix4x4 model;
    model.translate(float(m_rect.x()), float(m_rect.y()), 0.0f);
    model.scale(float(m_rect.width()), float(m_rect.height()), 1.0f);

    QMatrix4x4 mvp = (*state->projectionMatrix()) * (*matrix()) * model;

    const QVector4D c(
        float(m_color.redF()),
        float(m_color.greenF()),
        float(m_color.blueF()),
        float(m_color.alphaF())
    );

    alignas(16) struct U {
        float mvp[16];
        float color[4];
    } ubufData;

    const float *m = mvp.constData();
    for (int i = 0; i < 16; ++i) ubufData.mvp[i] = m[i];
    ubufData.color[0] = c.x();
    ubufData.color[1] = c.y();
    ubufData.color[2] = c.z();
    ubufData.color[3] = c.w();

    QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
    u->updateDynamicBuffer(m_ubuf, 0, sizeof(U), &ubufData);
    cb->resourceUpdate(u);

    cb->setGraphicsPipeline(m_ps);
    cb->setShaderResources(m_srb);

    if (state->scissorEnabled()) {
        const QRect r = state->scissorRect();
        cb->setScissor(QRhiScissor(r.x(), r.y(), r.width(), r.height()));
    }

    QRhiCommandBuffer::VertexInput vb(m_vbuf, 0);
    cb->setVertexInput(0, 1, &vb);

    cb->draw(4);
}

void GXClearNode::releaseResources()
{
    destroyRhiResources();
}

void GXClearNode::ensureRhiResources(QRhi *rhi)
{
    if (m_rhi == rhi && m_ps) return;

    destroyRhiResources();
    m_rhi = rhi;

    struct V { float x, y; };
    const V verts[4] = { {0,0}, {1,0}, {0,1}, {1,1} };

    m_vbuf = m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(verts));
    m_vbuf->create();

    m_ubuf = m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 80);
    m_ubuf->create();

    m_srb = m_rhi->newShaderResourceBindings();
    m_srb->setBindings({
        QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, m_ubuf)
    });
    m_srb->create();

    m_ps = m_rhi->newGraphicsPipeline();
    m_ps->setTopology(QRhiGraphicsPipeline::TriangleStrip);
    m_ps->setShaderStages({
        { QRhiShaderStage::Vertex, m_vs },
        { QRhiShaderStage::Fragment, m_fs }
    });

    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({ { sizeof(V) } });
    inputLayout.setAttributes({
        { 0, 0, QRhiVertexInputAttribute::Float2, 0 }
    });
    m_ps->setVertexInputLayout(inputLayout);
    m_ps->setShaderResourceBindings(m_srb);
}

void GXClearNode::destroyRhiResources()
{
    if (!m_rhi) return;

    delete m_ps; m_ps = nullptr;
    delete m_srb; m_srb = nullptr;
    delete m_vbuf; m_vbuf = nullptr;
    delete m_ubuf; m_ubuf = nullptr;

    m_rhi = nullptr;
}
