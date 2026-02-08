// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Utils/GXMeshHelper.h>

#include <QColor>

using namespace gx::gx3d::utils;

GXMeshHelper::GXMeshHelper() {}

QMap<QString, QVariant> GXMeshHelper::materials(const QJsonObject &obj)
{
    QMap<QString, QVariant> m;

    if (!emissionColor(obj).contains("emissionColor")) {
        m["baseColor"] = baseColor(obj).value("baseColor");
    }
    m["emissionColor"] = emissionColor(obj).contains("emissionColor") ? emissionColor(obj).value("emissionColor") : "";
    m["emissionStrength"] = extensions(obj).contains("emissionStrength")
                                ? extensions(obj).value("emissionStrength") == ""
                                      ? (m["emissionColor"] != "" ? "1" : "")
                                      : extensions(obj).value("emissionStrength")
                                : (m["emissionColor"] != "" ? "1" : "");

    return m;
}

QMap<QString, QVariant> GXMeshHelper::baseColor(const QJsonObject &bc)
{
    QMap<QString, QVariant> m;
    const QJsonObject pbr = bc.value("pbrMetallicRoughness").toObject();
    const auto c = pbr.value("baseColorFactor").toArray();

    if (c.size() >= 3) {
        const float r = float(c.at(0).toDouble());
        const float g = float(c.at(1).toDouble());
        const float b = float(c.at(2).toDouble());

        m["baseColor"] = "\"" + QColor::fromRgbF(r, g, b, 1.0f).name() + "\"";
    }

    return m;
}

QMap<QString, QVariant> GXMeshHelper::baseColorTexture(const QJsonObject &bct)
{
    QMap<QString, QVariant> m;
    const QJsonObject pbr = bct.value("pbrMetallicRoughness").toObject();
    const auto c = pbr.value("baseColorTexture").toArray();

    return m;
}

QMap<QString, QVariant> GXMeshHelper::emissionColor(const QJsonObject &ec)
{
    QMap<QString, QVariant> m;
    const auto c = ec.value("emissiveFactor").toArray();

    if (c.size() >= 3) {
        const float r = float(c.at(0).toDouble());
        const float g = float(c.at(1).toDouble());
        const float b = float(c.at(2).toDouble());

        m["emissionColor"] = "\"" + QColor::fromRgbF(r, g, b, 1.0f).name() + "\"";
    }

    return m;
}

QMap<QString, QVariant> GXMeshHelper::extensions(const QJsonObject& ex)
{
    QMap<QString, QVariant> m;
    if (ex.contains("extensions")) {
        const auto e = ex.value("extensions").toObject();

        m["emissionStrength"] = emissionStrength(e).contains("emissionStrength") ? emissionStrength(e).value("emissionStrength") : "";
    }

    return m;
}

QMap<QString, QVariant> GXMeshHelper::emissionStrength(const QJsonObject &em)
{
    QMap<QString, QVariant> m;

    if (em.contains("KHR_materials_emissive_strength")) {
        const QJsonObject emissiveStrength = em.value("KHR_materials_emissive_strength").toObject();
        qDebug() << "found" << emissiveStrength;
        m["emissionStrength"] = emissiveStrength["emissiveStrength"].toInt();
    } else {
        qDebug() << "still to do:" << em;
    }

    return m;
}
