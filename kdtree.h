#ifndef KDTREE_H
#define KDTREE_H

#include <QVector3D>
#include <QVector>
#include "vertex.h"

inline bool sortByX( const QVector3D& p1, const QVector3D& p2) { return p1.x() < p2.x(); }
inline bool sortByY( const QVector3D& p1, const QVector3D& p2) { return p1.y() < p2.y(); }
inline bool sortByZ( const QVector3D& p1, const QVector3D& p2) { return p1.z() < p2.z(); }

inline bool inRange(const QVector3D& point, const QVector3D& min, const QVector3D& max)
{
    return point.x() >= min.x() && point.x() <= max.x() &&
           point.y() >= min.y() && point.y() <= max.y() &&
           point.z() >= min.z() && point.z() <= max.z();
}

class KdTree
{
public:
    KdTree() {}

    void build(QVector<Vertex>& vertices);

    ~KdTree() { delete m_tree; }

    void pointsInBox(const QVector3D& min, const QVector3D& max, QVector<int>& indices);
    void pointsInSphere(const QVector3D& center, const float distance, QVector<int>& indices);

    Vertex* nearestPoint(QVector3D& position);

private:
    struct KdTreeNode
    {
        ~KdTreeNode()
        {
            delete leftChild;
            delete rightChild;
        }

        float median = 0;

        KdTreeNode* leftChild = 0;
        KdTreeNode* rightChild = 0;

        Vertex* begin = 0;
        Vertex* end = 0;
    };

    KdTreeNode* buildKdTree(Vertex* begin, Vertex* end, unsigned int depth);
    void rangeQuery(const QVector3D& min, const QVector3D& max, QVector<int>& indices, KdTreeNode* node, const uint depth);

    // FIXME: finish implementation
    Vertex* nearestPointApprox(const QVector3D& point, KdTreeNode* node, uint depth);
    Vertex* nearestPoint(const QVector3D& point, QVector3D& approx,  KdTreeNode* node, uint depth);

    KdTreeNode* m_tree = 0;
    Vertex* m_vertexArrayPointer = 0;
};
#endif // KDTREE_H
