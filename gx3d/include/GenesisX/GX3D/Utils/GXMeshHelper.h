// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXMESHHELPER_H
#define GXMESHHELPER_H

#include <QJsonObject>
#include <QVariantMap>
#include <QJsonArray>
#include <QVariant>
#include <QString>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

namespace gx::gx3d::utils {

class GENESISX_GX3D_EXPORT GXMeshHelper
{
public:
    GXMeshHelper();

    static QMap<QString, QVariant> materials(const QJsonObject& obj);
// static QMap<QString, QVariant> textures(const QJsonObject& obj, const QJson)

private:
    static QMap<QString, QVariant> baseColor(const QJsonObject& bc);
    static QMap<QString, QVariant> baseColorTexture(const QJsonObject& bct);
    static QMap<QString, QVariant> emissionColor(const QJsonObject& ec);
    static QMap<QString, QVariant> extensions(const QJsonObject& ex);
    static QMap<QString, QVariant> emissionStrength(const QJsonObject& em);
};

}

#endif // GXMESHHELPER_H
