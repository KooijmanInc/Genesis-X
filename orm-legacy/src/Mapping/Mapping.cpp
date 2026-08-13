// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/Mapping.h>

#include <QSet>
#include <QMetaObject>
#include <QMetaProperty>

namespace gx::orm::mapping {

static inline QSet<QString> toSet(const QStringList& list)
{
    return QSet<QString>(list.begin(), list.end());
}

static inline QString classInfoValue(const QMetaObject* mo, const char* key)
{
    const int idx = mo->indexOfClassInfo(key);
    if (idx < 0) return {};
    return QString::fromLatin1(mo->classInfo(idx).value()).trimmed();
}

static inline QString renamedKey(const QMetaObject* mo, const QString& propName)
{
    // Convention: Q_CLASSINFO("gx.orm.rename.<prop>", "<externalName>")
    const QByteArray k = QByteArray("gx.orm.rename.") + propName.toLatin1();
    const QString v = classInfoValue(mo, k.constData());
    return v.isEmpty() ? propName : v;
}

static inline QString propNameFromExternalKey(const QMetaObject* mo, const QString& externalKey)
{
    // Reverse lookup: scan gx.orm.rename.* (simple + fine for typical entity sizes)
    for (int i = 0; i < mo->classInfoCount(); ++i) {
        const auto ci = mo->classInfo(i);
        const QString name = QString::fromLatin1(ci.name());
        if (!name.startsWith("gx.orm.rename.")) continue;

        const QString value = QString::fromLatin1(ci.value()).trimmed();
        if (value == externalKey) {
            return name.mid(QString("gx.orm.rename.").size());
        }
    }
    return externalKey;
}

static inline QStringList csvList(const QString& s)
{
    if (s.isEmpty()) return {};
    QStringList parts = s.split(',', Qt::SkipEmptyParts);
    for (auto& p : parts) p = p.trimmed();
    return parts;
}

QVariantMap toMap(const QObject *obj, const QStringList &include, const QStringList &exclude, bool includeReadOnly)
{
    QVariantMap out;
    if (!obj) return out;

    const QMetaObject* mo = obj->metaObject();
    const QSet<QString> inc = toSet(include);
    const QSet<QString> exc = toSet(exclude);

    for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
        const QMetaProperty p = mo->property(i);
        const QString prop = QString::fromLatin1(p.name());

        if (!p.isReadable()) continue;
        if (!includeReadOnly && !p.isWritable()) continue;
        if (!inc.isEmpty() && !inc.contains(prop)) continue;
        if (exc.contains(prop)) continue;

        const QString key = renamedKey(mo, prop);
        out.insert(key, p.read(obj));
    }

    return out;
}

bool fromMap(QObject *obj, const QVariantMap &map, const QStringList &include, const QStringList &exclude, bool strict)
{
    if (!obj) return false;

    const QMetaObject* mo = obj->metaObject();
    const QSet<QString> inc = toSet(include);
    const QSet<QString> exc = toSet(exclude);

    bool ok = true;

    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        const QString externalKey = it.key();
        const QString prop = propNameFromExternalKey(mo, externalKey);

        if (!inc.isEmpty() && !inc.contains(prop)) continue;
        if (exc.contains(prop)) continue;

        const QByteArray propBa = prop.toLatin1();
        const int idx = mo->indexOfProperty(propBa.constData());

        if (idx < 0) {
            if (strict) ok = false;
            continue;
        }

        const QMetaProperty p = mo->property(idx);
        if (!p.isWritable()) continue;

        QVariant v = it.value();
        if (v.isValid() && v.metaType() != p.metaType()) {
            v.convert(p.metaType()); // best-effort coercion
        }

        if (!p.write(obj, v)) {
            if (strict) ok = false;
        }
    }

    return ok;
}

QVariantMap toMapUsingClassInfo(const QObject *obj, bool includeReadOnly)
{
    if (!obj) return {};

    const QMetaObject* mo = obj->metaObject();
    const QStringList include = csvList(classInfoValue(mo, "gx.orm.include"));
    const QStringList exclude = csvList(classInfoValue(mo, "gx.orm.exclude"));

    return toMap(obj, include, exclude, includeReadOnly);
}

bool fromMapUsingClassInfo(QObject *obj, const QVariantMap &map, bool strict)
{
    if (!obj) return false;

    const QMetaObject* mo = obj->metaObject();
    const QStringList include = csvList(classInfoValue(mo, "gx.orm.include"));
    const QStringList exclude = csvList(classInfoValue(mo, "gx.orm.exclude"));

    return fromMap(obj, map, include, exclude, strict);
}

}
