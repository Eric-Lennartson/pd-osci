//
//  Circle~.c
//  Circle~
//
//  Created by Eric Lennartson on 3/26/20.

#include "m_pd.h"

static t_sample mod1(t_sample val) {return val - (int)val; }

static t_class *trace_class;

typedef struct _trace
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    t_inlet *offset, *interp_amt;
} t_trace;

static t_int *trace_perform(t_int *w)
{
    // you can't have init arguments, and sig args be the same, b/c the sig args will overwrite them
    t_sample *driver        = (t_sample *)(w[1]);
    t_sample *offset_in     = (t_sample *)(w[2]);
    t_sample *interp_amt_in = (t_sample *)(w[3]);
    t_sample *out           = (t_sample *)(w[4]);
    int      nblock         =        (int)(w[5]); // get block size
    
    while (nblock--)
    {
        t_sample t          = *driver++;
        t_sample offset     = *offset_in++;
        t_sample interp_amt = *interp_amt_in++;
        
        offset = offset < 0 ? 0 : offset; // prevent strange images from negative offset
        
        *out++ = mod1(t * interp_amt + offset);
    }
    
    return (w + 6);
}

static void trace_dsp(t_trace *x, t_signal **sp)
{
    dsp_add(trace_perform, 5,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // offset
            sp[2]->s_vec, // interp_amt
            sp[3]->s_vec, // out
            sp[0]->s_n); // block size
}

// FREE
static void *trace_free(t_trace *x)
{
    inlet_free(x->offset);
    inlet_free(x->interp_amt);
    
    return (void *)x;
}

static void *trace_new( void )
{
    t_trace *x = (t_trace *)pd_new(trace_class);
    
    x->offset     = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->interp_amt = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

    outlet_new(&x->x_obj, &s_signal); // default provided outlet
    
    return (x);
}

void trace_tilde_setup(void)
{
    trace_class = class_new(gensym("trace~"),
                            (t_newmethod)trace_new, //ctor
                            (t_method)trace_free, //dtor
                            sizeof(t_trace), // data space
                            CLASS_DEFAULT, // gui apperance
                            0); // no more args
    
    class_addcreator((t_newmethod)trace_new,
                     gensym("osci/trace~"),
                     0); // no more args
    
    class_sethelpsymbol(trace_class, gensym("trace~")); // links to the help patch
    
    class_addmethod(trace_class, (t_method)trace_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(trace_class, t_trace, f); // signal inlet as first inlet
}
