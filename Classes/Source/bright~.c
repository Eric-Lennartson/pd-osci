#include "m_pd.h"
#include "Audio_Math.h"

static t_class *bright_class;

typedef struct _bright
{
    t_object obj;
    t_sample f; // dummy variable for 1st inlet
    bool bypass;
    t_inlet *offset_in, *strength_in;
} t_bright;

static t_int *bright_perform(t_int *w)
{
    // you can't have init arguments, and sig args be the same, b/c the sig args will overwrite them
    t_bright *this        = (t_bright *)(w[1]);
    t_sample *driver      = (t_sample *)(w[2]);
    t_sample *offset_in   = (t_sample *)(w[3]);
    t_sample *strength_in = (t_sample *)(w[4]);
    t_sample *out         = (t_sample *)(w[5]);
    int nblock            = (int)(w[6]); // get block size

    while (nblock--)
    {
        t_sample t = driver[nblock];
        if(!this->bypass) {
            t_sample offset = offset_in[nblock];
            t_sample strength = strength_in[nblock];

            offset = offset < 0 ? 0 : offset; // prevent strange images from negative offset
            strength = strength < 1 ? 1 : strength; // crashes when given negative values

            out[nblock] = mod1( pow(t,strength) + offset);
        } else {
            out[nblock] = t;
        }
    }

    return (w + 7);
}

static void bright_dsp(t_bright *this, t_signal **sp)
{
    dsp_add(bright_perform, 6, this,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // offset
            sp[2]->s_vec, // strength
            sp[3]->s_vec, // out
            sp[0]->s_n); // block size
}

static void bright_bypass(t_bright *this, t_floatarg bypass) {
    this->bypass = (int)bypass;
}

static void *bright_free(t_bright *this)
{
    inlet_free(this->offset_in);
    inlet_free(this->strength_in);

    return (void *)this;
}

static void *bright_new(t_symbol *s, int argc, t_atom *argv)
{
    t_bright *this = (t_bright *)pd_new(bright_class);

    this->bypass = false;

    t_float offset = argc ? atom_getfloat(argv) : 0.f;
    t_float strength = argc > 1 ? atom_getfloat(argv+1) : 1.f;

    this->offset_in   = inlet_new(&this->obj, &this->obj.ob_pd, &s_signal, &s_signal);
    this->strength_in = inlet_new(&this->obj, &this->obj.ob_pd, &s_signal, &s_signal);

    pd_float((t_pd*)this->offset_in, offset);
    pd_float((t_pd*)this->strength_in, strength);

    outlet_new(&this->obj, &s_signal); // default provided outlet

    return (void *)this;
}

void bright_tilde_setup(void)
{
    bright_class = class_new(gensym("bright~"),
                            (t_newmethod)bright_new, //ctor
                            (t_method)bright_free, //dtor
                            sizeof(t_bright), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_GIMME, //offset, strength
                            0); // no more args

    class_sethelpsymbol(bright_class, gensym("bright~")); // links to the help patch

    class_addmethod(bright_class, (t_method)bright_bypass, gensym("bypass"), A_DEFFLOAT, 0);

    class_addmethod(bright_class, (t_method)bright_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(bright_class, t_bright, f); // signal inlet as first inlet
}
