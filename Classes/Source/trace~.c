#include "m_pd.h"
#include "Audio_Math.h"

static t_class *trace_class;

typedef struct _trace
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    t_inlet *offset_in, *length_in;
} t_trace;

static t_int *trace_perform(t_int *w)
{
    // you can't have init arguments, and sig args be the same, b/c the sig args will overwrite them
    t_sample *phase = (t_sample *)(w[1]);
    t_sample *offset_in = (t_sample *)(w[2]);
    t_sample *length_in = (t_sample *)(w[3]);
    t_sample *out = (t_sample *)(w[4]);
    int nblock = (int)(w[5]); // get block size

    while (nblock--)
    {
        t_sample t = phase[nblock];
        t_sample offset = offset_in[nblock];
        t_sample length = length_in[nblock];

        offset = offset < 0 ? 0 : offset; // prevent strange images from negative offset_in

        out[nblock] = (length <= 1) ? t * length + offset : mod1(t * length + offset);
    }

    return (w + 6);
}

static void trace_dsp(t_trace *x, t_signal **sp)
{
    dsp_add(trace_perform, 5,
            sp[0]->s_vec, // phase
            sp[1]->s_vec, // offset_in
            sp[2]->s_vec, // length
            sp[3]->s_vec, // out
            sp[0]->s_n);  // block size
}

// FREE
static void *trace_free(t_trace *x)
{
    inlet_free(x->offset_in);
    inlet_free(x->length_in);

    return (void *)x;
}

static void *trace_new(t_symbol *s, int argc, t_atom *argv)
{
    t_trace *x = (t_trace *)pd_new(trace_class);

    // set offset and length based on args
    t_float offset = argc ? mod1( atom_getfloat(argv) ) : 0;
    t_float length = argc > 1 ? atom_getfloat(argv+1) : 1;
    length =  length <= 1 ? length : mod1(length);

    x->offset_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->length_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

    pd_float((t_pd *)x->offset_in, offset);
    pd_float((t_pd *)x->length_in, length);

    outlet_new(&x->x_obj, &s_signal); // default provided outlet

    return (x);
}

void trace_tilde_setup(void)
{
    trace_class = class_new(gensym("trace~"),
                            (t_newmethod)trace_new, //ctor
                            (t_method)trace_free,   //dtor
                            sizeof(t_trace),        // data space
                            CLASS_DEFAULT,          // gui apperance
                            A_GIMME, //offset, length
                            0);                     // no more args

    class_sethelpsymbol(trace_class, gensym("trace~")); // links to the help patch

    class_addmethod(trace_class, (t_method)trace_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(trace_class, t_trace, f); // signal inlet as first inlet
}
