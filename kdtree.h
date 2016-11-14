#ifndef KDTREE_H
#define KDTREE_H

#include <QVector3D>
#include <QVector>
#include <algorithm>
#include "vertex.h"

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

inline bool sortByX( const QVector3D& p1, const QVector3D& p2) { return p1.x() < p2.x(); }
inline bool sortByY( const QVector3D& p1, const QVector3D& p2) { return p1.y() < p2.y(); }
inline bool sortByZ( const QVector3D& p1, const QVector3D& p2) { return p1.z() < p2.z(); }

KdTreeNode* buildKdTree(Vertex* begin, Vertex* end, unsigned int depth);
Vertex* nearestPoint(const QVector3D& point, KdTreeNode* node, uint depth);
void rangeQuery(const QVector3D& min, const QVector3D& max, QVector<int>& vertexPtrs, KdTreeNode* node, uint depth);

#endif // KDTREE_H
