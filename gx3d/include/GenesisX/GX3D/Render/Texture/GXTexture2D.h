// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXTEXTURE2D_H
#define GXTEXTURE2D_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Render/Texture/GXTexture.h>

#include <QImage>
#include <QUrl>

namespace gx::gx3d::render {

class GENESISX_GX3D_EXPORT GXTexture2D : public GXTexture
{
    Q_OBJECT

    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(bool flipVertical READ flipVertical WRITE setFlipVertical NOTIFY flipVerticalChanged)

    Q_PROPERTY(TilingMode wrapU READ wrapU WRITE setWrapU NOTIFY wrapChanged)
    Q_PROPERTY(TilingMode wrapV READ wrapV WRITE setWrapV NOTIFY wrapChanged)

    Q_PROPERTY(MagFilter magFilter READ magFilter WRITE setMagFilter NOTIFY magFilterChanged)
    Q_PROPERTY(MinFilter minFilter READ minFilter WRITE setMinFilter NOTIFY minFilterChanged)
    Q_PROPERTY(MipFilter mipFilter READ mipFilter WRITE setMipFilter NOTIFY mipFilterChanged)

public:
    explicit GXTexture2D(QObject* parent = nullptr);

    QUrl source() const { return m_source; }
    void setSource(const QUrl& url);

    bool flipVertical() const { return m_flipVertical; }
    void setFlipVertical(bool on);

    void setImage(const QImage& img);
    const QImage& image() const { return m_image; }

    TilingMode wrapU() const { return m_wrapU; }
    void setWrapU(TilingMode w);

    TilingMode wrapV() const { return m_wrapV; }
    void setWrapV(TilingMode w);

    MagFilter magFilter() const { return m_magFilter; }
    void setMagFilter(MagFilter mf);

    MinFilter minFilter() const { return m_minFilter; }
    void setMinFilter(MinFilter mf);

    MipFilter mipFilter() const { return m_mipFilter; }
    void setMipFilter(MipFilter mf);

    void setDefaultImage(const QColor& color);
    void setDefaultNormalImage();

signals:
    void sourceChanged();
    void flipVerticalChanged();
    void wrapChanged();
    void magFilterChanged();
    void minFilterChanged();
    void mipFilterChanged();

protected:
    void ensureTexture(QRhi* rhi, QRhiCommandBuffer* cb) override;

private:
    bool loadFromSource();
    QImage preparedForUpload() const;

private:
    QUrl m_source;
    QImage m_image;
    TilingMode m_wrapU = TilingMode::Repeat;
    TilingMode m_wrapV = TilingMode::Repeat;

    MagFilter m_magFilter = MagFilter::Linear;
    MinFilter m_minFilter = MinFilter::Linear;
    MipFilter m_mipFilter = MipFilter::None;

    bool m_flipVertical = true;
    bool m_hasCpuImage = false;
};

}

#endif // GXTEXTURE2D_H
