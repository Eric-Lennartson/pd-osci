#include "m_pd.h"

static t_class *trans_tilde_class;

typedef struct _trans_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    int bypass;
    t_inlet *y_in, *z_in, *xoff_in, *yoff_in, *zoff_in; // x_in default provided
    t_outlet *y_out, *z_out; // x_out default provided
} t_trans_tilde;

static t_int *trans_tilde_perform(t_int *w)
{
    t_trans_tilde *this = (t_trans_tilde *)(w[1]);
    t_sample *x_in          = (t_sample *)(w[2]);
    t_sample *y_in          = (t_sample *)(w[3]);
    t_sample *z_in          = (t_sample *)(w[4]);
    t_sample *xoff_in       = (t_sample *)(w[5]);
    t_sample *yoff_in       = (t_sample *)(w[6]);
    t_sample *zoff_in       = (t_sample *)(w[7]);
    t_sample *x_out         = (t_sample *)(w[8]);
    t_sample *y_out         = (t_sample *)(w[9]);
    t_sample *z_out         = (t_sample *)(w[10]);
    int nblock              = (int)(w[11]); // get block size

    while (nblock--) // dsp here
    {
        t_sample x = x_in[nblock];
        t_sample y = y_in[nblock];
        t_sample z = z_in[nblock];

        if(!this->bypass)
        {
            t_sample xoff = xoff_in[nblock];
            t_sample yoff = yoff_in[nblock];
            t_sample zoff = zoff_in[nblock];

            x_out[nblock] = x + xoff;
            y_out[nblock] = y + yoff;
            z_out[nblock] = z + zoff;
        } else {
            x_out[nblock] = x;
            y_out[nblock] = y;
            z_out[nblock] = z;
        }
    }

    return (w + 12);
}

static void trans_tilde_dsp(t_trans_tilde *x, t_signal **sp)
{
    dsp_add(trans_tilde_perform, 11, x,
            sp[0]->s_vec, // x_in
            sp[1]->s_vec, // y_in
            sp[2]->s_vec, // z_in
            sp[3]->s_vec, // xoff
            sp[4]->s_vec, // yoff
            sp[5]->s_vec, // zoff
            sp[6]->s_vec, // xOut
            sp[7]->s_vec, // yOut
            sp[8]->s_vec, // zOut
            sp[0]->s_n); // block size
}

static void trans_tilde_bypass(t_trans_tilde *x, t_floatarg bypass) {
  x->bypass = (int)bypass;
}

// ctor
static void *trans_tilde_new(t_floatarg xoff, t_floatarg yoff, t_floatarg zoff)
{
    t_trans_tilde *x = (t_trans_tilde *)pd_new(trans_tilde_class);

    x->bypass = 0;

    x->y_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->z_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->xoff_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->yoff_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->zoff_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

    pd_float((t_pd*)x->xoff_in, xoff); // send creation args to inlets
    pd_float((t_pd*)x->yoff_in, yoff);
    pd_float((t_pd*)x->zoff_in, zoff);

    outlet_new(&x->x_obj, &s_signal); // default provided outlet

    x->y_out = outlet_new(&x->x_obj, &s_signal);
    x->z_out = outlet_new(&x->x_obj, &s_signal);

    return (x);
}

// dtor / free
static void *trans_tilde_free(t_trans_tilde *x)
{
    inlet_free(x->y_in);
    inlet_free(x->z_in);
    inlet_free(x->xoff_in);
    inlet_free(x->yoff_in);
    inlet_free(x->zoff_in);

    outlet_free(x->y_out);
    outlet_free(x->z_out);

    return (void *)x;
}

void trans_tilde_setup(void)
{
    trans_tilde_class = class_new(gensym("trans~"),
                            (t_newmethod)trans_tilde_new, //ctor
                            (t_method)trans_tilde_free, //dtor
                            sizeof(t_trans_tilde), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_DEFFLOAT, // xoff
                            A_DEFFLOAT, // yoff
                            A_DEFFLOAT, // zoff
                            0); // no more args

    class_sethelpsymbol(trans_tilde_class, gensym("trans~")); // links to the help patch
    class_addmethod(trans_tilde_class, (t_method)trans_tilde_bypass, gensym("bypass"), A_DEFFLOAT, 0);

    class_addmethod(trans_tilde_class, (t_method)trans_tilde_dsp, gensym("dsp"), A_CANT, 0); // add a dsp method to data space
    CLASS_MAINSIGNALIN(trans_tilde_class, t_trans_tilde, f); // signal inlet as first inlet
}
