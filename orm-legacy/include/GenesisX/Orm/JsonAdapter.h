// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef JSONADAPTER_H
#define JSONADAPTER_H

#include <QtConcurrent/QtConcurrent>
#include <QJsonDocument>
#include <QFuture>

#include <GenesisX/Orm/ConnectionController.h>
#include <GenesisX/Orm/HttpResponse.h>

namespace gx::orm {

template <typename T>
inline QFuture<HttpResponse> postModel(ConnectionController* conn, const QString & path, const T& payload) {
    return conn->postJson(path, toJsonObject(payload));
}

template <typename T>
inline QFuture<T> getModel(ConnectionController* conn, const QString& path) {
    return conn->getJson(path).then([](const HttpResponse& r) {
        const auto obj = QJsonDocument::fromJson(r.body).object();
        return fromJsonObject<T>(obj);
    });
}

template <typename T, typename R=T>
inline QFuture<R> postmodelParse(ConnectionController* conn, const QString& path, const T& payload) {
    return conn->postJson(path, toJsonObject(payload)).then([](const HttpResponse& r) {
        const auto obj = QJsonDocument::fromJson(r.body).object();
        return fromJsonObject<R>(obj);
    });
}

}

#endif // JSONADAPTER_H
