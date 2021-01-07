#ifndef Phase_Cut_h
#define Phase_Cut_h

#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include "utils.h"

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
double sum_weights(int size, double *array);
PhaseCut calc_cut(int n_args, int n_cuts, int idx, double t, double sum, double *weights);
PhaseCut cut_weights(int n_args, double t, double *weights);
PhaseCut cut_mix(float t, int n_cuts, float which, float fadeRatio);
PhaseCut cut_equal(float t, int n_cuts);

#endif /* Phase_Cut_h */
