// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Utils/GXGltfAccess.h>

#include <QtGlobal>

namespace gx::gx3d::render {

GXGltfAccess::GXGltfAccess(const GXGltfDocument &doc, QVector<GXGltfError> *errors)
    : m_doc(doc)
    , m_errors(errors)
{
}

bool GXGltfAccess::isValid() const
{
    return m_doc.isValid();
}

void GXGltfAccess::pushError(const QString &msg) const
{
    if (m_errors)
        m_errors->push_back({ msg });
}

QJsonArray GXGltfAccess::array(const char *name) const
{
    return m_doc.json().value(QString::fromLatin1(name)).toArray();
}

QJsonObject GXGltfAccess::objectAt(const QJsonArray &a, int index, const char *what) const
{
    if (index < 0 || index >= a.size()) {
        pushError(QStringLiteral("Index out of range for %1: %2").arg(QString::fromLatin1(what)).arg(index));
        return {};
    }
    return a.at(index).toObject();
}

int GXGltfAccess::meshCount() const
{
    return array("meshes").size();
}

QJsonObject GXGltfAccess::meshObject(int meshIndex) const
{
    return objectAt(array("meshes"), meshIndex, "meshes");
}

QJsonArray GXGltfAccess::meshPrimitives(int meshIndex) const
{
    const QJsonObject mesh = meshObject(meshIndex);
    return mesh.value("primitives").toArray();
}

// ─────────────────────────────────────────────
// glTF constants helpers
// componentType: 5120..5126
// type: "SCALAR", "VEC2", "VEC3", "VEC4", "MAT4" etc.
// ─────────────────────────────────────────────
int GXGltfAccess::componentsForType(const QString &type)
{
    if (type == "SCALAR") return 1;
    if (type == "VEC2")   return 2;
    if (type == "VEC3")   return 3;
    if (type == "VEC4")   return 4;
    if (type == "MAT2")   return 4;
    if (type == "MAT3")   return 9;
    if (type == "MAT4")   return 16;
    return 0;
}

int GXGltfAccess::bytesPerComponent(int componentType)
{
    switch (componentType) {
    case 5120: return 1; // BYTE
    case 5121: return 1; // UNSIGNED_BYTE
    case 5122: return 2; // SHORT
    case 5123: return 2; // UNSIGNED_SHORT
    case 5125: return 4; // UNSIGNED_INT
    case 5126: return 4; // FLOAT
    default:   return 0;
    }
}

const uchar* GXGltfAccess::resolveAccessorData(const QJsonObject &accessorObj,
                                               const QJsonObject &bufferViewObj,
                                               int &outByteLength) const
{
    const int bufferIndex = bufferViewObj.value("buffer").toInt(-1);
    if (bufferIndex < 0 || bufferIndex >= m_doc.buffers().size()) {
        pushError("bufferView.buffer out of range");
        outByteLength = 0;
        return nullptr;
    }

    const int bvOffset = bufferViewObj.value("byteOffset").toInt(0);
    const int bvLen    = bufferViewObj.value("byteLength").toInt(0);

    const int accOffset = accessorObj.value("byteOffset").toInt(0);

    const QByteArray &buf = m_doc.buffers().at(bufferIndex);

    const int start = bvOffset + accOffset;
    const int end   = bvOffset + bvLen;

    if (start < 0 || end < 0 || start > end || end > buf.size()) {
        pushError("Accessor byte range out of buffer bounds");
        outByteLength = 0;
        return nullptr;
    }

    outByteLength = end - start;
    return reinterpret_cast<const uchar*>(buf.constData() + start);
}

GXGltfAccess::AccessorInfo GXGltfAccess::accessorInfo(int accessorIndex) const
{
    AccessorInfo info;

    const QJsonArray accessors = array("accessors");
    const QJsonArray bufferViews = array("bufferViews");

    const QJsonObject acc = objectAt(accessors, accessorIndex, "accessors");
    if (acc.isEmpty())
        return info;

    info.count = acc.value("count").toInt(0);
    info.componentType = acc.value("componentType").toInt(0);
    info.normalized = acc.value("normalized").toBool(false);
    const QString type = acc.value("type").toString();
    info.components = componentsForType(type);

    if (info.count <= 0 || info.components <= 0) {
        pushError("Accessor has invalid count/type");
        return info;
    }

    const int bufferViewIndex = acc.value("bufferView").toInt(-1);
    if (bufferViewIndex < 0) {
        pushError("Accessor missing bufferView (sparse not supported yet)");
        return info;
    }

    const QJsonObject bv = objectAt(bufferViews, bufferViewIndex, "bufferViews");
    if (bv.isEmpty())
        return info;

    // stride: if not set, tightly packed
    info.byteStride = bv.value("byteStride").toInt(0);

    info.data = resolveAccessorData(acc, bv, info.byteLength);
    if (!info.data)
        return info;

    // If no stride, compute tightly packed stride
    if (info.byteStride == 0) {
        const int bpc = bytesPerComponent(info.componentType);
        if (bpc == 0) {
            pushError("Unknown componentType");
            return {};
        }
        info.byteStride = bpc * info.components;
    }

    return info;
}

// ─────────────────────────────────────────────
// Public high-level readers
// ─────────────────────────────────────────────
QVector<QVector3D> GXGltfAccess::readVec3Accessor(int accessorIndex) const
{
    const AccessorInfo a = accessorInfo(accessorIndex);
    QVector<QVector3D> out;

    if (!a.data)
        return out;

    if (a.componentType != 5126 || a.components != 3) { // FLOAT + VEC3
        pushError("Expected FLOAT VEC3 accessor");
        return out;
    }

    out.reserve(a.count);

    for (int i = 0; i < a.count; ++i) {
        const uchar *p = a.data + i * a.byteStride;
        const float *f = reinterpret_cast<const float*>(p);
        out.push_back(QVector3D(f[0], f[1], f[2]));
    }

    return out;
}

QVector<QVector2D> GXGltfAccess::readVec2Accessor(int accessorIndex) const
{
    const AccessorInfo a = accessorInfo(accessorIndex);
    QVector<QVector2D> out;

    if (!a.data)
        return out;

    if (a.componentType != 5126 || a.components != 2) { // FLOAT + VEC2
        pushError("Expected FLOAT VEC2 accessor");
        return out;
    }

    out.reserve(a.count);

    for (int i = 0; i < a.count; ++i) {
        const uchar *p = a.data + i * a.byteStride;
        const float *f = reinterpret_cast<const float*>(p);
        out.push_back(QVector2D(f[0], f[1]));
    }

    return out;
}

QVector<QVector3D> GXGltfAccess::readPositionsFromPrimitive(const QJsonObject &primitive) const
{
    const QJsonObject attrs = primitive.value("attributes").toObject();
    const int accIndex = attrs.value("POSITION").toInt(-1);
    if (accIndex < 0) {
        pushError("Primitive has no POSITION attribute");
        return {};
    }
    return readVec3Accessor(accIndex);
}

QVector<quint32> GXGltfAccess::readIndicesFromPrimitive(const QJsonObject &primitive) const
{
    const int idxAccessor = primitive.value("indices").toInt(-1);
    if (idxAccessor < 0) {
        // Some primitives are non-indexed: caller can generate sequential indices if wanted
        return {};
    }

    const AccessorInfo a = accessorInfo(idxAccessor);
    QVector<quint32> out;

    if (!a.data)
        return out;

    if (a.components != 1) {
        pushError("Indices accessor must be SCALAR");
        return out;
    }

    out.reserve(a.count);

    for (int i = 0; i < a.count; ++i) {
        const uchar *p = a.data + i * a.byteStride;

        switch (a.componentType) {
        case 5121: // UNSIGNED_BYTE
            out.push_back(*reinterpret_cast<const quint8*>(p));
            break;
        case 5123: // UNSIGNED_SHORT
            out.push_back(*reinterpret_cast<const quint16*>(p));
            break;
        case 5125: // UNSIGNED_INT
            out.push_back(*reinterpret_cast<const quint32*>(p));
            break;
        default:
            pushError("Unsupported indices componentType (expected u8/u16/u32)");
            return {};
        }
    }

    return out;
}

}
