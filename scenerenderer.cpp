#include "scenerenderer.h"

#include <QDebug>
#include <QOpenGLContext>
#include <QQuickWindow>
#include <QOpenGLShaderProgram>

#include <QQuaternion>

#include "vertexfileloader.h"

SceneRenderer::SceneRenderer()
{
    connect(this, &QQuickItem::windowChanged, this, &SceneRenderer::handleWindowChanged);

    int pointRes = 50;
    float pointStep = 0.1f / pointRes;

    for(int i=0; i <= pointRes; ++i)
    {
        for(int j=0; j <= pointRes; ++j)
        {
            m_vertices.push_back( QVector3D(i * pointStep, j * pointStep, 0.0f) );
            m_vertices.push_back( QVector3D(i * pointStep, j * pointStep, 0.1f) );
        }
    }
    for(int i=0; i <= pointRes; ++i)
    {
        for(int j=1; j < pointRes; ++j)
        {
            m_vertices.push_back( QVector3D(0.0f, i * pointStep, j * pointStep) );
            m_vertices.push_back( QVector3D(0.1f, i * pointStep, j * pointStep) );
        }
    }
    for(int i=1; i < pointRes; ++i)
    {
        for(int j=1; j < pointRes; ++j)
        {
            m_vertices.push_back( QVector3D(i * pointStep, 0.0f, j * pointStep) );
            m_vertices.push_back( QVector3D(i * pointStep, 0.1f, j * pointStep) );
        }
    }

    generatePointIndices();
    setupModelView();

    m_vertexBufferInvalidated = true;
}

void SceneRenderer::updateRotation(float deltaX, float deltaY)
{
    QMatrix4x4 rotLeft;
    rotLeft.rotate(deltaX, 0, 1, 0);
    rotLeft.rotate(deltaY, 1, 0, 0);

    m_rotation = rotLeft * m_rotation;

    setupModelView();

    // schedule repaint for next frame
    window()->update();
}

void SceneRenderer::setGeometryFilePath(const QString& geometryFilePath)
{
    m_geometryFilePath = geometryFilePath;

    char* stringByteData = m_geometryFilePath.toLatin1().data();
    VertexFileLoader::loadVerticesFromFile(stringByteData, m_vertices);
    generatePointIndices();

    m_rotation = QMatrix4x4();
    setupModelView();

    m_vertexBufferInvalidated = true;

    // schedule repaint for next frame when loading is done
    window()->update();
}

void SceneRenderer::generatePointIndices()
{
    m_indices.clear();
    int index = 0;
    for(auto vertex : m_vertices) m_indices.push_back( index++ );
}

void SceneRenderer::setupModelView()
{
    m_modelview = m_rotation;

    QVector3D cog = calculateCOG(m_vertices);
    m_modelview.translate(-cog);

    QMatrix4x4 view;

    QVector3D eye(0, 0, 0.3f);
    QVector3D center(0, 0, 0);
    QVector3D up(0, 1.f, 0);
    view.lookAt(eye, center, up);

    m_modelview = view * m_modelview;
}

QVector3D SceneRenderer::calculateCOG(QVector<QVector3D>& vertices)
{
    QVector3D cog;
    for(auto vertex : vertices) cog += vertex;
    cog /= vertices.size();

    return cog;
}

void SceneRenderer::paint()
{
    qDebug() << "SceneRenderer: repaint scene";
    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());
    glClearColor(0, 0.18f, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program->bind();

    m_program->setUniformValue("modelview", m_modelview);
    m_program->setUniformValue("projection", m_projection);

    drawGeometry();

    // restore OpenGL state in order to not mess up QML rendering
    // (which is also done with OpenGL)
    window()->resetOpenGLState();
}

void SceneRenderer::handleWindowChanged(QQuickWindow *window)
{
    if(!window)
    {
        qWarning() << "SceneRenderer::handleWindowChanged(): window is null!";
        return;
    }

    QObject::connect(window, SIGNAL(beforeSynchronizing()), this, SLOT(synchronize()), Qt::DirectConnection);
    window->setClearBeforeRendering(false);
}

void SceneRenderer::initShader()
{
    // TODO: load shader from separate file
    m_program = new QOpenGLShaderProgram();
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                       "#version 430\n"

                                       "layout(location=0) in highp vec3 position;"

                                       "uniform highp mat4 modelview;"
                                       "uniform highp mat4 projection;"

                                       "void main()"
                                       "{"
                                       "    gl_Position = projection* modelview * vec4(position, 1.0);"
                                       "}");
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                       "#version 430\n"

                                       "void main()"
                                       "{"
                                       "    gl_FragColor = vec4(1., 1., 1., 1.);"
                                       "}");

    m_program->bindAttributeLocation("position", 0);
    if( m_program->link() ) qWarning() << "linking of shader failed!";
}

void SceneRenderer::synchronize()
{
    if( !m_glInitialized )
    {
        // needed before Qt OpenGL wrapper functions can be called
        // essentially makes use of GLUT headers obsolete
        initializeOpenGLFunctions();

        connect(window(), &QQuickWindow::beforeRendering, this, &SceneRenderer::paint, Qt::DirectConnection);

        m_glInitialized = true;
    }

    if(m_vertexBufferInvalidated)
    {
        qInfo() << "SceneRender::syncronize(): connecting beforeRendering signal";

        initVertexArrayObject();
        initShader();

        m_vertexBufferInvalidated = false;
    }
    m_viewportSize = window()->size() * window()->devicePixelRatio();

    float aspect = m_viewportSize.width() / (float) m_viewportSize.height();

    m_projection = QMatrix4x4();
    m_projection.perspective(50, aspect, 0.1f, 10.0f);
}

void SceneRenderer::drawGeometry()
{
    if(m_vertices.empty()) return;

    // bind VAO and IBO
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

    glDrawElements(GL_POINTS, m_indices.size(), GL_UNSIGNED_INT, 0);

    // unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // flush and swap buffers
    glFlush();
}

void SceneRenderer::initVertexArrayObject()
{
    if(m_vertices.empty()) return;

    // create vertex buffer object
    uint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(QVector3D), &m_vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // create index buffer object
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(int), &m_indices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // create vertex arry object
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), (char*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), (char*)12);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}
