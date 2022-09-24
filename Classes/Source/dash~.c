
#include "m_pd.h"
#include "Audio_Math.h"

static t_class *dash_class;

typedef struct _dash
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    bool bypass;
    t_inlet *n_points, *dash_length;
} t_dash;

static t_int *dash_perform(t_int *w)
{
    t_dash *x                = (t_dash *)(w[1]);
    t_sample *driver         = (t_sample *)(w[2]);
    t_sample *n_points_in    = (t_sample *)(w[3]);
    t_sample *dash_length_in = (t_sample *)(w[4]);
    t_sample *out            = (t_sample *)(w[5]);
    int nblock               = (int)(w[6]); // get block size

    while (nblock--)
    {
        t_sample t = driver[nblock];
        if(!x->bypass)
        {
            int pnts = n_points_in[nblock];
            t_sample dash_length = fabsf(dash_length_in[nblock]);

            // bounds checks
            dash_length = (dash_length <= 1) ? dash_length : mod1(dash_length);
            pnts = pnts < 1 ? 1 : pnts; // prevent negative pnts

            t_float tn = t * pnts;
            int idx = tn;
            t_float alpha = mod1(tn);
            out[nblock] = (idx + alpha * dash_length ) / pnts;
        } else {
            out[nblock] = t;
        }
    }

    return (w + 7);
}

static void dash_dsp(t_dash *x, t_signal **sp)
{
    dsp_add(dash_perform, 6, x,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // n_points
            sp[2]->s_vec, // dash_length
            sp[3]->s_vec, // out
            sp[0]->s_n); // block size
}

static void dash_bypass(t_dash *x, t_floatarg bypass) {
    x->bypass = (int)bypass;
}

static void *dash_free(t_dash *x)
{
    inlet_free(x->n_points);
    inlet_free(x->dash_length);

    return (void *)x;
}

static void *dash_new(t_symbol *s, int argc, t_atom *argv)
{
    t_dash *x = (t_dash *)pd_new(dash_class);

    x->bypass = false;

    int n_points = argc ? atom_getfloat(argv) : 1;
    t_float dash_length = argc > 1 ? atom_getfloat(argv+1) : 0.f;

    x->n_points    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->dash_length = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

    pd_float((t_pd*)x->n_points, n_points);
    pd_float((t_pd*)x->dash_length, dash_length);

    outlet_new(&x->x_obj, &s_signal); // default provided outlet

    return (x);
}

void dash_tilde_setup(void)
{
    dash_class = class_new(gensym("dash~"),
                            (t_newmethod)dash_new, //ctor
                            (t_method)dash_free, //dtor
                            sizeof(t_dash), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_GIMME, //n_points, dash_length
                            0); // no more args

    class_sethelpsymbol(dash_class, gensym("dash~")); // links to the help patch

    class_addmethod(dash_class, (t_method)dash_bypass, gensym("bypass"), A_DEFFLOAT, 0);

    class_addmethod(dash_class, (t_method)dash_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(dash_class, t_dash, f); // signal inlet as first inlet
}
