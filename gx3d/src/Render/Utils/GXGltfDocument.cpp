// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Utils/GXGltfDocument.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonParseError>

namespace gx::gx3d::render {

static void pushError(QVector<GXGltfError>* errors, const QString& msg)
{
    if (errors) errors->push_back({ msg });
}

GXGltfDocument GXGltfDocument::fromFile(QString path, QVector<GXGltfError> *errors)
{
    GXGltfDocument doc;
    doc.m_sourcePath = path;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        pushError(errors, QStringLiteral("Failed to open file: %1").arg(path));
        return doc;
    }

    const QByteArray data = file.readAll();
    file.close();

    // ─────────────────────────────
    // Detect GLB vs GLTF
    // ─────────────────────────────
    const bool isGlb = data.size() >= 12 && qstrncmp(data.constData(), "glTF", 4) == 0;

    if (isGlb) {
        quint32 version = *reinterpret_cast<const quint32 *>(data.constData() + 4);
        if (version != 2) {
            pushError(errors, "Unsupported GLB version");
            return doc;
        }

        int offset = 12;
        while (offset + 8 <= data.size()) {
            quint32 chunckLength = *reinterpret_cast<const quint32 *>(data.constData() + offset);
            quint32 chunckType = *reinterpret_cast<const quint32 *>(data.constData() + offset + 4);
            offset += 8;

            if (offset + int(chunckLength) > data.size()) break;

            const QByteArray chunk = data.mid(offset, chunckLength);

            if (chunckType == 0x4E4F534A) {
                QJsonParseError err{};
                QJsonDocument jdoc = QJsonDocument::fromJson(chunk, &err);
                if (err.error != QJsonParseError::NoError) {
                    pushError(errors, err.errorString());
                    return doc;
                }
                doc.m_json = jdoc.object();
            } else if (chunckType == 0x004E4942) {
                doc.m_binChunk = chunk;
            }

            offset += chunckLength;
        }

        doc.m_buffers.push_back(doc.m_binChunk);
    } else {
        // ─────────────────────────────
        // Plain .gltf (JSON + external buffers)
        // ─────────────────────────────
        QJsonParseError err{};
        QJsonDocument jdoc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError) {
            pushError(errors, err.errorString());
            return doc;
        }

        doc.m_json = jdoc.object();

        const QFileInfo fi(path);
        const QJsonArray buffers = doc.m_json.value("buffers").toArray();

        for (const QJsonValue &v : buffers) {
            const QJsonObject obj = v.toObject();
            const QString uri = obj.value("uri").toString();

            if (uri.isEmpty()) {
                pushError(errors, "Buffer URI missing");
                continue;
            }

            QFile bufFile(fi.dir().filePath(uri));
            if (!bufFile.open(QIODevice::ReadOnly)) {
                pushError(errors, "Failed to open buffer: " + uri);
                continue;
            }

            doc.m_buffers.push_back(bufFile.readAll());
        }
    }

    doc.m_valid = !doc.m_json.isEmpty();

    return doc;
}

}
