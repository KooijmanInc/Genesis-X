// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXMODELNODE_H
#define GXMODELNODE_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Render/Meshes/GXCubeMesh.h>
#include <GenesisX/GX3D/Render/Meshes/GXPlaneMesh.h>
#include <GenesisX/GX3D/Render/Nodes/GXRenderableNode.h>
#include <GenesisX/GX3D/Scene/Nodes/GXNode.h>
#include <GenesisX/GX3D/Render/Resources/GXMesh.h>

namespace gx::gx3d::render {

class GENESISX_GX3D_EXPORT GXModel : public GXRenderableNode
{
    Q_OBJECT

    Q_PROPERTY(GXMesh* mesh READ mesh WRITE setMesh NOTIFY meshChanged)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(Primitive primitive READ primitive WRITE setPrimitive NOTIFY primitiveChanged)

public:
    enum Primitive {
        None,
        Cube,
        Plane,
        Sphere,
        Torus
    };
    Q_ENUM(Primitive)

    explicit GXModel(QObject* parent = nullptr);
    ~GXModel() override;

    GXMesh* mesh() const { return m_mesh; }
    void setMesh(GXMesh* mesh);

    QString source() const { return m_source; }
    void setSource(QString src);

    Primitive primitive() const { return m_primitive; }
    void setPrimitive(Primitive p);

    // GXMaterial* material() const { return m_material; }
    // void setMaterial(GXMaterial* m);

    void ensureResources(QRhi* rhi, QRhiRenderTarget* rt) override;
    void recordRender(QRhiCommandBuffer* cb, QRhiRenderTarget* rt) override;
    void releaseResources() override;
    void setFrameLightingUbo(QRhiBuffer* ubo) override {
        if (m_frameLightingUbo == ubo) return;
        m_frameLightingUbo = ubo;
        invalidPipeline();
    }

signals:
    void meshChanged();
    void sourceChanged();
    void primitiveChanged();

private:
#ifdef Q_OS_ANDROID
    struct alignas(16) VSUniforms {
        float mvp[16];
        float model[16];
    };
    struct alignas(16) FSUniforms {
        float baseColor[4];
        float lightPos[4];
        float lightColor[4];
        float lightParams[4];
    };
#else
    alignas(16) struct VSUniforms {
        float mvp[16];
        float model[16];
    };
    alignas(16) struct FSUniforms {
        float baseColor[4];
        float lightPos[4];
        float lightColor[4];
        float lightParams[4];
    };
#endif
private:
    GXMesh* m_mesh = nullptr;
    QString m_source;
    Primitive m_primitive = None;
    // GXMaterial* m_material = nullptr;

    QRhiGraphicsPipeline* m_ps = nullptr;
    QRhiShaderResourceBindings* m_srb = nullptr;
    QRhiBuffer* m_vsUbuf = nullptr;
    QRhiBuffer* m_fsUbuf = nullptr;
    // QRhiBuffer* m_frameLightingUbo = nullptr;
    QRhiBuffer* m_boundLightingUbo = nullptr;
    bool m_boundHadLighting = false;

    void destroyRhiResources();
    void destroyPipelineResources();
};

}

#endif // GXMODELNODE_H
