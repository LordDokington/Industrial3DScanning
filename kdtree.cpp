#include "kdtree.h"
#include <QDebug>

KdTreeNode* buildKdTree(QVector3D* begin, QVector3D* end, uint depth)
{
    unsigned int currentDimension = depth % 3;
    unsigned int numPoints = (end - begin);

    float median = 0;
    unsigned int centerPos = numPoints/2;

    if(currentDimension == 0)
    {
        std::nth_element( begin, begin + centerPos, end, sortByX );
        median = (begin + centerPos)->x();
    }
    else if(currentDimension == 1)
    {
        std::nth_element( begin, begin + centerPos, end, sortByY );
        median = (begin + centerPos)->y();
    }
    else
    {
        std::nth_element( begin, begin + centerPos, end, sortByZ );
        median = (begin + centerPos)->z();
    }

    KdTreeNode* childNode = new KdTreeNode; //create new node
    childNode->median = median;
    childNode->begin = begin;
    childNode->end = end;

    if(numPoints > 1)
    {
        childNode->leftChild = buildKdTree(begin, begin + centerPos, depth + 1);
        childNode->rightChild = buildKdTree(begin + centerPos, end, depth + 1);
    }

    return childNode;
}

bool inRange(const QVector3D& point, const QVector3D& min, const QVector3D& max)
{
    return point.x() >= min.x() && point.x() <= max.x() &&
           point.y() >= min.y() && point.y() <= max.y() &&
           point.z() >= min.z() && point.z() <= max.z();
}

void rangeQuery(const QVector3D& min, const QVector3D& max, QVector<int>& vertexPtrs, KdTreeNode* node, uint depth)
{
    if(node == 0) return;

    unsigned int numPoints = (node->end - node->begin);
    if(numPoints == 0) return;
    else if(numPoints == 1)
    {
        QVector3D point = *node->begin;
        if( inRange(point, min, max) )
        {
            vertexPtrs.push_back((int) node->begin);
            return;
        }
    }

    unsigned int currentDimension = depth % 3;
    if(currentDimension == 0)
    {
        if(min.x() <= node->median)
            rangeQuery(min, max, vertexPtrs, node->leftChild, depth+1);
        if(max.x() > node->median)
            rangeQuery(min, max, vertexPtrs, node->rightChild, depth+1);
    }
    else if(currentDimension == 1)
    {
        if(min.y() <= node->median)
            rangeQuery(min, max, vertexPtrs, node->leftChild, depth+1);
        if(max.y() > node->median)
            rangeQuery(min, max, vertexPtrs, node->rightChild, depth+1);
    }
    else
    {
        if(min.z() <= node->median)
            rangeQuery(min, max, vertexPtrs, node->leftChild, depth+1);
        if(max.z() > node->median)
            rangeQuery(min, max, vertexPtrs, node->rightChild, depth+1);
    }
}

// TODO: fix implementation
QVector3D* nearestPoint(const QVector3D& point, KdTreeNode* node, uint depth)
{
    qDebug() << "depth is" << depth;

    if(node == 0) return 0;

    unsigned int numPoints = (node->end - node->begin);
    unsigned int currentDimension = depth % 3;

    if(numPoints == 0)
    {
        return 0;
    }
    else if(numPoints == 1)
    {
        return node->begin;
    }

    if(currentDimension == 0)
    {
        if(point.x() <= node->median)
        {
            return node->leftChild ?
                nearestPoint(point, node->leftChild, depth+1) :
                nearestPoint(point, node->rightChild, depth+1);
        }
        else
        {
            return node->rightChild ?
                nearestPoint(point, node->rightChild, depth+1) :
                nearestPoint(point, node->leftChild, depth+1);
        }
    }
    else if(currentDimension == 1)
    {
        if(point.y() <= node->median)
        {
            return node->leftChild ?
                nearestPoint(point, node->leftChild, depth+1) :
                nearestPoint(point, node->rightChild, depth+1);
        }
        else
        {
            return node->rightChild ?
                nearestPoint(point, node->rightChild, depth+1) :
                nearestPoint(point, node->leftChild, depth+1);
        }
    }
    else
    {
        if(point.z() <= node->median)
        {
            return node->leftChild ?
                nearestPoint(point, node->leftChild, depth+1) :
                nearestPoint(point, node->rightChild, depth+1);
        }
        else
        {
            return node->rightChild ?
                nearestPoint(point, node->rightChild, depth+1) :
                nearestPoint(point, node->leftChild, depth+1);
        }
    }
}
