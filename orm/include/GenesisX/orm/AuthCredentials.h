#ifndef AUTHCREDENTIALS_H
#define AUTHCREDENTIALS_H

#include "genesisx_orm_global.h"
#include <QString>

namespace gx::orm {

struct GENESISX_ORM_EXPORT AuthCredentials
{
    QString appKey;
    QString apiToken;
    QString bearerToken;
    bool hasAuth() const { return !appKey.isEmpty() || !apiToken.isEmpty() || !bearerToken.isEmpty(); }
};

}

#endif // AUTHCREDENTIALS_H
