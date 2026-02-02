// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <QQmlContext>
#include <QApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>

#include <QDebug>
#include <QIcon>
#include <QDir>

#include "utils/GXGltfConverter.h"

using namespace Qt::StringLiterals;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);



    if (argc > 1) {
        QCommandLineParser p;
        p.setApplicationDescription("Genesis-X 3D model converter");
        p.addHelpOption();

        QCommandLineOption inOpt ({"i", "gltf"}, "GLTF .glb in file", "GLB");

        p.addOption(inOpt);

        p.process(app);

        if (!p.isSet(inOpt)) {
            p.showHelp(1);
        }

        return 0;
    } else {
        GXGltfConverter* converter = new GXGltfConverter();

        app.setWindowIcon(QIcon(":/core/logo.ico"));

        QQmlApplicationEngine engine;
        QQmlContext *rootContext = engine.rootContext();

        engine.addImportPath("qrc:/");

        rootContext->setContextProperty("converter", converter);

        const QUrl url(u"qrc:/views/MainView.qml"_s);
        QObject::connect(
            &engine,
            &QQmlApplicationEngine::objectCreated,
            &app,
            [url](QObject *obj, const QUrl &objUrl) {
                if (!obj && url == objUrl)
                    QCoreApplication::exit(-1);
            },
            Qt::QueuedConnection);
        engine.load(url);

        return app.exec();
    }

}
