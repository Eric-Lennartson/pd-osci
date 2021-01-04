#include "m_pd.h"
#include <math.h>

#define FLT_EPSILON 1.19209290E-07F

/*
 Algorithm from OsciStudio's knee effect
 Thank You Hansi Raber
 */

static t_sample mod1(t_sample val) {return val - (int)val; }

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

static t_class *knee_tilde_class;

typedef struct _knee_tilde
{
    t_object x_obj;
    t_float f; // dummy
    t_inlet  *phase_split_point_in, *segment_length_in; // driver default provided
    // out default provided
} t_knee_tilde;

static t_int *knee_tilde_perform(t_int *w)
{
    t_float *driver_in            =        (t_float *)(w[1]);
    t_float *phase_split_point_in =        (t_float *)(w[2]);
    t_float *segment_length_in    =        (t_float *)(w[3]);
    t_float *phase_out            =        (t_float *)(w[4]);
    int nblock                    =              (int)(w[5]);

    while (nblock--)
    {
        t_float t = mod1( driver_in[nblock] );
        t_float split_point = mod1( phase_split_point_in[nblock] );
        t_float length = mod1( segment_length_in[nblock] );
        
        if(t < split_point) {
            t = map(t, 0, split_point, 0, length);
        }
        else {
            t = length + map(t, split_point, 1, 0, 1-length);
        }
        
        phase_out[nblock] = t;
    }
    return (w + 6);
}

static void knee_tilde_dsp(t_knee_tilde *x, t_signal **sp)
{
    dsp_add(knee_tilde_perform, 5,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // split_point
            sp[2]->s_vec, // segment_length
            sp[3]->s_vec, // adjusted phase
            sp[0]->s_n);  // block size
}


static void *knee_tilde_free(t_knee_tilde *x)
{
    inlet_free(x->phase_split_point_in);
    inlet_free(x->segment_length_in);
    return (void *)x;
}

static void *knee_tilde_new(t_floatarg split_point, t_floatarg length)
{
    t_knee_tilde *x = (t_knee_tilde *)pd_new(knee_tilde_class);
    
    // bounds checking
    split_point = mod1(split_point);
    length = mod1(length);
    
    
    // inlet memory alloc
    x->phase_split_point_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->segment_length_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
    // assign arguments
    pd_float((t_pd*)x->phase_split_point_in, split_point);
    pd_float((t_pd*)x->segment_length_in, length);
    
    outlet_new(&x->x_obj, &s_signal); // default provided outlet
    
    return (x);
}

void knee_tilde_setup(void)
{
    knee_tilde_class = class_new(gensym("knee~"),
                                (t_newmethod)knee_tilde_new,
                                (t_method)knee_tilde_free,
                                sizeof(t_knee_tilde), // data space
                                CLASS_DEFAULT, // gui appearance
                                A_DEFFLOAT, // phase split point
                                A_DEFFLOAT, // length
                                0);
    
    class_addcreator((t_newmethod)knee_tilde_new,
                     gensym("osci/knee~"),
                     A_DEFFLOAT, // phase split point
                     A_DEFFLOAT, // length
                     0); // no more args

    
    class_sethelpsymbol(knee_tilde_class, gensym("knee~")); // links to the help patch
    
    class_addmethod(knee_tilde_class, (t_method)knee_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(knee_tilde_class, t_knee_tilde, f); // signal inlet as first inlet
}
