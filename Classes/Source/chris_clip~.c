#include "m_pd.h"
#include "math.h"

// might want to refactor FLT_EPISLON later
#define FLT_EPSILON 1.19209290E-07F

static t_class *chris_clip_tilde_class;

typedef struct _chris_clip_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    int mod, mirror, bypass;
    t_inlet *min_in, *max_in; // signal in and out default provided
} t_chris_clip_tilde;

static void chris_clip_mod(t_chris_clip_tilde *x, t_floatarg f) {
    x->mod = (f > 0) ? 1 : 0;
}

static void chris_clip_mirror(t_chris_clip_tilde *x, t_floatarg f) {
    x->mirror = (f > 0) ? 1 : 0;
}

static void chris_clip_bypass(t_chris_clip_tilde *x, t_floatarg f) {
    x->bypass = (int)f;
}

static t_float chris_clip(t_chris_clip_tilde *x, t_floatarg value, t_float min, t_float max)
{
    if(!x->mod && !x->mirror){ // standard clipping
        value = (value > max) ? max :
                (value < min) ? min : value;
    }

    if(x->mod){
        if(value < min){ value = fmodf(value, min); }
        else if(value > max) { value = fmodf(value, max); }
    }
    // no else because I want it to be possible to do both
    if(x->mirror){ // mirror will be based of the max value only
        if(max <= 0.f) { max = 0.1; } // values less than zero cause pd to crash
        else {
            while(value > max || value < -max) {
                if(value > max) { value -= 2 * (value - max); }
                if(value < -max) { value -= 2 * (value + max); }
            }
        }
    }
    return value;
}

static t_int *chris_clip_tilde_perform(t_int *w)
{
    t_chris_clip_tilde *x = (t_chris_clip_tilde *)(w[1]);
    t_float *value_in     = (t_float *)(w[2]);
    t_float *min_in       = (t_float *)(w[3]);
    t_float *max_in       = (t_float *)(w[4]);
    t_float *out          = (t_float *)(w[5]);
    int nblock            = (int)(w[6]);

    while (nblock--)
    {
        t_float value = value_in[nblock];

        if(!x->bypass)
        {
            t_float min = min_in[nblock];
            t_float max = max_in[nblock];

            if(x->mirror)
                if(max <= 0.f) { max = 0.001; }

            value = chris_clip(x, value, min, max);
        }
        out[nblock] = value;

    }
    return (w + 7);
}

static void chris_clip_tilde_dsp(t_chris_clip_tilde *x, t_signal **sp)
{
    dsp_add(chris_clip_tilde_perform, 6, x,
            sp[0]->s_vec, // value_in
            sp[1]->s_vec, // min_in
            sp[2]->s_vec, // max_in
            sp[3]->s_vec, // out
            sp[0]->s_n);  // block size
}

// ctor
static void *chris_clip_new(t_symbol *s, int argc, t_atom *argv)
{
    t_chris_clip_tilde *x = (t_chris_clip_tilde *)pd_new(chris_clip_tilde_class);

    x->bypass = 0;

    t_float min = argc>0 ? atom_getfloat(argv)   : -1.f;
    t_float max = argc>1 ? atom_getfloat(argv+1) :  1.f;
    x->mirror = argc>2 ? atom_getfloat(argv+2) :  0;
    x->mod    = argc>3 ? atom_getfloat(argv+3) :  0;

    // allocate memory for inlets
    x->min_in  = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->max_in  = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);

    // init inlets values
    pd_float((t_pd*)x->min_in, min);
    pd_float((t_pd*)x->max_in, max);

    outlet_new(&x->x_obj, &s_signal);

    return (x);
}

// dtor
static void *chris_clip_free(t_chris_clip_tilde *x) {
    return (void *)x;
}

// NEVER MAKE THIS STATIC!!!
void chris_clip_tilde_setup(void)
{
    chris_clip_tilde_class = class_new(gensym("chris_clip~"),
                            (t_newmethod)chris_clip_new, //ctor
                            (t_method)chris_clip_free, //dtor
                            sizeof(t_chris_clip_tilde), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_GIMME, // min, max, mirror, mod
                            0); // no more args

    class_sethelpsymbol(chris_clip_tilde_class, gensym("chris_clip~")); // links to the help patch

    class_addmethod(chris_clip_tilde_class, (t_method)chris_clip_mod, gensym("mod"), A_DEFFLOAT, 0);
    class_addmethod(chris_clip_tilde_class, (t_method)chris_clip_mirror, gensym("mirror"), A_DEFFLOAT, 0);
    class_addmethod(chris_clip_tilde_class, (t_method)chris_clip_bypass, gensym("bypass"), A_DEFFLOAT, 0);
    class_addmethod(chris_clip_tilde_class, (t_method)chris_clip_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(chris_clip_tilde_class, t_chris_clip_tilde, f); // signal inlet as first inlet
}
