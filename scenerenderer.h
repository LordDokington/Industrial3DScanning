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

#include "tetrahedronsphere.h"

/*!
 * \brief The SceneRenderer class
 * \details manages and renders the point cloud and provides th
 */
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
        delete m_sphere;
    }

    /*!
     * \brief smooth mesh
     * \details applies a simple smoothing filter with a given radius
     * \param radius
     */
    void smoothMesh(const float radius);

    /*!
     * \brief undo smoothing
     * \details swaps ping and pong buffers for easy reversion of the smoothing operation
     */
    void undoSmooth();

    /*!
     * \brief estimate normals
     * \details estimates normals for all points by fitting a plane through all neighbors in a given radius
     * \param planeFitRadius
     */
    void estimateNormals(float planeFitRadius);

    /*!
     * \brief thinning
     * \details applies a thinning filter with a given radius - for all points, find neighbors within radius and remove them
     * \param radius
     */
    void thinning(float radius);

    void fitPlane();

    /*!
     * \brief rotate
     * \details 3D rotation using a virtual rotation ball - dragging the mouse outside the ball rotates around view direction vector
     * \param x1 last known cursor X coordinate
     * \param y1 last known cursor Y coordinate
     * \param x2 current cursor X coordinate
     * \param y2 current cursor Y coordinate
     */
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

    const bool useSpecular()
    {
        return m_useSpecular;
    }
    void setUseSpecular(const bool specular)
    {
        m_useSpecular = specular;
    }

    const bool useDiffuse()
    {
        return m_useSpecular;
    }
    void setUseDiffuse(const bool diffuse)
    {
        m_useDiffuse = diffuse;
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
    TetrahedronSphere* m_sphere;

    VertexArrayObject m_defaultVAO;
    VertexArrayObject m_highlightedVAO;
    VertexArrayObject m_targetPointVAO;

    VertexArrayObject m_planeVAO;
    VertexArrayObject m_sphereVAO;

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

    /*!
     * \brief mapZDistance
     * \details interpolate new Z distance between MIN_DIST and MAX_DIST with factor t
     * \param t interpolation factor
     * \return new distance
     */
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

    bool m_useSpecular = true;
    bool m_useDiffuse = true;
};

#endif // SCENERENDERER_H
