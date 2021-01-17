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

t_float map(t_float value, t_float inputMin, t_float inputMax, t_float outputMin, t_float outputMax);
double lerp(const double t, const double a, const double b);
t_vec3 blend(float t, t_vec3 a, t_vec3 b);
double mod1(const double value);

// doesn't avoid gimbal lock problems
// this should probably be in vec3.h
void rotate(t_vec3* v, t_vec3* rot);

#endif /* Audio_Math_h */
