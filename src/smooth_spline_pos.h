//
// create two splines from latitude and longitude,
// calculate distance between each two consecutive coordinates
// take velocity as distance over time difference.
//

extern "C" {
#include "gcvspl.h"
}

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

class Smooth {

  static double distVincenty(double lat1, double lon1, double lat2, double lon2) {
    // from http://www.movable-type.co.uk/scripts/latlong-vincenty.html
    const double a = 6378137.0;// WGS-84 ellipsoid params
    const double b = 6356752.314245;// WGS-84 ellipsoid params
    const double  f = 1.0/298.257223563;  // WGS-84 ellipsoid params
    const double L = (lon2-lon1);
    const double U1 = atan((1.0-f) * tan(lat1));
    const double U2 = atan((1.0-f) * tan(lat2));
    const double sinU1 = sin(U1), cosU1 = cos(U1);
    const double sinU2 = sin(U2), cosU2 = cos(U2);

    double lambda = L, lambdaP, iterLimit = 100;
    double sigma;
    double sinSigma, cosSigma;
    double cosSqAlpha, cos2SigmaM;
    do {
      double sinLambda = sin(lambda);
      double cosLambda = cos(lambda);
      sinSigma = sqrt((cosU2*sinLambda) * (cosU2*sinLambda) + 
        (cosU1*sinU2-sinU1*cosU2*cosLambda) * (cosU1*sinU2-sinU1*cosU2*cosLambda));
      if (sinSigma==0) return 0;  // co-incident points

      cosSigma = sinU1*sinU2 + cosU1*cosU2*cosLambda;
      sigma = atan2(sinSigma, cosSigma);
      double sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
      cosSqAlpha = 1.0 - sinAlpha*sinAlpha;
      if (cosSqAlpha==0) return 0; // equatorial line: cosSqAlpha=0 (§6)

      cos2SigmaM = cosSigma - 2*sinU1*sinU2/cosSqAlpha;
      double C = f/16.0*cosSqAlpha*(4.0+f*(4.0-3.0*cosSqAlpha));
      lambdaP = lambda;
      lambda = L + (1.0-C) * f * sinAlpha *
        (sigma + C*sinSigma*(cos2SigmaM+C*cosSigma*(-1.0+2.0*cos2SigmaM*cos2SigmaM)));
    } while (abs(lambda-lambdaP) > 1e-12 && --iterLimit>0);

    if (iterLimit==0) 
      return 0.0;  // formula failed to converge

    double uSq = cosSqAlpha * (a*a - b*b) / (b*b);
    double A = 1.0 + uSq/16384.0*(4096.0+uSq*(-768.0+uSq*(320.0-175.0*uSq)));
    double B = uSq/1024.0 * (256.0+uSq*(-128.0+uSq*(74.0-47.0*uSq)));
    double deltaSigma = B*sinSigma*(cos2SigmaM+B/4.0*(cosSigma*(-1.0+2.0*cos2SigmaM*cos2SigmaM)-
      B/6.0*cos2SigmaM*(-3.0+4.0*sinSigma*sinSigma)*(-3.0+4.0*cos2SigmaM*cos2SigmaM)));
    double s = b*A*(sigma-deltaSigma);

    return s;
  }

  static double bearing(double lat1, double lon1, double lat2, double lon2) {
    double y = sin(lon2-lon1) * cos(lat2);
    double x = cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(lon2-lon1);
    return atan2(y, x);
  }

public:
  static void smooth(int size, double *t,
                     double *distance, double * /* velocity*/,
                     double *latitudes, double *longitudes, double *alt,
                     double *output) {

    const int degree = 2; // 2 = cubic splines


    //// Try to calculate positions from longitude & latitude,
    //// then spline from positions and evaluate derivative
    //std::vector<double> x(size), y(size);

    //for (int i = 1; i < size; ++i) {
    //  double d = distVincenty(latitudes[i-1], longitudes[i-1], latitudes[i], longitudes[i]);
    //  double b = bearing(latitudes[i-1], longitudes[i-1], latitudes[i], longitudes[i]);
    //  x[i] = x[i-1] + d*cos(b);
    //  y[i] = y[i-1] + d*sin(b);
    //}

    //std::vector<double> w(size, 1.0);
    //std::vector<double> cx(size);
    //std::vector<double> cy(size);
    //std::vector<double> work(6*(size*degree+1)+size);
    //
    //gcvspl(&t[0], &x[0], &w[0], degree, size, &cx[0], -1.0, &work[0], 0);
    //gcvspl(&t[0], &y[0], &w[0], degree, size, &cy[0], -1.0, &work[0], 0);

    //int l = 0;
    //for (int i = 0; i < size; ++i) {
    //  double dx = splder(1, degree, size, t[i], &t[0], &cx[0], &l, &work[0]);
    //  double dy = splder(1, degree, size, t[i], &t[0], &cy[0], &l, &work[0]);
    //  output[i] = sqrt(dx*dx+dy*dy);
    //}

    // Create spline for longitude and latitude

    std::vector<double> w(size, 1.0);
    std::vector<double> cx(size);
    std::vector<double> cy(size);
    std::vector<double> work(6*(size*degree+1)+size);

    gcvspl(&t[0], latitudes, &w[0], degree, size, &cx[0], 1e-9, &work[0], 0);
    gcvspl(&t[0], longitudes, &w[0], degree, size, &cy[0], 1e-9, &work[0], 0);

    int l = 0;

    double lat, lng;
    lat = splder(0, degree, size, t[0], &t[0], &cx[0], &l, &work[0]) / 180.0 * M_PI;
    lng = splder(0, degree, size, t[0], &t[0], &cy[0], &l, &work[0]) / 180.0 * M_PI;

    output[0] = 0;

    for (int i = 1; i < size; ++i) {
      lat = splder(0, degree, size, t[0], &t[0], &cx[0], &l, &work[0]) / 180.0 * M_PI;
      lng = splder(0, degree, size, t[0], &t[0], &cy[0], &l, &work[0]) / 180.0 * M_PI;
      double dlat = splder(1, degree, size, t[i], &t[0], &cx[0], &l, &work[0]) / 180.0 * M_PI;
      double dlng = splder(1, degree, size, t[i], &t[0], &cy[0], &l, &work[0]) / 180.0 * M_PI;

      double d = distVincenty(lat, lng, lat+dlat, lng+dlng);
      output[i] = d;
    }
  }
};
