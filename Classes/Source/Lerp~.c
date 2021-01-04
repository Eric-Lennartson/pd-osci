//
//  Lerp~.c
//  Lerp~
//
//  Created by Eric Lennartson on 4/6/20.

#include "m_pd.h"

// SOMETHING IS HORRIBLY WRONG HERE!!! SECOND OPTION IN LERP IS REALLY ODD!!!!!!

static t_class *lerp_tilde_class;

typedef struct _lerp_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    
    t_inlet *x1_in, *y1_in, *z1_in, *x2_in, *y2_in, *z2_in; // amount_in default provided
    t_outlet *y_out, *z_out; // xChan1_out default provided
} t_lerp_tilde;

static t_int *lerp_tilde_perform(t_int *w)
{
    t_sample *amount_in = (t_sample *)(w[1]);
    t_sample *x1_in     = (t_sample *)(w[2]);
    t_sample *y1_in     = (t_sample *)(w[3]);
    t_sample *z1_in     = (t_sample *)(w[4]);
    t_sample *x2_in     = (t_sample *)(w[5]);
    t_sample *y2_in     = (t_sample *)(w[6]);
    t_sample *z2_in     = (t_sample *)(w[7]);
    t_sample *x_out     = (t_sample *)(w[8]);
    t_sample *y_out     = (t_sample *)(w[9]);
    t_sample *z_out     = (t_sample *)(w[10]);
    int      nblock     =        (int)(w[11]); // get block size
    
    while (nblock--)
    {
        t_sample amount = amount_in[nblock];
        t_sample x1 = x1_in[nblock];
        t_sample y1 = y1_in[nblock];
        t_sample z1 = z1_in[nblock];
        t_sample x2 = x2_in[nblock];
        t_sample y2 = y2_in[nblock];
        t_sample z2 = z2_in[nblock];
        
        // standard linear interpolation function (max - min) * norm + min
        x_out[nblock] = (x2 - x1) * amount + x1;
        y_out[nblock] = (y2 - y1) * amount + y1;
        z_out[nblock] = (z2 - z1) * amount + z1;
    }
    
    return (w + 12);
}

static void lerp_tilde_dsp(t_lerp_tilde *x, t_signal **sp)
{
    dsp_add(lerp_tilde_perform, 11,
            sp[0]->s_vec, // amount
            sp[1]->s_vec, // x1
            sp[2]->s_vec, // y1
            sp[3]->s_vec, // z1
            sp[4]->s_vec, // x2
            sp[5]->s_vec, // y2
            sp[6]->s_vec, // z2
            sp[7]->s_vec, // x_out
            sp[8]->s_vec, // y_out
            sp[9]->s_vec, // z_out
            sp[0]->s_n); // block size
}

static void *lerp_tilde_new(t_floatarg x1, t_floatarg y1, t_floatarg z1, t_floatarg x2, t_floatarg y2, t_floatarg z2)
{
    t_lerp_tilde *x = (t_lerp_tilde *)pd_new(lerp_tilde_class);
    
    x->x1_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->y1_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->z1_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->x2_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->y2_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->z2_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
    pd_float((t_pd*)x->x1_in, x1); // init inlets with creation args
    pd_float((t_pd*)x->y1_in, y1);
    pd_float((t_pd*)x->z1_in, z1);
    pd_float((t_pd*)x->x2_in, x2);
    pd_float((t_pd*)x->y2_in, y2);
    pd_float((t_pd*)x->z2_in, z2);
    
    outlet_new(&x->x_obj, &s_signal); // default provided outlet
    x->y_out = outlet_new(&x->x_obj, &s_signal);
    x->z_out = outlet_new(&x->x_obj, &s_signal);
    
    return (x);
}

// FREE
static void *lerp_tilde_free(t_lerp_tilde *x)
{
    inlet_free(x->x1_in);
    inlet_free(x->y1_in);
    inlet_free(x->z1_in);
    inlet_free(x->x2_in);
    inlet_free(x->y2_in);
    inlet_free(x->z2_in);
    
    outlet_free(x->y_out);
    outlet_free(x->z_out);
    
    return (void *)x;
}

void lerp_tilde_setup(void)
{
    lerp_tilde_class = class_new(gensym("lerp~"),
                            (t_newmethod)lerp_tilde_new, //ctor
                            (t_method)lerp_tilde_free, //dtor
                            sizeof(t_lerp_tilde), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_GIMME, // 6 or more args
                            0); // no more args
    
    class_addcreator((t_newmethod)lerp_tilde_new,
                     gensym("osci/lerp~"),
                     A_GIMME, // 6 or more args
                     0); // no more args
    
    class_sethelpsymbol(lerp_tilde_class, gensym("lerp~")); // links to the help patch
    
    class_addmethod(lerp_tilde_class, (t_method)lerp_tilde_dsp, gensym("dsp"), A_CANT, 0); // add dsp method to data space
    CLASS_MAINSIGNALIN(lerp_tilde_class, t_lerp_tilde, f); // signal inlet as first inlet
}
