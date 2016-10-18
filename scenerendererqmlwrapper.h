#ifndef SCENERENDERERQMLWRAPPER_H
#define SCENERENDERERQMLWRAPPER_H

#include <QObject>
#include <QQuickItem>

#include "scenerenderer.h"

class SceneRendererQMLWrapper : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString geometryFilePath READ geometryFilePath WRITE setGeometryFilePath)

public:
    SceneRendererQMLWrapper()
    {
        connect(this, &QQuickItem::windowChanged, this, &SceneRendererQMLWrapper::handleWindowChanged);
    }

    void setGeometryFilePath(const QString& geometryFilePath)
    {
        if( !m_sceneRenderer ) return;
        m_sceneRenderer->setGeometryFilePath(geometryFilePath);
    }

    const QString& geometryFilePath()
    {
        if( !m_sceneRenderer ) return QString();
        return m_sceneRenderer->geometryFilePath();
    }

    Q_INVOKABLE void updateRotation(float deltaX, float deltaY)
    {
        if(!m_sceneRenderer) return;
        m_sceneRenderer->updateRotation(deltaX, deltaY);
    }

public slots:
    void handleWindowChanged(QQuickWindow *window)
    {
        if(!window)
        {
            qWarning() << "SceneRenderer::handleWindowChanged(): window is null!";
            return;
        }

        connect(window, SIGNAL(beforeSynchronizing()), this, SLOT(synchronize()), Qt::DirectConnection);
        connect(window, SIGNAL(sceneGraphInvalidated()), this, SLOT(cleanup()), Qt::DirectConnection);

        window->setClearBeforeRendering(false);
    }

    void cleanup()
    {
        if (m_sceneRenderer)
        {
            delete m_sceneRenderer;
            m_sceneRenderer = 0;
        }
    }

    void synchronize()
    {
        QQuickWindow* win = window();

        if (!m_sceneRenderer)
        {
            m_sceneRenderer = new SceneRenderer();
            m_sceneRenderer->init();

            connect(win, &QQuickWindow::beforeRendering, m_sceneRenderer, &SceneRenderer::paint, Qt::DirectConnection);
        }

        m_sceneRenderer->setViewportSize(win->size() * win->devicePixelRatio());
        m_sceneRenderer->setWindow(win);
    }

private:
    SceneRenderer* m_sceneRenderer = 0;
};

#endif // SCENERENDERERQMLWRAPPER_H
