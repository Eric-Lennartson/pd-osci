#include "m_pd.h"
#include <math.h>

// PDBIGORSMALL ??
#define FLT_EPSILON 1.19209290E-07F

typedef struct _bezier_tilde
{
    t_object x_obj;
    t_float f; // dummy
    t_inlet  *min_in, *max_in, *min_out, *max_out;
    t_outlet *x_out, *y_out;
} t_bezier_tilde;

t_float bezier(t_float value,  t_float inputMin,  t_float inputMax,  t_float outputMin,  t_float outputMax)
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

static t_int *bezier_tilde_perform(t_int *w)
{
    t_float *val_in  = (t_float *)(w[1]);
    t_float *min_in  = (t_float *)(w[2]);
    t_float *max_in  = (t_float *)(w[3]);
    t_float *min_out = (t_float *)(w[4]);
    t_float *max_out = (t_float *)(w[5]);
    t_float *out     = (t_float *)(w[6]);
    int nblock       =       (int)(w[7]);
    
    while (nblock--)
    {
        t_float val  = val_in[nblock];
        t_float min1 = min_in[nblock];
        t_float max1 = max_in[nblock];
        t_float min2 = min_out[nblock];
        t_float max2 = max_out[nblock];
        t_float output = 1;

        output = bezier(val, min1, max1, min2, max2);

        out[nblock] = output;
    }
    return (w + 8);
}

static void bezier_tilde_dsp(t_bezier_tilde *x, t_signal **sp)
{
    dsp_add(bezier_tilde_perform, 7,
            sp[0]->s_vec, // val
            sp[1]->s_vec, // min1
            sp[2]->s_vec, // max1
            sp[3]->s_vec, // min2
            sp[4]->s_vec, // max2
            sp[5]->s_vec, // result
            sp[0]->s_n); // out
}

static void *bezier_tilde_free(t_bezier_tilde *x)
{
    inlet_free(x->min_in);
    inlet_free(x->max_in);
    inlet_free(x->min_out);
    inlet_free(x->max_out);
    
    outlet_free(x->result);
    return (void *)x;
}

static void *bezier_tilde_new(t_floatarg min_in, t_floatarg max_in, t_floatarg min_out, t_floatarg max_out)
{
    t_bezier_tilde *x = (t_bezier_tilde *)pd_new(bezier_tilde_class);
    
    x->min_in  = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->max_in  = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->min_out = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->max_out = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    
    pd_float((t_pd *)x->min_in, min_in);
    pd_float((t_pd *)x->max_in, max_in);
    pd_float((t_pd *)x->min_out, min_out);
    pd_float((t_pd *)x->max_out, max_out);

    x->result = outlet_new((t_object *)x, &s_signal);
    
    return (x);
}

void bezier_tilde_setup(void)
{
    bezier_tilde_class = class_new(gensym("bezier~"),
                (t_newmethod)bezier_tilde_new,
                (t_method)bezier_tilde_free,
                sizeof(t_bezier_tilde),
                CLASS_DEFAULT,
                A_DEFFLOAT, // min_in
                A_DEFFLOAT, // max_in
                A_DEFFLOAT, // min_out
                A_DEFFLOAT, // max_out
                0);
    
    class_sethelpsymbol(bezier_tilde_class, gensym("bezier~"));
    
    class_addmethod(bezier_tilde_class, (t_method)bezier_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(bezier_tilde_class, t_bezier_tilde, f); // signal inlet as first inlet
}
