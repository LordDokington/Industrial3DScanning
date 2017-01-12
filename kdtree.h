#ifndef KDTREE_H
#define KDTREE_H

#include <QVector3D>
#include <QVector>
#include "vertex.h"

/*!
 * \brief utility function for point sorting
 * \param p1
 * \param p2
 * \return returns true if p1.x < p2.x, false otherwise
 */
inline bool sortByX( const QVector3D& p1, const QVector3D& p2) { return p1.x() < p2.x(); }
/*!
 * \brief utility function for point sorting
 * \param p1
 * \param p2
 * \return returns true if p1.y < p2.y, false otherwise
 */
inline bool sortByY( const QVector3D& p1, const QVector3D& p2) { return p1.y() < p2.y(); }
/*!
 * \brief utility function for point sorting
 * \param p1
 * \param p2
 * \return returns true if p1.z < p2.z, false otherwise
 */
inline bool sortByZ( const QVector3D& p1, const QVector3D& p2) { return p1.z() < p2.z(); }

/*!
 * \brief utility function for searching points in a cuboid area
 * \param point
 * \param min minimum xyz coordinates
 * \param max maximum xyz coordinates
 * \return true if point is inside or on the cuboid defined by min and max
 */
inline bool inRange(const QVector3D& point, const QVector3D& min, const QVector3D& max)
{
    return point.x() >= min.x() && point.x() <= max.x() &&
           point.y() >= min.y() && point.y() <= max.y() &&
           point.z() >= min.z() && point.z() <= max.z();
}

/*!
 * \brief The KdTree class
 * \details This class is used for efficient filter and search operations on point clouds.
 */
class KdTree
{
public:
    KdTree() {} //!< constructor - nothing actually happens here, the KdTree must be built with KdTree::build

    /*!
     * \brief build KdTree
     * \details deletes current tree, assigns point data and calls KdTree::buildKdTree to build a new tree from given vertices
     * \param vertices reference to a QVector holding colors and positions of all points
     */
    void build(QVector<Vertex>& vertices);
    ~KdTree() { delete m_tree; } //!< destructor - deletes tree contents

    /*!
     * \brief find all points in a box
     * \details finds all points within a cuboid defined by two points
     * \param min minimum xyz boundaries for search box
     * \param max maximum xyz boundaries for search box
     * \param indices offsets of points from m_vertexArrayPointer that have been found inside the box
     */
    void pointsInBox(const QVector3D& min, const QVector3D& max, QVector<int>& indices);

    /*!
     * \brief find all points in a sphere
     * \details finds all points within a sphere defined by center point and radius
     * \param center center of the sphere
     * \param distance radius of the sphere
     * \param indices offsets of points from m_vertexArrayPointer that have been found inside the box
     */
    void pointsInSphere(const QVector3D& center, const float distance, QVector<int>& indices);

    /*!
     * \brief nearestPoint
     * \details starts search for nearest neighbor to given point with depth 0
     * \param point
     * \return nearest neighbor of point
     */
    Vertex* nearestPoint(QVector3D& point); //!<

private:
    /*!
     * \brief The KdTreeNode struct
     * \details a single node of the KdTree
     */
    struct KdTreeNode
    {
        ~KdTreeNode() //!< destructor - calls child node destructors
        {
            delete leftChild;
            delete rightChild;
        }

        float median = 0; //!< median value for the KdTree split

        KdTreeNode* leftChild = 0; //!< pointer to left child node
        KdTreeNode* rightChild = 0; //!< pointer to right child node

        Vertex* begin = 0;
        Vertex* end = 0;
    };

    /*!
     * \brief build KdTree
     * \details recursively builds KdTree from vertex data
     * \param begin
     * \param end
     * \param depth
     * \return root node
     */
    KdTreeNode* buildKdTree(Vertex* begin, Vertex* end, unsigned int depth);

    /*!
     * \brief range query
     * \details recursively find points within cuboid defined by min and max points
     * \param min
     * \param max
     * \param indices
     * \param node
     * \param depth
     */
    void rangeQuery(const QVector3D& min, const QVector3D& max, QVector<int>& indices, KdTreeNode* node, uint depth);

    // FIXME: finish implementation
    //Vertex* nearestPointApprox(const QVector3D& point, KdTreeNode* node, uint depth);
    //Vertex* nearestPoint(const QVector3D& point, QVector3D& approx,  KdTreeNode* node, uint depth);
    /*!
     * \brief nearest point
     * \details recursively find nearest neighbor to given point
     * \param point
     * \param node
     * \param dist distance to current neighbor
     * \param np current nearest point
     * \param depth current search depth
     */
    void nearestPoint(const QVector3D& point, KdTreeNode* node, double& dist, Vertex *np, int depth);

    KdTreeNode* m_tree = 0; //!< pointer to root node
    Vertex* m_vertexArrayPointer = 0; //!< pointer to point data
};
#endif // KDTREE_H
