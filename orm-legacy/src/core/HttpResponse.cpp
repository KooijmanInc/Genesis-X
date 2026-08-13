// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "include/GenesisX/Orm/HttpResponse.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace gx::orm;

static void parseOnce(const QByteArray& bytes, bool& parsed, QJsonDocument& out, QString& err) {
    if (parsed) return;
    parsed = true;
    QJsonParseError je{};
    out = QJsonDocument::fromJson(bytes, &je);
    if (je.error != QJsonParseError::NoError) {
        err = je.errorString();
        out = QJsonDocument(); // clear
    } else {
        err.clear();
    }
}

QJsonDocument HttpResponse::jsonDoc(QString* err) const {
    parseOnce(body, _jsonParsed, _jsonDoc, _jsonErr);
    if (err) *err = _jsonErr;
    return _jsonDoc;
}

QJsonObject HttpResponse::jsonObject(QString* err) const {
    const auto d = jsonDoc(err);
    return d.isObject() ? d.object() : QJsonObject{};
}

QJsonArray HttpResponse::jsonArray(QString* err) const {
    const auto d = jsonDoc(err);
    return d.isArray() ? d.array() : QJsonArray{};
}

QVariant HttpResponse::jsonVariant(QString* err) const {
    const auto d = jsonDoc(err);
    if (d.isObject()) return d.object().toVariantMap();
    if (d.isArray())  return d.array().toVariantList();
    return {};
}
