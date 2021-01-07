#include "m_pd.h"
#include "math.h"
#include "Audio_Math.h"

static t_class *heart_tilde_class;

typedef struct _heart_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    t_inlet *R_in, *r_in, *d_in; // driver default provided
    t_outlet *yChan_out; // *xChan_out default provided
} t_heart_tilde;

static t_int *heart_tilde_perform(t_int *w)
{
    t_sample *driver_in =      (t_sample *)(w[1]);
    t_sample *xChan_out =      (t_sample *)(w[2]);
    t_sample *yChan_out =      (t_sample *)(w[3]);
    int     nblock      =             (int)(w[4]); // get blocksize
    
    
    while (nblock--) // dsp here
    {
        t_float angle = 2 * PI * driver_in[nblock];
        
        // formula for heart
        t_float x = 16 * pow(sin(angle), 3.0);
        t_float y = 13 * cos(angle) - 5 * cos(2 * angle) - 2 * cos(3 * angle) - cos(4 * angle);
        
        xChan_out[nblock] = x * 0.06;
        yChan_out[nblock] = y * 0.06;
    }
    
    return (w + 5); // return end of the signal vector
}

static void heart_tilde_dsp(t_heart_tilde *x, t_signal **sp)
{
    dsp_add(heart_tilde_perform, 4,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // xOut
            sp[2]->s_vec, // yOut
            sp[0]->s_n // block size
            );
}


// ctor
static void *heart_tilde_new( void )
{
    t_heart_tilde *x = (t_heart_tilde *)pd_new(heart_tilde_class);
    
    outlet_new(&x->x_obj, &s_signal); // default provided outlet
    x->yChan_out = outlet_new(&x->x_obj, &s_signal); // outlet we made
    
    return (x);
}

// dtor / free
static void *heart_tilde_free(t_heart_tilde *x)
{
    outlet_free(x->yChan_out);
    
    return (void *)x;
}

// NEVER MAKE THIS STATIC!!!
void heart_tilde_setup(void)
{
    heart_tilde_class = class_new(gensym("heart~"),
                            (t_newmethod)heart_tilde_new, //ctor
                            (t_method)heart_tilde_free, //dtor
                            sizeof(t_heart_tilde), // data space
                            CLASS_DEFAULT, // gui apperance
                            0); // no more args

    class_addcreator((t_newmethod)heart_tilde_new,
                     gensym("<3~"),
                     0); // no more args
    
    class_addcreator((t_newmethod)heart_tilde_new,
                     gensym("osci/<3~"),
                     0); // no more args
    
    class_addcreator((t_newmethod)heart_tilde_new,
                     gensym("osci/heart~"),
                     0); // no more args

    
    class_sethelpsymbol(heart_tilde_class, gensym("heart~")); // links to the help patch
    
    class_addmethod(heart_tilde_class, (t_method)heart_tilde_dsp, gensym("dsp"), A_CANT, 0); // add a dsp method to data space
    CLASS_MAINSIGNALIN(heart_tilde_class, t_heart_tilde, f); // signal inlet as first inlet
}
