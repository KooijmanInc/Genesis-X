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

public:
    explicit GXTexture2D(QObject* parent = nullptr);

    QUrl source() const { return m_source; }
    void setSource(const QUrl& url);

    bool flipVertical() const { return m_flipVertical; }
    void setFlipVertical(bool on);

    void setImage(const QImage& img);
    const QImage& image() const { return m_image; }

signals:
    void sourceChanged();
    void flipVerticalChanged();

protected:
    void ensureTexture(QRhi* rhi, QRhiCommandBuffer* cb) override;

private:
    bool loadFromSource();
    QImage preparedForUpload() const;

private:
    QUrl m_source;
    QImage m_image;

    bool m_flipVertical = true;
    bool m_hasCpuImage = false;
};

}

#endif // GXTEXTURE2D_H
