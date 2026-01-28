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
    explicit GXTexture(QObject* parent = nullptr);
    ~GXTexture() override;

    QRhiTexture* rhiTexture() const { return m_texture; }
    QRhiSampler* rhiSampler() const { return m_sampler; }

    QSize size() const { return m_size; }
    bool isSrgb() const { return m_isSrgb; }

    void ensureRhi(QRhi* rhi, QRhiCommandBuffer* cb);
    void releaseRhi();

    static QImage makeFallback(const QColor& color);
    static QImage makeWhiteFallback();

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

    QSize m_size { 1, 1 };
    bool m_isSrgb = true;
    bool m_dirty = true;
};

}

#endif // GXTEXTURE_H
