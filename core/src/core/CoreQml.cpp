// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "include/GenesisX/CoreQml.h"

#include <QQmlEngine>

#include "src/utils/SystemInfoQml.h"
#include "src/navigation/NavigationQml.h"

#include "src/app/Background/GXPlatformQml.h"
// #include "src/app/Biometrics/BiometricsQml.h"
#ifndef Q_OS_WASM
#include "src/app/Cast/CastQml.h"
#endif
#include "src/app/Notifications/NotificationsQml.h"
#include "src/app/Permissions/PermissionsQml.h"

void registerGenesisXSystemInfo(QQmlEngine*);
void registerGenesisXNavigation(QQmlEngine*);

void registerGenesisXBackground(QQmlEngine*);
// void registerGenesisXBiometrics(QQmlEngine*);
#ifndef Q_OS_WASM
void registerGenesisXCast(QQmlEngine*);
#endif
void registerGenesisXNotifications(QQmlEngine*);
void registerGenesisXPermissions(QQmlEngine*);

namespace gx::core {

using Registrar = void(*)(QQmlEngine*);
struct Feature {
    QString name;
    Registrar fn;
};

inline const Feature kFeatures[] = {
    {"genesisx_app_background", &registerGenesisXBackground},
    // {"genesisx_app_biometrics", &registerGenesisXBiometrics},
#ifndef Q_OS_WASM
    {"genesisx_app_cast", &registerGenesisXCast},
#endif
    {"genesisx_app_notifications", &registerGenesisXNotifications},
    {"genesisx_app_permissions", &registerGenesisXPermissions}
};

inline QStringList gxValidFeatureKeys()
{
    QStringList out;
    out.reserve(int(std::size(kFeatures)));
    for (const auto& f : kFeatures) out << f.name;
    return out;
}

/*!
    \headerfile CoreQml.h
    \inmodule io.genesisx.core
    \title Core qml register types
    \since Qt 6.10
    \brief Register all enabled GenesisX QML modules on \a engine.

    Via \l CoreQml.h you link the different modules.
   Which modules are registered depends on your qmake QT,
   e.g. genesisx_app_notifications, genesisx_app_ab, etc.

   \section2 code snippet setting modules list in qmake:
   \code
   GX_LOADED_MODULES = $$QT
   GX_LOADED_MODULES_CSV = $$join(GX_LOADED_MODULES, ",")
   DEFINES += GX_LOADED_MODULES=\\\"$$GX_LOADED_MODULES_CSV\\\"
   \endcode

   \section2 And set in main.cpp.
   \code
   GXCore::registerEnabledQmlModules(&engine, GX_LOADED_MODULES);
   \endcode

   \section2 Optionally you can load everything by setting.
   \code
   GXCore::registerEnabledQmlModules(&engine);
   \endcode
*/

void registerEnabledQmlModules(QQmlEngine* engine, QString features)
{
    // qInfo().noquote() << "[GX] registerEnabledQmlModules called with features:"
                      // << (features.isEmpty() ? "<empty>" : features);

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

        // qInfo().noquote() << "[GX] considering feature:" << key
                          // << "take =" << (take ? "yes" : "no");

        if (take && f.fn) {
            // qInfo().noquote() << "[GX] about to register feature:" << key;
            f.fn(engine);
            // qInfo().noquote() << "[GX] registered feature:" << key;
            registered << key;
        }
        requested.remove(key);
    }
    requested.remove(QStringLiteral("all"));
    requested.remove(QStringLiteral("genesisx"));
    requested.remove(QStringLiteral("genesisx_assets"));

    qInfo().noquote() << "[GX] QML modules requested:" << include.values().join(',');
    qInfo().noquote() << "[GX] QML modules registered:" << registered.join(',');
    if (!requested.isEmpty())
        qWarning().noquote() << "[GX] Unknown QML modules ignored:" << requested.values().join(',');

    if (qEnvironmentVariableIsSet("GX_LIST_FEATURE_KEYS"))
        qInfo().noquote() << "[GX] Valid feature keys:" << gxValidFeatureKeys().join(',');
}

}
