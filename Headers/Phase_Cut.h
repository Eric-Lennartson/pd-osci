#ifndef Phase_Cut_h
#define Phase_Cut_h

#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <utils.h>

/* I have two versions running around, one with va_args, another with arrays
   I think the one with arrays is a better design?
*/
typedef struct PhaseCut
{
    int idx;
    float t; // phase
    float w; // weight
    int N;   // number of cuts
} PhaseCut;

// MUST RCV DOUBLES!!!!
double sum_weights(int size, double *array)
{
    double sum = 0.f;
    for(int i = 0; i < size; ++i)
    {
        sum += array[i];
    }
    return sum;
}

PhaseCut calc_cut(int n_args, int n_cuts, int idx, double t, double sum, double *weights)
{
    for(int i = 0; i < n_args; ++i)
    {
        double w = weights[i];
        if(t < w)
        {
            return (PhaseCut){idx, map_to_unit(t, 0, w, true), w, n_cuts};
        }
        else
        {
            ++idx;
            t -= w;
            sum -= w;
        }
    }
    // this prevents crashes when weights are equal to zero.
    // otherwise the index never updates and the loop never exits?
    // not sure but this prevents the crash.
    static int index = 0;
    PhaseCut cut = (PhaseCut){index++};
    index %= n_args;
    return cut;
}

PhaseCut cut_weights(int n_args, double t, double *weights)
{
    double sum = sum_weights(n_args, weights);
    return calc_cut(n_args, n_args, 0, t * sum, sum, weights);
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

PhaseCut cut_equal(float t, int n_cuts)
{
    float tn = t * n_cuts;
    int idx = tn;
    return (PhaseCut){idx, tn-idx, 1/(double)n_cuts, n_cuts};
}

#endif /* Phase_Cut_h */
