#include "Audio_Math.h"

static t_class *hypotrochoid_tilde_class;

typedef struct _hypotrochoid_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    t_inlet *R_in, *r_in, *d_in; // driver default provided
    t_outlet *yChan_out; // *xChan_out default provided
} t_hypotrochoid_tilde;

static t_int *hypotrochoid_tilde_perform(t_int *w)
{
    t_sample *driver_in =      (t_sample *)(w[1]);
    t_sample *R_in      =      (t_sample *)(w[2]);
    t_sample *r_in      =      (t_sample *)(w[3]);
    t_sample *d_in      =      (t_sample *)(w[4]);
    t_sample *xChan_out =      (t_sample *)(w[5]);
    t_sample *yChan_out =      (t_sample *)(w[6]);
    int     nblock      =             (int)(w[7]); // get blocksize
    
    t_float last_R = 0;
    t_float last_r = 0;
    int multiple = 1;
    
    while (nblock--) // dsp here
    {
        t_float R  = R_in[nblock];
        t_float r  = r_in[nblock];
        t_float d  = d_in[nblock];
        
        if(last_R != R || last_r != r) // don't call gcd if we don't have to
            multiple = (R == 1) ? 1 : gcd( (int)R, (int)r);
        
        t_float angle = 2 * PI * (r / multiple) * driver_in[nblock];
        
        // formula for hypotrochoid
        t_float x = (R - r) * cos(angle) + ( d * cos( ((R-r) / r) * angle ) );
        t_float y = (R - r) * sin(angle) - ( d * sin( ((R-r) / r) * angle ) );
        
        //update last_R and last_r
        last_R = R;
        last_r = r;
        
        xChan_out[nblock] = x;
        yChan_out[nblock] = y;
    }
    
    return (w + 8); // return end of the signal vector
}

static void hypotrochoid_tilde_dsp(t_hypotrochoid_tilde *x, t_signal **sp)
{
    dsp_add(hypotrochoid_tilde_perform, 7,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // R
            sp[2]->s_vec, // r
            sp[3]->s_vec, // d
            sp[4]->s_vec, // xOut
            sp[5]->s_vec, // yOut
            sp[0]->s_n // block size
            );
}


// ctor
static void *hypotrochoid_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_hypotrochoid_tilde *x = (t_hypotrochoid_tilde *)pd_new(hypotrochoid_tilde_class);
   
    x->R_in  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->r_in  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->d_in  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
    t_float R = argc ? atom_getfloat(argv) : 2.f;
    t_float r = argc > 1 ? atom_getfloat(argv+1) : 1.f;
    t_float d = argc > 2 ? atom_getfloat(argv+2) : 0.f;

    pd_float((t_pd*)x->R_in, R);  // init the inlets with creation args
    pd_float((t_pd*)x->r_in, r);
    pd_float((t_pd*)x->d_in, d);
    
    outlet_new(&x->x_obj, &s_signal); // default provided outlet
    x->yChan_out = outlet_new(&x->x_obj, &s_signal); // outlet we made
    
    return (x);
}

// dtor / free
static void *hypotrochoid_tilde_free(t_hypotrochoid_tilde *x)
{
    inlet_free(x->R_in);
    inlet_free(x->r_in);
    inlet_free(x->d_in);
    
    outlet_free(x->yChan_out);
    
    return (void *)x;
}

// NEVER MAKE THIS STATIC!!!
void hypotrochoid_tilde_setup(void)
{
    hypotrochoid_tilde_class = class_new(gensym("hypotrochoid~"),
                            (t_newmethod)hypotrochoid_tilde_new, //ctor
                            (t_method)hypotrochoid_tilde_free, //dtor
                            sizeof(t_hypotrochoid_tilde), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_GIMME, // R, r, d
                            0); // no more args
    
    class_sethelpsymbol(hypotrochoid_tilde_class, gensym("hypotrochoid~")); // links to the help patch
    
    class_addmethod(hypotrochoid_tilde_class, (t_method)hypotrochoid_tilde_dsp, gensym("dsp"), A_CANT, 0); // add a dsp method to data space
    CLASS_MAINSIGNALIN(hypotrochoid_tilde_class, t_hypotrochoid_tilde, f); // signal inlet as first inlet
}
