#include "m_pd.h"
#include "math.h"

const float PI = 3.14159265358979323846;

static t_class *supershape_tilde_class;

typedef struct _supershape_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    t_inlet *n1_in, *n2_in, *n3_in, *m_in, *a_in, *b_in; // driver default provided
    t_outlet *yChan_out; // *xChan_out default provided
} t_supershape_tilde;

static t_int *supershape_tilde_perform(t_int *w)
{
    t_sample *driver_in =      (t_sample *)(w[1]);
    t_sample *n1_in     =      (t_sample *)(w[2]);
    t_sample *n2_in     =      (t_sample *)(w[3]);
    t_sample *n3_in     =      (t_sample *)(w[4]);
    t_sample *m_in      =      (t_sample *)(w[5]);
    t_sample *a_in      =      (t_sample *)(w[6]);
    t_sample *b_in      =      (t_sample *)(w[7]);
    t_sample *xChan_out =      (t_sample *)(w[8]);
    t_sample *yChan_out =      (t_sample *)(w[9]);
    int     nblock      =             (int)(w[10]); // get blocksize
    
    
    while (nblock--) // dsp here
    {
        t_float angle = 2 * PI * driver_in[nblock];
        t_float n1 = n1_in[nblock];
        t_float n2 = n2_in[nblock];
        t_float n3 = n3_in[nblock];
        t_float m  = m_in[nblock];
        t_float a  = a_in[nblock];
        t_float b  = b_in[nblock];
        
        t_float part1 = (1.f/ a) * cos((angle * m) / 4.f);
        t_float part2 = (1.f/ b) * sin((angle * m) / 4.f);
        
        part1 = powf( fabs(part1), n2);
        part2 = powf( fabs(part2), n3);
        
        part1 += part2; // combine them
        part1 = powf(part1, 1.f / n1);
        part1 = (part1 == 0.0) ? 0 : 1.f / part1; // divide by zero check
        
        xChan_out[nblock] = part1 * cos(angle);
        yChan_out[nblock] = part1 * sin(angle);
    }
    
    return (w + 11); // 1 larger than number of pointers (10 + 1 = 11)
}

static void supershape_tilde_dsp(t_supershape_tilde *x, t_signal **sp)
{
    dsp_add(supershape_tilde_perform, 10,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // n1
            sp[2]->s_vec, // n2
            sp[3]->s_vec, // n3
            sp[4]->s_vec, // m
            sp[5]->s_vec, // a
            sp[6]->s_vec, // b
            sp[7]->s_vec, // xOut
            sp[8]->s_vec, // yOut
            sp[0]->s_n // block size
            );
}


// ctor                     // name used for creation, number of args, pointer to arg list
static void *supershape_tilde_new(t_symbol *s, int argc, t_atom *argv) // this is because of A_GIMME
{
    t_supershape_tilde *x = (t_supershape_tilde *)pd_new(supershape_tilde_class);
   
    t_float n1, n2, n3, m, a, b;
    
    n1 = argc ? atom_getfloat(argv) : 1;
    n2 = argc>1 ? atom_getfloat(argv+1) : 1;
    n3 = argc>2 ? atom_getfloat(argv+2) : 1;
    m  = argc>3 ? atom_getfloat(argv+3) : 1;
    a  = argc>4 ? atom_getfloat(argv+4) : 1;
    b  = argc>5 ? atom_getfloat(argv+5) : 1;
    
    x->n1_in  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->n2_in  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->n3_in  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->m_in   = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->a_in   = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->b_in   = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
    
    pd_float((t_pd*)x->n1_in, n1); // send creation args to inlets, allows for floats to inlets
    pd_float((t_pd*)x->n2_in, n2);
    pd_float((t_pd*)x->n3_in, n3);
    pd_float((t_pd*)x->m_in, m);
    pd_float((t_pd*)x->a_in, a);
    pd_float((t_pd*)x->b_in, b);
    
    outlet_new(&x->x_obj, &s_signal); // default provided outlet
    x->yChan_out = outlet_new(&x->x_obj, &s_signal); // outlet we made
    
    return (x);
}

// dtor / free
static void *supershape_tilde_free(t_supershape_tilde *x)
{
    inlet_free(x->n1_in);
    inlet_free(x->n2_in);
    inlet_free(x->n3_in);
    inlet_free(x->m_in);
    inlet_free(x->a_in);
    inlet_free(x->b_in);
    
    outlet_free(x->yChan_out);
    
    return (void *)x;
}

void supershape_tilde_setup(void)
{
    supershape_tilde_class = class_new(gensym("supershape~"),
                            (t_newmethod)supershape_tilde_new, //ctor
                            (t_method)supershape_tilde_free, //dtor
                            sizeof(t_supershape_tilde), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_GIMME,
                            0); // no more args
    
    class_sethelpsymbol(supershape_tilde_class, gensym("supershape~")); // links to the help patch
    
    class_addmethod(supershape_tilde_class, (t_method)supershape_tilde_dsp, gensym("dsp"), A_CANT, 0); // add a dsp method to data space
    CLASS_MAINSIGNALIN(supershape_tilde_class, t_supershape_tilde, f); // signal inlet as first inlet
}
