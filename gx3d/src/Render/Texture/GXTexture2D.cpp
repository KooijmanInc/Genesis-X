// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Texture/GXTexture2D.h>

#include <QImageReader>
#include <QDebug>

#include <rhi/qrhi.h>

using namespace gx::gx3d::render;

GXTexture2D::GXTexture2D(QObject *parent)
    : GXTexture{parent}
{
    // m_image = GXTexture::makeFallback();
    // m_size = m_image.size();
    // m_dirty = true;
}

void GXTexture2D::setSource(const QUrl &url)
{
    const bool firstInit = m_image.isNull();
    if (!firstInit && m_source == url) return;

    m_source = url;

    m_hasCpuImage = loadFromSource();

    emit sourceChanged();
    markDirty();
}

void GXTexture2D::setFlipVertical(bool on)
{
    if (m_flipVertical == on) return;

    m_flipVertical = on;

    emit flipVerticalChanged();
    markDirty();
}

void GXTexture2D::setImage(const QImage &img)
{
    m_image = img;
    m_hasCpuImage = !m_image.isNull();

    if (m_hasCpuImage) m_size = m_image.size();

    markDirty();
}

void GXTexture2D::setDefaultImage(const QColor &color)
{
    m_image = GXTexture::makeFallback(color);
    setImage(m_image);
    markDirty();
}

void GXTexture2D::ensureTexture(QRhi *rhi, QRhiCommandBuffer *cb)
{
    if (!rhi) return;

    if (!m_hasCpuImage && m_source.isValid() && !m_source.isEmpty()) m_hasCpuImage = loadFromSource();

    if (!m_hasCpuImage || m_image.isNull()) return;

    if (!m_sampler) {
        m_sampler = rhi->newSampler(
            QRhiSampler::Linear,
            QRhiSampler::Linear,
            QRhiSampler::None,
            QRhiSampler::Repeat,
            QRhiSampler::Repeat
        );
        if (!m_sampler->create()) {
            qWarning() << "[GXTexture2D] sampler create failed";
            return;
        }
    }

    const QImage uploadImg = preparedForUpload();

    if (uploadImg.isNull()) return;

    const QSize sz = uploadImg.size();
    if (sz.isEmpty()) return;

    if (!m_texture || m_texture->pixelSize() != sz) {
        if (m_texture) {
            m_texture->destroy();
            delete m_texture;
            m_texture = nullptr;
        }

        m_texture = rhi->newTexture(QRhiTexture::RGBA8, sz, 1);
        if (!m_texture->create()) {
            qWarning() << "GXTexture2D: texture create failed";
            return;
        }
    }

    if (!cb || !m_dirty) return;

    QRhiTextureSubresourceUploadDescription sub;
    sub.setImage(uploadImg);

    QRhiTextureUploadEntry entry(0, 0, sub);
    QRhiTextureUploadDescription desc({ entry });

    QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
    u->uploadTexture(m_texture, desc);
    cb->resourceUpdate(u);
}

bool GXTexture2D::loadFromSource()
{
    if (!m_source.isValid() || m_source.isEmpty()) {
        m_image = GXTexture::makeWhiteFallback();
        m_size = m_image.size();
    } else {
        QString path;

        if (m_source.isLocalFile()) {
            path = m_source.toLocalFile();
        } else if (m_source.scheme() == "qrc") {
            path = ":" + m_source.path();
        } else if (m_source.scheme().isEmpty() && m_source.toString().startsWith(":/")) {
            path = m_source.toString();
        } else {
            qWarning() << "[GXTexture2D] unsupported source scheme:" << m_source;
            m_image = makeWhiteFallback();
            return false;
        }

        QImageReader reader(path);
        reader.setAutoTransform(true);

        QImage img = reader.read();
        if (img.isNull()) {
            qWarning() << "[GXTexture2D] failed to read image:" << path << reader.errorString();
            m_image = GXTexture::makeWhiteFallback();
            return false;
        }

        m_image = img;
        m_size = m_image.size();
    }

    return true;
}

QImage GXTexture2D::preparedForUpload() const
{
    if (m_image.isNull()) return GXTexture::makeWhiteFallback();

    QImage img;
    if (m_image.format() != QImage::Format_RGBA8888) {
        img = m_image.convertToFormat(QImage::Format_RGBA8888);
    } else {
        img = m_image;
    }

    if (m_flipVertical) img = img.flipped(Qt::Vertical);

    return img;
}
