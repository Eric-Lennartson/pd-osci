#include "m_pd.h"
#include "Audio_Math.h"

static t_class *clamp_tilde_class;

// this object is the same as pd's clip~, but with signal inlets for minimum and maximum
typedef struct _clamp_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    t_inlet *min_in, *max_in;
    // value in and out are default provided
} t_clamp_tilde;

static t_int *clamp_tilde_perform(t_int *w)
{
    t_float *value_in = (t_float *)(w[1]);
    t_float *min_in = (t_float *)(w[2]);
    t_float *max_in = (t_float *)(w[3]);
    t_float *value_out = (t_float *)(w[4]);
    int nblock = (int)(w[5]);

    while (nblock--)
    {
        t_float value = value_in[nblock];
        t_float min = min_in[nblock];
        t_float max = max_in[nblock];

        if(max < min) {
            t_float tmp = max;
            max = min;
            min = tmp;
        }

        value = CLAMP(value, min, max);

        value_out[nblock] = value;
    }
    return (w + 6);
}

static void clamp_tilde_dsp(t_clamp_tilde *x, t_signal **sp)
{
    dsp_add(clamp_tilde_perform, 5,
            sp[0]->s_vec, // value_in
            sp[1]->s_vec, // min_in
            sp[2]->s_vec, // max_in
            sp[3]->s_vec, // value_out
            sp[0]->s_n);  // block size
}

// ctor
static void *clamp_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_clamp_tilde *x = (t_clamp_tilde *)pd_new(clamp_tilde_class);

    t_float min = argc ? atom_getfloat(argv) : -1.f;
    t_float max = argc > 1 ? atom_getfloat(argv + 1) : 1.f;

    if(max < min) {
        t_float tmp = max;
        max = min;
        min = tmp;
    }

    // allocate memory for inlets
    x->min_in = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->max_in = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);

    // init inlets values
    pd_float((t_pd *)x->min_in, min);
    pd_float((t_pd *)x->max_in, max);

    outlet_new(&x->x_obj, &s_signal);

    return (x);
}

// dtor
static void *clamp_tilde_free(t_clamp_tilde *x)
{
    inlet_free(x->min_in);
    inlet_free(x->max_in);

    return (void *)x;
}

// NEVER MAKE THIS STATIC!!!
void clamp_tilde_setup(void)
{
    clamp_tilde_class = class_new(gensym("clamp~"),
                                  (t_newmethod)clamp_tilde_new, //ctorå
                                  (t_method)clamp_tilde_free,   //dtor
                                  sizeof(t_clamp_tilde),        // data space
                                  CLASS_DEFAULT,                // gui apperance
                                  A_GIMME,                      // min, max
                                  0);                           // no more args

    class_sethelpsymbol(clamp_tilde_class, gensym("clamp~")); // links to the help patch

    class_addmethod(clamp_tilde_class, (t_method)clamp_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(clamp_tilde_class, t_clamp_tilde, f); // signal inlet as first inlet
}
