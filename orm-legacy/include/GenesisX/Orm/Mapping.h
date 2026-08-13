// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef MAPPING_H
#define MAPPING_H

#include <QObject>
#include <QVariantMap>
#include <QStringList>

#include <GenesisX/Orm/genesisx_orm_global.h>

namespace gx::orm::mapping {

GENESISX_ORM_EXPORT QVariantMap toMap(const QObject* obj, const QStringList& include = {}, const QStringList& exclude = {}, bool includeReadOnly = false);

GENESISX_ORM_EXPORT bool fromMap(QObject* obj, const QVariantMap& map, const QStringList& include = {}, const QStringList& exclude = {}, bool strict = false);

GENESISX_ORM_EXPORT QVariantMap toMapUsingClassInfo(const QObject* obj, bool includeReadOnly = false);

GENESISX_ORM_EXPORT bool fromMapUsingClassInfo(QObject* obj, const QVariantMap& map, bool strict = false);

}

#endif // MAPPING_H
