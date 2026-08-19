// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef CONFIGIO_H
#define CONFIGIO_H

#include <GenesisX/Orm/Core/AbstractConfig.h>
#include <GenesisX/Orm/Core/genesisx_orm_global.h>
#include <GenesisX/Orm/Config/TransportConfig.h>

#include <GenesisX/Orm/Utils/Json.h>

#include <QString>

#include <cstdint>

namespace gx::orm {

struct TransportConfig;

enum class Env
{
    Production,
    Staging,
    Development
};

enum class Authentication
{
    None,
    UsernamePassword,
    Certificate
};

struct DatabaseConnection
{
    std::string name;
    std::string engine;
    std::string driver;
    std::string host;
    std::uint16_t port = 3306;
    std::string user;
    std::string pass;
    std::string charset;
    std::string ca;
    std::string client;
    std::string key;
    Authentication authentication;
};

GENESISX_ORM_EXPORT bool loadTransportConfig(const AbstractConfig* config, TransportConfig& out, const Env& env = Env::Production, const QString& language = "");

GENESISX_ORM_EXPORT void setBackend(const GXOrm::Backend& backend);
GENESISX_ORM_EXPORT void setConnections(const DatabaseConnection& connection);
// Json jsonHelper;

}

#endif // CONFIGIO_H
