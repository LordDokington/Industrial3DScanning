#ifndef SCENERENDERERQMLWRAPPER_H
#define SCENERENDERERQMLWRAPPER_H

#include <QObject>
#include <QQuickItem>

#include "scenerenderer.h"

/*!
 * \brief The SceneRendererQMLWrapper class
 * \details connects SceneRenderer and main.qml
 */
class SceneRendererQMLWrapper : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString geometryFilePath READ geometryFilePath WRITE setGeometryFilePath)
    Q_PROPERTY(float zDistance READ zDistance WRITE setZDistance)
    Q_PROPERTY(float pointSize READ pointSize WRITE setPointSize)

    Q_PROPERTY(bool usePerVertexColor READ usePerVertexColor WRITE setUsePerVertexColor)

public:
    SceneRendererQMLWrapper()
    {
        connect(this, &QQuickItem::windowChanged, this, &SceneRendererQMLWrapper::handleWindowChanged);
    }

    const QString& geometryFilePath()
    {
        if( !m_sceneRenderer ) return QString();
        return m_sceneRenderer->geometryFilePath();
    }
    void setGeometryFilePath(const QString& geometryFilePath)
    {
        if( !m_sceneRenderer ) return;
        m_sceneRenderer->setGeometryFilePath(geometryFilePath);
    }

    const float zDistance()
    {
        if( !m_sceneRenderer ) return 0;
        return m_sceneRenderer->mappedZDistance();
    }
    void setZDistance(const float zDistance)
    {
        if( !m_sceneRenderer ) return;
        m_sceneRenderer->setMappedZDistance(zDistance);
    }

    const float pointSize()
    {
        if( !m_sceneRenderer ) return 0;
        return m_sceneRenderer->pointSize();
    }
    void setPointSize(const float pointSize)
    {
        if( !m_sceneRenderer ) return;
        m_sceneRenderer->setPointSize(pointSize);
    }

    const bool usePerVertexColor()
    {
        if( !m_sceneRenderer ) return true;
        return m_sceneRenderer->vertexColor() == QVector4D();
    }
    void setUsePerVertexColor(const bool enabled)
    {
        if( !m_sceneRenderer ) return;
        m_sceneRenderer->setVertexColor(enabled ? QVector4D() : QVector4D(1, 171.0f / 255, 51.0f / 255, 1));
    }

    Q_INVOKABLE void rotate(float x1, float y1, float x2, float y2)
    {
        if(!m_sceneRenderer) return;
        m_sceneRenderer->rotate(x1, y1, x2, y2);
    }

    Q_INVOKABLE void smoothMesh(float radius)
    {
        if(!m_sceneRenderer) return;
        m_sceneRenderer->smoothMesh(radius);
    }

    Q_INVOKABLE void undoSmooth()
    {
        if(!m_sceneRenderer) return;
        m_sceneRenderer->undoSmooth();
    }

    Q_INVOKABLE void estimateNormals(float radius)
    {
        if(!m_sceneRenderer) return;
        m_sceneRenderer->estimateNormalsForCurrentBuffer(radius);
    }

    Q_INVOKABLE void thinning(float radius)
    {
        if(!m_sceneRenderer) return;
        m_sceneRenderer->thinning(radius);
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

        m_sceneRenderer->setViewportSize(win->size());
        m_sceneRenderer->setWindow(win);
    }

private:
    SceneRenderer* m_sceneRenderer = 0;
};

#endif // SCENERENDERERQMLWRAPPER_H
