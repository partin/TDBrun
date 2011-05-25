#define NO_BLAS_WRAP

// take 21 nearest points and take velocity as the incline of a least squares fitted line.

#include "lapack/blaswrap.h"
#include "lapack/f2c.h"
#include "lapack/clapack.h"

#include <vector>

namespace {
  static void polyFit(int n, int deg, double *x, double *y, double *poly) {
    const int rows = n;
    const int cols = deg + 1;
    std::vector<double> a(rows * cols);
    for (int j = 0; j < rows; ++j) {
      double currx = 1.0;
      for (int i = 0; i < cols; ++i) {
        a[i*rows+j] = currx;
        currx *= x[j];
      }
    }

    char JOBU='A', JOBVT='A';
    integer M = rows;
    integer N = cols;
    int mn = min(M, N);
    int MN = max(M, N);
    integer LDA = M;
    integer LDU = M;
    integer LDVT = N;
    integer LWORK = 2*MN*MN;
    integer INFO;

    std::vector<double> s(mn);
    std::vector<double> uu(M*M);
    std::vector<double> vt(N*N);
    std::vector<double> wk(LWORK);

    // SVD [A[M,N] -> vt[N,N], s[mn,mn], uu[M,M]] : A = vt' s uu

    dgesvd_(&JOBU, &JOBVT, &M, &N, &a[0], &LDA, &s[0], &uu[0],
      &LDU, &vt[0], &LDVT, &wk[0], &LWORK, &INFO);

    char TRANSA='T';
    double alpha = 1.0;
    integer incx = 1;
    integer incy = 1;
    double beta = 0.0;

    // c[N] = uu'[M,M] y[M]

    std::vector<double> c(M);
    dgemv_(&TRANSA, &M, &M, &alpha, &uu[0], &M, y, &incx, &beta, &c[0], &incy);

    // yy[M] = inv(s) * c = [c1/s1 c2/s2 ... c[mn]/s[mn] 0 ... 0]'

    std::vector<double> yy(N);
    for (int i = 0; i < mn; ++i)
      yy[i] = c[i] / s[i];

    // poly[M] = vt' yy

    std::vector<double> tmp(deg+1);
    dgemv_(&TRANSA, &N, &N, &alpha, &vt[0], &N, &yy[0], &incx, &beta, &tmp[0], &incy);
    for (size_t i = 0; i < tmp.size(); ++i)
      poly[i] = tmp[tmp.size() - i - 1];
  }

  static double evalPolyDer(double x, int n, double *poly) {
    double res = 0.0;
    double currx = 1.0;
    for (int i = 1; i <= n; ++i) {
      res += i * poly[i] * currx;
      currx *= x;
    }
    return res;
  }
}

class Smooth {
public:
  static void smooth(int size, double *x, double * /*dist*/, double *y,
                     double * /*lat*/, double * /*lng*/, double * /*alt*/, 
                     double *vel) {

    //N1=21;  % should be a odd number, 3 <= N1 < length(time)
    //N2=1;   % should be 1 (or max 2)
    //n1=(N1-1)/2;    % help variable
    //for i=n1+1:length(time)-n1
    //    Pl=polyfit(time(i-n1:i+n1)-time(i),distance(i-n1:i+n1)',N2);
    //    dpl=(N2:-1:1).*(Pl(1:end-1)); %% differentiate poly
    //    vel(i-n1)=polyval(dpl,0);
    //end 

    //const int n1 = 21; // must be odd
    const int deg = 1;
    const int maxnum = 11;
    
    std::vector<double> poly(deg+1);
    std::vector<double> time(maxnum);



    for (int i = 0; i < size; ++i) {

      // decide number of points to use
      int n1 = min(i, size-i-1);
      if (n1 == 0) {
        vel[i] = y[i];
        continue;
      }
      if ((n1 & 1) == 0) // must be odd
        --n1;
      if (n1 > maxnum) // use at most max maxnum
        n1 = maxnum;

      int n3 = (n1-1)/2;

      for (int j = 0; j < n1; ++j)
        time[j] = x[i+j-n3] - x[i];

      polyFit(n1, deg, &time[0], &y[i-n3], &poly[0]);
      vel[i] = evalPolyDer(0.0, deg, &poly[0]);

    }
  }
};
