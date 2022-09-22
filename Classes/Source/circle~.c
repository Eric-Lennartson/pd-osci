#include "m_pd.h"
#include <math.h>

static t_class *circle_tilde_class;

typedef struct _circle_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    int bypass;
    t_inlet *xPos_in, *yPos_in, *radius_in;
    t_outlet *y_out;
    // in1 and out1 are automatically provided
} t_circle_tilde;

static t_int *circle_tilde_perform(t_int *w)
{
    t_circle_tilde *x   = (t_circle_tilde *)(w[1]);
    t_sample *driver    = (t_sample *)(w[2]);
    t_sample *xPos_in   = (t_sample *)(w[3]);
    t_sample *yPos_in   = (t_sample *)(w[4]);
    t_sample *radius_in = (t_sample *)(w[5]);
    t_sample *x_out     = (t_sample *)(w[6]);
    t_sample *y_out     = (t_sample *)(w[7]);
    int      nblock     =        (int)(w[8]); // get block size

    while (nblock--)
    {
        t_sample t = driver[nblock];
        if(!x->bypass)
        {
            t_sample xPos = xPos_in[nblock];
            t_sample yPos = yPos_in[nblock];
            t_sample radius = radius_in[nblock];

            t_sample angle = 2 * 3.14159 * t;

            x_out[nblock] = (radius * cosf(angle)) + xPos;
            y_out[nblock] = (radius * sinf(angle)) + yPos;
        } else {
            x_out[nblock] = t;
            y_out[nblock] = 0;
        }
    }

    return (w + 9);
}

static void circle_tilde_dsp(t_circle_tilde *x, t_signal **sp)
{
    dsp_add(circle_tilde_perform, 8, x,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // xPos
            sp[2]->s_vec, // yPos
            sp[3]->s_vec, // radius
            sp[4]->s_vec, // xOut
            sp[5]->s_vec, // yOut
            sp[0]->s_n); // block size
}

static void circle_tilde_bypass(t_circle_tilde *x, t_floatarg bypass) {
    x->bypass = bypass;
}


static void *circle_tilde_free(t_circle_tilde *x)
{
    inlet_free(x->xPos_in);
    inlet_free(x->yPos_in);
    inlet_free(x->radius_in);

    outlet_free(x->y_out);

    return (void *)x;
}

static void *circle_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_circle_tilde *x = (t_circle_tilde *)pd_new(circle_tilde_class);

    x->bypass = 0;

    // Init inlets and variables
    t_float xpos = argc ? atom_getfloat(argv) : 0.f;
    t_float ypos = argc > 1 ? atom_getfloat(argv+1) : 0.f;
    t_float radius = argc > 2 ? atom_getfloat(argv+2) : 1.f;

    x->xPos_in      = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->yPos_in      = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->radius_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

    pd_float((t_pd*)x->xPos_in, xpos); // send creation args to inlets
    pd_float((t_pd*)x->yPos_in, ypos);
    pd_float((t_pd*)x->radius_in, radius);

    outlet_new(&x->x_obj, &s_signal); // default provided outlet

    x->y_out = outlet_new(&x->x_obj, &s_signal); // outlet we made

    return (x);
}

void circle_tilde_setup(void)
{
    circle_tilde_class = class_new(gensym("circle~"),
                            (t_newmethod)circle_tilde_new, //ctor
                            (t_method)circle_tilde_free, //dtor
                            sizeof(t_circle_tilde), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_GIMME, // xpos, ypos, radius
                            0); // no more args

    class_sethelpsymbol(circle_tilde_class, gensym("circle~")); // links to the help patch

    class_addmethod(circle_tilde_class, (t_method)circle_tilde_bypass, gensym("bypass"), A_DEFFLOAT, 0);

    class_addmethod(circle_tilde_class, (t_method)circle_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(circle_tilde_class, t_circle_tilde, f); // signal inlet as first inlet
}
