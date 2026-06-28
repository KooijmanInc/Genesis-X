// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXTEXTURE_H
#define GXTEXTURE_H

#include <QObject>
#include <QSize>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

class QRhi;
class QRhiTexture;
class QRhiSampler;
class QRhiCommandBuffer;

namespace gx::gx3d::render {

class GENESISX_GX3D_EXPORT GXTexture : public QObject
{
    Q_OBJECT

public:
    enum class ColorSpace {
        Linear,
        SRGB
    };
    Q_ENUM(ColorSpace)

    enum class TilingMode {
        Repeat = 0,
        ClampToEdge = 1,
        MirroredRepeat = 2
    };
    Q_ENUM(TilingMode)

    enum class MagFilter {
        None = 0,
        Linear = 1,
        Nearest = 2
    };
    Q_ENUM(MagFilter)

    enum class MinFilter {
        None = 0,
        Linear = 1,
        Nearest = 2
    };
    Q_ENUM(MinFilter)

    enum class MipFilter {
        None = 0,
        Linear = 1,
        Nearest = 2
    };
    Q_ENUM(MipFilter)

    explicit GXTexture(QObject* parent = nullptr);
    ~GXTexture() override;

    QRhiTexture* rhiTexture() const { return m_texture; }
    QRhiSampler* rhiSampler() const { return m_sampler; }

    QSize size() const { return m_size; }
    bool isSrgb() const { return m_isSrgb; }

    ColorSpace colorSpace() const { return m_colorSpace; }
    void setColorSpace(ColorSpace cs) { m_colorSpace = cs; }

    void ensureRhi(QRhi* rhi, QRhiCommandBuffer* cb);
    void releaseRhi();

    static QImage makeFallback(const QColor& color);
    static QImage makeWhiteFallback();
    static QImage makeNormalFallback();

    bool isNormalMap() const { return m_isNormalMap; }
    void setIsNormalMap(bool on)
    {
        if (m_isNormalMap == on) return;
        m_isNormalMap = on;
    }

signals:
    void textureChanged();

protected:
    virtual void ensureTexture(QRhi* rhi, QRhiCommandBuffer* cb) = 0;
    void destroyTexture();

    void markDirty();

protected:
    QRhi* m_rhi = nullptr;
    QRhiTexture* m_texture = nullptr;
    QRhiSampler* m_sampler = nullptr;

    ColorSpace m_colorSpace = ColorSpace::Linear;
    QSize m_size { 1, 1 };
    bool m_isSrgb = true;
    bool m_dirty = true;

    bool m_isNormalMap = false;
};

}

#endif // GXTEXTURE_H
