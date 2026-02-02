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

namespace gx::gx3d::render {

class GXMeshReader;

class GENESISX_GX3D_EXPORT GXModel : public GXRenderableNode
{
    Q_OBJECT

    Q_PROPERTY(GXMesh* mesh READ mesh WRITE setMesh NOTIFY meshChanged)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QQmlListProperty<GXMaterial> materials READ materials NOTIFY materialsChanged)

public:
    explicit GXModel(QObject* parent = nullptr);
    ~GXModel() override;

    GXMesh* mesh() const { return m_mesh; }
    void setMesh(GXMesh* mesh);

    QString source() const { return m_source; }
    void setSource(QString src);

    QQmlListProperty<GXMaterial> materials();
    const QVector<GXMaterial*>& materialsVector() const { return m_materials; }

    void addMaterial(GXMaterial* m);
    void clearMaterials();

    void ensureResources(QRhi* rhi, QRhiRenderTarget* rt, QRhiCommandBuffer* cb) override;
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
    void materialsChanged();

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

    QRhiShaderResourceBindings* srbForMaterial(GXMaterial* mat, QRhiCommandBuffer* cb);

    QRhiGraphicsPipeline* m_ps = nullptr;
    QRhiBuffer* m_vsUbuf = nullptr;
    QRhiBuffer* m_fsUbuf = nullptr;
    QRhiBuffer* m_boundLightingUbo = nullptr;
    bool m_boundHadLighting = false;

    QString m_loadedMeshSource;
    GXMeshData m_meshCpu;
    bool m_meshLoaded = false;

    bool ensureMeshLoaded(QString* err = nullptr);

    void destroyRhiResources();
    void destroyPipelineResources();
};

}

#endif // GXMODELNODE_H
