// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXIMAGEPICKER_H
#define GXIMAGEPICKER_H

#include <QtQml/qqml.h>
#include <QObject>
#include <QUrl>

#include <GenesisX/genesisx_global.h>

namespace gx::app::imagepicker {

class GENESISX_CORE_EXPORT NativePhotoPicker : public QObject
{
    Q_OBJECT
    QML_SINGLETON

public:
    explicit NativePhotoPicker(QObject* parent = nullptr);

    Q_INVOKABLE void takePhoto();
    Q_INVOKABLE void pickFromGallery();

signals:
    void imageReady(const QUrl& localFile);
    void error(const QString& message);
};

}

#endif // GXIMAGEPICKER_H
