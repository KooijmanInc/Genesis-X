// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/ImagePicker/GXImageCropService.h>

#include <QStandardPaths>
#include <QImageReader>
#include <QImage>
#include <QDir>

namespace gx::app::imagepicker {

GXImageCropService::GXImageCropService(QObject *parent)
    : QObject{parent}
{
}

QUrl GXImageCropService::normalizeForUi(const QUrl &inputFileUrl)
{
    const QString inPath = inputFileUrl.toLocalFile();
    QImageReader r(inPath);
    r.setAutoTransform(true);              // ✅ fixes rotation
    QImage img = r.read();
    if (img.isNull()) { emit error("Failed to read image"); return {}; }

    const QString outDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDir().mkpath(outDir);
    const QString outPath = outDir + "/gx_normalized.jpg";

    // Optional: also downscale for UI (prevents “huge” memory usage)
    // If you want, cap the longer side to e.g. 2048:
    if (img.width() > 2048 || img.height() > 2048)
        img = img.scaled(2048, 2048, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    img.save(outPath, "JPG", 92);
    return QUrl::fromLocalFile(outPath);
}

QUrl GXImageCropService::cropAndSave(const QUrl &inputFile, const QRectF &cropRectN, int outW, int outH, const QString &format, int quality)
{
    const QString inPath = inputFile.toLocalFile();
    if (inPath.isEmpty()) {
        emit error("Invalid input file URL");
        return {};
    }

    QImageReader reader(inPath);
    reader.setAutoTransform(true);
    QImage img = reader.read();
    if (img.isNull()) {
        emit error("Failed to read image");
        return {};
    }

    const int W = img.width();
    const int H = img.height();

    QRectF rN = cropRectN.normalized();

    rN.setLeft(qBound(0.0, rN.left(), 1.0));
    rN.setTop(qBound(0.0, rN.top(), 1.0));
    rN.setRight(qBound(0.0, rN.right(), 1.0));
    rN.setBottom(qBound(0.0, rN.bottom(), 1.0));

    QRect rPx(qRound(rN.x() * W), qRound(rN.y() * H), qRound(rN.width() * W), qRound(rN.height() * H));

    rPx = rPx.intersected(QRect(0, 0, W, H));
    if (rPx.width() < 2 || rPx.height() < 2) {
        emit error("Crop rect too small");
        return {};
    }

    QImage cropped = img.copy(rPx).scaled(outW, outH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QString outDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDir().mkpath(outDir);

    const QString ext = format.toLower();
    const QString outPath = outDir + "/gx_crop_" + QString::number(qHash(inPath)) + "." + ext;

    if (!cropped.save(outPath, format.toUtf8().constData(), quality)) {
        emit error("Failed to save cropped image");
        return {};
    }

    return QUrl::fromLocalFile(outPath);
}

}
