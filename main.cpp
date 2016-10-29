#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <scenerendererqmlwrapper.h>

int main(int argc, char *argv[])
{
    //QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    // register SceneRenderer as QML type (make it available as component)
    qmlRegisterType<SceneRendererQMLWrapper>("Renderer", 1, 0, "SceneRenderer");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));

    return app.exec();
}
