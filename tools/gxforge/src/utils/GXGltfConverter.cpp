// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "GXGltfConverter.h"

#include <GenesisX/GX3D/Render/Utils/GXGltfDocument.h>
#include <GenesisX/GX3D/Render/Utils/GXMeshWriter.h>
#include <GenesisX/GX3D/Utils/GXMeshHelper.h>

#include <QQuaternion>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <numeric>
#include <QColor>

#include <cstddef>

static QString indent(int n)
{
    return QString(n * 4, QLatin1Char(' '));
}

static QString vec3ToQml(const QJsonArray& a)
{
    if (a.size() != 3) return "Qt.vector3d(0, 0, 0)";
    return QString("Qt.vector3d(%1, %2, %3)")
        .arg(a.at(0).toDouble(0.0), 0, 'g', 12)
        .arg(a.at(1).toDouble(0.0), 0, 'g', 12)
        .arg(a.at(2).toDouble(0.0), 0, 'g', 12);
}

static QString quatToQml(const QJsonArray &a)
{
    if (a.size() != 4) return "Qt.quaternion(1, 0, 0, 0)";
    const double x = a.at(0).toDouble(0.0);
    const double y = a.at(1).toDouble(0.0);
    const double z = a.at(2).toDouble(0.0);
    const double w = a.at(3).toDouble(1.0);
    // Qt.quaternion(w, x, y, z)
    return QString("Qt.quaternion(%1, %2, %3, %4)")
        .arg(w, 0, 'g', 12)
        .arg(x, 0, 'g', 12)
        .arg(y, 0, 'g', 12)
        .arg(z, 0, 'g', 12);
}

static QString safeIdFromName(QString name)
{
    name = name.trimmed();
    if (name.isEmpty())
        return "node";

    // keep letters/digits/underscore only
    QString out;
    out.reserve(name.size());
    const auto names = name;
    for (QChar c : names) {
        if (c.isLetterOrNumber() || c == '_')
            out.append(c);
        else
            out.append('_');
    }
    if (out.front().isDigit())
        out.prepend('_');
    return out;
}

// static QString makeMeshFileName(const QString& baseName, int meshIndex)
// {
//     return QString("%1_m%2.mesh").arg(baseName).arg(meshIndex);
// }

static QVector<QVector3D> readNormalsOrEmpty(const QJsonObject& prim, gx::gx3d::render::GXGltfAccess &access)
{
    const QJsonObject attrs = prim.value("attributes").toObject();
    const int acc = attrs.value("NORMAL").toInt(-1);
    return (acc >= 0) ? access.readVec3Accessor(acc) : QVector<QVector3D>{};
}

static QVector<QVector2D> readUv0OrEmpty(const QJsonObject& prim, gx::gx3d::render::GXGltfAccess& access)
{
    const QJsonObject attrs = prim.value("attributes").toObject();
    const int acc = attrs.value("TEXCOORD_0").toInt(-1);
    if (acc >= 0) {
        auto uv = access.readVec2Accessor(acc);

        // debug first few
        // for (int i = 0; i < 5 && i < uv.size(); ++i)
            // qDebug() << "TEXCOORD_0 acc" << acc << "UV" << i << uv[i].x() << uv[i].y();

        return uv;
    }
    return {};
    // return (acc >= 0) ? access.readVec2Accessor(acc) : QVector<QVector2D>{};
}

static QVector<QVector4D> readTangentsOrEmpty(const QJsonObject& prim, gx::gx3d::render::GXGltfAccess& access)
{
    // Implement this in GXGltfAccess (recommended), or parse accessor here.
    // In glTF, tangents are VEC4 float: xyz + w sign.
    if (!prim.value("attributes").toObject().contains("TANGENT"))
        return {};
    return access.readTangentsFromPrimitive(prim); // you add this method
}

static void autoUvPlanarXY(gx::gx3d::render::GXMeshData& md)
{
    if (md.vertices.isEmpty())
        return;

    float minX =  1e9f, minY =  1e9f;
    float maxX = -1e9f, maxY = -1e9f;

    for (const auto& v : md.vertices) {
        minX = std::min(minX, v.position.x());
        minY = std::min(minY, v.position.y());
        maxX = std::max(maxX, v.position.x());
        maxY = std::max(maxY, v.position.y());
    }

    const float dx = std::max(1e-6f, maxX - minX);
    const float dy = std::max(1e-6f, maxY - minY);

    for (auto& v : md.vertices) {
        const float u = (v.position.x() - minX) / dx;
        const float w = (v.position.y() - minY) / dy;
        v.uv0 = QVector2D(u, w);
    }
}

static void autoUvPlanarXY_WithRotation(gx::gx3d::render::GXMeshData& md, const QQuaternion& rot)
{
    if (md.vertices.isEmpty())
        return;

    float minX= 1e9f, minY= 1e9f;
    float maxX=-1e9f, maxY=-1e9f;

    // bounds in rotated space
    for (const auto& v : md.vertices) {
        const QVector3D p = rot.isNull() ? v.position : rot.rotatedVector(v.position);
        minX = std::min(minX, p.x());
        minY = std::min(minY, p.y());
        maxX = std::max(maxX, p.x());
        maxY = std::max(maxY, p.y());
    }

    const float dx = std::max(1e-6f, maxX - minX);
    const float dy = std::max(1e-6f, maxY - minY);

    for (auto& v : md.vertices) {
        const QVector3D p = rot.isNull() ? v.position : rot.rotatedVector(v.position);
        const float u = (p.x() - minX) / dx;
        const float w = (p.y() - minY) / dy;
        v.uv0 = QVector2D(u, w);
    }
}

static bool isUvDegenerate(const gx::gx3d::render::GXMeshData& md)
{
    if (md.vertices.isEmpty())
        return true;

    float minU =  1e9f, minV =  1e9f;
    float maxU = -1e9f, maxV = -1e9f;

    for (const auto& v : md.vertices) {
        minU = std::min(minU, v.uv0.x());
        minV = std::min(minV, v.uv0.y());
        maxU = std::max(maxU, v.uv0.x());
        maxV = std::max(maxV, v.uv0.y());
    }

    const float du = maxU - minU;
    const float dv = maxV - minV;

    // If either axis is almost constant, we call it degenerate
    return (du < 1e-3f) || (dv < 1e-3f);
}

static QQuaternion findFirstNodeRotationForMesh(const QJsonObject& docJson, int meshIndex)
{
    const QJsonArray nodes = docJson.value("nodes").toArray();
    for (int i = 0; i < nodes.size(); ++i) {
        const QJsonObject n = nodes.at(i).toObject();
        if (n.value("mesh").toInt(-1) == meshIndex) {
            const QJsonArray r = n.value("rotation").toArray();
            if (r.size() == 4) {
                // glTF rotation is [x,y,z,w]
                return QQuaternion(float(r[3].toDouble()),
                                   float(r[0].toDouble()),
                                   float(r[1].toDouble()),
                                   float(r[2].toDouble()));
            }
            break;
        }
    }
    return QQuaternion(); // identity
}

struct GxGltfSamplerInfo {
    int wrapS = 10497;   // REPEAT
    int wrapT = 10497;   // REPEAT
    int minFilter = -1;
    int magFilter = -1;
};

// static GxGltfSamplerInfo readSamplerInfo(const QJsonObject& doc, int samplerIndex)
// {
//     GxGltfSamplerInfo s;
//     const QJsonArray samplers = doc.value("samplers").toArray();
//     if (samplerIndex < 0 || samplerIndex >= samplers.size())
//         return s;

//     const QJsonObject so = samplers.at(samplerIndex).toObject();
//     s.wrapS = so.value("wrapS").toInt(10497);
//     s.wrapT = so.value("wrapT").toInt(10497);
//     s.minFilter = so.value("minFilter").toInt(-1);
//     s.magFilter = so.value("magFilter").toInt(-1);
//     return s;
// }

struct GxGltfTextureBinding {
    int textureIndex = -1;   // index into textures[]
    int imageIndex = -1;     // index into images[]
    int samplerIndex = -1;   // index into samplers[]
    GxGltfSamplerInfo sampler;
};

// static GxGltfTextureBinding resolveBaseColorTextureBinding(const QJsonObject& doc, const QJsonObject& material)
// {
//     GxGltfTextureBinding out;

//     const QJsonObject pbr = material.value("pbrMetallicRoughness").toObject();
//     const QJsonObject baseColorTexture = pbr.value("baseColorTexture").toObject();
//     const int texIndex = baseColorTexture.value("index").toInt(-1);
//     out.textureIndex = texIndex;

//     const QJsonArray textures = doc.value("textures").toArray();
//     if (texIndex < 0 || texIndex >= textures.size())
//         return out;

//     const QJsonObject texObj = textures.at(texIndex).toObject();
//     out.imageIndex = texObj.value("source").toInt(-1);
//     out.samplerIndex = texObj.value("sampler").toInt(-1);
//     out.sampler = readSamplerInfo(doc, out.samplerIndex);
//     return out;
// }

// static QString glWrapToString(int wrap)
// {
//     switch (wrap) {
//     case 33071: return "CLAMP_TO_EDGE";
//     case 33648: return "MIRRORED_REPEAT";
//     case 10497: return "REPEAT";
//     default:    return QString("UNKNOWN(%1)").arg(wrap);
//     }
// }

static void logUv0AccessorMeta(QString& status,
                               const QJsonObject& docJson,
                               const QJsonObject& prim,
                               int meshIdx,
                               int primIdx)
{
    const QJsonObject attrs = prim.value("attributes").toObject();
    const int accIndex = attrs.value("TEXCOORD_0").toInt(-1);
    if (accIndex < 0) {
        status.append(QString("mesh=%1 prim=%2 UV0 accessor: <missing>\n").arg(meshIdx).arg(primIdx));
        return;
    }

    const QJsonArray accessors = docJson.value("accessors").toArray();
    if (accIndex >= accessors.size()) {
        status.append(QString("mesh=%1 prim=%2 UV0 accessor: <out of range %3>\n").arg(meshIdx).arg(primIdx).arg(accIndex));
        return;
    }

    const QJsonObject acc = accessors.at(accIndex).toObject();
    const int componentType = acc.value("componentType").toInt(-1);
    const bool normalized   = acc.value("normalized").toBool(false);
    const QString type      = acc.value("type").toString();
    const int count         = acc.value("count").toInt(-1);

    status.append(QString("mesh=%1 prim=%2 UV0 accessor=%3 type=%4 componentType=%5 normalized=%6 count=%7\n")
                      .arg(meshIdx).arg(primIdx).arg(accIndex)
                      .arg(type).arg(componentType)
                      .arg(normalized ? "true" : "false")
                      .arg(count));
}


GXGltfConverter::GXGltfConverter(QObject *parent)
    : QObject{parent}
{
}

void GXGltfConverter::setSelectedDirectory(const QUrl &path)
{
    if (m_selectedDirectory == path) return;
    m_selectedDirectory = path;
    m_outputFileDirectory = path;

    emit selectedDirectoryChanged();
    emit outputFileDirectoryChanged();
}

void GXGltfConverter::setOutputFileDirectory(const QUrl &path)
{
    if (m_outputFileDirectory == path) return;

    m_outputFileDirectory = path;
    m_selectedDirectory = path;

    m_status.append("Output directory set: " + path.toLocalFile() + "\n");

    emit statusChanged();
    emit selectedDirectoryChanged();
    emit outputFileDirectoryChanged();
}

void GXGltfConverter::setFileUrl(const QUrl &file)
{
    if (m_fileUrl == file) return;
    m_fileUrl = file;
    m_file = file.toLocalFile();

    m_status.append("File added: " + m_file + "\n");

    emit statusChanged();
    emit fileChanged();
    emit fileUrlChanged();
}

void GXGltfConverter::setFile(const QString &file)
{
    if (m_file == file) return;
    m_file = file;
    m_fileUrl = QUrl::fromLocalFile(file);

    m_status.append("File added: " + m_file + "\n");

    emit statusChanged();
    emit fileChanged();
    emit fileUrlChanged();
}

// void GXGltfConverter::setStatus(const QString &status)
// {
//     m_status.append(status);

//     emit statusChanged();
// }

void GXGltfConverter::convertToMesh()
{
    m_status.append("\nConverting: " + m_file + "\n");

    QVector<gx::gx3d::render::GXGltfError> errors;

    auto doc = gx::gx3d::render::GXGltfDocument::fromFile(m_file, &errors);
    if (!doc.isValid()) {
        const auto& errs = errors;
        for (const auto &e : errs)
            m_status.append("Error: " + e.message + "\n");
        emit statusChanged();
        return;
    }

    gx::gx3d::render::GXGltfAccess access(doc, &errors);

    const QString baseName = QFileInfo(m_file).completeBaseName();
    const QString outDirPath = m_selectedDirectory.toLocalFile();

    QVector<QString> meshSources;
    QVector<QVector<QString>> meshMaterialIds;

    const QJsonArray mats = doc.json().value("materials").toArray();

    QVector<QString> gltfMatQmlId;
    gltfMatQmlId.reserve(mats.size());

    for (int i = 0; i < mats.size(); ++i) {
        const QJsonObject ma = mats.at(i).toObject();
        QString name = ma.value("name").toString().trimmed();

        // fallback if missing name
        if (name.isEmpty())
            name = QString("mat%1").arg(i);

        // create the same id rules you already use (lowercase first char, keep rest)
        QString idName;
        for (int j = 0; j < name.size(); ++j) {
            const QChar c = name.at(j);
            idName.append(j == 0 ? c.toLower() : c);
        }

        // optional: sanitize for QML id safety (recommended)
        idName.replace(' ', '_');
        idName.replace('-', '_');
        idName.remove(QRegularExpression("[^A-Za-z0-9_]"));
        if (idName.isEmpty() || !idName.at(0).isLetter())
            idName = QString("mat%1").arg(i);

        gltfMatQmlId.push_back(idName);
    }

    writeMeshFiles(doc.json(), access, outDirPath, baseName, gltfMatQmlId, meshSources, meshMaterialIds);

    writeQmlFiles(doc.json(), meshSources, meshMaterialIds, gltfMatQmlId);

    // Minimal: take first mesh, first primitive, read positions/indices
    if (access.meshCount() > 0) {
        for (int mi = 0; mi < access.meshCount(); ++mi) {
            const auto prims = access.meshPrimitives(mi);
            for (int pi = 0; pi < prims.size(); ++pi) {
                const QJsonObject prim = prims.at(pi).toObject(); // ✅ correct
                const auto positions = access.readPositionsFromPrimitive(prim);
                const auto indices   = access.readIndicesFromPrimitive(prim);
                m_status.append(QString("mesh=%1 prim=%2 pos=%3 idx=%4\n")
                                    .arg(mi).arg(pi).arg(positions.size()).arg(indices.size()));
            }
        }

        // for (int i = 0; i < access.meshCount(); ++i) {
        //     const auto prims = access.meshPrimitives(i);
        //     if (!prims.isEmpty()) {
        //         const QJsonObject prim = prims.at(i).toObject();


        //         const auto positions = access.readPositionsFromPrimitive(prim);
        //         const auto indices   = access.readIndicesFromPrimitive(prim);

        //         m_status.append(QString("positions: %1\n").arg(positions.size()));
        //         m_status.append(QString("indices:   %1\n").arg(indices.size()));
        //     }
        // }
    }

    const auto& errs = errors;
    for (const auto &e : errs)
        m_status.append("Warn: " + e.message + "\n");

    emit statusChanged();
}

void GXGltfConverter::writeQmlFiles(const QJsonObject &doc, const QVector<QString>& meshSources, const QVector<QVector<QString>>& meshMaterialIds, const QVector<QString>& gltfMatQmlId)
{
    const QJsonArray scenes = doc.value("scenes").toArray();
    const QJsonArray nodes  = doc.value("nodes").toArray();
    const QJsonObject extensions = doc.value("extensions").toObject();
    const auto mats = doc.value("materials").toArray();

    if (scenes.isEmpty() || nodes.isEmpty()) {
        m_status.append("Error: glTF has no scenes or nodes\n");
        emit statusChanged();
        return;
    }

    const int sceneIndex = doc.value("scene").toInt(0);
    const QJsonObject sceneObj = (sceneIndex >= 0 && sceneIndex < scenes.size())
                                     ? scenes.at(sceneIndex).toObject()
                                     : scenes.at(0).toObject();

    const QJsonArray rootNodes = sceneObj.value("nodes").toArray();

    const QString outDirPath = m_selectedDirectory.toLocalFile();
    QDir().mkpath(outDirPath);

    auto file = m_file.split("/");
    auto newFileName = file[file.size() - 1];
    QString fileName;

    if (newFileName.contains(".glb")) {
        fileName = newFileName.replace(".glb", ".qml");
    } else {
        fileName = newFileName.replace(".gltf", ".qml");
    }
    QString objectName = newFileName.replace(".qml", "");

    const QString outFile = QDir(outDirPath).filePath(fileName);
    QFile f(outFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        m_status.append("Error: failed to write " + outFile + "\n");
        emit statusChanged();
        return;
    }

    QTextStream ts(&f);
    ts.setEncoding(QStringConverter::Utf8);

    // Header
    ts << "import QtQuick\n";
    ts << "import GenesisX3D 1.0\n"; // <-- change to your actual module import
    ts << "\n";
    ts << "GXNode {\n";
    ts << "    id: node\n";
    ts << "    objectName: \"" + objectName + "\"\n";

    writeMaterials(ts, mats, gltfMatQmlId, 1, doc);

    ts << "    GXNode {\n";
    // Write root nodes
    for (const QJsonValue &v : rootNodes) {
        const int nodeIndex = v.toInt(-1);
        writeNodeRecursive(ts, nodes, meshSources, meshMaterialIds, nodeIndex, extensions, 2);
    }
    ts << "    }\n";

    ts << "}\n";

    f.close();

    m_status.append("Wrote: " + outFile + "\n");
    emit statusChanged();
}

void GXGltfConverter::writeNodeRecursive(QTextStream &ts, const QJsonArray &nodes, const QVector<QString>& meshSources, const QVector<QVector<QString>>& meshMaterialIds, int nodeIndex, const QJsonObject& extensions, int level)
{
    if (nodeIndex < 0 || nodeIndex >= nodes.size()) {
        ts << indent(level) << "// invalid node index " << nodeIndex << "\n";
        return;
    }

    const QJsonObject n = nodes.at(nodeIndex).toObject();
    const QString name = n.value("name").toString();
    const QString id = safeIdFromName(name.isEmpty() ? QString("node_%1").arg(nodeIndex) : name);
    QString safeName = name;
    safeName.replace('"', "\\\"");

    int i = 0;
    QString idSafe;
    for (auto l : id) {
        if (i == 0) idSafe.append(l.toLower());
        else idSafe.append(l);
        i++;
    }

    ts << indent(level) << "GXNode {\n";
    ts << indent(level + 1) << "id: " << idSafe << "\n";

    if (!name.isEmpty())
        ts << indent(level + 1) << "objectName: \"" << safeName << "\"\n";

    // TRS
    if (n.contains("translation"))
        ts << indent(level + 1) << "position: " << vec3ToQml(n.value("translation").toArray()) << "\n";
    if (n.contains("scale"))
        ts << indent(level + 1) << "scale: " << vec3ToQml(n.value("scale").toArray()) << "\n";
    if (n.contains("rotation"))
        ts << indent(level + 1) << "rotation: " << quatToQml(n.value("rotation").toArray()) << "\n";

    // Mesh reference (placeholder)
    if (n.contains("mesh")) {
        const int meshIndex = n.value("mesh").toInt(-1);
        ts << indent(level + 1) << "GXModel {\n";

        if (meshIndex >= 0 && meshIndex < meshSources.size()) {
            ts << indent(level + 2) << "source: \"" << meshSources[meshIndex] << "\"\n";

            const auto &mats = meshMaterialIds[meshIndex];
            if (!mats.isEmpty()) {
                ts << indent(level + 2) << "materials: [\n";
                for (int i = 0; i < mats.size(); ++i) {
                    if (i) ts << ",\n";
                    ts << indent(level + 3) << mats[i];
                }
                ts << "\n" << indent(level + 2) << "]\n";
            }
        } else {
            ts << indent(level + 2) << "// invalid mesh index " << meshIndex << "\n";
        }
        ts << indent(level + 1) << "}\n";
    }

    // Children
    const QJsonArray children = n.value("children").toArray();
    for (const QJsonValue &cv : children) {
        writeNodeRecursive(ts, nodes, meshSources, meshMaterialIds, cv.toInt(-1), extensions, level + 1);
    }

    if (n.contains("extensions")) {
        const QJsonObject ext = n.value("extensions").toObject();
        if (ext.contains("KHR_lights_punctual")) {
            const QJsonObject lights = ext.value("KHR_lights_punctual").toObject();
            const QJsonObject khR = extensions.value("KHR_lights_punctual").toObject();
            const QJsonArray lightsA = khR.value("lights").toArray();
            if (lights.value("light").toDouble() < lightsA.size()) {
                const QJsonObject lightSpecs = lightsA[lights.value("light").toDouble()].toObject();
                const QJsonArray color = lightSpecs.value("color").toArray();
                float intensity = 0;
                if (lightSpecs.contains("scale")) {
                    qDebug() << "light intensity" << lightSpecs.value("intensity").toDouble();
                } else {
                    intensity = 10;
                }
                // if type === spot: get key spot
                if (lightSpecs.value("type").toString() == "spot") {
                    QJsonObject cones = lightSpecs.value("spot").toObject();
                    const float r = float(color.at(0).toDouble());
                    const float g = float(color.at(1).toDouble());
                    const float b = float(color.at(2).toDouble());

                    float innerCone = float(cones.value("innerConeAngle").toDouble() * (360.0f / M_PI));
                    float outerCone = float(cones.value("outerConeAngle").toDouble() * (360.0f / M_PI));
                    QString innerConeAngle = QString::number(innerCone);
                    QString outerConeAngle = QString::number(outerCone);

                    ts << indent(level + 1) << "GXSpotLight {\n";
                    ts << indent(level + 2) << "color: \"" << QColor::fromRgbF(r, g, b, 1.0f).name() << "\"\n";
                    ts << indent(level + 2) << "intensity: " << QString::number(intensity) << "\n";
                    ts << indent(level + 2) << "innerConeAngle: " << innerConeAngle << "\n";
                    ts << indent(level + 2) << "outerConeAngle: " << outerConeAngle << "\n";
                    ts << indent(level + 1) << "}\n";
                } else if (lightSpecs.value("type").toString() == "point") {
                    const float r = float(color.at(0).toDouble());
                    const float g = float(color.at(1).toDouble());
                    const float b = float(color.at(2).toDouble());

                    ts << indent(level + 1) << "GXPointLight {\n";
                    ts << indent(level + 2) << "color: \"" << QColor::fromRgbF(r, g, b, 1.0f).name() << "\"\n";
                    ts << indent(level + 2) << "intensity: " << QString::number(intensity) << "\n";

                    ts << indent(level + 1) << "}\n";
                } else {
                    qDebug() << "light type" << lightSpecs.value("type");
                    qDebug() << "all light specs" << lightSpecs;
                }
            }
        }
    }

    ts << indent(level) << "}\n";
}

void GXGltfConverter::writeMaterials(QTextStream &ts, const QJsonArray &mats, const QVector<QString>& gltfMatQmlId, int level, const QJsonObject& doc)
{//qDebug() << doc;
    QString normals;
    QString normalNames;
    QString textureNames;
    QString textures;
    QString materials;
    for (int i = 0; i < mats.size(); ++i) {
        const QJsonObject ma = mats.at(i).toObject();
        // qDebug() << "\n" << ma;
        QString name = ma.value("name").toString();
        int j = 0;
        const QString idName = gltfMatQmlId.at(i);
        QString objectName;
        const auto& nms = name;
        for (auto l : nms) {
            if (j == 0) {
                objectName.append(l.toUpper());
            } else {
                objectName.append(l);
            }
            j++;
        }

        const QJsonObject pbr = ma.value("pbrMetallicRoughness").toObject();

        // ts << indent(level) << "GXPrincipledMaterial { \n";
        // ts << indent(level + 1) << "id: " << idName << "\n";
        // ts << indent(level + 1) << "objectName: \"" << objectName << "\"\n";
        materials.append(indent(level) + "GXPrincipledMaterial { \n");
        materials.append(indent(level + 1) + "id: " + idName + "\n");
        materials.append(indent(level + 1) + "objectName: \"" + objectName + "\"\n");

        const QMap<QString, QVariant> mat = gx::gx3d::utils::GXMeshHelper::materials(ma);

        for (auto it = mat.cbegin(), end = mat.cend(); it != end; ++it) {
            const QString key = it.key();
            const QVariant v = it.value();

            if (!v.isValid() || v.isNull())
                continue;

            // If it's a string, skip empty
            if (v.metaType().id() == QMetaType::QString && v.toString().trimmed().isEmpty())
                continue;

            // ts << indent(level + 1) << key << ": " << v.toString() << "\n";
            materials.append(indent(level + 1) + key + ": " + v.toString() + "\n");

            // if (!i.value().isNull() && i.value() != "") {
            //     ts << indent(level + 1) << qPrintable(i.key()) << ": " + i.value().toString() + "\n";
            // } else {
            //     qDebug() << "value null, missing?" << i.value() << "key" << qPrintable(i.key());
            // }
        }

        if (pbr.contains("baseColorTexture")) {
            const QJsonObject bct = pbr.value("baseColorTexture").toObject();
            const QJsonArray tex = doc.value("images").toArray();

            if (bct.value("index").toInt() < tex.count()) {
                const QJsonObject image = tex[bct.value("index").toInt()].toObject();
                auto ext = image.value("mimeType").toString().split("/");
                QString idName;
                int k = 0;
                for (auto n : image.value("name").toString()) {
                    if (k == 0) idName.append(n.toLower());
                    else idName.append(n);
                }

                textureNames.append(indent(level) + "property url " + image.value("name").toString() + "Img: \"textures/" + image.value("name").toString() + "." + ext[1] + "\"\n");
                textures.append(indent(level) + "GXTexture2D {\n");
                textures.append(indent(level + 1) + "id: " + idName + "Tex\n");
                textures.append(indent(level + 1) + "objectName: \"" + image.value("name").toString() + "Tex\"\n");
                textures.append(indent(level + 1) + "source: " + image.value("name").toString() + "Img\n");
                textures.append(indent(level) + "}\n");
                materials.append(indent(level + 1) + "baseColorTexture: " + idName + "Tex\n");
                // qDebug() << tex[bct.value("index").toInt()];
                // const auto binding = resolveBaseColorTextureBinding(doc, ma);
                // m_status.append(QString("mat[%1] baseColorTexture: tex=%2 img=%3 sampler=%4 wrapS=%5 wrapT=%6\n")
                //                     .arg(i)
                //                     .arg(binding.textureIndex)
                //                     .arg(binding.imageIndex)
                //                     .arg(binding.samplerIndex)
                //                     .arg(glWrapToString(binding.sampler.wrapS))
                //                     .arg(glWrapToString(binding.sampler.wrapT)));
            }
        }

        if (ma.contains("normalTexture")) {
            const QJsonObject nt = ma.value("normalTexture").toObject();
            const QJsonArray tex = doc.value("images").toArray();

            if (nt.value("index").toInt() < tex.count()) {
                const QJsonObject image = tex[nt.value("index").toInt()].toObject();
                auto ext = image.value("mimeType").toString().split("/");
                QString idName;
                int k = 0;
                for (auto n : image.value("name").toString()) {
                    if (k == 0) idName.append(n.toLower());
                    else idName.append(n);
                }

                normalNames.append(indent(level) + "property url " + image.value("name").toString() + "Img: \"textures/" + image.value("name").toString() + "." + ext[1] + "\"\n");
                normals.append(indent(level) + "GXTexture2D {\n");
                normals.append(indent(level + 1) + "id: " + idName + "Nrm\n");
                normals.append(indent(level + 1) + "objectName: \"" + image.value("name").toString() + "Nrm\"\n");
                normals.append(indent(level + 1) + "source: " + image.value("name").toString() + "Img\n");
                materials.append(indent(level + 1) + "normalScale: " + QString::number(nt.value("scale").toDouble()) + "\n");
                normals.append(indent(level) + "}\n");
                materials.append(indent(level + 1) + "normalTexture: " + idName + "Nrm\n");
            }
        }

        materials.append(indent(level + 1) + "alphaMode: GXPrincipledMaterial.");
        if (ma.contains("alphaMode")) {
            QString alphaMode;
            int k = 0;
            for (auto a : ma.value("alphaMode").toString()) {
                if (k != 0) alphaMode.append(a.toLower());
                else alphaMode.append(a);
            }
            materials.append(alphaMode);
        } else {
            materials.append("Opaque");
        }
        materials.append("\n");
        // const auto emf = ma.value("emissiveFactor").toArray();

        // if (emf.size() >= 3) {
        //     const float r = float(emf.at(0).toDouble());
        //     const float g = float(emf.at(1).toDouble());
        //     const float b = float(emf.at(2).toDouble());
        //     ts << indent(level + 1) << "emissionColor: \"" << QColor::fromRgbF(r, g, b, 1.0f).name() + "\"\n";
        // }

        // if (ma.contains("extensions")) {
        //     const auto extensions = ma.value("extensions").toObject();
        //     if (extensions.contains("KHR_materials_emissive_strength")) {
        //         QString emissionStrength = "1";
        //         const auto emissiveStrength = extensions.value("KHR_materials_emissive_strength").toObject();
        //         emissionStrength = emissiveStrength["emissiveStrength"].toString();
        //         ts << indent(level + 1) << "emissionStrength: " << emissionStrength << "\n";
        //     }
        // }

        materials.append(indent(level) + "}\n");
    }
    ts << "\n";
    ts << normalNames;
    ts << textureNames;
    ts << normals;
    ts << textures;
    ts << materials << "\n";
}

void GXGltfConverter::writeMeshFiles(const QJsonObject &docJson, gx::gx3d::render::GXGltfAccess &access, const QString &outDirPath, const QString &baseName, const QVector<QString>& gltfMatQmlId, QVector<QString> &outMeshSources, QVector<QVector<QString> > &outMeshMaterialIds)
{
    Q_UNUSED(baseName);
    QSet<QString> usedMeshNames;
    const QJsonArray meshes = docJson.value("meshes").toArray();
    const QJsonArray materials = docJson.value("materials").toArray();

    // Build a global material-id list (index aligned with glTF materials[])
    // If you already generate ids like red/white/etc manually, you can instead
    // store those in this list. For now, we auto-generate: mat0, mat1, ...
    QVector<QString> gltfMatId;
    gltfMatId.reserve(materials.size());
    for (int i = 0; i < materials.size(); ++i) {
        // If glTF material has a "name", you could map it. Safe default:
        gltfMatId.push_back(QString("mat%1").arg(i));
    }

    // Ensure meshes directory exists
    const QString meshesDir = QDir(outDirPath).filePath("meshes");
    QDir().mkpath(meshesDir);

    outMeshSources.clear();
    outMeshMaterialIds.clear();
    outMeshSources.resize(meshes.size());
    outMeshMaterialIds.resize(meshes.size());

    gx::gx3d::render::GXMeshWriter writer;

    for (int m = 0; m < meshes.size(); ++m) {
        const QJsonObject meshObj = meshes.at(m).toObject();
        QString meshName = meshObj.value("name").toString().trimmed();
        auto safeMeshName = [&](const QString &n, int fallbackIndex) {
            QString s = n;
            if (s.isEmpty())
                s = QString("mesh_%1").arg(fallbackIndex);

            s.replace(' ', '_');
            s.replace('-', '_');
            s.remove(QRegularExpression("[^A-Za-z0-9_]"));
            return s.toLower();
        };
        const QJsonArray prims = meshObj.value("primitives").toArray();

        m_status.append(QString("Mesh %1: primitives=%2\n").arg(m).arg(prims.size()));

        gx::gx3d::render::GXMeshData meshData;

        // Map: global glTF material index -> local slot in GXModel.materials[]
        QHash<int,int> globalToLocal;
        QVector<QString> localMaterialIds; // QML ids in local order

        // We'll merge primitives by concatenating vertices and indices.
        // (Simple and correct. You can optimize later.)
        for (int p = 0; p < prims.size(); ++p) {
            const QJsonObject prim = prims.at(p).toObject();

            const auto positions = access.readPositionsFromPrimitive(prim);
            m_status.append(QString("  prim %1: pos=%2\n").arg(p).arg(positions.size()));
            if (positions.isEmpty()) {
                m_status.append(QString("    WARNING: positions empty (skipping prim)\n"));
                continue;
            }

            auto indices = access.readIndicesFromPrimitive(prim);
            m_status.append(QString("    idx=%1\n").arg(indices.size()));
            if (indices.isEmpty()) {
                // Non-indexed primitive: generate 0..N-1
                indices.resize(positions.size());
                std::iota(indices.begin(), indices.end(), 0u);
            }

            const auto normals = readNormalsOrEmpty(prim, access);
            const auto uv0 = readUv0OrEmpty(prim, access);
            const auto tangents = readTangentsOrEmpty(prim, access);
            auto logUvRange = [&](const QVector<QVector2D>& uv0, int meshIdx, int primIdx) {
                if (uv0.isEmpty()) {
                    m_status.append(QString("mesh=%1 prim=%2 UV0: <empty>\n").arg(meshIdx).arg(primIdx));
                    return;
                }

                float minU =  1e9f, minV =  1e9f;
                float maxU = -1e9f, maxV = -1e9f;

                for (const auto& uv : uv0) {
                    minU = std::min(minU, uv.x());
                    minV = std::min(minV, uv.y());
                    maxU = std::max(maxU, uv.x());
                    maxV = std::max(maxV, uv.y());
                }

                m_status.append(QString("mesh=%1 prim=%2 UV0 range: U[%3..%4] V[%5..%6]\n")
                                    .arg(meshIdx).arg(primIdx)
                                    .arg(minU, 0, 'g', 6).arg(maxU, 0, 'g', 6)
                                    .arg(minV, 0, 'g', 6).arg(maxV, 0, 'g', 6));
            };
            logUvRange(uv0, m, p);
            logUv0AccessorMeta(m_status, docJson, prim, m, p);


            // Resolve material index (global glTF)
            const int globalMat = prim.value("material").toInt(-1);
            int localMatSlot = 0;
            if (globalMat >= 0 && globalMat < gltfMatQmlId.size()) {
                auto it = globalToLocal.find(globalMat);
                if (it == globalToLocal.end()) {
                    localMatSlot = localMaterialIds.size();          // slot = next index
                    globalToLocal.insert(globalMat, localMatSlot);
                    localMaterialIds.push_back(gltfMatQmlId.at(globalMat)); // ✅ ONLY push the real id
                } else {
                    localMatSlot = it.value();
                }
            } else {
                // No material specified → ensure at least one material exists
                if (localMaterialIds.isEmpty())
                    localMaterialIds.push_back("defaultMaterial");   // or gltfMatQmlId.at(0) if you prefer
                localMatSlot = 0;
            }
            // if (globalMat >= 0) {
            //     localMaterialIds.push_back(gltfMatQmlId.at(globalMat));
            //     auto it = globalToLocal.find(globalMat);
            //     if (it == globalToLocal.end()) {
            //         localMatSlot = localMaterialIds.size();
            //         globalToLocal.insert(globalMat, localMatSlot);

            //         // Use your material id list here.
            //         // If you generate "red/white/etc" in QML, align this to those.
            //         localMaterialIds.push_back(gltfMatId.at(globalMat));
            //     } else {
            //         localMatSlot = it.value();
            //     }
            // } else {
            //     // No material specified → slot 0 (or create a default later)
            //     localMatSlot = 0;
            //     if (localMaterialIds.isEmpty())
            //         localMaterialIds.push_back("mat0");
            // }

            // Vertex base offset for this primitive within merged vertex buffer
            const quint32 baseVertex = quint32(meshData.vertices.size());

            // Append vertices
            meshData.vertices.reserve(meshData.vertices.size() + positions.size());
            for (int v = 0; v < positions.size(); ++v) {
                gx::gx3d::render::GXVertex vx;
                vx.position = positions[v];
                vx.normal   = (v < normals.size()) ? normals[v] : QVector3D(0, 1, 0);

                QVector2D uv = (v < uv0.size()) ? uv0[v] : QVector2D(0, 0);

                // bake proper glTF->engine V flip
                uv.setY(1.0f - uv.y());

                // handle tiny float noise like -1.19209e-07 that would become >1.0
                // if (uv.y() < 0.0f) uv.setY(0.0f);
                // if (uv.y() > 1.0f) uv.setY(1.0f);
                auto snapEps = [](float x) {
                    constexpr float eps = 1e-6f;
                    if (std::abs(x) < eps) return 0.0f;
                    return x;
                };
                uv.setX(snapEps(uv.x()));
                uv.setY(snapEps(uv.y()));

                vx.uv0 = uv;

                m_status.append(QString("    tangents=%1 normals=%2 uv0=%3\n").arg(tangents.size()).arg(normals.size()).arg(uv0.size()));

                vx.tangent = (v < tangents.size()) ? tangents[v] : QVector4D(1, 0, 0, 1);

                meshData.vertices.push_back(vx);
            }

            // Submesh index range
            const quint32 firstIndex = quint32(meshData.indices.size());
            const quint32 indexCount = quint32(indices.size());

            // Append indices (rebased)
            meshData.indices.reserve(meshData.indices.size() + indices.size());
            const auto& idxs = indices;
            for (quint32 idx : idxs) {
                if (idx >= quint32(positions.size())) {
                    m_status.append(QString("    WARNING: index %1 out of range (pos=%2)\n").arg(idx).arg(positions.size()));
                    continue;
                }
                meshData.indices.push_back(baseVertex + idx);
            }

            gx::gx3d::render::GXSubMeshData sm;
            sm.firstIndex = firstIndex;
            sm.indexCount = indexCount;
            sm.materialIndex = quint32(localMatSlot); // ✅ local slot
            meshData.subMeshes.push_back(sm);
        }

        // Write file
        QString baseMeshName = safeMeshName(meshName, m);
        QString finalMeshName = baseMeshName;
        int suffix = 1;
        while (usedMeshNames.contains(finalMeshName)) {
            finalMeshName = baseMeshName + "_" + QString::number(suffix++);
        }
        usedMeshNames.insert(finalMeshName);
        // const QString fileName = makeMeshFileName(baseName, m);
        const QString fileName = finalMeshName + ".mesh";
        const QString absPath = QDir(meshesDir).filePath(fileName);

        auto dumpSubMeshesU32 = [&](const gx::gx3d::render::GXMeshData &md) {
            const auto *p = reinterpret_cast<const quint32*>(md.subMeshes.constData());
            const int words = md.subMeshes.size() * 3; // 3 u32 per submesh
            // m_status.append(QString("RAW subMeshes u32 (%1 words): ").arg(words));
            for (int i = 0; i < words; ++i) {
                m_status.append(QString::number(p[i]));
                m_status.append(i + 1 == words ? "\n" : ",");
            }
        };

        dumpSubMeshesU32(meshData);

        // m_status.append(QString("GXSubMesh offs: firstIndex=%1 indexCount=%2 materialIndex=%3\n")
        //                     .arg(offsetof(gx::gx3d::render::GXSubMesh, firstIndex))
        //                     .arg(offsetof(gx::gx3d::render::GXSubMesh, indexCount))
        //                     .arg(offsetof(gx::gx3d::render::GXSubMesh, materialIndex)));
        // m_status.append(QString("GXSubMesh sizeof=%1 type=%2\n")
        //                     .arg(sizeof(gx::gx3d::render::GXSubMesh))
        //                     .arg(typeid(gx::gx3d::render::GXSubMesh).name()));
        // m_status.append(QString("Mesh %1 submesh table:\n").arg(m));
        for (int s = 0; s < meshData.subMeshes.size(); ++s) {
            const auto &sm = meshData.subMeshes[s];
            m_status.append(QString("  submesh %1: firstIndex=%2 indexCount=%3 materialIndex=%4\n")
                                .arg(s)
                                .arg(sm.firstIndex)
                                .arg(sm.indexCount)
                                .arg(sm.materialIndex));
        }

        if (meshData.vertices.isEmpty() || meshData.subMeshes.isEmpty()) {
            m_status.append(QString("Skip mesh %1: empty meshData (verts=%2 submeshes=%3)\n")
                                .arg(m).arg(meshData.vertices.size()).arg(meshData.subMeshes.size()));
            emit statusChanged();
            continue;
        }

        if (isUvDegenerate(meshData)) {
            const QQuaternion rot = findFirstNodeRotationForMesh(docJson, m);
            m_status.append(QString("AutoUV: planar XY applied (degenerate UVs) (node-rotated space)\n"));
            autoUvPlanarXY_WithRotation(meshData, rot);
            // for (auto &v : meshData.vertices)
                // v.uv0 = QVector2D(0.25f, 0.75f);

        }
        // auto uvRange = [&](const gx::gx3d::render::GXMeshData& md) {
        //     float minU= 1e9f, minV= 1e9f, maxU=-1e9f, maxV=-1e9f;
        //     for (const auto& v : md.vertices) {
        //         minU = std::min(minU, v.uv0.x());
        //         minV = std::min(minV, v.uv0.y());
        //         maxU = std::max(maxU, v.uv0.x());
        //         maxV = std::max(maxV, v.uv0.y());
        //     }
        //     return QString("U[%1..%2] V[%3..%4]")
        //         .arg(minU,0,'g',6).arg(maxU,0,'g',6)
        //         .arg(minV,0,'g',6).arg(maxV,0,'g',6);
        // };

        // m_status.append("Before AutoUV: " + uvRange(meshData) + "\n");
        // autoUvPlanarXY(meshData);
        // m_status.append("After  AutoUV: " + uvRange(meshData) + "\n");


        QString err;
        if (!writer.write(meshData, absPath, &err)) {
            m_status.append(QString("Error writing mesh %1: %2\n").arg(absPath, err));
            emit statusChanged();
            continue;
        }

        // Store relative source for QML (relative to scene file folder)
        outMeshSources[m] = QString("meshes/%1").arg(fileName);
        outMeshMaterialIds[m] = localMaterialIds;

        m_status.append(QString("Wrote mesh: %1 (submeshes=%2)\n")
                            .arg(absPath).arg(meshData.subMeshes.size()));
    }
}
