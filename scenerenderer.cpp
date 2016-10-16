#include "scenerenderer.h"

#include <QDebug>
#include <QOpenGLContext>
#include <QQuickWindow>

#include <QOpenGLShaderProgram>

SceneRenderer::SceneRenderer()
{
    connect(this, &QQuickItem::windowChanged, this, &SceneRenderer::handleWindowChanged);

    m_vertices.push_back( QVector3D(0, 0, 0) );
    m_vertices.push_back( QVector3D(0, 1, 0) );
    m_vertices.push_back( QVector3D(1, 1, 0) );
    m_vertices.push_back( QVector3D(1, 0, 0) );

    m_indices.push_back(0);
    m_indices.push_back(1);
    m_indices.push_back(2);
    m_indices.push_back(0);
    m_indices.push_back(2);
    m_indices.push_back(3);

    m_modelview = QMatrix4x4();
    m_modelview.translate(-0.5f,-0.5f,0);
    QVector3D eye(0, 0, 1.f);
    QVector3D center(0, 0, 0);
    QVector3D up(0, 1.f, 0);
    m_modelview.lookAt(eye, center, up);
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

    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    drawGeometry();

    // restore OpenGL state in order to not mess up QML rendering
    // (which is also done with OpenGL)
    window()->resetOpenGLState();
}

void SceneRenderer::handleWindowChanged(QQuickWindow *window)
{
    if(window)
    {
        QObject::connect(window, SIGNAL(beforeSynchronizing()), this, SLOT(synchronize()), Qt::DirectConnection);
        window->setClearBeforeRendering(false);
    }
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
        //initTransformBlock();

        m_connected = true;
    }
    m_viewportSize = window()->size() * window()->devicePixelRatio();

    float aspect = m_viewportSize.width() / (float) m_viewportSize.height();

    m_projection = QMatrix4x4();
    m_projection.perspective(90, aspect, 1.0f, 1000.0f);
}

void SceneRenderer::drawGeometry()
{
    // bind VAO and IBO
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);

    // unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // flush and swap buffers
    glFlush();
}

void SceneRenderer::initVertexArrayObject()
{
    // Create vertex buffer object
    uint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(QVector3D), &m_vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create index buffer object
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(int), &m_indices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Create vertex array object
    // VAOs contain states for specific vertex data
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
        // bind vbo
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), (char*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), (char*)12);
        // unbind vbo
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

/*
void SceneRenderer::initTransformBlock()
{
    uint bindingPoint = 1;
    uint blockBuffer;
    uint blockIndex;

    uint programId = m_program->programId();

    blockIndex = glGetUniformBlockIndex(programId, "Transforms");
    glUniformBlockBinding(programId, blockIndex, bindingPoint);

    glGenBuffers(1, &blockBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, blockBuffer);

    glBufferData(GL_UNIFORM_BUFFER, 32 * sizeof(float), (float*) m_transformBlockBuffer, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, blockBuffer);
}
*/
