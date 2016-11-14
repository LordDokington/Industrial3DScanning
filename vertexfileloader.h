#ifndef VERTEXFILELOADER_H
#define VERTEXFILELOADER_H

#include <QVector3D>
#include <QVector>
#include <QDebug>

#include <iostream>
#include <fstream>
#include "vertex.h"

class VertexFileLoader
{
public:
    static void loadVerticesFromFile(const char* filename, QVector<Vertex>& vertices, bool append = false)
    {
        // clear buffer if append flag is not set
        if(!append) vertices.clear();

        std::ifstream file(filename, std::ios::in);
        if( file.is_open() )
        {
            Vertex v;
            while(!file.eof())
            {
                // read vertex coordinates
                file >> v.position[0]
                     >> v.position[1]
                     >> v.position[2];
                vertices.append( v );
            }
            file.close();
        }
        else qWarning() << "could not open file " << filename;

        file.close();
    }

    // default point cloud without file loading (quick to load)
    static void cubePointCloudVertices(int pointRes, float size, QVector<Vertex>& vertices, bool append = false)
    {
        // clear buffer if append flag is not set
        if(!append) vertices.clear();

        float pointStep = size / pointRes;

        for(int i=0; i <= pointRes; ++i)
        {
            for(int j=0; j <= pointRes; ++j)
            {
                vertices.push_back( Vertex( QVector3D(i * pointStep, j * pointStep, 0.0f) ) );
                vertices.push_back( Vertex( QVector3D(i * pointStep, j * pointStep, size) ) );
            }
        }
        for(int i=0; i <= pointRes; ++i)
        {
            for(int j=1; j < pointRes; ++j)
            {
                vertices.push_back( Vertex( QVector3D(0.0f, i * pointStep, j * pointStep) ) );
                vertices.push_back( Vertex( QVector3D(size, i * pointStep, j * pointStep) ) );
            }
        }
        for(int i=1; i < pointRes; ++i)
        {
            for(int j=1; j < pointRes; ++j)
            {
                vertices.push_back( Vertex( QVector3D(i * pointStep, 0.0f, j * pointStep) ) );
                vertices.push_back( Vertex( QVector3D(i * pointStep, size, j * pointStep) ) );
            }
        }
    }
};

#endif // VERTEXFILELOADER_H
