#include "m_pd.h"
#include <math.h>

static t_class *bezier_tilde_class;
typedef struct _bezier_tilde
{
    t_object x_obj;
    t_float f; // dummy
    t_inlet  *x1, *y1, *x2, *y2, *x3, *y3; // x2 and y2 is the control point
    t_outlet *x_out, *y_out;
} t_bezier_tilde;

t_float bezier(t_float t, t_float v1, t_float v2, t_float v3) 
{ // takes all of one type of coordinate, e.g. only x values etc.
    return pow(1 - t, 2) * v1 + (1 - t) * 2 * t * v2 + t * t * v3;
}

static t_int *bezier_tilde_perform(t_int *w)
{
    t_float *driver_in  = (t_float *)(w[1]);
    t_float *x1_in      = (t_float *)(w[2]);
    t_float *y1_in      = (t_float *)(w[3]);
    t_float *x2_in      = (t_float *)(w[4]);
    t_float *y2_in      = (t_float *)(w[5]);
    t_float *x3_in      = (t_float *)(w[6]);
    t_float *y3_in      = (t_float *)(w[7]);
    t_float *x_out      = (t_float *)(w[8]);
    t_float *y_out      = (t_float *)(w[9]);
    int nblock          =       (int)(w[10]);
    
    while (nblock--)
    {
        t_float t  = driver_in[nblock];
        t_float x1 = x1_in[nblock];
        t_float y1 = y1_in[nblock];
        t_float x2 = x2_in[nblock];
        t_float y2 = y2_in[nblock];
        t_float x3 = x3_in[nblock];
        t_float y3 = y3_in[nblock];

        t_float x = bezier(t, x1, x2, x3);
        t_float y = bezier(t, y1, y2, y3);

        x_out[nblock] = x;
        y_out[nblock] = y;
    }
    return (w + 11);
}

static void bezier_tilde_dsp(t_bezier_tilde *x, t_signal **sp)
{
    dsp_add(bezier_tilde_perform, 10,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // x1
            sp[2]->s_vec, // y1
            sp[3]->s_vec, // x2
            sp[4]->s_vec, // y2
            sp[5]->s_vec, // x3
            sp[6]->s_vec, // y3
            sp[7]->s_vec, // x_out
            sp[8]->s_vec, // y_out
            sp[0]->s_n); // block size
}

static void *bezier_tilde_free(t_bezier_tilde *x)
{
    inlet_free(x->x1);
    inlet_free(x->y1);
    inlet_free(x->x2);
    inlet_free(x->y2);
    inlet_free(x->x3);
    inlet_free(x->y3);
    
    outlet_free(x->x_out);
    outlet_free(x->y_out);
    
    return (void *)x;
}

static void *bezier_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_bezier_tilde *x = (t_bezier_tilde *)pd_new(bezier_tilde_class);
    
    x->x1  = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->y1  = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->x2  = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->y2  = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->x3  = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    x->y3  = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    
    switch(argc)
    {
        case 6:
        {
            pd_float((t_pd *)x->x1, atom_getfloat(argv) );
            pd_float((t_pd *)x->y1, atom_getfloat(argv+1) );
            pd_float((t_pd *)x->x2, atom_getfloat(argv+2) );
            pd_float((t_pd *)x->y2, atom_getfloat(argv+3) );
            pd_float((t_pd *)x->x3, atom_getfloat(argv+4) );
            pd_float((t_pd *)x->y3, atom_getfloat(argv+5) );
            break;
        }
        case 5:
        {
            pd_float((t_pd *)x->x1, atom_getfloat(argv) );
            pd_float((t_pd *)x->y1, atom_getfloat(argv+1) );
            pd_float((t_pd *)x->x2, atom_getfloat(argv+2) );
            pd_float((t_pd *)x->y2, atom_getfloat(argv+3) );
            pd_float((t_pd *)x->x3, atom_getfloat(argv+4) );
            pd_float((t_pd *)x->y3, 0 );
            break;
        }
        case 4:
        {
            pd_float((t_pd *)x->x1, atom_getfloat(argv) );
            pd_float((t_pd *)x->y1, atom_getfloat(argv+1) );
            pd_float((t_pd *)x->x2, atom_getfloat(argv+2) );
            pd_float((t_pd *)x->y2, atom_getfloat(argv+3) );
            pd_float((t_pd *)x->x3, 0 );
            pd_float((t_pd *)x->y3, 0 );
            break;
        }
        case 3:
        {
            pd_float((t_pd *)x->x1, atom_getfloat(argv) );
            pd_float((t_pd *)x->y1, atom_getfloat(argv+1) );
            pd_float((t_pd *)x->x2, atom_getfloat(argv+2) );
            pd_float((t_pd *)x->y2, 0 );
            pd_float((t_pd *)x->x3, 0 );
            pd_float((t_pd *)x->y3, 0 );
            break;
        }
        case 2:
        {
            pd_float((t_pd *)x->x1, atom_getfloat(argv) );
            pd_float((t_pd *)x->y1, atom_getfloat(argv+1) );
            pd_float((t_pd *)x->x2, 0 );
            pd_float((t_pd *)x->y2, 0 );
            pd_float((t_pd *)x->x3, 0 );
            pd_float((t_pd *)x->y3, 0 );
            break;
        }
        case 1:
        {
            pd_float((t_pd *)x->x1, atom_getfloat(argv) );
            pd_float((t_pd *)x->y1, 0 );
            pd_float((t_pd *)x->x2, 0 );
            pd_float((t_pd *)x->y2, 0 );
            pd_float((t_pd *)x->x3, 0 );
            pd_float((t_pd *)x->y3, 0 );
            break;
        }
        default:
        {
            pd_float((t_pd *)x->x1, 0 );
            pd_float((t_pd *)x->y1, 0 );
            pd_float((t_pd *)x->x2, 0 );
            pd_float((t_pd *)x->y2, 0 );
            pd_float((t_pd *)x->x3, 0 );
            pd_float((t_pd *)x->y3, 0 );
            break;
        }
    }

    x->x_out = outlet_new((t_object *)x, &s_signal);
    x->y_out = outlet_new((t_object *)x, &s_signal);
    
    return (x);
}

void bezier_tilde_setup(void)
{
    bezier_tilde_class = class_new(gensym("bezier~"),
                (t_newmethod)bezier_tilde_new,
                (t_method)bezier_tilde_free,
                sizeof(t_bezier_tilde),
                CLASS_DEFAULT,
                A_GIMME,
                0);
    
    class_sethelpsymbol(bezier_tilde_class, gensym("bezier~"));
    
    class_addmethod(bezier_tilde_class, (t_method)bezier_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(bezier_tilde_class, t_bezier_tilde, f); // signal inlet as first inlet
}
