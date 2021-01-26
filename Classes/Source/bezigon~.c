#include "m_pd.h"
#include <math.h>

static t_class *bezigon_tilde_class;

typedef struct _bezigon_tilde
{
    t_object x_obj;
    t_float f; // dummy
    int n_points;
    t_float end;
    t_inlet  *end_in;
    t_float *x_points, *y_points;
    t_outlet *out1, *out2;
} t_bezigon_tilde;

// TODO refactor so to include Audio_Math
static t_float mod1(t_float val) { return val - (int)val; }

static void onlistmsg(t_bezigon_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    if(argc != 3)
        pd_error(x, "[bezigon~] takes a list of 3 args: index, x, y.");
    else
    {
        if(atom_getfloat(argv) < 0 )
            pd_error(x, "warning: [bezigon~] is being passed a negative index, taking the abs value.");
        int idx = fabs( atom_getfloat(argv) );
        
        if(idx < x->n_points)
        {
            t_float x_val = atom_getfloat(argv+1);
            t_float y_val = atom_getfloat(argv+2);
            
            x->x_points[idx] = x_val;
            x->y_points[idx] = y_val;
        }
        else
            pd_error(x, "index: %d is out of bounds, [bezigon~] only has %d points.", idx, x->n_points);
    }
}

// bezigon sounds cooler than cubigon.
t_float cubic_interp(t_float t, t_float *points, int n_points, int end)
{
    t_float a, b, c, d;
    
    t_float tn = t * (n_points - !end);
    int idx = tn;

    int idx_prev = idx > 0 ? idx - 1 : n_points - 1;
    int idx_1 = (idx + 1) % n_points;
    int idx_2 = (idx + 2) % n_points;

    a = points[idx_prev];
    b = points[idx];
    c = points[idx_1];
    d = points[idx_2];


    t_float p0, p1, p2;

    p0 = d - a + b - c;
    p1 = a - b - p0;
    p2 = c - a;
    
    t_float frac = mod1(tn);

    // copied from wave.c from the cyclone library
    t_float out = (b + frac * (p2 + frac * (p1+frac*p0)));

    return out;
}

static t_int *bezigon_tilde_perform(t_int *w)
{
    t_bezigon_tilde *x  = (t_bezigon_tilde*)(w[1]);
    t_float *driver_in  =        (t_float *)(w[2]);
    t_float *x_out      =        (t_float *)(w[3]);
    t_float *y_out      =        (t_float *)(w[4]);
    int nblock          =              (int)(w[5]);

    while (nblock--)
    {
        t_float t = driver_in[nblock];
        
        x_out[nblock] = cubic_interp(t, x->x_points, x->n_points, x->end); // this could be better too?
        y_out[nblock] = cubic_interp(t, x->y_points, x->n_points, x->end);
    }
    return (w + 6);
}

static void bezigon_tilde_dsp(t_bezigon_tilde *x, t_signal **sp)
{
    dsp_add(bezigon_tilde_perform, 5, x,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // out1
            sp[2]->s_vec, // out2
            sp[0]->s_n);  // block size
}


static void *bezigon_tilde_free(t_bezigon_tilde *x)
{
    if(x->x_points)
        freebytes(x->x_points, x->n_points * sizeof(t_float) );
    if(x->y_points)
        freebytes(x->y_points, x->n_points * sizeof(t_float) );
    
    return (void *)x;
}

static void *bezigon_tilde_new(t_floatarg n_points, t_floatarg end)
{
    t_bezigon_tilde *x = (t_bezigon_tilde *)pd_new(bezigon_tilde_class);
    
    // n_points can't be less than 1 (or maybe 3?)
    x->n_points = (int)n_points > 0 ? (int)n_points : 1;
    
    x->end = end ? 1.f : 0.f; // float to bool

    floatinlet_new(&x->x_obj, &x->end);
    
    x->out1 = outlet_new(&x->x_obj, &s_signal);
    x->out2 = outlet_new(&x->x_obj, &s_signal);

    x->x_points = getbytes( x->n_points * sizeof(t_float) ); // allocate memory for the array
    x->y_points = getbytes( x->n_points * sizeof(t_float) ); // allocate memory for the array
    
    return (x);
}

void bezigon_tilde_setup(void)
{
    bezigon_tilde_class = class_new(gensym("bezigon~"),
                (t_newmethod)bezigon_tilde_new,
                (t_method)bezigon_tilde_free,
                sizeof(t_bezigon_tilde),
                CLASS_DEFAULT,
                A_DEFFLOAT, // n_points
                A_DEFFLOAT, // end
                0);
    
    class_addlist(bezigon_tilde_class, onlistmsg);
    
    class_addmethod(bezigon_tilde_class, (t_method)bezigon_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(bezigon_tilde_class, t_bezigon_tilde, f); // signal inlet as first inlet
}
