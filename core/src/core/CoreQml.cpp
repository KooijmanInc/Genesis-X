// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "include/GenesisX/CoreQml.h"

#include <QQmlEngine>

#include "src/utils/SystemInfoQml.h"
#include "src/navigation/NavigationQml.h"

#include "src/app/notifications/NotificationsQml.h"

void registerGenesisXSystemInfo(QQmlEngine*);
void registerGenesisXNavigation(QQmlEngine*);
void registerGenesisXNotifications(QQmlEngine*);

namespace gx::core {

using Registrar = void(*)(QQmlEngine*);
struct Feature {
    QString name;
    Registrar fn;
};

inline const Feature kFeatures[] = {
    {"genesisx_app_notifications", &registerGenesisXNotifications}
};

inline QStringList gxValidFeatureKeys()
{
    QStringList out;
    out.reserve(int(std::size(kFeatures)));
    for (const auto& f : kFeatures) out << f.name;
    return out;
}

/*!
 * \headerfile CoreQml.h
 *  \inmodule GenesisX
 *  \since 6.10
 *  \brief Register all enabled GenesisX QML modules on \a engine.
 *
 *  Which modules are registered depends on your qmake QT,
 *  e.g. genesisx_app_notifications, genesisx_app_ab, etc.
 *
 *  code snippet setting modules list in qmake:
 *  \code
 *  GX_LOADED_MODULES = $$QT
 *  GX_LOADED_MODULES_CSV = $$join(GX_LOADED_MODULES, ",")
 *  DEFINES += GX_LOADED_MODULES=\\\"$$GX_LOADED_MODULES_CSV\\\"
 *  \endcode
 *  \brief And set in main.cpp.
 *  \code
 *  GXCore::registerEnabledQmlModules(&engine, GX_LOADED_MODULES);
 *  \endcode
 *  \brief Optionally you can load everything by setting.
 *  \code
 *  GXCore::registerEnabledQmlModules(&engine);
 *  \endcode
*/
void registerEnabledQmlModules(QQmlEngine* engine, QString features)
{
    registerGenesisXSystemInfo(engine);
    registerGenesisXNavigation(engine);

    QSet<QString> include, exclude;
    if (features.isEmpty()) {
        include.insert(QStringLiteral("all"));
    } else {
        const auto parts = features.split(',', Qt::SkipEmptyParts);
        for (QString p : parts) {
            p = p.trimmed().toLower();
            if (p.startsWith(QLatin1Char('-')))
                exclude.insert(p.mid(1));
            else
                include.insert(p);
        }
        if (include.isEmpty()) include.insert(QStringLiteral("all"));
    }

    const bool takeAll = include.contains(QStringLiteral("all"));
    QStringList registered;
    QSet<QString> requested = include;

    for (const auto& f : kFeatures) {
        const QString key = f.name;
        const bool take = (takeAll || include.contains(key)) && !exclude.contains(key);
        if (take && f.fn) {
            f.fn(engine);
            registered << key;
        }
        requested.remove(key);
    }
    requested.remove(QStringLiteral("all"));

    qInfo().noquote() << "[GX] QML modules requested:" << include.values().join(',');
    qInfo().noquote() << "[GX] QML modules registered:" << registered.join(',');
    if (!requested.isEmpty())
        qWarning().noquote() << "[GX] Unknown QML modules ignored:" << requested.values().join(',');

    if (qEnvironmentVariableIsSet("GX_LIST_FEATURE_KEYS"))
        qInfo().noquote() << "[GX] Valid feature keys:" << gxValidFeatureKeys().join(',');


}

}
