#include "scenerenderer.h"

#include <QDebug>
#include <QOpenGLContext>
#include <QQuickWindow>
#include <QOpenGLShaderProgram>

#include "vertexfileloader.h"

SceneRenderer::SceneRenderer()
{
    connect(this, &QQuickItem::windowChanged, this, &SceneRenderer::handleWindowChanged);

    VertexFileLoader::loadVerticesFromFile("bin/Stanford Bunny.xyz", m_vertices);
    generatePointIndices();

    QVector3D cog = calculateCOG(m_vertices);

    m_modelview = QMatrix4x4();
    m_modelview.translate( -cog );
    QVector3D eye(0, 0, 0.5f);
    QVector3D center(0, 0, 0);
    QVector3D up(0, 1.f, 0);
    m_modelview.lookAt(eye, center, up);
}

void SceneRenderer::generatePointIndices()
{
    int index = 0;
    for(auto vertex : m_vertices) m_indices.push_back( index++ );
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
    qDebug() << "paint()";

    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());

    glClearColor(0, 0.3f, 0, 1);
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
                                       "layout(location=0) in vec3 position;"

                                       "uniform mat4 modelview;"
                                       "uniform mat4 projection;"

                                       "void main()"
                                       "{"
                                       "    gl_Position = projection* modelview * vec4(position, 1.0);"
                                       "}");
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                       "void main()"
                                       "{"
                                       "    gl_FragColor = vec4(1., 0., 0., 1.);"
                                       "}");

    m_program->bindAttributeLocation("position", 0);

    if( m_program->link() ) qWarning() << "linking of shader failed!";
}

void SceneRenderer::synchronize()
{
    qInfo() << "SceneRender::syncronize()";

    if (!m_connected) {
        qInfo() << "SceneRender::syncronize(): connecting beforeRendering signal";

        // needed before Qt OpenGL wrapper functions can be called
        // essentially makes use of GLUT headers obsolete
        initializeOpenGLFunctions();
        connect(window(), &QQuickWindow::beforeRendering, this, &SceneRenderer::paint, Qt::DirectConnection);

        initVertexArrayObject();
        initShader();

        m_connected = true;
    }
    m_viewportSize = window()->size() * window()->devicePixelRatio();

    float aspect = m_viewportSize.width() / (float) m_viewportSize.height();

    m_projection = QMatrix4x4();
    m_projection.perspective(60, aspect, 0.1f, 10.0f);
}

void SceneRenderer::drawGeometry()
{
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
