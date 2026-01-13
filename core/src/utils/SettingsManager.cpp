// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <limits>

#include <GenesisX/utils/SettingsManager.h>

namespace gx::utils {

static SettingsManager* s_settingsManager = nullptr;

static constexpr const char* kMetaSchemaVersion = "meta/settingsVersion";

static inline QString joinKey(const QString& a, const QString& b)
{
    return a.endsWith('/') ? (a + b) : (a + '/' + b);
}

SettingsManager::SettingsManager(const SettingsOptions &opts, QObject *parent)
    : QObject{parent}
    , m_opts{opts}
    , m_settings{opts.format, opts.scope, opts.organization, opts.application}
{
    s_settingsManager = this;
    ensureSchemaUpToDate();
    qDebug() << "Settings file:" << m_settings.fileName();
}

int SettingsManager::schemaVersion() const
{
    return m_settings.value(QString::fromLatin1(kMetaSchemaVersion), 0).toInt();
}

bool SettingsManager::contains(const QString &key) const
{
    return m_settings.contains(deviceKey(key));
}

bool SettingsManager::remove(const QString &key)
{
    const QString finalKey = deviceKey(key);
    if (!m_settings.contains(finalKey)) return false;

    m_settings.remove(finalKey);
    emit valueChanged(finalKey);

    return true;
}

bool SettingsManager::containsUser(const QString &userId, const QString &key) const
{
    return m_settings.contains(userKey(userId, key));
}

bool SettingsManager::removeUser(const QString &userId, const QString &key)
{
    const QString finalKey = userKey(userId, key);
    if (!m_settings.contains(finalKey)) return false;

    m_settings.remove(finalKey);
    emit valueChanged(finalKey);
    return true;
}

QString SettingsManager::deviceKey(const QString &key) const
{
    return joinKey(QStringLiteral("device"), key);
}

QString SettingsManager::userKey(const QString &userId, const QString &key) const
{
    return joinKey(joinKey(QStringLiteral("users"), userId), key);
}

int SettingsManager::getInt(const QString &key, int defaultValue) const
{
    return getIntImpl(key, defaultValue, /*applyDevicePrefix*/true);
}

void SettingsManager::setInt(const QString &key, int value)
{
    setIntImpl(key, value, /*applyDevicePrefix*/true);
}

int SettingsManager::getIntImpl(const QString &key, int defaultValue, bool applyDevicePrefix) const
{
    const QString finalKey = applyDevicePrefix ? deviceKey(key) : key;

    return m_settings.value(finalKey, defaultValue).toInt();
}

void SettingsManager::setIntImpl(const QString &key, int value, bool applyDevicePrefix)
{
    const QString finalKey = applyDevicePrefix ? deviceKey(key) : key;

    if (getIntImpl(finalKey, std::numeric_limits<int>::min(), /*applyDevicePrefix*/false) == value) return;

    m_settings.setValue(finalKey, value);

    emit valueChanged(finalKey);
}

int SettingsManager::getUserInt(const QString &userId, const QString &key, int defaultValue) const
{
    return getIntImpl(userKey(userId, key), defaultValue, /*applyDevicePrefix*/false);
}

void SettingsManager::setUserInt(const QString &userId, const QString &key, int value)
{
    setIntImpl(userKey(userId, key), value, /*applyDevicePrefix*/false);
}

bool SettingsManager::getBool(const QString &key, bool defaultValue) const
{
    return getBoolImpl(key, defaultValue, /*applyDevicePrefix*/true);
}

void SettingsManager::setBool(const QString &key, bool value)
{
    setBoolImpl(key, value, /*applyDevicePrefix*/true);
}

bool SettingsManager::getBoolImpl(const QString &key, bool defaultValue, bool applyDevicePrefix) const
{
    const QString finalKey = applyDevicePrefix ? deviceKey(key) : key;

    return m_settings.value(finalKey, defaultValue).toBool();
}

void SettingsManager::setBoolImpl(const QString &key, bool value, bool applyDevicePrefix)
{
    const QString finalKey = applyDevicePrefix ? deviceKey(key) : key;

    if (getBoolImpl(finalKey, !value, /*applyDevicePrefix*/false) == value) return;

    m_settings.setValue(finalKey, value);

    emit valueChanged(finalKey);
}

bool SettingsManager::getUserBool(const QString &userId, const QString &key, bool defaultValue) const
{
    return getBoolImpl(userKey(userId, key), defaultValue, /*applyDevicePrefix*/false);
}

void SettingsManager::setUserBool(const QString &userId, const QString &key, bool value)
{
    setBoolImpl(userKey(userId, key), value, /*applyDevicePrefix*/false);
}

SettingsManager::TriState SettingsManager::getTriState(const QString &key, TriState defaultValue) const
{
    return getTriStateImpl(key, defaultValue, /*applyDevicePrefix*/true);
}

void SettingsManager::setTriState(const QString &key, TriState value)
{
    setTriStateImpl(key, value, /*applyDevicePrefix*/true);
}

SettingsManager::TriState SettingsManager::getTriStateImpl(const QString &key, TriState defaultValue, bool applyDevicePrefix) const
{
    const QString finalKey = applyDevicePrefix ? deviceKey(key) : key;

    const int v = m_settings.value(finalKey, static_cast<int>(defaultValue)).toInt();

    return sanitizeTriState(v);
}

void SettingsManager::setTriStateImpl(const QString &key, TriState value, bool applyDevicePrefix)
{
    const QString finalKey = applyDevicePrefix ? deviceKey(key) : key;
    const int v = static_cast<int>(value);

    if (m_settings.contains(finalKey) && m_settings.value(finalKey).toInt() == v) return;

    m_settings.setValue(finalKey, v);
    emit valueChanged(finalKey);
}

SettingsManager::TriState SettingsManager::getUserTriState(const QString &userId, const QString &key, TriState defaultValue) const
{
    return getTriStateImpl(userKey(userId, key), defaultValue, /*applyDevicePrefix*/false);
}

void SettingsManager::setUserTriState(const QString &userId, const QString &key, TriState value)
{
    setTriStateImpl(userKey(userId, key), value, /*applyDevicePrefix*/false);
}

SettingsManager::TriState SettingsManager::sanitizeTriState(int v)
{
    switch (v) {
    case TriState::False:
    case TriState::True:
    case TriState::Undefined:
        return static_cast<TriState>(v);
    default:
        return TriState::Undefined;
    }
}

void SettingsManager::ensureSchemaUpToDate()
{
    const int from = schemaVersion();
    const int to   = CurrentSchemaVersion;

    if (from >= to) return;
    migrate(from, to);

    m_settings.setValue(QString::fromLatin1(kMetaSchemaVersion), to);

    m_settings.sync();
}

void SettingsManager::migrate(int fromVersion, int toVersion)
{
    for (int v = fromVersion; v < toVersion; ++v) {
        switch (v) {
        case 0:
            break;
        default:
            break;
        }
    }
}

}
