#include "NavigationQml.h"
#include "GxRouter.h"

#include <QtQml/qqml.h>
#include <QQmlEngine>

using namespace gx::navigation;

namespace {
// constexpr auto kUri = "GenesisX.Core.Navigation";
static gx::navigation::GxRouter s_router;
}



void registerGenesisXNavigation(QQmlEngine* engine)
{
    Q_UNUSED(engine);
    qDebug() << "navigation activated";
    qmlRegisterSingletonInstance("GenesisX.Core.Navigation", 1, 0, "Router", &s_router);
}
