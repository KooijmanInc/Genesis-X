// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXMODELNODE_H
#define GXMODELNODE_H

#include <QQmlListProperty>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Render/Meshes/GXCubeMesh.h>
#include <GenesisX/GX3D/Render/Meshes/GXPlaneMesh.h>
#include <GenesisX/GX3D/Render/Nodes/GXRenderableNode.h>
#include <GenesisX/GX3D/Scene/Nodes/GXNode.h>
#include <GenesisX/GX3D/Render/Resources/GXMesh.h>
#include <GenesisX/GX3D/Render/Utils/GXMeshData.h>
// #include <GenesisX/GX3D/Render/Materials/GXMaterial.h>

namespace gx::gx3d::render {

class GXMeshReader;

class GENESISX_GX3D_EXPORT GXModel : public GXRenderableNode
{
    Q_OBJECT

    Q_PROPERTY(GXMesh* mesh READ mesh WRITE setMesh NOTIFY meshChanged)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QQmlListProperty<GXMaterial> materials READ materials NOTIFY materialsChanged)
    Q_PROPERTY(bool pickable READ pickable WRITE setPickable NOTIFY pickableChanged)
    Q_PROPERTY(int pickPriority READ pickPriority WRITE setPickPriority NOTIFY pickPriorityChanged)

public:
    explicit GXModel(QObject* parent = nullptr);
    ~GXModel() override;

    GXMesh* mesh() const { return m_mesh; }
    void setMesh(GXMesh* mesh);

    QString source() const { return m_source; }
    void setSource(QString src);

    QQmlListProperty<GXMaterial> materials();
    const QVector<GXMaterial*>& materialsVector() const { return m_materials; }

    bool pickable() const { return m_pickable; }
    void setPickable(bool on);

    int pickPriority() const { return m_pickPriority; }
    void setPickPriority(int v);

    bool localBounds(QVector3D& outMinLS, QVector3D& outMaxLS) const;

    void addMaterial(GXMaterial* m);
    void clearMaterials();

    void ensureResources(QRhi* rhi, QRhiRenderTarget* rt, QRhiCommandBuffer* cb) override;
    void recordRender(QRhiCommandBuffer* cb, QRhiRenderTarget* rt, const QRect &scissor) override;
    void releaseResources() override;
    void setFrameLightingUbo(QRhiBuffer* ubo) override {
        if (m_frameLightingUbo == ubo) return;
        m_frameLightingUbo = ubo;
        invalidPipeline();
    }
    void setFrameEnvironmentUbo(QRhiBuffer* ubo) override {
        if (m_environmentUbo == ubo) return;
        m_environmentUbo = ubo;
        invalidPipeline();
    }
    void setBrdfLutTex(QRhiTexture* tex) override {
        if (m_brdfLutTex == tex) return;
        m_brdfLutTex = tex;
        invalidPipeline();
    }
    void setBrdfLutSampler(QRhiSampler* sampler) override {
        if (m_brdfLutSampler == sampler) return;
        m_brdfLutSampler = sampler;
        invalidPipeline();
    }
    void setEnvCubeTex(QRhiTexture* tex) override {
        if (m_envCubeTex == tex) return;
        m_envCubeTex = tex;
        invalidPipeline();
    }
    void setEnvCubeSampler(QRhiSampler* sampler) override {
        if (m_envCubeSampler == sampler) return;
        m_envCubeSampler = sampler;
        invalidPipeline();
    }
    void setPrefilterSpecCubeTex(QRhiTexture* tex) override {
        if (m_prefilterSpecCubeTex == tex) return;
        m_prefilterSpecCubeTex = tex;
        invalidPipeline();
    }
    void setPrefilterSpecCubeSampler(QRhiSampler* sampler) override {
        if (m_prefilterSpecCubeSampler == sampler) return;
        m_prefilterSpecCubeSampler = sampler;
        invalidPipeline();
    }
    void setIrradianceCubeTex(QRhiTexture* tex) override {
        if (m_irradianceCubeTex == tex) return;
        m_irradianceCubeTex = tex;
        invalidPipeline();
    }
    void setIrradianceCubeSampler(QRhiSampler* sampler) override {
        if (m_irradianceCubeSampler == sampler) return;
        m_irradianceCubeSampler = sampler;
        invalidPipeline();
    }

    void recordPick(QRhiCommandBuffer *cb);

signals:
    void meshChanged();
    void sourceChanged();
    void materialsChanged();
    void pickableChanged();
    void pickPriorityChanged();

// private:
// #ifdef Q_OS_ANDROID
//     struct alignas(16) VSUniforms {
//         float mvp[16];
//         float model[16];
//     };
//     struct alignas(16) FSUniforms {
//         float baseColor[4];
//         float lightPos[4];
//         float lightColor[4];
//         float lightParams[4];
//     };
// #else
//     alignas(16) struct VSUniforms {
//         float mvp[16];
//         float model[16];
//     };
//     alignas(16) struct FSUniforms {
//         float baseColor[4];
//         float lightPos[4];
//         float lightColor[4];
//         float lightParams[4];
//     };
// #endif
private:
    GXMesh* m_mesh = nullptr;
    QString m_source;
    QVector<GXMaterial*> m_materials;
    QHash<GXMaterial*, QRhiShaderResourceBindings*> m_materialSrbs;
    QHash<GXMaterial*, QRhiBuffer*> m_vsUbufPerMat;
    QHash<GXMaterial*, QRhiBuffer*> m_fsUbufPerMat;
    QByteArray m_vsScratch;
    QByteArray m_fsScratch;

    // QPointer<GXMaterial*> m_defaultMat;

    QRhiRenderPassDescriptor* m_lastRp = nullptr;
    int m_lastSampleCount = 0;

    QRhiShaderResourceBindings* srbForMaterial(GXMaterial* mat, QRhiCommandBuffer* cb);

    QRhiGraphicsPipeline* m_ps = nullptr;
    QRhiBuffer* m_vsUbuf = nullptr;
    QRhiBuffer* m_fsUbuf = nullptr;
    QRhiBuffer* m_boundLightingUbo = nullptr;
    bool m_boundHadLighting = false;

    QString m_loadedMeshSource;
    GXMeshData m_meshCpu;
    bool m_meshLoaded = false;

    bool m_pickable = false;
    int m_pickPriority = 0;

    mutable bool m_boundsValid = false;
    mutable QVector3D m_boundsMinLS;
    mutable QVector3D m_boundsMaxLS;

    bool ensureMeshLoaded(QString* err = nullptr);

    void destroyRhiResources();
    void destroyPipelineResources();
};

}

#endif // GXMODELNODE_H
