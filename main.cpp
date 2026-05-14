#include "src/AppController.h"

#include <QApplication>
#include <QQmlContext>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);

    QQmlApplicationEngine engine;
    AppController appController;
    engine.rootContext()->setContextProperty("appController", &appController);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("DeskPal", "Main");

    return QApplication::exec();
}
