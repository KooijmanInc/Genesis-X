// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXGLTFACCESS_H
#define GXGLTFACCESS_H

#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QJsonArray>
#include <QJsonObject>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Render/Utils/GXGltfDocument.h>

namespace gx::gx3d::render {

class GENESISX_GX3D_EXPORT GXGltfAccess
{
public:
    explicit GXGltfAccess(const GXGltfDocument& doc, QVector<GXGltfError>* errors = nullptr);

    bool isValid() const;

    // ── high-level JSON access
    int meshCount() const;
    QJsonObject meshObject(int meshIndex) const;
    QJsonArray meshPrimitives(int meshIndex) const;

    // ── attribute + accessor decoding helpers
    QVector<QVector3D> readPositionsFromPrimitive(const QJsonObject &primitive) const;
    QVector<quint32>   readIndicesFromPrimitive(const QJsonObject &primitive) const;
    QVector<QVector4D> readTangentsFromPrimitive(const QJsonObject &primitive) const;

    // If you need normals/uvs later:
    QVector<QVector4D> readVec4Accessor(int accessorIndex) const;
    QVector<QVector3D> readVec3Accessor(int accessorIndex) const;
    QVector<QVector2D> readVec2Accessor(int accessorIndex) const;

private:
    struct AccessorInfo {
        const uchar *data = nullptr;
        int byteLength = 0;

        int count = 0;
        int componentType = 0;
        int components = 0;     // e.g. VEC3 => 3
        int byteStride = 0;     // 0 means tightly packed
        bool normalized = false;
    };

    const GXGltfDocument &m_doc;
    QVector<GXGltfError> *m_errors = nullptr;

    void pushError(const QString &msg) const;

    QJsonArray array(const char *name) const;
    QJsonObject objectAt(const QJsonArray &a, int index, const char *what) const;

    static int componentsForType(const QString &type);
    static int bytesPerComponent(int componentType);

    AccessorInfo accessorInfo(int accessorIndex) const;
    const uchar* resolveAccessorData(const QJsonObject &accessorObj,
                                     const QJsonObject &bufferViewObj,
                                     int &outByteLength) const;
};

}
#endif // GXGLTFACCESS_H
