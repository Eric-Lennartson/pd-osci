//
//  Audio_Functions.h
//  polygon~
//
//  Created by Eric Lennartson on 5/14/20.
//  Copyright © 2020 Eric Lennartson. All rights reserved.
//

#ifndef Audio_Functions_h
#define Audio_Functions_h

#include "Audio_Math.h"
#include "Phase_Cut.h"
vec3 circle(float t)
{
    return (vec3){cos(two_pi * t), sin(two_pi * t), 0};
}

vec3 polygon(float t, int N)
{
    PhaseCut cut = cut_equal(t, N);
    vec3 a = circle(cut.idx / (float)N); // get a point on a circle
    vec3 b = circle((cut.idx+1) / (float)N); // get the next point on a circle
    return blend(a, b, cut.t);
}
#endif /* Audio_Functions_h */
