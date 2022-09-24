//Code ported from Hansi Raber's PhaseCut
//copyright (c) 2018, Hansi Raber

#ifndef __PHASE_CUT_H
#define __PHASE_CUT_H

#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include "Audio_Math.h" // for mod1()

#define MINOUTLETS 2
#define MAXOUTLETS 100

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

#endif /* __PHASE_CUT_H */
