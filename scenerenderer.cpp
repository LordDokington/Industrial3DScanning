#include "scenerenderer.h"

#include <QDebug>
#include <QOpenGLContext>
#include <QQuickWindow>
#include <QtMath>
#include <algorithm>
#include <omp.h>

#include "vertexfileloader.h"
#include "kdtree.h"
#include "utils.h"

const float SceneRenderer::MIN_DIST = 0.5f;
const float SceneRenderer::MAX_DIST = 5.0f;

SceneRenderer::SceneRenderer()
{
    m_vertexBufferPing = new QVector<Vertex>();
    m_vertexBufferPong = new QVector<Vertex>();

    m_zDistance = mapZDistance(0.3f);

    VertexFileLoader::cubePointCloudVertices(200, 0.1f, *m_vertexBufferPing);
    generatePointIndices(*m_vertexBufferPing, m_indices);

    createKdTreeColoring();

    setupModelView();
}

void SceneRenderer::createKdTreeColoring()
{
    m_tree.build(*m_vertexBufferPing);

    QVector3D min, max;
    pointCloudBounds(*m_vertexBufferPing, min, max);

    QVector3D center(-0.0008, 0.0666, 0.0699);
    QVector3D offset(0.01, 0.01, 0.01);

    m_tree.pointsInSphere(center, 0.01, m_highlightedIndices);
    // dont show selected vertices for now
    m_highlightedIndices.clear();
    m_targetPointIndices.clear();

    int idx = 0;
    for(auto& vertex : *m_vertexBufferPing)
    {
        vertex.color = colorFromGradientHSV( (double) idx / m_vertexBufferPing->length() );
        ++idx;
    }
}

void SceneRenderer::setGeometryFilePath(const QString& geometryFilePath)
{
    m_geometryFilePath = geometryFilePath;

    char* stringByteData = m_geometryFilePath.toLatin1().data();
    VertexFileLoader::loadVerticesFromFile(stringByteData, *m_vertexBufferPing);

    generatePointIndices(*m_vertexBufferPing, m_indices);

    createKdTreeColoring();

    // reset rotation
    m_rotation = QMatrix4x4();
    setupModelView();

    // geometry changed, hence make paint function recreate VAO
    m_isGeometryInvalidated = true;

    // schedule repaint for next frame when loading is done
    m_window->update();
}

void SceneRenderer::generatePointIndices(const QVector<Vertex>& vertices,
                                         QVector<int>& indices)
{
    indices.clear();
    int index = 0;
    for(auto vertex : vertices) indices.push_back( index++ );
}

void SceneRenderer::setupModelView()
{
    m_modelview = m_rotation;

    QVector3D cog = centerOfGravity(*m_vertexBufferPing);
    m_modelview.translate(-cog);

    QVector3D min, max;
    pointCloudBounds(*m_vertexBufferPing, min, max);
    max -= min;
    float maxScale = std::max( max.x(), std::max(max.y(), max.z()) );

    //m_modelview.translate(-cog * maxScale);

    QMatrix4x4 view;

    QVector3D eye(0, 0, m_zDistance);
    QVector3D center(0, 0, 0);
    QVector3D up(0, 1.f, 0);

    view.lookAt(eye, center, up);

    view.scale(1.0f / maxScale);

    m_modelview = view * m_modelview;
}

void SceneRenderer::setupProjection()
{
    m_projection = QMatrix4x4();
    float aspect = m_viewportSize.width() / (float) m_viewportSize.height();
    m_projection.perspective(50, aspect, 0.1f, 10.0f);
}

void SceneRenderer::paint()
{
    if( m_isGeometryInvalidated )
    {
        initVertexData();
        m_isGeometryInvalidated = false;
    }

    //qDebug() << "SceneRenderer: repaint scene";
    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());
    glClearColor(0, 0.18f, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program->bind();

    m_program->setUniformValue("modelview", m_modelview);
    m_program->setUniformValue("projection", m_projection);

    glPointSize( m_pointSize );
    // this color acts as "switch" to enable vertex coloring
    m_program->setUniformValue("color", m_vertexColor);
    m_defaultVAO.draw(GL_POINTS);

    glPointSize(4);
    m_program->setUniformValue("color", QVector4D(1, 1, 1, 1));
    m_highlightedVAO.draw(GL_POINTS);

    glPointSize(10);
    m_program->setUniformValue("color", QVector4D(1, 0, 0, 1));
    m_targetPointVAO.draw(GL_POINTS);

    // flush and swap buffers
    glFlush();

    // restore OpenGL state in order to not mess up QML rendering
    // (which is also done with OpenGL)
    m_window->resetOpenGLState();
}

void SceneRenderer::init()
{
    if( !m_program )
    {
        qDebug() << "SceneRenderer::init(): setup shader and VAO";

        // needed before Qt OpenGL wrapper functions can be called
        // essentially makes use of GLUT headers obsolete
        initializeOpenGLFunctions();
        initShader();

        m_isGeometryInvalidated = true;
    }
}

void SceneRenderer::initShader()
{
    m_program = new QOpenGLShaderProgram();
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                       "#version 430\n"

                                       "layout(location=0) in highp vec3 position;"
                                       "layout(location=1) in highp vec3 normal;"
                                       "layout(location=2) in highp vec3 color;"

                                       "uniform highp mat4 modelview;"
                                       "uniform highp mat4 projection;"

                                       "out highp vec3 vertexColor;"
                                       "out highp vec3 ws_normal;"
                                       "out highp vec3 ws_position;"

                                       "void main()"
                                       "{"
                                       "    gl_Position = projection* modelview * vec4(position, 1.0);"
                                       "    vertexColor = color;"
                                       "    ws_normal = normal;"
                                       "    ws_position = position;"
                                       "}");
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                       "#version 430\n"

                                       "uniform highp vec4 color;"
                                       "in highp vec3 vertexColor;"

                                       "in highp vec3 ws_normal;"
                                       "in highp vec3 ws_position;"

                                       "void main()"
                                       "{"
                                       "    vec3 view = normalize(-ws_position);"
                                       "    vec3 light = normalize(vec3(3.0, 3.0, 3.0) - ws_position);"
                                       "    float diffuse = max( dot(light, ws_normal), dot(light, -ws_normal));"

                                       "    vec3 halfVec = normalize(light + view);"
                                       "    float specular = pow( max(dot(halfVec, ws_normal), 0.0), 20 ) * (20.0 + 8.0) / 100.0;"

                                       "    vec4 baseColor = (color != vec4(0.0)) ? color : vec4(vertexColor, 1.0);"

                                       "    gl_FragColor = baseColor;"
                                       "    if( ws_normal != vec3(0.0) ) gl_FragColor = vec4(vec3(diffuse), 1.0) * baseColor;" //+ vec4(vec3(specular), 0.0);"

                                       "}");

    m_program->bindAttributeLocation("position", 0);
    m_program->bindAttributeLocation("normal", 1);
    m_program->bindAttributeLocation("color", 2);
    if( !m_program->link() ) qWarning() << "linking of shader failed!";
}

void SceneRenderer::initVertexData()
{
    m_defaultVAO.init(*m_vertexBufferPing, m_indices);
    m_highlightedVAO.init(*m_vertexBufferPing, m_highlightedIndices);
    m_targetPointVAO.init(*m_vertexBufferPing, m_targetPointIndices);
}

void SceneRenderer::rotate(float lastposX, float lastposY, float currposX, float currposY)
{
    //The center of our virtual rotation ball is in the center of the screen
    const float x0 = m_viewportSize.width() / 2.0f;
    const float y0 = m_viewportSize.height() / 2.0f;
    //We set the radius of rotation ball to half of the screen height
    const float r = m_viewportSize.height() / 2.0f; //ball radius is half the window height
    //const float r  = sqrt(sqr(m_winWidth) + sqr(m_winHeight)) / 2; //ball radius is half the window diagonal;
    float lastPosZ = (sqr(r) - sqr(lastposX - x0) - sqr(lastposY - y0));
    float currPosZ = (sqr(r) - sqr(currposX - x0) - sqr(currposY - y0));

    //if z is negative then we are outside the virtual ball and the rotation is just around the current view direction/z-axis
    lastPosZ = lastPosZ < 0 ? 0 : sqrt(lastPosZ);
    currPosZ = currPosZ < 0 ? 0 : sqrt(currPosZ);

    //take into account that the screen origin is in the top left corner (and not bottom left) -> x'=x-x0 and y'=y0-y
    QVector3D lastPos3d(lastposX - x0, y0 - lastposY, lastPosZ);
    QVector3D currPos3d(currposX - x0, y0 - currposY, currPosZ);
    lastPos3d.normalize(); //make unit normal vector
    currPos3d.normalize(); //make unit normal vector

    //the current mouse interaction results in this 3d rotation in camera space (unit sphere)
    QVector3D axisCS = QVector3D::crossProduct(lastPos3d, currPos3d);
    float angle = acos( QVector3D::dotProduct(lastPos3d, currPos3d) );
    QVector3D axisWS = m_rotation.transposed() * axisCS;

    m_rotation.rotate( qRadiansToDegrees(angle), axisWS);

    setupModelView();
    // schedule repaint for next frame
    m_window->update();
}

void SceneRenderer::smoothMesh(const float radius)
{
    // selection highlight will become incorrect, remove it
    m_highlightedIndices.clear();

    m_tree.build(*m_vertexBufferPing);

    QVector<int> neighbors;
    m_vertexBufferPong->clear();

    //#pragma omp parallel for
    for(const auto vertex : *m_vertexBufferPing)
    {
        m_tree.pointsInSphere(vertex.position, radius, neighbors);

        if(neighbors.empty())
        {
            m_vertexBufferPong->append(vertex);
            continue;
        }

        QVector3D meanPosition;
        double totalWeight = 0;
        for(int index: neighbors)
        {
            const QVector3D neighbor = m_vertexBufferPing->at(index).position;
            double dist = neighbor.distanceToPoint(vertex.position);
            double weight = std::exp( -dist/radius );

            meanPosition += weight * neighbor;
            totalWeight += weight;
        }

        meanPosition /= totalWeight;
        m_vertexBufferPong->append( meanPosition );
    }

    swapVertexBuffers();

    // trigger recreation of vertex buffers
    m_isGeometryInvalidated = true;
}

void SceneRenderer::undoSmooth()
{
    swapVertexBuffers();

    // trigger recreation of vertex buffers
    m_isGeometryInvalidated = true;
}

QVector3D SceneRenderer::fittedPlaneNormal(QVector<const Vertex*> vertices)
{
    int numPoints = vertices.length();

    if(numPoints < 3)
    {
        qWarning() << "At least three points are needed to fit plane";
        return QVector3D();
    }

    // center of gravity
    QVector3D cog;
    for(auto vertex : vertices) cog += vertex->position;
    cog /= numPoints;

    // calc full 3x3 covariance matrix, excluding symmetries:
    double xx = 0.0;
    double xy = 0.0;
    double xz = 0.0;
    double yy = 0.0;
    double yz = 0.0;
    double zz = 0.0;

    for(auto vertex : vertices)
    {
        QVector3D r = vertex->position - cog;
        xx += r.x() * r.x();
        xy += r.x() * r.y();
        xz += r.x() * r.z();
        yy += r.y() * r.y();
        yz += r.y() * r.z();
        zz += r.z() * r.z();
    }

    // determinants from cramers rule
    double det_x = yy*zz - yz*yz;
    double det_y = xx*zz - xz*xz;
    double det_z = xx*yy - xy*xy;

    double det_max = std::max( det_x, std::max(det_y, det_z) );
    if(det_max <= 0.0)
    {
        qWarning() << "the points do not span a plane (are collinear)";
        return QVector3D();
    }

    QVector3D normal;

    // pick largest determinant
    if(det_max == det_x)
    {
        double y = (xz*yz - xy*zz) / det_x;
        double z = (xy*yz - xz*yy) / det_x;
        normal = QVector3D(1.0, y, z);
    }
    else if(det_max == det_y)
    {
        double x = (yz*xz - xy*zz) / det_y;
        double z = (xy*xz - yz*xx) / det_y;
        normal = QVector3D(x, 1.0, z);
    }
    else
    {
        double x = (yz*xy - xz*yy) / det_z;
        double y = (xz*xy - yz*xx) / det_z;
        normal = QVector3D(x, y, 1.0);
    };

    //qDebug() << "normal is " << normal.x() << " " << normal.y() << " " << normal.z();
    return normal.normalized();
}

void SceneRenderer::estimateNormalsForCurrentBuffer(float planeFitRadius)
{
    m_tree.build(*m_vertexBufferPing);

    QVector<int> neighborIndices;
    QVector<const Vertex*> neighborReferences;
    for(auto& vertex: *m_vertexBufferPing)
    {
        m_tree.pointsInSphere(vertex.position, planeFitRadius, neighborIndices);
        // create references list
        neighborReferences.clear();
        for(int index: neighborIndices)
        {
            neighborReferences.append( &m_vertexBufferPing->at(index) );
        }

        vertex.normal = fittedPlaneNormal(neighborReferences);
    }

    m_isGeometryInvalidated = true;
}

void SceneRenderer::thinning(float radius)
{
    qDebug() << "SceneRenderer::thinning()";

    m_tree.build(*m_vertexBufferPing);

    QVector<int> neighborIndices;
    for(auto& vertex: *m_vertexBufferPing)
    {
        if( std::isnan( vertex.color.x() ) ) continue;

        m_tree.pointsInSphere(vertex.position, radius, neighborIndices);
        qDebug() << "neighbors length: " << neighborIndices.length();
        for(int index : neighborIndices)
        {
            Vertex* vertexToBeRemoved = m_vertexBufferPing->data() + index;
            // the vertex which neighbors where queried itself should not be removed
            if(&vertex == vertexToBeRemoved) continue;

            vertexToBeRemoved->color.setX( std::numeric_limits<float>::quiet_NaN() );
        }
    }

    qDebug() << "ping Buffer count before: " << m_vertexBufferPing->length();

    m_vertexBufferPong->clear();
    for(auto& vertex : *m_vertexBufferPing)
    {
        if( !std::isnan( vertex.color.x() ) ) m_vertexBufferPong->append(vertex);
    }

    qDebug() << "pong Buffer count after: " << m_vertexBufferPong->length();

    swapVertexBuffers();

    generatePointIndices(*m_vertexBufferPing, m_indices);
    m_isGeometryInvalidated = true;
}
