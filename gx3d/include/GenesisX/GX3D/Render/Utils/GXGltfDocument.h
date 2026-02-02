// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXGLTFDOCUMENT_H
#define GXGLTFDOCUMENT_H

#include <QVector>
#include <QString>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

namespace gx::gx3d::render {

struct GENESISX_GX3D_EXPORT GXGltfError
{
    QString message;
};

class GENESISX_GX3D_EXPORT GXGltfDocument
{
public:
    static GXGltfDocument fromFile(QString path, QVector<GXGltfError> *errors = nullptr);

    bool isValid() const { return m_valid; }

    const QJsonObject& json() const { return m_json; }
    const QByteArray& binaryChunk() const { return m_binChunk; }
    const QVector<QByteArray>& buffers() const { return m_buffers; }

    QString sourcePath() const { return m_sourcePath; }

private:
    bool m_valid = false;
    QString m_sourcePath;

    QJsonObject m_json;
    QByteArray m_binChunk;
    QVector<QByteArray> m_buffers;
};

}

#endif // GXGLTFDOCUMENT_H
