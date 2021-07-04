#ifndef Audio_Math_h
#define Audio_Math_h

#include "m_pd.h"
#include "math.h"
#include "vec3.h"

#ifndef PI
const double PI;
#endif

#ifndef TWO_PI
const double TWO_PI;
#endif

#ifndef HALF_PI
const double HALF_PI;
#endif

#define DEG_TO_RAD (PI / 180.0)
#define FLT_EPSILON 1.19209290E-07F

#define min(a, b) a < b ? a : b
#define max(a, b) a > b ? a : b
#define clamp(val,min,max) ((val) < (min) ? (min) : ((val > max) ? (max) : (val)))

int gcd(int a, int b);
t_float map_lin( t_float value,  t_float inputMin,  t_float inputMax,  t_float outputMin,  t_float outputMax, bool clamp);
t_float map_to_unit(t_float value, t_float inputMin, t_float inputMax, bool clamp);
t_float lerp(const t_float t, const t_float a, const t_float b);
t_vec3 blend(float t, t_vec3 a, t_vec3 b);
t_float mod1(const t_float value);

// doesn't avoid gimbal lock problems
// this should probably be in vec3.h
void rotate(t_vec3* v, t_vec3* rot);

#endif /* Audio_Math_h */