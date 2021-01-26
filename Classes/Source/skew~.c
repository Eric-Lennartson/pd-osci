#include "m_pd.h"
#include "math.h"
#include <stdlib.h> // abs()

#define FLT_EPSILON 1.19209290E-07F

static t_class *skew_tilde_class;

typedef struct _skew_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    int symmetric; // bool 0 or 1
    int from_center; // bool 0 or 1
    t_float start, end, skew;
    t_inlet *start_in, *end_in, *skew_in;
    
} t_skew_tilde;

static inline t_float clamp0to1(t_float value) {
    return (value > 1) ? 1 : (value < 0) ? 0 : value;
}

static t_float convertTo0to1(t_skew_tilde *x, t_float value)
{
    t_float proportion = clamp0to1( (value - x->start) / (x->end - x->start) );
    
    if (x->skew == 1)
        return proportion;

    if (!x->symmetric)
        return pow(proportion, x->skew);

    t_float dist_from_mid = 2.f * proportion - 1.f;

    return (1 + pow(fabs(dist_from_mid), x->skew) * (dist_from_mid < 0 ? -1 : 1)) / 2;
}

static void onSymmetricMsg(t_skew_tilde *x, t_floatarg f) {
    x->symmetric = (f > 0) ? 1 : 0;
}

static void onFromCenterMsg(t_skew_tilde *x, t_floatarg f) {
    x->from_center = (f > 0) ? 1 : 0;
}

static t_float fromCenter(t_skew_tilde *x, t_floatarg center)
{
    if(x->start < x->end) // could be refactored to some clamping function
        center = (center < x->start) ? x->start : (center > x->end) ? x->end : center;
    else
        center = (center < x->end) ? x->end : (center > x->start) ? x->start : center;
    
    return log(0.5) / log((center - x->start) / (x->end - x->start));
}

static inline t_float map(t_float value, t_float inputMin, t_float inputMax, t_float outputMin, t_float outputMax)
{
    if(fabs(inputMin - inputMax) < FLT_EPSILON) // is it 0?
        return outputMin;
    else
       return (value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin;
}

static t_float skew(t_skew_tilde *x, t_floatarg value)
{ // not sure why I have to make it go from 0 to 1 and then map it, change it later?
    x->skew = (x->skew < 0) ? 0 : x->skew;
    
    t_float result = convertTo0to1(x, value);
    return map(result, 0, 1, x->start, x->end);
}

static t_int *skew_tilde_perform(t_int *w)
{
    t_skew_tilde *x    = (t_skew_tilde *)(w[1]);
    t_float *val_in    =      (t_float *)(w[2]);
    t_float *start_in  =      (t_float *)(w[3]);
    t_float *end_in    =      (t_float *)(w[4]);
    t_float *skew_in   =      (t_float *)(w[5]);
    t_float *out       =      (t_float *)(w[6]);
    int nblock         =            (int)(w[7]);
    
    while (nblock--)
    {
        t_float value = val_in[nblock];
        x->start      = start_in[nblock];
        x->end        = end_in[nblock];
        if(x->from_center) // skew is treated as a center point somewhere in the range here
            x->skew = fromCenter(x, skew_in[nblock]);
        else
            x->skew = skew_in[nblock];

        t_float result = skew(x, value);
        
        out[nblock] = result;
    }
    return (w + 8);
}

static void skew_tilde_dsp(t_skew_tilde *x, t_signal **sp)
{
    dsp_add(skew_tilde_perform, 7, x,
            sp[0]->s_vec, // value_in
            sp[1]->s_vec, // start_in
            sp[2]->s_vec, // end_in
            sp[3]->s_vec, // skew_in
            sp[4]->s_vec, // out
            sp[0]->s_n);  // block size
}

// ctor
static void *skew_new(t_floatarg start, t_floatarg end, t_floatarg skew, t_floatarg from_center, t_floatarg symmetric)
{
    t_skew_tilde *x = (t_skew_tilde *)pd_new(skew_tilde_class);
    

    // this could be better with defaults.
    x->start = start;
    x->end   = end;
    x->skew = (skew < 0) ? 0 : skew;
    x->from_center = (from_center > 0) ? 1 : 0;
    x->symmetric = (symmetric > 0) ? 1 : 0;
    
    // allocate memory for inlets
    x->start_in  = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->end_in    = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->skew_in   = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);

    // init inlets with creation args
    pd_float((t_pd*)x->start_in, x->start);
    pd_float((t_pd*)x->end_in, x->end);
    pd_float((t_pd*)x->skew_in, x->skew);
    
    outlet_new(&x->x_obj, &s_signal);
    
    return (x);
}

// dtor
static void *skew_free(t_skew_tilde *x) {
    return (void *)x;
}

// NEVER MAKE THIS STATIC!!!
void skew_tilde_setup(void)
{
    skew_tilde_class = class_new(gensym("skew~"),
                            (t_newmethod)skew_new, //ctor
                            (t_method)skew_free, //dtor
                            sizeof(t_skew_tilde), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_DEFFLOAT, // start
                            A_DEFFLOAT, // end
                            A_DEFFLOAT, // skew
                            A_DEFFLOAT, // from center (bool)
                            A_DEFFLOAT, // symmetric (bool)
                            0); // no more args
    
    class_sethelpsymbol(skew_tilde_class, gensym("skew~")); // links to the help patch
    
    class_addmethod(skew_tilde_class, (t_method)onSymmetricMsg, gensym("symmetric"), A_DEFFLOAT, 0);
    class_addmethod(skew_tilde_class, (t_method)onFromCenterMsg, gensym("fromCenter"), A_DEFFLOAT, 0);
    class_addmethod(skew_tilde_class, (t_method)skew_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(skew_tilde_class, t_skew_tilde, f); // signal inlet as first inlet
}
