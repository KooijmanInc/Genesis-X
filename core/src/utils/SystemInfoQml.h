// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef SYSTEMINFOQML_H
#define SYSTEMINFOQML_H

#include <QObject>
#include <QtQml/qqml.h>
#include <QAbstractNativeEventFilter>

#include <GenesisX/genesisx_global.h>

class QQmlEngine;

namespace gx::utils {

GENESISX_CORE_EXPORT void registerGenesisXSystemInfo(QQmlEngine* engine);

class GENESISX_CORE_EXPORT SystemInfoQml final : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
    QML_SINGLETON

    Q_PROPERTY(Qt::ColorScheme colorScheme READ colorScheme NOTIFY colorSchemeChanged FINAL)
    Q_PROPERTY(bool darkMode READ darkMode NOTIFY colorSchemeChanged FINAL)

public:
    explicit SystemInfoQml(QObject* parent = nullptr);
    ~SystemInfoQml() override;

    [[nodiscard]] Qt::ColorScheme colorScheme() const;
    [[nodiscard]] bool darkMode() const;

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr* result) override;

    Q_INVOKABLE QString ensureAppUuid() const;
    Q_INVOKABLE QString operatingSystem() const;
    Q_INVOKABLE QString platform() const;
    Q_INVOKABLE QString systemLanguage() const;

signals:
    void colorSchemeChanged();

private:
    void updateColorScheme();
    [[nodiscard]] Qt::ColorScheme systemColorScheme() const;

    Qt::ColorScheme m_colorScheme = Qt::ColorScheme::Unknown;
};

}

#endif // SYSTEMINFOQML_H
