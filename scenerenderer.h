#ifndef SCENERENDERER_H
#define SCENERENDERER_H

// forward declarations
class QQuickWindow;
class QOpenGLShaderProgram;

#include <QObject>
#include <QQuickItem>
#include <QOpenGLFunctions_4_3_Core>

#include <QVector>
#include <QVector3D>

#include <QMatrix4x4>

class SceneRenderer : public QQuickItem,
                      protected QOpenGLFunctions_4_3_Core
{
    Q_OBJECT

public:
    SceneRenderer();

    ~SceneRenderer()
    {
        delete m_program;
    }

public slots:
    // plain old OpenGL paint function
    void paint();

    void handleWindowChanged(QQuickWindow *window);
    void synchronize();

private:
    QOpenGLShaderProgram* m_program = 0;
    uint m_vao = 0;
    uint m_ibo = 0;
    bool m_connected = false;
    QSize m_viewportSize;

    void initVertexArrayObject();
    void initShader();

    QVector3D calculateCOG(QVector<QVector3D>& vertices);

    void drawGeometry();
    void generatePointIndices();

    QVector<QVector3D> m_vertices;
    QVector<int> m_indices;

    QMatrix4x4 m_modelview;
    QMatrix4x4 m_projection;
};

#endif // SCENERENDERER_H
