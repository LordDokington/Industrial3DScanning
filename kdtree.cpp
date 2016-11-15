#include "kdtree.h"
#include <QDebug>

void KdTree::build(QVector<Vertex>& vertices)
{
    delete m_tree;
    m_tree = buildKdTree(vertices.begin(), vertices.end(), 0);
    m_vertexArrayPointer = vertices.data();
}

void KdTree::pointsInBox(const QVector3D& min, const QVector3D& max, QVector<int>& indices)
{
    if(!m_tree || !m_vertexArrayPointer) return;

    indices.clear();
    rangeQuery(min, max, indices, m_tree, 0);
    // index of pointers in array is pointeradress - pointeradress of first pointer since memory is linear
    for(int& index : indices) index = (int) ((Vertex*) index - m_vertexArrayPointer);
}

void KdTree::pointsInSphere(const QVector3D& center, const float distance, QVector<int>& indices)
{
    if(!m_tree || !m_vertexArrayPointer) return;

    indices.clear();
    QVector3D min = center - QVector3D(distance, distance, distance);
    QVector3D max = center + QVector3D(distance, distance, distance);
    rangeQuery(min, max, indices, m_tree, 0);
    for(int& index : indices) index = (int) ((Vertex*) index - m_vertexArrayPointer);

    int i=0;
    while(i < indices.length())
    {
        int index = indices[i];
        if( center.distanceToPoint(m_vertexArrayPointer[index]) > distance )
            indices.removeAt(i);
        else
            ++i;
    }
}

KdTree::KdTreeNode* KdTree::buildKdTree(Vertex* begin, Vertex* end, uint depth)
{
    unsigned int currentDimension = depth % 3;
    unsigned int numPoints = (end - begin);

    float median = 0;
    unsigned int centerPos = numPoints/2;

    if(currentDimension == 0)
    {
        std::nth_element( begin, begin + centerPos, end, sortByX );
        median = (begin + centerPos)->position.x();
    }
    else if(currentDimension == 1)
    {
        std::nth_element( begin, begin + centerPos, end, sortByY );
        median = (begin + centerPos)->position.y();
    }
    else
    {
        std::nth_element( begin, begin + centerPos, end, sortByZ );
        median = (begin + centerPos)->position.z();
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

void KdTree::rangeQuery(const QVector3D& min, const QVector3D& max, QVector<int>& vertexPtrs, KdTreeNode* node, uint depth)
{
    //qDebug() << "depth is" << depth;
    if(node == 0) return;

    unsigned int numPoints = (node->end - node->begin);
    if(numPoints == 0) return;
    else if(numPoints == 1)
    {
        Vertex point = *node->begin;
        if( inRange(point.position, min, max) ) vertexPtrs.push_back( (int) node->begin );
        return;
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
Vertex* KdTree::nearestPoint(const QVector3D& point, KdTreeNode* node, uint depth)
{
    //qDebug() << "depth is" << depth;

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
