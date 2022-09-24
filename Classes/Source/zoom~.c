#include "m_pd.h"
#include "Audio_Math.h"

static t_class *zoom_tilde_class;

typedef struct _zoom_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    int mod, mirror, bypass;
    t_inlet *y_in, *z_in, *max_in, *zoom_in; // x_in default provided
    t_outlet *x_out, *y_out, *z_out;
} t_zoom_tilde;

static void mod_msg(t_zoom_tilde *x, t_floatarg f) {
    x->mod = (f > 0) ? 1 : 0;
}

static void mirror_msg(t_zoom_tilde *x, t_floatarg f) {
    x->mirror = (f > 0) ? 1 : 0;
}

static void bypass(t_zoom_tilde *x, t_floatarg f) {
    x->bypass = (int)f;
}

static t_float chris_clip(t_zoom_tilde *x, t_floatarg value, t_float min, t_float max)
{
    if (!x->mod && !x->mirror)
    { // standard clipping
        value = (value > max) ? max : (value < min) ? min
                                                    : value;
    }

    if (x->mod)
    {
        if (value < min)
        {
            value = fmodf(value, min);
        }
        else if (value > max)
        {
            value = fmodf(value, max);
        }
    }
    // no else because I want it to be possible to do both
    if (x->mirror)
    { // mirror will be based of the max value only
        if (max <= 0.f)
        {
            max = 0.1;
        } // values less than zero cause pd to crash
        else
        {
            while (value > max || value < -max)
            {
                if (value > max)
                {
                    value -= 2 * (value - max);
                }
                if (value < -max)
                {
                    value -= 2 * (value + max);
                }
            }
        }
    }
    return value;
}

static t_int *zoom_tilde_perform(t_int *w)
{
    t_zoom_tilde *x  = (t_zoom_tilde *)(w[1]);
    t_float *x_in    = (t_float *)(w[2]);
    t_float *y_in    = (t_float *)(w[3]);
    t_float *z_in    = (t_float *)(w[4]);
    t_float *max_in  = (t_float *)(w[5]);
    t_float *zoom_in = (t_float *)(w[6]);
    t_float *x_out   = (t_float *)(w[7]);
    t_float *y_out   = (t_float *)(w[8]);
    t_float *z_out   = (t_float *)(w[9]);
    int nblock       = (int)(w[10]);

    while (nblock--)
    {
        t_vec3 v = vec3(x_in[nblock], y_in[nblock], z_in[nblock]);
        if(!x->bypass)
        {
            t_float maximum = max_in[nblock];
            t_float zoom = zoom_in[nblock];

            maximum = maximum < 0.001 ? 0.001 : maximum;
            zoom = CLAMP(zoom, 0.001, maximum);

            // zoom should always be > 0, and <= maximum
            t_float zoom_factor = maximum / zoom;

            v.x = chris_clip(x, v.x, -zoom, zoom) * zoom_factor;
            v.y = chris_clip(x, v.y, -zoom, zoom) * zoom_factor;
            v.z = chris_clip(x, v.z, -zoom, zoom) * zoom_factor;
        }

        x_out[nblock] = v.x;
        y_out[nblock] = v.y;
        z_out[nblock] = v.z;
    }
    return (w + 11);
}

static void zoom_tilde_dsp(t_zoom_tilde *x, t_signal **sp)
{
    dsp_add(zoom_tilde_perform, 10, x,
            sp[0]->s_vec, // x_in
            sp[1]->s_vec, // y_in
            sp[2]->s_vec, // z_in
            sp[3]->s_vec, // max_in
            sp[4]->s_vec, // zoom_in
            sp[5]->s_vec, // x_out
            sp[6]->s_vec, // y_out
            sp[7]->s_vec, // z_out
            sp[0]->s_n);  // block size
}

// ctor
static void *zoom_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_zoom_tilde *x = (t_zoom_tilde *)pd_new(zoom_tilde_class);

    x->bypass = 0;

    t_float maximum = argc ? atom_getfloat(argv) : 1.f;
    maximum = maximum < 0.001 ? 0.001 : maximum;
    t_float zoom = argc > 1 ? atom_getfloat(argv + 1) : maximum;
    zoom = CLAMP(zoom, 0.001, maximum);

    x->mirror = argc > 2 ? atom_getfloat(argv + 2) : 0;
    x->mod = argc > 3 ? atom_getfloat(argv + 3) : 0;

    // allocate memory for inlets
    x->y_in = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->z_in = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->max_in = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->zoom_in = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);

    // init inlets values
    pd_float((t_pd *)x->max_in, maximum);
    pd_float((t_pd *)x->zoom_in, zoom);

    x->x_out = outlet_new(&x->x_obj, &s_signal);
    x->y_out = outlet_new(&x->x_obj, &s_signal);
    x->z_out = outlet_new(&x->x_obj, &s_signal);

    return (x);
}

// dtor
static void *zoom_tilde_free(t_zoom_tilde *x)
{
    inlet_free(x->y_in);
    inlet_free(x->z_in);
    inlet_free(x->max_in);
    inlet_free(x->zoom_in);

    outlet_free(x->x_out);
    outlet_free(x->y_out);
    outlet_free(x->z_out);

    return (void *)x;
}

// NEVER MAKE THIS STATIC!!!
void zoom_tilde_setup(void)
{
    zoom_tilde_class = class_new(gensym("zoom~"),
                                 (t_newmethod)zoom_tilde_new, //ctor
                                 (t_method)zoom_tilde_free,   //dtor
                                 sizeof(t_zoom_tilde),        // data space
                                 CLASS_DEFAULT,               // gui apperance
                                 A_GIMME,                     // maximum, zoom, mirror, mod
                                 0);                          // no more args

    class_sethelpsymbol(zoom_tilde_class, gensym("zoom~")); // links to the help patch

    class_addmethod(zoom_tilde_class, (t_method)mod_msg, gensym("mod"), A_DEFFLOAT, 0);
    class_addmethod(zoom_tilde_class, (t_method)mirror_msg, gensym("mirror"), A_DEFFLOAT, 0);
    class_addmethod(zoom_tilde_class, (t_method)bypass, gensym("bypass"), A_DEFFLOAT, 0);
    class_addmethod(zoom_tilde_class, (t_method)zoom_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(zoom_tilde_class, t_zoom_tilde, f); // signal inlet as first inlet
}
