#include "Audio_Functions.h"

t_vec3 circle(float t)
{
    return (t_vec3){cos(TWO_PI * t), sin(TWO_PI * t), 0};
}

t_vec3 polygon(float t, int N)
{
    PhaseCut cut = cut_equal(t, N);
    t_vec3 a = circle(cut.idx / (float)N); // get a point on a circle
    t_vec3 b = circle((cut.idx+1) / (float)N); // get the next point on a circle
    return blend(cut.t, a, b);
}