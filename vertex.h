#ifndef VERTEX_H
#define VERTEX_H

#include <QVector3D>

/*!
 * \brief The Vertex struct
 * \details holds position, normal vector and color data for a 3D point
 */
struct Vertex {
    operator QVector3D&() //!< returns position vector
    {
        return position;
    }

    /*!
     * \brief constructor
     * \details constructor - initializes color with (1,1,1)
     */
    Vertex()
    {
        color.setX(1);
        color.setY(1);
        color.setZ(1);
    }

    /*!
     * \brief constructor
     * \details constructor - initializes color with (1,1,1) and position with given position vector
     * \param position 3D vector holding point position
     */
    Vertex(const QVector3D& position): position(position)
    {
        color.setX(1);
        color.setY(1);
        color.setZ(1);
    }

    /*!
     * \brief constructor
     * \details constructor - initializes color and position with given vectors
     * \param position
     * \param color
     */
    Vertex(const QVector3D& position, const QVector3D& color): position(position), color(color)
    {}

    /*!
     * \brief constructor
     * \details constructor - initializes color, position and normal vector with given vectors
     * \param position
     * \param normal
     * \param color
     */
    Vertex(const QVector3D& position, const QVector3D& normal, const QVector3D& color):
        position(position), normal(normal), color(color)
    {}

    QVector3D position; //!< holds 3D position
    QVector3D normal;   //!< holds normal direction
    QVector3D color;    //!< holds RGB color
};

#endif // VERTEX_H
