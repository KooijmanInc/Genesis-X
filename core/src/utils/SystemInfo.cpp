// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "include/GenesisX/utils/SystemInfo.h"

#include <QSettings>
#include <QSysInfo>
#include <QLocale>
#include <QUuid>

/*!
    \headerfile SystemInfo.h
    \inmodule io.genesisx.core
    \ingroup core-classes
    \title System information
    \since Qt 6.10
    \brief Information about the current system.
 */

/*!
    \qmlmodule GenesisX.SystemInfo
    \inmodule io.genesisx.core
    \title Genesis-X SystemInfo (QML)
    \since Qt 6.10
    \brief QML APIs for current system information like platform.

    Import this module to use the \l SystemInfo type:

    \code
    import GenesisX.SystemInfo 1.0
    \endcode
*/

/*!
    \qmltype SystemInfo
    \inqmlmodule io.genesisx.core
    \since Qt 6.10
    \brief Handles requests for system information.

    \section2 Example
    \qml
    import GenesisX.SystemInfo 1.0

    SystemInfo {
        onCompleted: console.log(SystemInfo.platform())
    }
    \endqml
*/

using namespace gx::utils;

/**
    \qmlmethod string SystemInfo::ensureAppUuid()

    Returns a persistant uuid to recognize known registered devices
 */
QString SystemInfo::ensureAppUuid()
{
    QSettings s;
    QString id;
#if defined(Q_OS_ANDROID)
    id = QSysInfo::bootUniqueId();
    s.setValue("app/uuid", id);
#elif defined(Q_OS_LINUX)
    id = QSysInfo::bootUniqueId();
    s.setValue("app/uuid", id);
#elif defined(Q_OS_WIN)
    id = QSysInfo::machineUniqueId();
    s.setValue("app/uuid", id);
#else
    id = s.value("app/uuid").toString();
    if (id.isEmpty()) {
        id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        s.setValue("app/uuid", id);
    }
#endif
    return id;
}

/**
 * \brief SystemInfo::operatingSystem
 * \return
 */
QString SystemInfo::operatingSystem()
{
    return QSysInfo::kernelType();
}

/**
    \qmlmethod string SystemInfo::platform()

    Returns the current platform where app is running on
 */
QString SystemInfo::platform()
{
    return QSysInfo::productType();
}

/**
    \qmlmethod string SystemInfo::systemLanguage()

    returns the system locale as "en-US"
 */
QString SystemInfo::systemLanguage()
{
    return QLocale::system().name().replace("_", "-");
}

