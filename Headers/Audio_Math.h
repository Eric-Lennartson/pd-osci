#ifndef Audio_Math_h
#define Audio_Math_h

#include "m_pd.h"
#include "math.h"
#include "vec3.h"

#ifndef pi
const double pi = 3.14159265358979323846;
#endif

#ifndef two_pi
const double two_pi = 6.28318530717958647693;
#endif

#ifndef half_pi
const double half_pi = 1.57079632679489661923;
#endif

#define DEG_TO_RAD (pi / 180.0)

double lerp(const double t, const double a, const double b)
{
    return a + (t * (b - a));
}

t_vec3 blend(float t, t_vec3 a, t_vec3 b)
{
    return (t_vec3) { lerp(t, a.x, b.x),
                    lerp(t, a.y, b.y),
                    lerp(t, a.z, b.z)
                    };
}

double mod1(const double value)
{
    return value - (int)value;
}

// doesn't avoid gimbal lock problems
void rotate(t_vec3* v, t_sample ax, t_sample ay, t_sample az)
{
    t_sample a = cosf(DEG_TO_RAD * ax);
    t_sample b = sinf(DEG_TO_RAD * ax);
    t_sample c = cosf(DEG_TO_RAD * ay);
    t_sample d = sinf(DEG_TO_RAD * ay);
    t_sample e = cosf(DEG_TO_RAD * az);
    t_sample f = sinf(DEG_TO_RAD * az);

    t_sample nx = c * e * v->x - c * f * v->y + d * v->z;
    t_sample ny = (a * f + b * d * e) * v->x + (a * e - b * d * f) * v->y - b * c * v->z;
    t_sample nz = (b * f - a * d * e) * v->x + (a * d * f + b * e) * v->y + a * c * v->z;

    v->x = nx;
    v->y = ny;
    v->z = nz;
}

#endif /* Audio_Math_h */
