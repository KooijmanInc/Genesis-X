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

    QColor baseColor() const { return m_baseColor; }

    // void applyTo(QRhiGraphicsPipeline* ps) const;

    // const QShader& vertexShader() const { return m_vs; }
    // const QShader& fragmentShader() const { return m_fs; }

signals:
    void renderStateChanged();

protected:
    GXShaderUtils m_shaderUtils;

private:
    GXRenderState m_state;

    QShader m_vs, m_fs;
    QColor m_baseColor = Qt::blue;
};

}

#endif // GXMATERIAL_H
