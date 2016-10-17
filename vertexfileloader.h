#ifndef VERTEXFILELOADER_H
#define VERTEXFILELOADER_H

#include <QVector3D>
#include <QVector>
#include <QDebug>

#include <iostream>
#include <fstream>

class VertexFileLoader
{
public:
    static void loadVerticesFromFile(const char* filename, QVector<QVector3D>& vertices, bool append = false)
    {
        // clear buffer if append flag is not set
        if(!append) vertices.clear();

        std::ifstream file(filename, std::ios::in);
        if( file.is_open() )
        {
            float x, y, z;
            while(!file.eof())
            {
                file >> x >> y >> z;
                vertices.append( QVector3D(x, y, z) );
            }
            file.close();
        }
        else qWarning() << "could not open file " << filename;

        file.close();
    }
};

#endif // VERTEXFILELOADER_H
