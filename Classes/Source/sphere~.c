#include "m_pd.h"
#include "Audio_Math.h"

static t_class *sphere_tilde_class;

// creating a lookup table would probably add some performance benefits

/* 
* If I want to mess with the equation to draw the sphere, I'll leave that up to future me.
* I'm not sure if there's much to be gained by adding that functionality in. How much will
* I really be using it?
* This is really a case for messing with livecoding in pd, creating a zillion objects to do
* every conceivable variation isn't the best idea in the world.
*/

typedef struct _sphere_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    int phase_mode;
    t_inlet *phase2_in, *n_points_in, *radius_in, 
            *lat_min_in, *lat_max_in, *lon_min_in, 
            *lon_max_in;     // phase inlet default provided
    t_outlet *y_out, *z_out; // *x_out default provided
} t_sphere_tilde;

void set_phase_mode(t_sphere_tilde *x, t_float mode)
{
    mode = (int)mode;
    if(mode < 0 || mode > 2) 
        mode = 0;
    x->phase_mode = mode;
}


static t_int *sphere_tilde_perform(t_int *w)
{
    t_sphere_tilde *x     = (t_sphere_tilde *)(w[1]);
    t_sample *phase_in    = (t_sample *)(w[2]);
    t_sample *phase2_in   = (t_sample *)(w[3]);
    t_sample *lat_min_in  = (t_sample *)(w[4]);
    t_sample *lat_max_in  = (t_sample *)(w[5]);
    t_sample *lon_min_in  = (t_sample *)(w[6]);
    t_sample *lon_max_in  = (t_sample *)(w[7]);
    t_sample *n_points_in = (t_sample *)(w[8]);
    t_sample *radius_in   = (t_sample *)(w[9]);
    t_sample *x_out       = (t_sample *)(w[10]);
    t_sample *y_out       = (t_sample *)(w[11]);
    t_sample *z_out       = (t_sample *)(w[12]);
    int nblock            = (int)(w[13]);

    while (nblock--) // dsp here
    {
        t_sample t        = phase_in[nblock];
        t_sample t2       = phase2_in[nblock];
        t_sample lat_min  = lat_min_in[nblock];
        t_sample lat_max  = lat_max_in[nblock];
        t_sample lon_min  = lon_min_in[nblock];
        t_sample lon_max  = lon_max_in[nblock];
        t_sample n_points = n_points_in[nblock];
        t_sample radius   = radius_in[nblock];

        t_float tn = t * n_points;

        switch(x->phase_mode) 
        {
            case 0:
                t2 = mod1(tn) * n_points;
                break;
            case 1:
                t2 = mod1(tn+t2) * n_points;
                break;
            case 2:
                t2 = mod1(t2 * n_points) * n_points;
                break;
            default:
                t2 = mod1(tn) * n_points;
                break;
        }

        t_float lat = map_lin(tn, 0, n_points, lat_min, lat_max, false);
        t_float lon = map_lin(t2, 0, n_points, lon_min, lon_max, false);

        x_out[nblock] = radius * sin(lat) * cos(lon);
        y_out[nblock] = radius * sin(lat) * sin(lon);
        z_out[nblock] = radius * cos(lat);
    }

    return (w + 14);
}

static void sphere_tilde_dsp(t_sphere_tilde *x, t_signal **sp)
{
    dsp_add(sphere_tilde_perform, 13, x,
            sp[0]->s_vec, // phase
            sp[1]->s_vec, // phase2
            sp[2]->s_vec, // lat_min
            sp[3]->s_vec, // lat_max
            sp[4]->s_vec, // lon_min
            sp[5]->s_vec, // lon_max
            sp[6]->s_vec, // n_points
            sp[7]->s_vec, // radius
            sp[8]->s_vec, // x_out
            sp[9]->s_vec, // y_out
            sp[10]->s_vec, // z_out
            sp[0]->s_n    // block size
    );
}

// ctor
static void *sphere_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_sphere_tilde *x = (t_sphere_tilde *)pd_new(sphere_tilde_class);

    x->phase_mode = 0;
    t_float lat_min = argc>0 ? atom_getfloat(argv)   : 0;
    t_float lat_max = argc>1 ? atom_getfloat(argv+1) : PI;
    t_float lon_min = argc>2 ? atom_getfloat(argv+2) : 0;
    t_float lon_max = argc>3 ? atom_getfloat(argv+3) : TWO_PI;
    int n_points    = argc>4 ? atom_getfloat(argv+4) : 10;
    t_float radius  = argc>5 ? atom_getfloat(argv+5) : 1;

    n_points = (n_points < 1) ? 1 : n_points;

    x->phase2_in   = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->lat_min_in  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->lat_max_in  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->lon_min_in  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->lon_max_in  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->n_points_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->radius_in   = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

    // send creation args to inlets, allows for floats to inlets
    pd_float((t_pd *)x->lat_min_in, lat_min);
    pd_float((t_pd *)x->lat_max_in, lat_max);
    pd_float((t_pd *)x->lon_min_in, lon_min);
    pd_float((t_pd *)x->lon_max_in, lon_max);
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
    inlet_free(x->phase2_in);
    inlet_free(x->lat_min_in);
    inlet_free(x->lat_max_in);
    inlet_free(x->lon_min_in);
    inlet_free(x->lon_max_in);
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
                                   A_GIMME, // lat min, lat max, lon min, lon max, n_points, radius
                                   0);                            // no more args

    class_sethelpsymbol(sphere_tilde_class, gensym("sphere~")); // links to the help patch

    class_addmethod(sphere_tilde_class, (t_method)set_phase_mode, gensym("mode"), A_DEFFLOAT, 0);
    class_addmethod(sphere_tilde_class, (t_method)sphere_tilde_dsp, gensym("dsp"), A_CANT, 0); // add a dsp method to data space
    CLASS_MAINSIGNALIN(sphere_tilde_class, t_sphere_tilde, f);                                 // signal inlet as first inlet
}
