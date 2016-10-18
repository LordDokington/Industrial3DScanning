#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include <QObject>
#include <QOpenGLFunctions_4_3_Core>
#include <QQuickWindow>

#include <QOpenGLShaderProgram>

#include <QVector>
#include <QVector3D>

#include <QMatrix4x4>

class SceneRenderer : public QObject,
                      protected QOpenGLFunctions_4_3_Core
{
    Q_OBJECT

public:

    void updateRotation(float deltaX, float deltaY);

    SceneRenderer();

    ~SceneRenderer()
    {
        delete m_program;
    }

    void setViewportSize(const QSize& viewportSize)
    {
        m_viewportSize = viewportSize;
        setupProjection();
    }
    void setWindow(QQuickWindow* window) { m_window = window; }

    void setGeometryFilePath(const QString& geometryFilePath);
    QString& geometryFilePath() { return m_geometryFilePath; }
public slots:
    // plain old OpenGL paint function
    void paint();
    void init();

private:
    QSize m_viewportSize;
    QString m_geometryFilePath;
    QQuickWindow* m_window = 0;

    QVector<QVector3D> m_vertices;
    QVector<int> m_indices;

    QMatrix4x4 m_rotation;
    QMatrix4x4 m_modelview;
    QMatrix4x4 m_projection;

    QOpenGLShaderProgram* m_program = 0;
    uint m_vao = 0;
    uint m_ibo = 0;

    bool m_isGeometryInvalidated = false;

    void initVertexArrayObject();
    void initShader();

    void drawGeometry();
    void generatePointIndices();

    void setupModelView();
    void setupProjection();

    QVector3D calculateCOG(QVector<QVector3D>& vertices);
};

#endif // SCENERENDERER_H
