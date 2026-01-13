// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QString>
#include <QObject>
#include <QSettings>

#include <GenesisX/genesisx_global.h>

namespace gx::utils {

struct SettingsOptions {
    QSettings::Format format = QSettings::IniFormat;
    QSettings::Scope scope = QSettings::UserScope;
    QString organization = QStringLiteral("KooijmanInc");
    QString application = QStringLiteral("GenesisX");
    QString customPath;
};

class GENESISX_CORE_EXPORT SettingsManager : public QObject
{
    Q_OBJECT

public:
    enum TriState : int {
        False     = 0,
        True      = 1,
        Undefined = 2
    };
    Q_ENUM(TriState);

    static constexpr int CurrentSchemaVersion = 1;

    explicit SettingsManager(const SettingsOptions& opts = {}, QObject* parent = nullptr);

    Q_INVOKABLE int schemaVersion() const;

    Q_INVOKABLE void sync() { m_settings.sync(); }

    Q_INVOKABLE bool contains(const QString& key) const;
    Q_INVOKABLE bool remove(const QString& key);

    Q_INVOKABLE bool containsUser(const QString& userId, const QString& key) const;
    Q_INVOKABLE bool removeUser(const QString& userId, const QString& key);

    Q_INVOKABLE QString deviceKey(const QString& key) const;
    Q_INVOKABLE QString userKey(const QString& userId, const QString& key) const;

    Q_INVOKABLE int  getInt(const QString& key, int defaultValue = 0) const;
    Q_INVOKABLE void setInt(const QString& key, int value);

    int  getIntImpl(const QString& key, int defaultValue = 0, bool applyDevicePrefix = true) const;
    void setIntImpl(const QString& key, int value, bool applyDevicePrefix = true);

    Q_INVOKABLE int  getUserInt(const QString& userId, const QString& key, int defaultValue = 0) const;
    Q_INVOKABLE void setUserInt(const QString& userId, const QString& key, int value);

    Q_INVOKABLE bool getBool(const QString& key, bool defaultValue = false) const;
    Q_INVOKABLE void setBool(const QString& key, bool value);

    bool getBoolImpl(const QString& key, bool defaultValue = false, bool applyDevicePrefix = true) const;
    void setBoolImpl(const QString& key, bool value, bool applyDevicePrefix = true);

    Q_INVOKABLE bool getUserBool(const QString& userId, const QString& key, bool defaultValue = false) const;
    Q_INVOKABLE void setUserBool(const QString& userId, const QString& key, bool value);

    Q_INVOKABLE TriState getTriState(const QString& key, TriState defaultValue = Undefined) const;
    Q_INVOKABLE void     setTriState(const QString& key, TriState value);

    TriState getTriStateImpl(const QString& key, TriState defaultValue = Undefined, bool applyDevicePrefix = true) const;
    void     setTriStateImpl(const QString& key, TriState value, bool applyDevicePrefix = true);

    Q_INVOKABLE TriState getUserTriState(const QString& userId, const QString& key, TriState defaultValue = Undefined) const;
    Q_INVOKABLE void     setUserTriState(const QString& userId, const QString& key, TriState value);

signals:
    void valueChanged(const QString& key);

private:
    static TriState sanitizeTriState(int v);

    void ensureSchemaUpToDate();
    void migrate(int fromVersion, int toVersion);

    SettingsOptions m_opts;
    QSettings m_settings;

    bool m_device = true;
};

}

#endif // SETTINGSMANAGER_H
