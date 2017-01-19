#ifndef I3DSCANNING_UTILS_H
#define I3DSCANNING_UTILS_H

#include <QVector>
#include <iostream>
#include "vertex.h"
#include "SVD.h"

QVector3D centerOfGravity(const QVector<Vertex>& vertices)
{
    QVector3D cog;
    for(auto vertex : vertices) cog += vertex.position;
    cog /= vertices.size();

    return cog;
}

Matrix inverse3x3(Matrix& in)
{
    return in;
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

QVector3D fittedPlaneNormal(const QVector<const Vertex*> vertices)
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

void computeCovarianceMatrix3x3(const QVector<Vertex>& vertices, Matrix& M)
{
  M.resize(3, 3);
  const ptrdiff_t N(vertices.size());
  if (N<1) return;

  //compute the mean value (center) of the points cloud
  QVector3D mean = centerOfGravity(vertices);

  //Compute the entries of the (symmetric) covariance matrix
  double Mxx(0), Mxy(0), Mxz(0), Myy(0), Myz(0), Mzz(0);
  //#pragma omp parallel for reduction(+: Mxx,Mxy,Mxz,Myy,Myz,Mzz) //omp reduction enables parallel sum up of values
  for (ptrdiff_t i = 0; i<N; ++i)
  {
    const QVector3D& pt = vertices[i].position;

    //generate mean-free coorinates
    const double x1(pt.x() - mean.x());
    const double y1(pt.y() - mean.y());
    const double z1(pt.z() - mean.z());

    //Sum up the entries for the covariance matrix
    Mxx += x1*x1; Mxy += x1*y1; Mxz += x1*z1;
    Myy += y1*y1; Myz += y1*z1;
    Mzz += z1*z1;
  }

  //setting the sums to the matrix (division by N just for numerical reason if we have very large sums)
  M(0, 0) = Mxx / N; M(0, 1) = Mxy / N; M(0, 2) = Mxz / N;
  M(1, 0) = M(0, 1); M(1, 1) = Myy / N; M(1, 2) = Myz / N;
  M(2, 0) = M(0, 2); M(2, 1) = M(1, 2); M(2, 2) = Mzz / N;
}

double distancePt2Plane(const QVector3D& point, const QVector3D& pointOnPlane, const QVector3D& planeDirection)
{
  const QVector3D PQ = point - pointOnPlane;
  float distance = QVector3D::dotProduct(PQ, planeDirection);
  return distance;
}

/** @brief computes best-fit approximations.
    @param points vector of points
*/
void computeBestFitPlane(QVector<Vertex>& vertices, QVector<QVector3D>& corners, bool colorCodeDistance = false)
{
  Matrix M(3, 3);

  const QVector3D center = centerOfGravity(vertices);
  computeCovarianceMatrix3x3(vertices, M);
  SVD::computeSymmetricEigenvectors(M);

  const QVector3D ev0(M(0, 0), M(1, 0), M(2, 0)); //first column of M == Eigenvector corresponding to the largest Eigenvalue == direction of biggest variance
  const QVector3D ev1(M(0, 1), M(1, 1), M(2, 1));
  const QVector3D ev2(M(0, 2), M(1, 2), M(2, 2)); //third column of M == Eigenvector corresponding to the smallest Eigenvalue == direction of lowest variance

  //best-fit plane
  std::cout << "*** Best-fit plane ***\n";
  std::cout << "Point    : " << center.x() << ", " << center.y() << ", " << center.z() << std::endl;
  std::cout << "Direction: " << ev2.x() << ", " << ev2.y() << ", " << ev2.z() << std::endl;

  if( colorCodeDistance )
  {
      for(auto& vertex : vertices)
      {
          float dist = distancePt2Plane(vertex.position, center, ev2);
          vertex.color = colorFromGradientHSV(dist * 15);
      }
  }

  //computing the mean distance to plane
  double meanDistance = 0;
  for (size_t i = 0; i < vertices.size(); ++i)
  {
    meanDistance += std::abs(distancePt2Plane(vertices[i].position, center, ev2));
  }
  meanDistance /= vertices.size();
  std::cout << "mean distance to plane: " << meanDistance << std::endl;


  double mindist0 = +std::numeric_limits<double>::max();
  double maxdist0 = -std::numeric_limits<double>::max();
  double mindist1 = +std::numeric_limits<double>::max();
  double maxdist1 = -std::numeric_limits<double>::max();
  for (size_t i = 0; i < vertices.size(); ++i)
  {
    const double dist0 = distancePt2Plane(vertices[i].position, center, ev0);
    if (dist0 < mindist0) mindist0 = dist0;
    if (dist0 > maxdist0) maxdist0 = dist0;

    const double dist1 = distancePt2Plane(vertices[i].position, center, ev1);
    if (dist1 < mindist1) mindist1 = dist1;
    if (dist1 > maxdist1) maxdist1 = dist1;
  }

  QVector3D corner1 = center + ev0*maxdist0 + ev1*maxdist1;
  QVector3D corner2 = center + ev0*maxdist0 + ev1*mindist1;
  QVector3D corner3 = center + ev0*mindist0 + ev1*mindist1;
  QVector3D corner4 = center + ev0*mindist0 + ev1*maxdist1;

  corners.push_back(corner1);
  corners.push_back(corner2);
  corners.push_back(corner3);
  corners.push_back(corner4);
}

#endif // I3DSCANNING_UTILS_H
