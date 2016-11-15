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
#include "utils.h"

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

    QVector3D centerOfGravity(QVector<Vertex>& vertices)
    {
        QVector3D cog;
        for(auto vertex : vertices) cog += vertex.position;
        cog /= vertices.size();

        return cog;
    }

    QVector3D colorFromGradientHSV(double index)
    {
        if     (index < 0) index = 0;
        else if(index > 1) index = 1;

        const double H (240.0*(1.0-index));
        const double hi(std::floor(H/60.0));
        const double f (H/60-hi);
        const double V (1.0);
        const double S (1.0);
        const double p (V*(1.0-S));
        const double q (V*(1.0-S*f));
        const double t (V*(1.0-S*(1-f)));

        if     (hi==1) return QVector3D(q,V,p);
        else if(hi==2) return QVector3D(p,V,t);
        else if(hi==3) return QVector3D(p,q,V);
        else if(hi==4) return QVector3D(t,p,V);
        else if(hi==5) return QVector3D(V,p,q);
        else           return QVector3D(V,t,p); }

    void pointCloudBounds(QVector<Vertex>& vertices, QVector3D& min, QVector3D& max)
    {
        if(vertices.empty()) return;

        min = vertices[0].position;
        max = vertices[0].position;
        for(auto vertex : vertices)
        {
            if(vertex.position.x() < min.x()) min.setX( vertex.position.x() );
            if(vertex.position.y() < min.y()) min.setY( vertex.position.y() );
            if(vertex.position.z() < min.z()) min.setZ( vertex.position.z() );

            if(vertex.position.x() > max.x()) max.setX( vertex.position.x() );
            if(vertex.position.y() > max.y()) max.setY( vertex.position.y() );
            if(vertex.position.z() > max.z()) max.setZ( vertex.position.z() );
        }
    }

    void rotate(float x1, float y1, float x2, float y2);

    void setViewportSize(const QSize& viewportSize)
    {
        m_viewportSize = viewportSize;
        setupProjection();
    }

    void setWindow(QQuickWindow* window) { m_window = window; }

    const float zDistance()
    {
        return m_zDistance;
    }
    void setZDistance(const float zDistance)
    {
        m_zDistance = zDistance;
        setupModelView();
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

    QVector<Vertex> m_vertices;
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
    void generatePointIndices(const QVector<Vertex>& vertices,
                              QVector<int>& indices);

    void createSelectionWithKdTree();

    void setupModelView();
    void setupProjection();

    float m_zDistance = 0.3f;

    QVector4D m_vertexColor;
};

#endif // SCENERENDERER_H
