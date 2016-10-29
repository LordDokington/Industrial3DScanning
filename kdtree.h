#ifndef KDTREE_H
#define KDTREE_H

#include <QVector3D>
#include <algorithm>

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

    QVector3D* firstPoint = 0;
    QVector3D* lastPoint = 0;
};

inline bool sortByX( const QVector3D& p1, const QVector3D& p2) { return p1.x() < p2.x(); }
inline bool sortByY( const QVector3D& p1, const QVector3D& p2) { return p1.y() < p2.y(); }
inline bool sortByZ( const QVector3D& p1, const QVector3D& p2) { return p1.z() < p2.z(); }

KdTreeNode* buildKdTree(QVector3D* begin, QVector3D* end, unsigned int depth);

#endif // KDTREE_H
