#ifndef NAVIGATIONQML_H
#define NAVIGATIONQML_H

#include <QQmlEngine>

#include "GenesisX/Navigation/GxRouter.h"

namespace gx::navigation {

static GxRouter s_router;

void registerGenesisXNavigation()
{
    qmlRegisterSingletonInstance("GenesisX.Core", 1, 0, "Router", &s_router);
}

}

#endif // NAVIGATIONQML_H
