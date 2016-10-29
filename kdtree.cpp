#include "kdtree.h"
#include <QDebug>

KdTreeNode* buildKdTree(QVector3D* begin, QVector3D* end, unsigned int depth)
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
        median = (begin + centerPos)->x();
    }
    else
    {
        std::nth_element( begin, begin + centerPos, end, sortByZ );
        median = (begin + centerPos)->x();
    }

    KdTreeNode* childNode = new KdTreeNode; //create new node
    childNode->median = median;
    childNode->firstPoint = begin;
    childNode->firstPoint = end;

    if(numPoints > 1)
    {
        childNode->leftChild = buildKdTree(begin, begin + centerPos, depth + 1);
        childNode->rightChild = buildKdTree(begin + centerPos, end, depth + 1);
    }

    return childNode;
}

// TODO: implement
//QVector3D* nearestPoint(QVector3D* point, )
