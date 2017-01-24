#ifndef TRETRAEHEDRONSPHERE_H
#define TRETRAEHEDRONSPHERE_H

#include <QVector>
#include <QVector3D>
#include "vertex.h"
#include "vertexarrayobject.h"

class TetrahedronSphere
{
public:
    TetrahedronSphere(int subdivision, float radius = 1.0f)
    {
        setupBuffer(subdivision, radius);
    }

    ~TetrahedronSphere()
    {
        delete m_vertexBuffer;
    }

    const QVector<Vertex>&vertices() { return *m_vertexBuffer; }

    void setRadius(float radius)
    {
        for(auto& v : *m_vertexBuffer)
        {
            v.position = v.position.normalized();
            v.position *= radius;
        }
    }

    void setPosition(QVector3D& position)
    {
        for(auto& v : *m_vertexBuffer)
        {
            v.position += position;
        }
    }

    void setupBuffer(int subdivision, float radius)
    {
        delete m_vertexBuffer;
        m_vertexBuffer = new QVector<Vertex>(12);

        QVector3D v1(-1.0f, 0.0f, -1.0f / SQRT2());
        QVector3D v2( 1.0f, 0.0f, -1.0f / SQRT2());
        QVector3D v3( 0.0f,-1.0f,  1.0f / SQRT2());
        QVector3D v4( 0.0f, 1.0f,  1.0f / SQRT2());

        v1 *= radius;
        v2 *= radius;
        v3 *= radius;
        v4 *= radius;

        m_vertexBuffer->append( v1 );
        m_vertexBuffer->append( v2 );
        m_vertexBuffer->append( v3 );

        m_vertexBuffer->append( v1 );
        m_vertexBuffer->append( v4 );
        m_vertexBuffer->append( v2 );

        m_vertexBuffer->append( v2 );
        m_vertexBuffer->append( v4 );
        m_vertexBuffer->append( v3 );

        m_vertexBuffer->append( v3 );
        m_vertexBuffer->append( v4 );
        m_vertexBuffer->append( v1 );

        subdivide(subdivision);
    }

private:
    const float SQRT2() { return 1.414213562373095f; }

    void subdivide(int subdivision)
    {
        for(int i=0; i< subdivision; ++i) subdivideOnce();
    }

    void subdivideOnce()
    {
        QVector<Vertex>* newVertexBuffer = new QVector<Vertex>();

        for(int i= 0; i< m_vertexBuffer->length(); i += 3)
        {
            QVector3D v1 = m_vertexBuffer->at(i).position;
            QVector3D v2 = m_vertexBuffer->at(i+1).position;
            QVector3D v3 = m_vertexBuffer->at(i+2).position;

            QVector3D e1 = v2 - v1;
            QVector3D e2 = v3 - v2;
            QVector3D e3 = v1 - v3;

            QVector3D v4 = v1 + e1 / 2;
            QVector3D v5 = v2 + e2 / 2;
            QVector3D v6 = v3 + e3 / 2;

            v4.normalize();
            v5.normalize();
            v6.normalize();

            float r = v1.length();

            v4 *= r;
            v5 *= r;
            v6 *= r;

            newVertexBuffer->append(v1);
            newVertexBuffer->append(v4);
            newVertexBuffer->append(v6);

            newVertexBuffer->append(v4);
            newVertexBuffer->append(v2);
            newVertexBuffer->append(v5);

            newVertexBuffer->append(v6);
            newVertexBuffer->append(v5);
            newVertexBuffer->append(v3);

            newVertexBuffer->append(v4);
            newVertexBuffer->append(v5);
            newVertexBuffer->append(v6);
        }

        delete m_vertexBuffer;
        m_vertexBuffer = newVertexBuffer;
    }

    QVector<Vertex>* m_vertexBuffer;
};

#endif // TRETRAEHEDRONSPHERE_H
