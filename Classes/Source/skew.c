#include "m_pd.h"
#include "math.h"
#include <stdlib.h> // abs()

#define FLT_EPSILON 1.19209290E-07F

static t_class *skew_class;

typedef struct _skew
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    int symmetric; // bool 0 or 1
    t_float start, end, skew;
} t_skew;

static inline t_float clamp0to1(t_float value) {
    return (value > 1) ? 1 : (value < 0) ? 0 : value;
}

static t_float convertTo0to1(t_skew *x, t_float value)
{
    t_float proportion = clamp0to1( (value - x->start) / (x->end - x->start) );
    
    if (x->skew == 1)
        return proportion;

    if (!x->symmetric)
        return pow(proportion, x->skew);

    t_float dist_from_mid = 2.f * proportion - 1.f;

    return (1 + pow(fabs(dist_from_mid), x->skew) * (dist_from_mid < 0 ? -1 : 1)) / 2;
}

//static t_float convertFrom0to1(t_skew *x, t_float value)
//{
//    t_float proportion = clamp0to1( (value - x->start) / (x->end - x->start) );
//
//    if (!x->symmetric)
//    {
//        if (x->skew != 1 && proportion > 0)
//            proportion = exp(log(proportion) / x->skew);
//
//        return x->start + (x->end - x->start) * proportion;
//    }
//
//    t_float dist_from_mid = 2.f * proportion - 1.f;
//
//    if (x->skew != 1 && dist_from_mid != 0)
//        dist_from_mid = exp(log(fabs(dist_from_mid)) / x->skew) * (dist_from_mid < 0 ? -1 : 1);
//
//    return x->start + (x->end - x->start) / 2 * (1 + dist_from_mid);
//}

static void onSymmetricMsg(t_skew *x, t_floatarg f) {
    x->symmetric = (f > 0) ? 1 : 0;
}

static void onFromCenterMsg(t_skew *x, t_floatarg center)
{
    if(x->start < x->end) // could be refactored to some clamping function
        center = (center < x->start) ? x->start : (center > x->end) ? x->end : center;
    else
        center = (center < x->end) ? x->end : (center > x->start) ? x->start : center;
    
    x->skew = log(0.5) / log((center - x->start) / (x->end - x->start));
}

static inline t_float map(t_float value, t_float inputMin, t_float inputMax, t_float outputMin, t_float outputMax)
{
    if(fabs(inputMin - inputMax) < FLT_EPSILON) // is it 0?
        return outputMin;
    else
       return (value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin;
}

static void skew_float(t_skew *x, t_floatarg value)
{ // not sure why I have to make it go from 0 to 1 and then map it, change it later?
    x->skew = (x->skew < 0) ? 0 : x->skew;
    
    t_float result = convertTo0to1(x, value);
    result = map(result, 0, 1, x->start, x->end);
    
    // could be useful somewhere else, but I'm commenting it out for now.
    //result = convertFrom0to1(x, value);
    
    
    outlet_float(x->x_obj.ob_outlet, result);
}

// ctor
static void *skew_new(t_floatarg start, t_floatarg end, t_floatarg skew, t_floatarg symmetric)
{
    t_skew *x = (t_skew *)pd_new(skew_class);
    
    // If I used A_GIMME, it would be easier to have default values for these things
    // map is also not checking for default values
    
    // init and bounds check
    x->start = start;
    x->end = end;
    x->skew = (skew < 0) ? 0 : skew;
    x->symmetric = (symmetric > 0) ? 1 : 0;
    
    
    
    
    // allocate memory
    floatinlet_new(&x->x_obj, &x->start);
    floatinlet_new(&x->x_obj, &x->end);
    floatinlet_new(&x->x_obj, &x->skew);

    
    outlet_new(&x->x_obj, &s_float);
    
    return (x);
}

// dtor
static void *skew_free(t_skew *x) {
    return (void *)x;
}

// NEVER MAKE THIS STATIC!!!
void skew_setup(void)
{
    skew_class = class_new(gensym("skew"),
                            (t_newmethod)skew_new, //ctor
                            (t_method)skew_free, //dtor
                            sizeof(t_skew), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_DEFFLOAT, // start
                            A_DEFFLOAT, // end
                            A_DEFFLOAT, // skew
                            A_DEFFLOAT, // symmetric (bool)
                            0); // no more args
    
    class_sethelpsymbol(skew_class, gensym("skew")); // links to the help patch
    
    class_addmethod(skew_class, (t_method)onSymmetricMsg, gensym("symmetric"), A_DEFFLOAT, 0);
    class_addmethod(skew_class, (t_method)onFromCenterMsg, gensym("fromCenter"), A_DEFFLOAT, 0);
    class_addfloat(skew_class, (t_method)skew_float); // called when receive float in first inlet
}
