// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Materials/GXDefaultLitMaterial.h>
#include <GenesisX/GX3D/Render/Lights/GXFrameLighting.h>

#include <GenesisX/GX3D/Render/Texture/GXTexture2D.h>

#include <QImage>
#include <QPainter>
#include <QPen>
#include <QFont>

using namespace gx::gx3d::render;

static QImage gxMakeUvGridImage(int w = 256, int h = 256)
{
    QImage img(w, h, QImage::Format_RGBA8888);
    img.fill(QColor(30, 30, 30, 255));

    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing, false);

    // Checker background
    const int checker = 32;
    for (int y = 0; y < h; y += checker) {
        for (int x = 0; x < w; x += checker) {
            const bool odd = ((x / checker) + (y / checker)) & 1;
            p.fillRect(x, y, checker, checker, odd ? QColor(55,55,55) : QColor(20,20,20));
        }
    }

    // Major/minor grid
    auto drawGrid = [&](int step, const QColor& c, int width) {
        QPen pen(c);
        pen.setWidth(width);
        p.setPen(pen);
        for (int x = 0; x <= w; x += step) p.drawLine(x, 0, x, h);
        for (int y = 0; y <= h; y += step) p.drawLine(0, y, w, y);
    };

    drawGrid(16, QColor(80, 80, 80, 255), 1);   // minor
    drawGrid(64, QColor(140,140,140,255), 2);   // major

    // Axis hints: red = +U (right), green = +V (up in UV space)
    p.setPen(QPen(QColor(200,60,60,255), 4));
    p.drawLine(0, h - 1, w, h - 1);             // bottom edge (U axis)
    p.setPen(QPen(QColor(60,200,60,255), 4));
    p.drawLine(0, h, 0, 0);                      // left edge (V axis)

    // Labels
    p.setPen(QColor(230,230,230,255));
    QFont f = p.font();
    f.setPointSize(18);
    f.setBold(true);
    p.setFont(f);
    p.drawText(QRect(8, 8, w - 16, 30), "UV GRID");
    p.drawText(QRect(w - 80, h - 40, 72, 32), "U+");
    p.drawText(QRect(8, 40, 72, 32), "V+");

    p.end();

    return img;
}

GXDefaultLitMaterial::GXDefaultLitMaterial(QObject *parent)
    : GXMaterial{parent}
{
    // m_baseColorSz.setWidth(256);
    // m_baseColorSz.setHeight(256);

    m_solidColorTex = new GXTexture2D(this);
    m_solidColorTex->setDefaultImage(Qt::white);
    // m_baseColorTexture = m_solidColorTex;
}

QShader GXDefaultLitMaterial::vertexShader() const
{
    return m_shaderUtils.gxLoadShader(":/gx3d/shaders/default_lit.vert.qsb");
}

QShader GXDefaultLitMaterial::fragmentShader() const
{
    return m_shaderUtils.gxLoadShader(":/gx3d/shaders/default_lit.frag.qsb");
}

void GXDefaultLitMaterial::applyTo(QRhiGraphicsPipeline *ps) const
{

    ps->setCullMode(QRhiGraphicsPipeline::Back);
    ps->setFrontFace(QRhiGraphicsPipeline::CCW);
    GXMaterial::applyTo(ps);
    ps->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
    ps->setDepthTest(true);
    ps->setDepthWrite(true);
}

void GXDefaultLitMaterial::setBaseColor(const QColor &c)
{
    if (m_baseColor == c) return;
    m_baseColor = c;

    m_solidColorTex->setDefaultImage(c);

    emit baseColorChanged();
}

void GXDefaultLitMaterial::setBaseColorTexture(GXTexture* tex)
{
    if (m_baseColorTexture == tex) return;
    m_baseColorTexture = tex;

    emit baseColorTextureChanged();
    markDirty();
}

void GXDefaultLitMaterial::setBaseColorTestTexture(TestMaterial id)
{
    switch (id) {
    case UvGrid:
        m_baseColorSz.setWidth(256);
        m_baseColorSz.setHeight(256);
        m_baseColorTexName = "GX3D_BuiltIn_UVGrid";
        m_baseColorImg = gxMakeUvGridImage(m_baseColorSz.width(), m_baseColorSz.height());
        emit baseColorTestTextureChanged();
        markDirty();
        break;
    case Checker:
        m_baseColorImg.load(":/gx3d/assets/checker.jpg");
        m_baseColorTexName = "GX3D_BuiltIn_Checker";
        m_baseColorSz.setWidth(m_baseColorImg.width());
        m_baseColorSz.setHeight(m_baseColorImg.height());
        break;
    case BrushedMetal:
        break;
    case WoodFloor:
        m_baseColorImg.load(":/gx3d/assets/WoodFloor.jpg");
        m_baseColorTexName = "GX3D_BuiltIn_WoodFloor";
        m_baseColorSz.setWidth(m_baseColorImg.width());
        m_baseColorSz.setHeight(m_baseColorImg.height());
        break;
    default:
        qWarning() << "[GXMaterial] no default test material choosen. Use TestMaterial to select one!";
        break;
    }
}

void GXDefaultLitMaterial::fillVS(void* dst, const QMatrix4x4 &mvp, const QMatrix4x4 &model) const
{
    auto& out = *reinterpret_cast<DefaultLitVSUBO*>(dst);
    memcpy(out.mvp, mvp.constData(), 16 * sizeof(float));
    memcpy(out.model, model.constData(), 16 * sizeof(float));
}

void GXDefaultLitMaterial::fillFS(void* dst) const
{
    auto& out = *reinterpret_cast<DefaultLitFSUBO*>(dst);
    out.baseColor[0] = float(m_baseColor.redF());
    out.baseColor[1] = float(m_baseColor.greenF());
    out.baseColor[2] = float(m_baseColor.blueF());
    out.baseColor[3] = float(m_baseColor.alphaF());
}

void GXDefaultLitMaterial::ensureBaseColorResources(QRhi* rhi, QRhiCommandBuffer* cb)
{
    if (!rhi) return;

    if (m_baseColorTexture) {
        m_baseColorTexture->ensureRhi(rhi, cb);
        m_baseColorTex = m_baseColorTexture->rhiTexture();
        m_baseColorSampler = m_baseColorTexture->rhiSampler();
    } else if (m_baseColorTexName != "") {
        if (!m_baseColorSampler) {
            m_baseColorSampler = rhi->newSampler(
                QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                QRhiSampler::Repeat, QRhiSampler::Repeat
                );
            if (!m_baseColorSampler->create()) qWarning() << "GXModelNode: baseColor sampler create failed";
        }

        if (!m_baseColorTex || m_baseColorTex->pixelSize() != m_baseColorSz) {
            if (m_baseColorTex) {
                m_baseColorTex->destroy();
                delete m_baseColorTex;
                m_baseColorTex = nullptr;
            }
            m_baseColorImg = m_baseColorImg.convertToFormat(QImage::Format_RGBA8888);

            m_baseColorTex = rhi->newTexture(QRhiTexture::RGBA8, m_baseColorSz, 1);
            m_baseColorTex->setName(m_baseColorTexName);

            if (!m_baseColorTex->create()) {
                qWarning() << "GXModelNode: baseColor texture create failed";
                return;
            }


            m_baseColorImg = m_baseColorImg.flipped(Qt::Vertical);

            QRhiTextureSubresourceUploadDescription sub;
            sub.setImage(m_baseColorImg);

            QRhiTextureUploadEntry entry(0, 0, sub);

            QRhiTextureUploadDescription desc({ entry });

            QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
            u->uploadTexture(m_baseColorTex, desc);
            cb->resourceUpdate(u);
        }
    } else {
        m_baseColorTexture = m_solidColorTex;
        m_baseColorTexture->ensureRhi(rhi, cb);
        m_baseColorTex = m_baseColorTexture->rhiTexture();
        m_baseColorSampler = m_baseColorTexture->rhiSampler();
    }
}

void GXDefaultLitMaterial::ensureNormalMapResources(QRhi *rhi, QRhiCommandBuffer *cb)
{
    Q_UNUSED(rhi);
    Q_UNUSED(cb);
}
