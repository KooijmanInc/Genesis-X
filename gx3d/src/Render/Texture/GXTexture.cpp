// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Texture/GXTexture.h>

#include <rhi/qrhi.h>

using namespace gx::gx3d::render;

GXTexture::GXTexture(QObject *parent)
    : QObject{parent}
{
}

GXTexture::~GXTexture()
{
    releaseRhi();
}

void GXTexture::ensureRhi(QRhi *rhi, QRhiCommandBuffer *cb)
{
    if (!rhi) return;

    if (m_rhi != rhi) {
        releaseRhi();
        m_rhi = rhi;
        m_dirty = true;
    }

    if (!m_dirty && m_texture && m_sampler) return;

    ensureTexture(rhi, cb);

    if (m_texture && m_sampler) m_dirty = false;
}

void GXTexture::releaseRhi()
{
    destroyTexture();
    m_rhi = nullptr;

    m_dirty = true;
}

QImage GXTexture::makeFallback(const QColor& color)
{
    QImage img(256, 256, QImage::Format_RGBA8888);
    img.fill(color);

    return img;
}

QImage GXTexture::makeWhiteFallback()
{
    QImage img(256, 256, QImage::Format_RGBA8888);
    img.fill(Qt::white);

    return img;
}

void GXTexture::destroyTexture()
{
    if (m_texture) {
        m_texture->destroy();
        delete m_texture;
        m_texture = nullptr;
    }

    if (m_sampler) {
        m_sampler->destroy();
        delete m_sampler;
        m_sampler = nullptr;
    }
}

void GXTexture::markDirty()
{
    m_dirty = true;

    emit textureChanged();
}
