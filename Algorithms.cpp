#include "Algorithms.h"
#include "SVD.h"

/** @brief Computes and returns the center of the point cloud.
@param points vector of points
*/
Point3d computeCenter(const std::vector<Point3d>& points)
{
  //compute the mean value (center) of the points cloud
  Point3d mean(0, 0, 0);

  const size_t N = points.size();
  if (N < 1) return mean; //an empty point cloud gets (0,0,0) as the center

  for (size_t i = 0; i < N; ++i)
  {
    mean += points[i];
  }
  mean *= 1.0 / N;

  return mean;
}

/** @brief computes the 3x3 Varianz matrix as the base for a Principal Component Analysis.
@param points vector of points
@param M      3x3 matrix
*/
void computeCovarianceMatrix3x3(const std::vector<Point3d>& points, Matrix& M)
{
  M.resize(3, 3);
  const ptrdiff_t N(points.size());
  if (N<1) return;

  //compute the mean value (center) of the points cloud
  Point3d mean = computeCenter(points);

  //Compute the entries of the (symmetric) covariance matrix
  double Mxx(0), Mxy(0), Mxz(0), Myy(0), Myz(0), Mzz(0);
  #pragma omp parallel for reduction(+: Mxx,Mxy,Mxz,Myy,Myz,Mzz) //omp reduction enables parallel sum up of values
  for (ptrdiff_t i = 0; i<N; ++i)
  {
    const Point3d& pt = points[i];

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

/** @brief computes best-fit approximations.
    @param points vector of points
*/
void computeBestFitLine(const std::vector<Point3d>& points, std::vector<Point3d>& corners)
{
  Matrix M(3, 3);
  const Point3d center = computeCenter(points);
  computeCovarianceMatrix3x3(points, M);
  SVD::computeSymmetricEigenvectors(M);

  const Point3d ev0(M(0, 0), M(1, 0), M(2, 0)); //first column of M == Eigenvector corresponding to the largest Eigenvalue == direction of biggest variance
  const Point3d ev1(M(0, 1), M(1, 1), M(2, 1));
  const Point3d ev2(M(0, 2), M(1, 2), M(2, 2)); //third column of M == Eigenvector corresponding to the smallest Eigenvalue == direction of lowest variance

  //best-fit line
  std::cout << "*** Best-fit line ***\n";
  std::cout << "Point    : " << center.x() << ", " << center.y() << ", " << center.z() << std::endl;
  std::cout << "Direction: " << ev0.x()    << ", " << ev0.y()    << ", " << ev0.z()    << std::endl;
  
  //computing the mean distance to line
  double meanDistance = 0;
  for (size_t i = 0; i < points.size(); ++i)
  {
    meanDistance += distancePt2Line(points[i], center, ev0);
  }
  meanDistance /= points.size();
  std::cout << "mean distance to line: " << meanDistance << std::endl;

  //Compute the end points of the line
  double mindist = +std::numeric_limits<double>::max();
  double maxdist = -std::numeric_limits<double>::max();
  for (size_t i = 0; i < points.size(); ++i)
  {
    const double dist = distancePt2Plane(points[i], center, ev0);
    if (dist < mindist) mindist = dist;
    if (dist > maxdist) maxdist = dist;
  }

  Point3d corner1 = center + ev0*mindist;
  Point3d corner2 = center + ev0*maxdist;

  corners.push_back(corner1);
  corners.push_back(corner2);
}

/** @brief computes best-fit approximations.
    @param points vector of points
*/
void computeBestFitPlane(const std::vector<Point3d>& points, std::vector<Point3d>& corners)
{
  Matrix M(3, 3);

  const Point3d center = computeCenter(points);
  computeCovarianceMatrix3x3(points, M);
  SVD::computeSymmetricEigenvectors(M);

  const Point3d ev0(M(0, 0), M(1, 0), M(2, 0)); //first column of M == Eigenvector corresponding to the largest Eigenvalue == direction of biggest variance
  const Point3d ev1(M(0, 1), M(1, 1), M(2, 1));
  const Point3d ev2(M(0, 2), M(1, 2), M(2, 2)); //third column of M == Eigenvector corresponding to the smallest Eigenvalue == direction of lowest variance
  
  //best-fit plane
  std::cout << "*** Best-fit plane ***\n";
  std::cout << "Point    : " << center.x() << ", " << center.y() << ", " << center.z() << std::endl;
  std::cout << "Direction: " << ev2.x() << ", " << ev2.y() << ", " << ev2.z() << std::endl;

  //computing the mean distance to plane
  double meanDistance = 0;
  for (size_t i = 0; i < points.size(); ++i)
  {
    meanDistance += std::abs(distancePt2Plane(points[i], center, ev2));
  }
  meanDistance /= points.size();
  std::cout << "mean distance to plane: " << meanDistance << std::endl;


  double mindist0 = +std::numeric_limits<double>::max();
  double maxdist0 = -std::numeric_limits<double>::max();
  double mindist1 = +std::numeric_limits<double>::max();
  double maxdist1 = -std::numeric_limits<double>::max();
  for (size_t i = 0; i < points.size(); ++i)
  {
    const double dist0 = distancePt2Plane(points[i], center, ev0);
    if (dist0 < mindist0) mindist0 = dist0;
    if (dist0 > maxdist0) maxdist0 = dist0;

    const double dist1 = distancePt2Plane(points[i], center, ev1);
    if (dist1 < mindist1) mindist1 = dist1;
    if (dist1 > maxdist1) maxdist1 = dist1;
  }

  Point3d corner1 = center + ev0*maxdist0 + ev1*maxdist1;
  Point3d corner2 = center + ev0*maxdist0 + ev1*mindist1;
  Point3d corner3 = center + ev0*mindist0 + ev1*mindist1;
  Point3d corner4 = center + ev0*mindist0 + ev1*maxdist1;

  corners.push_back(corner1);
  corners.push_back(corner2);
  corners.push_back(corner3);
  corners.push_back(corner4);
}

/** @brief Computes the distance of a point to a 3D line.
    @param point         point 
    @param pointOnLine   a point on the line (e.g. "center" point)
    @param lineDirection vector respresenting the 3d direction of the line (must be a unit vector -> length==1)
*/
double distancePt2Line(const Point3d& point, const Point3d& pointOnLine, const Point3d& lineDirection)
{
  const Point3d PQ = pointOnLine - point;
  double distance = vectorLength( crossProduct(PQ, lineDirection) );

  return distance;
}

/** @brief Computes the distance of a point to a plane.
    @param point          point
    @param pointOnPlane   a point on the plane (e.g. "center" point)
    @param planeDirection vector respresenting the 3d direction of the plane (must be a unit vector -> length==1)
*/
double distancePt2Plane(const Point3d& point, const Point3d& pointOnPlane, const Point3d& planeDirection)
{
  const Point3d PQ = point-pointOnPlane;
  double distance = dotProduct(PQ, planeDirection);

  return distance;
}
