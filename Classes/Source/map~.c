#include "Audio_Math.h"

// add a optional clamping argument?

static t_class *map_tilde_class;

typedef struct _map_tilde
{
    t_object x_obj;
    t_float f; // dummy
    int bypass;
    t_inlet  *min_in, *max_in, *min_out, *max_out;
    t_outlet *result_out;
} t_map_tilde;

static t_int *map_tilde_perform(t_int *w)
{
    t_map_tilde *x = (t_map_tilde *)(w[1]);
    t_float *val_in  = (t_float *)(w[2]);
    t_float *min_in  = (t_float *)(w[3]);
    t_float *max_in  = (t_float *)(w[4]);
    t_float *min_out = (t_float *)(w[5]);
    t_float *max_out = (t_float *)(w[6]);
    t_float *out     = (t_float *)(w[7]);
    int nblock       =       (int)(w[8]);

    while (nblock--)
    {
        t_float val = val_in[nblock];
        if(!x->bypass)
        {
            t_float min1 = min_in[nblock];
            t_float max1 = max_in[nblock];
            t_float min2 = min_out[nblock];
            t_float max2 = max_out[nblock];
            out[nblock] = map_lin(val, min1, max1, min2, max2, false);
        } else {
            out[nblock] = val;
        }
    }
    return (w + 9);
}

static void map_tilde_dsp(t_map_tilde *x, t_signal **sp)
{
    dsp_add(map_tilde_perform, 8, x,
            sp[0]->s_vec, // val
            sp[1]->s_vec, // min1
            sp[2]->s_vec, // max1
            sp[3]->s_vec, // min2
            sp[4]->s_vec, // max2
            sp[5]->s_vec, // result
            sp[0]->s_n); // out
}

static void map_tilde_bypass(t_map_tilde *x, t_floatarg f)
{
    x->bypass = (int)f;
}

static void *map_tilde_free(t_map_tilde *x)
{
    inlet_free(x->min_in);
    inlet_free(x->max_in);
    inlet_free(x->min_out);
    inlet_free(x->max_out);

    outlet_free(x->result_out);
    return (void *)x;
}

static void *map_tilde_new(t_floatarg min_in, t_floatarg max_in, t_floatarg min_out, t_floatarg max_out)
{
    t_map_tilde *x = (t_map_tilde *)pd_new(map_tilde_class);

    x->bypass = 0;

    x->min_in  = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->max_in  = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->min_out = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->max_out = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);

    pd_float((t_pd *)x->min_in, min_in);
    pd_float((t_pd *)x->max_in, max_in);
    pd_float((t_pd *)x->min_out, min_out);
    pd_float((t_pd *)x->max_out, max_out);

    x->result_out = outlet_new((t_object *)x, &s_signal);

    return (x);
}

void map_tilde_setup(void)
{
    map_tilde_class = class_new(gensym("map~"),
                (t_newmethod)map_tilde_new,
                (t_method)map_tilde_free,
                sizeof(t_map_tilde),
                CLASS_DEFAULT,
                A_DEFFLOAT, // min_in
                A_DEFFLOAT, // max_in
                A_DEFFLOAT, // min_out
                A_DEFFLOAT, // max_out
                0);

    class_sethelpsymbol(map_tilde_class, gensym("map~"));

    class_addmethod(map_tilde_class, (t_method)map_tilde_dsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(map_tilde_class, (t_method)map_tilde_bypass, gensym("bypass"), A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(map_tilde_class, t_map_tilde, f); // signal inlet as first inlet
}
