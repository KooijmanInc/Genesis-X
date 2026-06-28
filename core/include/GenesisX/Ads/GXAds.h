// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXADS_H
#define GXADS_H

#include <QObject>
#include <QVariantMap>

#include <GenesisX/genesisx_global.h>

namespace gx::app::ads {

class GENESISX_CORE_EXPORT GXAds : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool initialized READ initialized NOTIFY initializedChanged)

public:
    explicit GXAds(QObject* parent = nullptr);

    Q_INVOKABLE void initialize(const QVariantMap& options = {});
    bool initialized() const;

signals:
    void initializedChanged();

private:
    bool m_initialized = false;
};

}

#endif // GXADS_H
