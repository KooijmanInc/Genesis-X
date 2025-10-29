// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "include/GenesisX/utils/SystemInfo.h"

#include <QSettings>
#include <QSysInfo>
#include <QLocale>
#include <QUuid>

/*!
 *  \struct SystemInfo
 *  \inmodule GenesisX
 *  \ingroup genesisx-core
 *  \title System information
 *  \since 6.10
 *  \brief Information about the current system.
 */
using namespace gx::utils;

/*!
 * \brief SystemInfo::ensureAppUuid
 * \return
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
    QSettings s;
    id = s.value("app/uuid").toString();
    if (id.isEmpty()) {
        id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        s.setValue("app/uuid", id);
    }
#endif
    return id;
}

/*!
 * \brief SystemInfo::operatingSystem
 * \return
 */
QString SystemInfo::operatingSystem()
{
    return QSysInfo::kernelType();
}

/*!
 * \brief SystemInfo::platform
 * \return
 */
QString SystemInfo::platform()
{
    return QSysInfo::productType();
}

/*!
 * \brief SystemInfo::systemLanguage
 * \return
 */
QString SystemInfo::systemLanguage()
{
    return QLocale::system().name().replace("_", "-");
}
