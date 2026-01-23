// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Materials/GXDefaultLitMaterial.h>
#include <GenesisX/GX3D/Render/Lights/GXFrameLighting.h>
#include <QDebug>
#include <QFile>

using namespace gx::gx3d::render;

GXDefaultLitMaterial::GXDefaultLitMaterial(QObject *parent)
    : GXMaterial{parent}
{
}

QShader GXDefaultLitMaterial::vertexShader() const
{
    return m_shaderUtils.gxLoadShader(":/gx3d/shaders/default_lit.vert.qsb");
}

QShader GXDefaultLitMaterial::fragmentShader() const
{
    return m_shaderUtils.gxLoadShader(":/gx3d/shaders/default_lit.frag.qsb");
}

void GXDefaultLitMaterial::applyTo(QRhiGraphicsPipeline *ps) const
{
    ps->setCullMode(QRhiGraphicsPipeline::Back);
    ps->setFrontFace(QRhiGraphicsPipeline::CCW);
    ps->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
    ps->setDepthTest(true);
    ps->setDepthWrite(true);
}

void GXDefaultLitMaterial::setBaseColor(const QColor &c)
{
    if (m_baseColor == c) return;
    m_baseColor = c;

    emit baseColorChanged();
}

void GXDefaultLitMaterial::fillVS(DefaultLitVSUBO &out, const QMatrix4x4 &mvp, const QMatrix4x4 &model) const
{
    memcpy(out.mvp, mvp.constData(), 16 * sizeof(float));
    memcpy(out.model, model.constData(), 16 * sizeof(float));
}

void GXDefaultLitMaterial::fillFS(DefaultLitFSUBO &out, const GXPointLightData &light) const
{
    out.baseColor[0] = float(m_baseColor.redF());
    out.baseColor[1] = float(m_baseColor.greenF());
    out.baseColor[2] = float(m_baseColor.blueF());
    out.baseColor[3] = float(m_baseColor.alphaF());

    out.lightPos[0] = light.positionWS.x();
    out.lightPos[1] = light.positionWS.y();
    out.lightPos[2] = light.positionWS.z();
    out.lightPos[3] = 1.0f;

    out.lightColor[0] = light.color.x();
    out.lightColor[1] = light.color.y();
    out.lightColor[2] = light.color.z();
    out.lightColor[3] = light.intensity;

    out.lightParams[0] = light.range;
    out.lightParams[1] = 0.0f;
    out.lightParams[2] = 0.0f;
    out.lightParams[3] = 0.0f;
}
