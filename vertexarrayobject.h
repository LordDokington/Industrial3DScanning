#ifndef VERTEXARRAYOBJECT_H
#define VERTEXARRAYOBJECT_H

#include <QOpenGLFunctions_4_3_Core>
#include <QVector>
#include <QDebug>

#include "vertex.h"

class VertexArrayObject : protected QOpenGLFunctions_4_3_Core
{
public:
    void draw(uint primitiveType)
    {
        // bind VAO and IBO
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

        glDrawElements(primitiveType, m_indexCount, GL_UNSIGNED_INT, 0);

        // unbind
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void init(const QVector<Vertex>& vertices, const QVector<int>& indices)
    {
        if(!m_glFunctionInitialized)
        {
            initializeOpenGLFunctions();
            m_glFunctionInitialized = true;
        }

        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_ibo);
        glDeleteVertexArrays(1, &m_vao);

        if(vertices.empty() || indices.empty())
        {
            qDebug() << "VertexArrayObject::init(): vertex or index array is empty, VAO not created";
            return;
        }

        m_vertexCount = vertices.length();
        m_indexCount = indices.length();

        // create vertex buffer object
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertexCount * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // create index buffer object
        glGenBuffers(1, &m_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCount * sizeof(int), indices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // create vertex array object
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)12);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

private:
    uint m_vao = 0;
    uint m_ibo = 0;
    uint m_vbo = 0;

    uint m_vertexCount = 0;
    uint m_indexCount = 0;

    bool m_glFunctionInitialized = false;
};

#endif // VERTEXARRAYOBJECT_H
