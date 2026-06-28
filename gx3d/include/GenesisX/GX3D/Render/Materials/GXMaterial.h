// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXMATERIAL_H
#define GXMATERIAL_H

#include <QObject>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>
#include <GenesisX/GX3D/Render/Materials/GXRenderState.h>
#include <GenesisX/GX3D/Render/Utils/GXShaderUtils.h>

namespace gx::gx3d::render {

class GENESISX_GX3D_EXPORT GXMaterial : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool doubleSided READ doubleSided WRITE setDoubleSided NOTIFY renderStateChanged)
    Q_PROPERTY(bool depthTest READ depthTest WRITE setDepthTest NOTIFY renderStateChanged)
    Q_PROPERTY(bool depthWrite READ depthWrite WRITE setDepthWrite NOTIFY renderStateChanged)
    Q_PROPERTY(float alphaCutoff READ alphaCutoff WRITE setAlphaCutoff NOTIFY alphaCutoffChanged)
    Q_PROPERTY(AlphaMode alphaMode READ alphaMode WRITE setAlphaMode NOTIFY alphaModeChanged)
    Q_PROPERTY(float colorCorrection READ colorCorrection WRITE setColorCorrection NOTIFY colorCorrectionChanged)

public:
    enum CullMode {
        CullNone,
        CullBack,
        CullFront
    };
    Q_ENUM(CullMode)

    enum FrontFace {
        FrontCCW,
        FrontCW
    };
    Q_ENUM(FrontFace)

    enum AlphaMode {
        Default,
        Opaque,
        Mask,
        Blend
    };
    Q_ENUM(AlphaMode)

    enum SpecularChannel {
        R,
        G,
        B,
        A
    };
    Q_ENUM(SpecularChannel)

    enum ShadingVariant {
        Lit,
        UiSafe
    };

    explicit GXMaterial(QObject* parent = nullptr);

    virtual QShader vertexShader() const = 0;
    virtual QShader fragmentShader() const = 0;
    virtual void applyTo(QRhiGraphicsPipeline* ps) const = 0;

    virtual int vsBinding() const = 0;
    virtual int fsBinding() const = 0;
    virtual int vsUboSize() const = 0;
    virtual int fsUboSize() const = 0;

    virtual void fillVS(void* dst, const QMatrix4x4& mvp, const QMatrix4x4& model) const = 0;
    virtual void fillFS(void* dst) const = 0;

    const GXRenderState& renderState() const { return m_state; }
    void setRenderState(const GXRenderState& s);

    bool doubleSided() const { return m_state.cullMode == QRhiGraphicsPipeline::None; }
    void setDoubleSided(bool on);

    bool depthTest() const { return m_state.depthTest; }
    void setDepthTest(bool on);

    bool depthWrite() const { return m_state.depthWrite; }
    void setDepthWrite(bool on);

    float alphaCutoff() const { return m_alphaCutoff; }
    void setAlphaCutoff(float a);

    CullMode cullMode() const;
    void setCullMode(CullMode m);

    FrontFace frontFace() const;
    void setFrontFace(FrontFace f);

    AlphaMode alphaMode() const { return m_alphaMode; }
    void setAlphaMode(AlphaMode a);

    float colorCorrection() const { return m_colorCorrection; }
    void setColorCorrection(float a);

    bool isDirty() const { return m_dirty; }
    bool consumeDirty();

    QRhiTexture* baseColorTex() const { return m_baseColorTex; }
    QRhiSampler* baseColorSampler() const { return m_baseColorSampler; }
    QRhiTexture* normalTex() const { return m_normalTex; }
    QRhiSampler* normalSampler() const { return m_normalSampler; }

    void ensureRhi(QRhi* rhi, QRhiCommandBuffer* cb);

    static QColor gxParseColor(const QString& s);

signals:
    void renderStateChanged();
    void materialChanged();
    void alphaModeChanged();
    void alphaCutoffChanged();
    void colorCorrectionChanged();

protected:
    GXShaderUtils m_shaderUtils;
    void markDirty();

    virtual void ensureBaseColorResources(QRhi* rhi, QRhiCommandBuffer* cb) = 0;
    virtual void ensureNormalMapResources(QRhi* rhi, QRhiCommandBuffer* cb) = 0;
    virtual void destroyRhiResources();

    virtual quint32 variantKey() const { return 0; }

    QRhi* m_rhi = nullptr;
    QRhiTexture* m_baseColorTex = nullptr;
    QRhiSampler* m_baseColorSampler = nullptr;
    QRhiTexture* m_normalTex = nullptr;
    QRhiSampler* m_normalSampler = nullptr;

    float m_colorCorrection = 0;
    QVector3D m_gammaVec = {0.45454545,0.45454545,0.45454545};

private:
    GXRenderState m_state;
    AlphaMode m_alphaMode = Opaque;
    float m_alphaCutoff = 0.5f;

    bool m_dirty = true;

};

}

#endif // GXMATERIAL_H
