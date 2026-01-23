// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXDEFAULTLITMATERIAL_H
#define GXDEFAULTLITMATERIAL_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>
#include <GenesisX/GX3D/Render/Materials/GXMaterial.h>
#include <GenesisX/GX3D/Render/Materials/GXDefaultLitUniforms.h>

#include <QColor>
#include <QMatrix4x4>

namespace gx::gx3d::render {

struct GXPointLightData;

class GENESISX_GX3D_EXPORT GXDefaultLitMaterial : public GXMaterial
{
    Q_OBJECT

    Q_PROPERTY(QColor baseColor READ baseColor WRITE setBaseColor NOTIFY baseColorChanged)

public:
    explicit GXDefaultLitMaterial(QObject* parent = nullptr);

    QShader vertexShader() const override;
    QShader fragmentShader() const override;
    void applyTo(QRhiGraphicsPipeline* ps) const override;

    // Layout contract (size + binding numbers)
    int vsBinding() const { return DefaultLit_VS_Binding; }
    int fsBinding() const { return DefaultLit_FS_Binding; }
    int vsUboSize() const {return int(sizeof(DefaultLitVSUBO)); }
    int fsUboSize() const {return int(sizeof(DefaultLitFSUBO)); }

    QColor baseColor() const { return m_baseColor; }
    void setBaseColor(const QColor& c);

    void fillVS(DefaultLitVSUBO& out, const QMatrix4x4& mvp, const QMatrix4x4& model) const;
    void fillFS(DefaultLitFSUBO& out, const GXPointLightData& light) const;

signals:
    void baseColorChanged();

private:
    QColor m_baseColor = Qt::white;
};

}

#endif // GXDEFAULTLITMATERIAL_H
