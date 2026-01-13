// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXIMAGECROPSERVICE_H
#define GXIMAGECROPSERVICE_H

#include <QObject>
#include <QRectF>
#include <QUrl>

#include <GenesisX/genesisx_global.h>

namespace gx::app::imagepicker {

class GENESISX_CORE_EXPORT GXImageCropService : public QObject
{
    Q_OBJECT

public:
    explicit GXImageCropService(QObject* parent = nullptr);

    Q_INVOKABLE QUrl normalizeForUi(const QUrl& inputFileUrl);
    Q_INVOKABLE QUrl cropAndSave(const QUrl& inputFile, const QRectF& cropRectN, int outW, int outH, const QString& format = "JPG", int quality = 90);

signals:
    void error(const QString& msg);
    void cropSaved(const QString& path);
};

}

#endif // GXIMAGECROPSERVICE_H
