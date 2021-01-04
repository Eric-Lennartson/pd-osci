// Phase_Cut.h
// Thank you Hansi

#include <stdarg.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <utils.h>

typedef struct PhaseCut
{
    int idx;
    float t; // phase
    float w; // weight
    int N;   // number of cuts
} PhaseCut;

// MUST RCV DOUBLES!!!!
double sum_weights(int n_args, double w1, va_list weights)
{
    double sum = w1;
    for(int i = 0; i < n_args; i++)
    {
        double temp = va_arg(weights, double);
        if(temp < 0) break;
        else sum += temp;
    }
    va_end(weights);
    
    return sum;
}

PhaseCut calc_cut(int n_args, int n_cuts, int idx, double t, double sum, double w1, va_list weights)
{
    double w = w1;
    
    if(t < w)
    {
        return (PhaseCut){idx, map_to_unit(t, 0, w1, true), w, n_cuts};
    }
    else
    {
        w = va_arg(weights, double);
        return calc_cut(n_args - 1, n_cuts, idx + 1, t - w1, sum - w1, w, weights);
    }
}

PhaseCut cut_weights(int n_args, double t, double w1, ...)
{
    int n_cuts = n_args;
    va_list ap, ap2;
    va_start(ap, w1);
    double sum = sum_weights(n_args - 1, w1, ap);
    
    va_start(ap2, w1);
    return calc_cut(n_args-1, n_cuts, 0, t * sum, sum, w1, ap2);
}

PhaseCut cut_mix(float t, int n_cuts, float which, float fadeRatio)
{
      int mainIdx = (int)(n_cuts * which) % n_cuts;
      int nextIdx = (mainIdx+1) % n_cuts;
 
      float mix = mod1(n_cuts*which);
 
      fadeRatio = clamp(1-fadeRatio,0,1);
      mix = map_lin(mix, fadeRatio/2, 1-fadeRatio/2, 0,1, true);
 
      if(t<mix) return (PhaseCut){nextIdx,map_lin(t,0,mix,0,1,false),1/(float)n_cuts,n_cuts};
      else return (PhaseCut){mainIdx,map_lin(t,mix,1,0,1,false), 1/(float)n_cuts,n_cuts};
}
