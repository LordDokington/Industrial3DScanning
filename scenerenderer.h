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
    Q_PROPERTY(QString geometryFilePath READ geometryFilePath WRITE setGeometryFilePath)

public:

    Q_INVOKABLE void updateRotation(float deltaX, float deltaY);

    SceneRenderer();

    ~SceneRenderer()
    {
        delete m_program;
    }

    void setGeometryFilePath(const QString& geometryFilePath);

    QString& geometryFilePath() { return m_geometryFilePath; }
public slots:
    // plain old OpenGL paint function
    void paint();

    void handleWindowChanged(QQuickWindow *window);
    void synchronize();

private:
    QOpenGLShaderProgram* m_program = 0;
    uint m_vao = 0;
    uint m_ibo = 0;
    bool m_vertexBufferInvalidated = true;
    QSize m_viewportSize;

    bool m_glInitialized = false;

    QString m_geometryFilePath;

    QVector<QVector3D> m_vertices;
    QVector<int> m_indices;

    QMatrix4x4 m_rotation;
    QMatrix4x4 m_modelview;
    QMatrix4x4 m_projection;

    void initVertexArrayObject();
    void initShader();
    void setupModelView();
    void drawGeometry();
    void generatePointIndices();

    QVector3D calculateCOG(QVector<QVector3D>& vertices);
};

#endif // SCENERENDERER_H
