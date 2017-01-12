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

    SceneRenderer(); //!< constructor - sets up vertex buffers and kdTree

    ~SceneRenderer()
    {
        delete m_vertexBufferPing;
        delete m_vertexBufferPong;
        delete m_program;
    }

    /*!
     * \brief centerOfGravity
     * \param vertices a list of points
     * \return average position of all points
     */
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

    /*!
     * \brief determine point cloud boundaries
     * \details determines the bounding box defined by two 3D points (min, max)
     * \param vertices point cloud
     * \param min minimum XYZ coordinates
     * \param max maximum XYZ coordinates
     */
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
    void estimateNormalsForCurrentBuffer(float planeFitRadius);

    /*!
     * \brief thinning
     * \details applies a thinning filter with a given radius - for all points, find neighbors within radius and remove them
     * \param radius
     */
    void thinning(float radius);

    /*!
     * \brief rotate
     * \details 3D rotation using a virtual rotation ball - dragging the mouse outside the ball rotates around view direction vector
     * \param x1 last known cursor X coordinate
     * \param y1 last known cursor Y coordinate
     * \param x2 current cursor X coordinate
     * \param y2 current cursor Y coordinate
     */
    void rotate(float x1, float y1, float x2, float y2);

    void setViewportSize(const QSize& viewportSize) //!< viewport setter
    {
        m_viewportSize = viewportSize;
        setupProjection();
    }

    void setWindow(QQuickWindow* window) { m_window = window; } //!< window setter

    const float mappedZDistance() //!< Z distance getter
    {
        return m_zDistance;
    }
    void setMappedZDistance(const float zDistance) //!< Z distance setter - also updates ModelView
    {
        m_zDistance = mapZDistance(zDistance);
        setupModelView();
    }

    const float pointSize() //!< point size getter
    {
        return m_pointSize;
    }
    void setPointSize(const float pointSize) //!< point size setter
    {
        m_pointSize = pointSize;
    }

    const QVector4D& vertexColor() //!< point color getter
    {
        return m_vertexColor;
    }
    void setVertexColor(const QVector4D& vertexColor) //!< point color setter
    {
        m_vertexColor = vertexColor;
    }

    void setGeometryFilePath(const QString& geometryFilePath); //!< set new file path, load point cloud file and setup geometry structures
    QString& geometryFilePath() { return m_geometryFilePath; } //!< file path getter
public slots:
    // plain old OpenGL paint function
    void paint(); //!< plain old OpenGL paint function
    void init(); //!< initializes OpenGL and shaders

private:
    QSize m_viewportSize; //!< viewport size
    QString m_geometryFilePath; //!< path for point cloud file
    QQuickWindow* m_window = 0; //!< application window

    QVector<Vertex>* m_vertexBufferPing = 0; //!< vertex buffers
    QVector<Vertex>* m_vertexBufferPong = 0; //!< vertex buffers

    QVector<int> m_indices;             //!< point indices
    QVector<int> m_highlightedIndices;  //!< indices of highlighted points
    QVector<int> m_targetPointIndices;  //!< indices of targeted points

    QMatrix4x4 m_rotation;  //!< rotation matrix
    QMatrix4x4 m_modelview; //!< modelview matrix
    QMatrix4x4 m_projection;//!< projection matrix

    QOpenGLShaderProgram* m_program = 0; //!< shader program

    VertexArrayObject m_defaultVAO;     //!< point cloud vertex array object
    VertexArrayObject m_highlightedVAO; //!< VAO for highlighted points
    VertexArrayObject m_targetPointVAO; //!< VAO for targeted points

    QVector4D m_vertexColor; //!< point color

    KdTree m_tree; //!< kdTree

    bool m_isGeometryInvalidated = false; //!< switch for invalidation of vertex buffers

    void swapVertexBuffers() //!< swaps ping pong buffers to revert filter operations
    {
        QVector<Vertex>* swap = m_vertexBufferPing;
        m_vertexBufferPing = m_vertexBufferPong;
        m_vertexBufferPong = swap;
    }

    /*!
     * \brief fittedPlaneNormal
     * \details fit a plane through given points to determine normal vector
     * \param vertices list of points
     * \return 3D normal vector
     */
    QVector3D fittedPlaneNormal(QVector<const Vertex*> vertices);

    void initVertexData(); //!< initialize VAOs
    void initShader(); //!< initialize vertex and fragment shaders

    //void drawGeometry();
    void generatePointIndices(const QVector<Vertex>& vertices,
                              QVector<int>& indices);

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

    static const float MIN_DIST; //!< minimum camera distance
    static const float MAX_DIST; //!< maximum camera distance

    void createKdTreeColoring();

    void setupModelView();
    void setupProjection();

    float m_zDistance = 0.0;

    float m_pointSize = 2.0f;
};

#endif // SCENERENDERER_H
