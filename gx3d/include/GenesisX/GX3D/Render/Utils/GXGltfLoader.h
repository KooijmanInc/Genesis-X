// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXGLTFLOADER_H
#define GXGLTFLOADER_H

#include <QObject>
#include <QUrl>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Scene/Nodes/GXNode.h>

namespace gx::gx3d::render {

class GXMesh;

class GENESISX_GX3D_EXPORT GXGltfLoader final : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

    static QVector<GXMesh*> loadMesh(const QUrl& source);
    static QVector<GXMesh*> loadMeshesFromFilePath(const QString& path);
    static gx::gx3d::scene::GXNode* loadSceneRoot(const QUrl& source);
};

}

#endif // GXGLTFLOADER_H
