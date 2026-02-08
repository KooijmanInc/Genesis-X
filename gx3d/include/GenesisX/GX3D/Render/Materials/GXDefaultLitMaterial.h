// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXDEFAULTLITMATERIAL_H
#define GXDEFAULTLITMATERIAL_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>
#include <GenesisX/GX3D/Render/Materials/GXMaterial.h>
#include <GenesisX/GX3D/Render/Materials/GXDefaultLitUniforms.h>

#include <GenesisX/GX3D/Render/Texture/GXTexture.h>
#include <GenesisX/GX3D/Render/Texture/GXTexture2D.h>

#include <QColor>
#include <QMatrix4x4>

namespace gx::gx3d::render {

struct GXPointLightData;

class GENESISX_GX3D_EXPORT GXDefaultLitMaterial : public GXMaterial
{
    Q_OBJECT

    Q_PROPERTY(QColor baseColor READ baseColor WRITE setBaseColor NOTIFY baseColorChanged)
    Q_PROPERTY(GXTexture* baseColorTexture READ baseColorTexture WRITE setBaseColorTexture NOTIFY baseColorTextureChanged)
    Q_PROPERTY(TestMaterial baseColorTestTexture READ baseColorTestTexture WRITE setBaseColorTestTexture NOTIFY baseColorTestTextureChanged)

public:
    enum TestMaterial {
        UvGrid,
        Checker,
        BrushedMetal,
        WoodFloor
    };
    Q_ENUM(TestMaterial)

public:
    explicit GXDefaultLitMaterial(QObject* parent = nullptr);

    QShader vertexShader() const override;
    QShader fragmentShader() const override;
    void applyTo(QRhiGraphicsPipeline* ps) const override;

    // Layout contract (size + binding numbers)
    int vsBinding() const override { return DefaultLit_VS_Binding; }
    int fsBinding() const override { return DefaultLit_FS_Binding; }
    int vsUboSize() const override {return int(sizeof(DefaultLitVSUBO)); }
    int fsUboSize() const override {return int(sizeof(DefaultLitFSUBO)); }

    QColor baseColor() const { return m_baseColor; }
    void setBaseColor(const QColor& c);

    GXTexture* baseColorTexture() const { return m_baseColorTexture; }
    void setBaseColorTexture(GXTexture* tex);

    TestMaterial baseColorTestTexture() const { return m_baseColorTestTexture; }
    void setBaseColorTestTexture(TestMaterial id);

    void fillVS(void* dst, const QMatrix4x4& mvp, const QMatrix4x4& model) const override;
    void fillFS(void* dst) const override;

    void ensureBaseColorResources(QRhi* rhi, QRhiCommandBuffer* cb) override;
    void ensureNormalMapResources(QRhi* rhi, QRhiCommandBuffer* cb) override;

signals:
    void baseColorChanged();
    void baseColorTextureChanged();
    void baseColorTestTextureChanged();

private:
    QShader m_vs, m_fs;
    QColor m_baseColor = Qt::white;
    GXTexture* m_baseColorTexture = nullptr;
    QString m_testMaterial;
    TestMaterial m_baseColorTestTexture;

    QByteArray m_baseColorTexName = "";

    QSize m_baseColorSz;
    QImage m_baseColorImg;

    GXTexture2D* m_solidColorTex = nullptr;
};

}

#endif // GXDEFAULTLITMATERIAL_H
