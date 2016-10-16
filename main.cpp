#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <scenerenderer.h>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    qmlRegisterType<SceneRenderer>("Renderer", 1, 0, "SceneRenderer");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));

    return app.exec();
}
