#include "m_pd.h"
#include <math.h>

static t_class *ellipse_tilde_class;

typedef struct _ellipse_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    int bypass;
    t_inlet *xPos_in, *yPos_in, *width_in, *height_in;
    t_outlet *y_out;
    // in1 for driver and out1 for xChan are automatically provided
} t_ellipse_tilde;

static t_int *ellipse_tilde_perform(t_int *w)
{
    t_ellipse_tilde *x  = (t_ellipse_tilde *)(w[1]);
    t_sample *driver    = (t_sample *)(w[2]);
    t_sample *xPos_in   = (t_sample *)(w[3]);
    t_sample *yPos_in   = (t_sample *)(w[4]);
    t_sample *width_in  = (t_sample *)(w[5]);
    t_sample *height_in = (t_sample *)(w[6]);
    t_sample *x_out     = (t_sample *)(w[7]);
    t_sample *y_out     = (t_sample *)(w[8]);
    int nblock          = (int)(w[9]); // get block size

    while (nblock--) // dsp here
    {
        t_sample t = driver[nblock];
        if(!x->bypass)
        {
            t_sample xPos = xPos_in[nblock];
            t_sample yPos = yPos_in[nblock];
            t_sample width = width_in[nblock];
            t_sample height = height_in[nblock];

            t_sample angle = 2 * 3.14159 * t;

            x_out[nblock] = (width * cosf(angle)) + xPos;
            y_out[nblock] = (height * sinf(angle)) + yPos;
        } else {
            x_out[nblock] = t;
            y_out[nblock] = 0;
        }
    }

    return (w + 10);
}

static void ellipse_tilde_dsp(t_ellipse_tilde *x, t_signal **sp)
{
    dsp_add(ellipse_tilde_perform, 9, x,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // xPos
            sp[2]->s_vec, // yPos
            sp[3]->s_vec, // width
            sp[4]->s_vec, // height
            sp[5]->s_vec, // xOut
            sp[6]->s_vec, // yOut
            sp[0]->s_n);  // block size
}

static void ellipse_tilde_bypass(t_ellipse_tilde *x, t_floatarg bypass) {
    x->bypass = bypass;
}

// ctor
static void *ellipse_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_ellipse_tilde *x = (t_ellipse_tilde *)pd_new(ellipse_tilde_class);

    x->bypass = 0;

    //Init inlets and variables
    t_float xpos = argc ? atom_getfloat(argv) : 0.f;
    t_float ypos = argc > 1 ? atom_getfloat(argv+1) : 0.f;
    t_float width = argc > 2 ? atom_getfloat(argv+2) : 1.f;
    t_float height = argc > 3 ? atom_getfloat(argv+3) : 1.f;

    x->xPos_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->yPos_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->width_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->height_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

    pd_float((t_pd *)x->xPos_in, xpos); // send creation args to inlets
    pd_float((t_pd *)x->yPos_in, ypos);
    pd_float((t_pd *)x->width_in, width);
    pd_float((t_pd *)x->height_in, height);

    outlet_new(&x->x_obj, &s_signal); // default provided outlet

    x->y_out = outlet_new(&x->x_obj, &s_signal); // outlet we made

    return (x);
}

// dtor / free
static void *ellipse_tilde_free(t_ellipse_tilde *x)
{
    inlet_free(x->xPos_in);
    inlet_free(x->yPos_in);
    inlet_free(x->width_in);
    inlet_free(x->height_in);

    outlet_free(x->y_out);

    return (void *)x;
}

void ellipse_tilde_setup(void)
{
    ellipse_tilde_class = class_new(gensym("ellipse~"),
                                    (t_newmethod)ellipse_tilde_new, //ctor
                                    (t_method)ellipse_tilde_free,   //dtor
                                    sizeof(t_ellipse_tilde),        // data space
                                    CLASS_DEFAULT,                  // gui apperance
                                    A_GIMME, // xpos, ypos, width, height
                                    0);                             // no more args

    class_sethelpsymbol(ellipse_tilde_class, gensym("ellipse~")); // links to the help patch
    class_addmethod(ellipse_tilde_class, (t_method)ellipse_tilde_bypass, gensym("bypass"), A_DEFFLOAT, 0);

    class_addmethod(ellipse_tilde_class, (t_method)ellipse_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(ellipse_tilde_class, t_ellipse_tilde, f); // signal inlet as first inlet
}
