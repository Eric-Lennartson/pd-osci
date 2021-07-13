#include "m_pd.h"
#include "Audio_Math.h"

static t_class *sphere_tilde_class;

// creating a lookup table would probably add some performance benefits

typedef struct _sphere_tilde
{
    t_object x_obj;
    t_sample f;                       // dummy variable for 1st inlet
    t_inlet *n_points_in, *radius_in; // phase inlet default provided
    t_outlet *y_out, *z_out;          // *x_out default provided
} t_sphere_tilde;

static t_int *sphere_tilde_perform(t_int *w)
{
    t_sample *phase_in = (t_sample *)(w[1]);
    t_sample *n_points_in = (t_sample *)(w[2]);
    t_sample *radius_in = (t_sample *)(w[3]);
    t_sample *x_out = (t_sample *)(w[4]);
    t_sample *y_out = (t_sample *)(w[5]);
    t_sample *z_out = (t_sample *)(w[6]);
    int nblock = (int)(w[7]); // get blocksize

    while (nblock--) // dsp here
    {
        t_sample t = phase_in[nblock];
        t_sample n_points = n_points_in[nblock];
        t_sample radius = radius_in[nblock];

        t_float tn = t * n_points;
        t_float latitude = map_lin(tn, 0, n_points, 0, PI, false);
        t_float longitude = map_lin(mod1(tn) * n_points, 0, n_points, 0, TWO_PI, false);

        x_out[nblock] = radius * sin(latitude) * cos(longitude);
        y_out[nblock] = radius * sin(latitude) * sin(longitude);
        z_out[nblock] = radius * cos(latitude);
    }

    return (w + 8);
}

static void sphere_tilde_dsp(t_sphere_tilde *x, t_signal **sp)
{
    dsp_add(sphere_tilde_perform, 7,
            sp[0]->s_vec, // phase
            sp[1]->s_vec, // n_points
            sp[2]->s_vec, // radius
            sp[3]->s_vec, // x_Out
            sp[4]->s_vec, // y_Out
            sp[5]->s_vec, // z_out
            sp[0]->s_n    // block size
    );
}

// ctor
static void *sphere_tilde_new(t_floatarg n_pts, t_floatarg r)
{
    t_sphere_tilde *x = (t_sphere_tilde *)pd_new(sphere_tilde_class);

    int n_points = (n_pts < 1) ? 1 : n_pts;
    t_float radius = (r < 0) ? 1 : r;

    x->n_points_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->radius_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

    // send creation args to inlets, allows for floats to inlets
    pd_float((t_pd *)x->n_points_in, n_points);
    pd_float((t_pd *)x->radius_in, radius);

    outlet_new(&x->x_obj, &s_signal); // default provided outlet
    x->y_out = outlet_new(&x->x_obj, &s_signal); 
    x->z_out = outlet_new(&x->x_obj, &s_signal);

    return (x);
}

// dtor / free
static void *sphere_tilde_free(t_sphere_tilde *x)
{
    inlet_free(x->n_points_in);
    inlet_free(x->radius_in);
    outlet_free(x->y_out);
    outlet_free(x->z_out);

    return (void *)x;
}

void sphere_tilde_setup(void)
{
    sphere_tilde_class = class_new(gensym("sphere~"),
                                   (t_newmethod)sphere_tilde_new, //ctor
                                   (t_method)sphere_tilde_free,   //dtor
                                   sizeof(t_sphere_tilde),        // data space
                                   CLASS_DEFAULT,                 // gui apperance
                                   A_DEFFLOAT,                    // n_points
                                   A_DEFFLOAT,                    // radius
                                   0);                            // no more args

    class_sethelpsymbol(sphere_tilde_class, gensym("sphere~")); // links to the help patch

    class_addmethod(sphere_tilde_class, (t_method)sphere_tilde_dsp, gensym("dsp"), A_CANT, 0); // add a dsp method to data space
    CLASS_MAINSIGNALIN(sphere_tilde_class, t_sphere_tilde, f);                                 // signal inlet as first inlet
}
