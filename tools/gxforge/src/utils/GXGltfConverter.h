// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXGLTFCONVERTER_H
#define GXGLTFCONVERTER_H

#include <GenesisX/GX3D/Render/Utils/GXGltfAccess.h>

#include <QObject>
#include <QString>
#include <QUrl>
#include <QDir>

class GXGltfConverter : public QObject
{
    Q_OBJECT

    // ─────────────────────────────
    // Directories / paths (QML-facing)
    // ─────────────────────────────
    Q_PROPERTY(QUrl currentDirectory READ currentDirectory NOTIFY currentDirectoryChanged)

    Q_PROPERTY(QUrl selectedDirectory READ selectedDirectory WRITE setSelectedDirectory NOTIFY selectedDirectoryChanged)

    Q_PROPERTY(QUrl outputFileDirectory READ outputFileDirectory WRITE setOutputFileDirectory NOTIFY outputFileDirectoryChanged)

    // ─────────────────────────────
    // Input file
    // ─────────────────────────────
    Q_PROPERTY(QUrl fileUrl READ fileUrl WRITE setFileUrl NOTIFY fileUrlChanged)

    Q_PROPERTY(QString file READ file WRITE setFile NOTIFY fileChanged)

    // ─────────────────────────────
    // Status output
    // ─────────────────────────────
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    explicit GXGltfConverter(QObject *parent = nullptr);

    // Directories
    QUrl currentDirectory() const { return m_currentDirectory; }

    QUrl selectedDirectory() const { return m_selectedDirectory; }
    void setSelectedDirectory(const QUrl &dir);

    QUrl outputFileDirectory() const { return m_outputFileDirectory; }
    void setOutputFileDirectory(const QUrl &dir);

    // Input file
    QUrl fileUrl() const { return m_fileUrl; }
    void setFileUrl(const QUrl &url);

    QString file() const { return m_file; }
    void setFile(const QString &file);

    // Status
    QString status() const { return m_status; }

    // Conversion entry point
    Q_INVOKABLE void convertToMesh();

    void writeQmlFiles(const QJsonObject& doc, const QVector<QString>& meshSources, const QVector<QVector<QString>>& meshMaterialIds, const QVector<QString>& gltfMatQmlId);
    void writeNodeRecursive(QTextStream& ts, const QJsonArray& nodes, const QVector<QString>& meshSources, const QVector<QVector<QString>>& meshMaterialIds, int nodeIndex, const QJsonObject& extensions, int level);
    void writeMaterials(QTextStream& ts, const QJsonArray& mats, const QVector<QString>& gltfMatQmlId, int level);

signals:
    void currentDirectoryChanged();
    void selectedDirectoryChanged();
    void outputFileDirectoryChanged();
    void fileUrlChanged();
    void fileChanged();
    void statusChanged();

private:
    void writeMeshFiles(const QJsonObject& docJson, gx::gx3d::render::GXGltfAccess& access, const QString& outDirPath, const QString& baseName, const QVector<QString>& gltfMatQmlId, QVector<QString>& outMeshSources, QVector<QVector<QString>>& outMeshMaterialIds);

    // Default directories
    QUrl m_currentDirectory = QUrl::fromLocalFile(QDir::currentPath());
    QUrl m_selectedDirectory = m_currentDirectory;
    QUrl m_outputFileDirectory = m_currentDirectory;

    // Input
    QUrl m_fileUrl;
    QString m_file;

    // Status buffer
    QString m_status;
};

#endif // GXGLTFCONVERTER_H
