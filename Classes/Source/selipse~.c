#include "m_pd.h"
#include "math.h"

const float PI = 3.14159265358979323846;

static inline int sign(const t_float val) {
    return (val > 0) ? 1 : (val < 0) ? -1 : 0;
}

static t_class *selipse_tilde_class;

typedef struct _selipse_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    t_inlet *a_in, *b_in, *n_in; // phase default provided
    t_outlet *yChan_out; // *xChan_out default provided
} t_selipse_tilde;

static t_int *selipse_tilde_perform(t_int *w)
{
    t_sample *phase_in =      (t_sample *)(w[1]);
    t_sample *a_in      =      (t_sample *)(w[2]);
    t_sample *b_in      =      (t_sample *)(w[3]);
    t_sample *n_in      =      (t_sample *)(w[4]);
    t_sample *xChan_out =      (t_sample *)(w[5]);
    t_sample *yChan_out =      (t_sample *)(w[6]);
    int     nblock      =             (int)(w[7]); // get blocksize
    
    
    while (nblock--) // dsp here
    {
        t_float angle = 2 * PI * phase_in[nblock];
        t_float a  = a_in[nblock];
        t_float b  = b_in[nblock];
        t_float n  = n_in[nblock];
        
        // formula for super ellipse
        t_float x = powf( fabs( cos(angle) ), 2 / n) * a * sign(cos(angle));
        t_float y = powf( fabs( sin(angle) ), 2 / n) * b * sign(sin(angle));
        
        xChan_out[nblock] = x;
        yChan_out[nblock] = y;
    }
    
    return (w + 8); // return end of the signal vector
}

static void selipse_tilde_dsp(t_selipse_tilde *x, t_signal **sp)
{
    dsp_add(selipse_tilde_perform, 7,
            sp[0]->s_vec, // phase
            sp[1]->s_vec, // a
            sp[2]->s_vec, // b
            sp[3]->s_vec, // n
            sp[4]->s_vec, // xOut
            sp[5]->s_vec, // yOut
            sp[0]->s_n // block size
            );
}


// ctor                     // name used for creation, number of args, pointer to arg list
static void *selipse_tilde_new(t_symbol *s, int argc, t_atom *argv) // this is because of A_GIMME
{
    t_selipse_tilde *x = (t_selipse_tilde *)pd_new(selipse_tilde_class);
   
    t_float a = argc ? atom_getfloat(argv) : 1;
    t_float b = argc>1 ? atom_getfloat(argv+1) : 1;
    t_float n = argc>2 ? atom_getfloat(argv+2) : 3;

    x->a_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->b_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->n_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
    pd_float((t_pd*)x->a_in, a);  // init the inlets with creation args
    pd_float((t_pd*)x->b_in, b);
    pd_float((t_pd*)x->n_in, n);
    
    outlet_new(&x->x_obj, &s_signal); // default provided outlet
    x->yChan_out = outlet_new(&x->x_obj, &s_signal); // outlet we made
    
    return (x);
}

// dtor / free
static void *selipse_tilde_free(t_selipse_tilde *x)
{
    inlet_free(x->a_in);
    inlet_free(x->b_in);
    inlet_free(x->n_in);
    
    outlet_free(x->yChan_out);
    
    return (void *)x;
}

// NEVER MAKE THIS STATIC!!!
void selipse_tilde_setup(void)
{
    selipse_tilde_class = class_new(gensym("selipse~"),
                            (t_newmethod)selipse_tilde_new, //ctor
                            (t_method)selipse_tilde_free, //dtor
                            sizeof(t_selipse_tilde), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_GIMME, // a, b, n
                            0); // no more args
    
    class_sethelpsymbol(selipse_tilde_class, gensym("selipse~")); // links to the help patch
    
    class_addmethod(selipse_tilde_class, (t_method)selipse_tilde_dsp, gensym("dsp"), A_CANT, 0); // add a dsp method to data space
    CLASS_MAINSIGNALIN(selipse_tilde_class, t_selipse_tilde, f); // signal inlet as first inlet
}
