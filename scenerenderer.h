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
#include "vertex.h"

class SceneRenderer : public QObject,
                      protected QOpenGLFunctions_4_3_Core
{
    Q_OBJECT

public:
    inline float sqr(float x) { return x*x; }

    SceneRenderer();

    ~SceneRenderer()
    {
        delete m_vertexBufferPing;
        delete m_vertexBufferPong;
        delete m_program;
    }

    void smoothMesh(const float radius);
    void undoSmooth();
    void estimateNormals(float planeFitRadius);
    void thinning(float radius);

    void fitPlane();

    void rotate(float x1, float y1, float x2, float y2);

    void setViewportSize(const QSize& viewportSize)
    {
        m_viewportSize = viewportSize;
        setupProjection();
    }

    void setWindow(QQuickWindow* window) { m_window = window; }

    const float mappedZDistance()
    {
        return m_zDistance;
    }
    void setMappedZDistance(const float zDistance)
    {
        m_zDistance = mapZDistance(zDistance);
        setupModelView();
    }

    const float pointSize()
    {
        return m_pointSize;
    }
    void setPointSize(const float pointSize)
    {
        m_pointSize = pointSize;
    }

    const QVector4D& vertexColor()
    {
        return m_vertexColor;
    }
    void setVertexColor(const QVector4D& vertexColor)
    {
        m_vertexColor = vertexColor;
    }

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

    QVector<Vertex>* m_vertexBufferPing = 0;
    QVector<Vertex>* m_vertexBufferPong = 0;

    QVector<Vertex> m_planeVertexBuffer;

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

    VertexArrayObject m_planeVAO;

    QVector4D m_vertexColor;

    KdTree m_tree;

    bool m_isGeometryInvalidated = false;

    void swapVertexBuffers()
    {
        QVector<Vertex>* swap = m_vertexBufferPing;
        m_vertexBufferPing = m_vertexBufferPong;
        m_vertexBufferPong = swap;
    }

    void initVertexData();
    void initShader();

    void drawGeometry();
    void generatePointIndices(const QVector<Vertex>& vertices,
                              QVector<int>& indices);

    float mapZDistance(float t)
    {
        t = std::max(0.0f, std::min(1.0f, t));
        return (1-t) * MIN_DIST + t * MAX_DIST;
    }

    static const float MIN_DIST;
    static const float MAX_DIST;

    void setupKdTree();

    void setupModelView();
    void setupProjection();

    float m_zDistance = 0.0;
    float m_pointSize = 2.0f;
};

#endif // SCENERENDERER_H
