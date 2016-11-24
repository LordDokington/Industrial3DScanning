#ifndef I3DSCANNING_UTILS_H
#define I3DSCANNING_UTILS_H

#include <QVector>
#include "vertex.h"

QVector3D centerOfGravity(QVector<Vertex>& vertices)
{
    QVector3D cog;
    for(auto vertex : vertices) cog += vertex.position;
    cog /= vertices.size();

    return cog;
}

void pointCloudBounds(QVector<Vertex>& vertices, QVector3D& min, QVector3D& max)
{
    if(vertices.empty()) return;

    min = vertices[0].position;
    max = vertices[0].position;
    for(auto vertex : vertices)
    {
        if(vertex.position.x() < min.x()) min.setX( vertex.position.x() );
        if(vertex.position.y() < min.y()) min.setY( vertex.position.y() );
        if(vertex.position.z() < min.z()) min.setZ( vertex.position.z() );

        if(vertex.position.x() > max.x()) max.setX( vertex.position.x() );
        if(vertex.position.y() > max.y()) max.setY( vertex.position.y() );
        if(vertex.position.z() > max.z()) max.setZ( vertex.position.z() );
    }
}

QVector3D colorFromGradientHSV(double index)
{
    if     (index < 0) index = 0;
    else if(index > 1) index = 1;

    const double H (240.0*(1.0-index));
    const double hi(std::floor(H/60.0));
    const double f (H/60-hi);
    const double V (1.0);
    const double S (1.0);
    const double p (V*(1.0-S));
    const double q (V*(1.0-S*f));
    const double t (V*(1.0-S*(1-f)));

    if     (hi==1) return QVector3D(q,V,p);
    else if(hi==2) return QVector3D(p,V,t);
    else if(hi==3) return QVector3D(p,q,V);
    else if(hi==4) return QVector3D(t,p,V);
    else if(hi==5) return QVector3D(V,p,q);
    else           return QVector3D(V,t,p);
}

QVector3D fittedPlaneNormal(QVector<const Vertex*> vertices)
{
    int numPoints = vertices.length();

    if(numPoints < 3)
    {
        qWarning() << "At least three points are needed to fit plane";
        return QVector3D();
    }

    // center of gravity
    QVector3D cog;
    for(auto vertex : vertices) cog += vertex->position;
    cog /= numPoints;

    // calc full 3x3 covariance matrix, excluding symmetries:
    double xx = 0.0;
    double xy = 0.0;
    double xz = 0.0;
    double yy = 0.0;
    double yz = 0.0;
    double zz = 0.0;

    for(auto vertex : vertices)
    {
        QVector3D r = vertex->position - cog;
        xx += r.x() * r.x();
        xy += r.x() * r.y();
        xz += r.x() * r.z();
        yy += r.y() * r.y();
        yz += r.y() * r.z();
        zz += r.z() * r.z();
    }

    // determinants from cramers rule
    double det_x = yy*zz - yz*yz;
    double det_y = xx*zz - xz*xz;
    double det_z = xx*yy - xy*xy;

    double det_max = std::max( det_x, std::max(det_y, det_z) );
    if(det_max <= 0.0)
    {
        qWarning() << "the points do not span a plane (are collinear)";
        return QVector3D();
    }

    QVector3D normal;

    // pick largest determinant
    if(det_max == det_x)
    {
        double y = (xz*yz - xy*zz) / det_x;
        double z = (xy*yz - xz*yy) / det_x;
        normal = QVector3D(1.0, y, z);
    }
    else if(det_max == det_y)
    {
        double x = (yz*xz - xy*zz) / det_y;
        double z = (xy*xz - yz*xx) / det_y;
        normal = QVector3D(x, 1.0, z);
    }
    else
    {
        double x = (yz*xy - xz*yy) / det_z;
        double y = (xz*xy - yz*xx) / det_z;
        normal = QVector3D(x, y, 1.0);
    };

    //qDebug() << "normal is " << normal.x() << " " << normal.y() << " " << normal.z();
    return normal.normalized();
}

#endif // I3DSCANNING_UTILS_H
