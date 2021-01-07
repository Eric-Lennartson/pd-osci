#ifndef utils_h
#define utils_h

#include "stdarg.h"
//#include "m_pd.h"
#include "stdbool.h"

#define min(a, b) a < b ? a : b
#define max(a, b) a > b ? a : b
#define clamp(val,min,max) ((val) < (min) ? (min) : ((val > max) ? (max) : (val)))

#define FLT_EPSILON 1.19209290E-07F

// copied from geeksforgeeks
static inline int gcd(int a, int b);
double va_minf(double num_args, ...);
int va_mini(int num_args, ...);
double va_maxf(double num_args, ...);
int va_maxi(int num_args, ...);
double map_lin( double value,  double inputMin,  double inputMax,  double outputMin,  double outputMax, bool clamp);
double map_to_unit(double value, double inputMin, double inputMax, double clamp);

#endif /* utils_h */
