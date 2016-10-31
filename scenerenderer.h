#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include <QObject>
#include <QOpenGLFunctions_4_3_Core>
#include <QQuickWindow>

#include <QOpenGLShaderProgram>

#include <QVector>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>

#include "kdtree.h"
#include "vertexarrayobject.h"

class SceneRenderer : public QObject,
                      protected QOpenGLFunctions_4_3_Core
{
    Q_OBJECT

public:
    inline float sqr(float x) { return x*x; }

    SceneRenderer();

    ~SceneRenderer()
    {
        delete m_program;
    }

    void rotate(float x1, float y1, float x2, float y2);

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
    QVector3D centerOfGravity(QVector<QVector3D>& vertices);

    QSize m_viewportSize;
    QString m_geometryFilePath;
    QQuickWindow* m_window = 0;

    QVector<QVector3D> m_vertices;
    QVector<int> m_indices;
    QVector<int> m_highlightedIndices;
    QVector<int> m_targetPointIndices;

    QMatrix4x4 m_rotation;
    QMatrix4x4 m_modelview;
    QMatrix4x4 m_projection;

    QOpenGLShaderProgram* m_program = 0;

    VertexArrayObject m_defaultVAO;
    VertexArrayObject m_highlightedVAO;
    VertexArrayObject m_targetPointVAO;

    bool m_isGeometryInvalidated = false;

    void initVertexData();
    void initShader();

    void drawGeometry();
    void generatePointIndices(const QVector<QVector3D>& vertices,
                              QVector<int>& indices);

    void createSelectionWithKdTree();

    void setupModelView();
    void setupProjection();
};

#endif // SCENERENDERER_H
