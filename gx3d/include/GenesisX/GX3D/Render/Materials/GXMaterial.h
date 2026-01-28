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

    CullMode cullMode() const;
    void setCullMode(CullMode m);

    FrontFace frontFace() const;
    void setFrontFace(FrontFace f);

    bool isDirty() const { return m_dirty; }
    bool consumeDirty();

    QRhiTexture* baseColorTex() const { return m_baseColorTex; }
    QRhiSampler* baseColorSampler() const { return m_baseColorSampler; }

    void ensureRhi(QRhi* rhi, QRhiCommandBuffer* cb);

    static QColor gxParseColor(const QString& s);

signals:
    void renderStateChanged();
    void materialChanged();

protected:
    GXShaderUtils m_shaderUtils;
    void markDirty();

    virtual void ensureBaseColorResources(QRhi* rhi, QRhiCommandBuffer* cb) = 0;
    virtual void destroyRhiResources();

    QRhi* m_rhi = nullptr;
    QRhiTexture* m_baseColorTex = nullptr;
    QRhiSampler* m_baseColorSampler = nullptr;

private:
    GXRenderState m_state;

    bool m_dirty = true;

};

}

#endif // GXMATERIAL_H
