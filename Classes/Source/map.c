#include "m_pd.h"
#include <math.h>

#define FLT_EPSILON 1.19209290E-07F

typedef struct _map
{
    t_object x_obj;
    t_float  min_in, max_in, min_out, max_out;
} t_map;

static t_class *map_class;

t_float map(t_float value,  t_float inputMin,  t_float inputMax,  t_float outputMin,  t_float outputMax)
{
    if(fabs(inputMin - inputMax) < FLT_EPSILON) // check if distance is basically zero
    {
        return outputMin;
    }
    else
    {
        double outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
        return outVal;
    }
}

static void map_float(t_map *x, t_floatarg f)
{
    t_float result = map(f, x->min_in, x->max_in, x->min_out, x->max_out);
    outlet_float(x->x_obj.ob_outlet, result);
}

static void *map_new(t_floatarg min_in, t_floatarg max_in, t_floatarg min_out, t_floatarg max_out)
{
    t_map *x = (t_map *)pd_new(map_class);
    
    x->min_in  = min_in;
    x->max_in  = max_in;
    x->min_out = min_out;
    x->max_out = max_out;
    
    floatinlet_new(&x->x_obj, &x->min_in);
    floatinlet_new(&x->x_obj, &x->max_in);
    floatinlet_new(&x->x_obj, &x->min_out);
    floatinlet_new(&x->x_obj, &x->max_out);
    
    outlet_new(&x->x_obj, &s_float);
    
    return (x);
}

static void *map_free(t_map *x)
{
    return (void *)x;
}

void map_setup(void)
{
    map_class = class_new(gensym("map"),
                (t_newmethod)map_new,
                (t_method)map_free,
                sizeof(t_map),
                CLASS_DEFAULT,
                A_DEFFLOAT, // min_in
                A_DEFFLOAT, // max_in
                A_DEFFLOAT, // min_out
                A_DEFFLOAT, // max_out
                0);
    
    class_sethelpsymbol(map_class, gensym("map"));
    class_addfloat(map_class, (t_method)map_float);
}
