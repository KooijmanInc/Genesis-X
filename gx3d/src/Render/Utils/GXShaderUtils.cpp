// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Utils/GXShaderUtils.h>

#include <QFile>
#include <QDebug>

using namespace gx::gx3d::render;

QShader GXShaderUtils::gxLoadShader(const QString &path) const
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open shader:" << path;
        return {};
    }
    const QByteArray data = f.readAll();
    QShader s = QShader::fromSerialized(data);

    if (!s.isValid()) qWarning() << "Invalid .qsb shader:" << path;

    return s;
}
