#ifndef GXPLATFORMQML_H
#define GXPLATFORMQML_H

#include <QObject>
#include <QtQml/qqml.h>
#include "include/GenesisX/genesisx_global.h"

class QQmlEngine;

namespace gx::app::background {

class GENESISX_CORE_EXPORT GXPlatform : public QObject
{
    Q_OBJECT
    QML_SINGLETON

public:
    explicit GXPlatform(QObject* parent = nullptr): QObject{parent} {}

    Q_INVOKABLE void gx_enableBackgroundAudio();
    Q_INVOKABLE void gx_stopBackgroundServiceIfAndroid();
};


GENESISX_CORE_EXPORT void registerGenesisXBackground(QQmlEngine* engine);
}

#endif // GXPLATFORMQML_H
