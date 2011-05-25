//
// approximate distance vector with regularized spline and evaluate the derivative
//

extern "C" {
#include "gcvspl.h"
}

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

class Smooth {
public:
  static void smooth(int size, double *t,
                     double *distance, double * /* velocity*/,
                     double *latitudes, double *longitudes, double *alt,
                     double *output) {

    const int degree = 2; // 2 = cubic splines

    std::vector<double> w(size, 1.0);
    std::vector<double> c(size);
    std::vector<double> work(6*(size*degree+1)+size);

    gcvspl(t, distance, &w[0], degree, size, &c[0], -1.0, &work[0], 0);

    int l = 0;
    for (int i = 0; i < size; ++i) {
      output[i] = abs(splder(1, degree, size, t[i], &t[0], &c[0], &l, &work[0]));
      if (output[i] < 1.0)
        output[i] = 1.0;
    }
  }
};
