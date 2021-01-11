#include "m_pd.h"

static t_class *translate_tilde_class;

typedef struct _translate_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    t_float xOffset, yOffset, zOffset;
    t_inlet *yChan_in, *zChan_in, *xOffset_in, *yOffset_in, *zOffset_in; // xChan_in default provided
    t_outlet *yChan_out, *zChan_out; // xChan_out default provided
} t_translate_tilde;

static t_int *translate_tilde_perform(t_int *w)
{
    t_sample *xChan_in     = (t_sample *)(w[1]);
    t_sample *yChan_in     = (t_sample *)(w[2]);
    t_sample *zChan_in     = (t_sample *)(w[3]);
    t_sample *xOffset_in   = (t_sample *)(w[4]);
    t_sample *yOffset_in   = (t_sample *)(w[5]);
    t_sample *zOffset_in   = (t_sample *)(w[6]);
    t_sample *xChan_out    = (t_sample *)(w[7]);
    t_sample *yChan_out    = (t_sample *)(w[8]);
    t_sample *zChan_out    = (t_sample *)(w[9]);
    int      nblock        =        (int)(w[10]); // get block size
    
    while (nblock--) // dsp here
    {
        t_sample xChan      = *xChan_in++;
        t_sample yChan      = *yChan_in++;
        t_sample zChan      = *zChan_in++;
        t_sample xOffset    = *xOffset_in++;
        t_sample yOffset    = *yOffset_in++;
        t_sample zOffset    = *zOffset_in++;
        
        *xChan_out++ = xChan + xOffset;
        *yChan_out++ = yChan + yOffset;
        *zChan_out++ = zChan + zOffset;
    }
    
    return (w + 11);
}

static void translate_tilde_dsp(t_translate_tilde *x, t_signal **sp)
{
    dsp_add(translate_tilde_perform, 10,
            sp[0]->s_vec, // xChan_in
            sp[1]->s_vec, // yChan_in
            sp[2]->s_vec, // zChan_in
            sp[3]->s_vec, // xOffset
            sp[4]->s_vec, // yOffset
            sp[5]->s_vec, // zOffset
            sp[6]->s_vec, // xOut
            sp[7]->s_vec, // yOut
            sp[8]->s_vec, // zOut
            sp[0]->s_n); // block size
}


// ctor
static void *translate_tilde_new(t_floatarg xOffset, t_floatarg yOffset, t_floatarg zOffset)
{
    t_translate_tilde *x = (t_translate_tilde *)pd_new(translate_tilde_class);
    
    //Init inlets and variables
    x->xOffset      = xOffset;
    x->yOffset      = yOffset;
    x->zOffset      = zOffset;
    
    x->yChan_in     = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->zChan_in     = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->xOffset_in   = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->yOffset_in   = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->zOffset_in   = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
    pd_float((t_pd*)x->xOffset_in, xOffset); // send creation args to inlets
    pd_float((t_pd*)x->yOffset_in, yOffset);
    pd_float((t_pd*)x->zOffset_in, zOffset);
    
    outlet_new(&x->x_obj, &s_signal); // default provided outlet
    
    x->yChan_out = outlet_new(&x->x_obj, &s_signal);
    x->zChan_out = outlet_new(&x->x_obj, &s_signal);
    
    return (x);
}

// dtor / free
static void *translate_tilde_free(t_translate_tilde *x)
{
    inlet_free(x->yChan_in);
    inlet_free(x->zChan_in);
    inlet_free(x->xOffset_in);
    inlet_free(x->yOffset_in);
    inlet_free(x->zOffset_in);
    
    outlet_free(x->yChan_out);
    outlet_free(x->zChan_out);
    
    return (void *)x;
}

void translate_tilde_setup(void)
{
    translate_tilde_class = class_new(gensym("trans~"),
                            (t_newmethod)translate_tilde_new, //ctor
                            (t_method)translate_tilde_free, //dtor
                            sizeof(t_translate_tilde), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_DEFFLOAT, // xOffset
                            A_DEFFLOAT, // yOffset
                            A_DEFFLOAT, // zOffset
                            0); // no more args
    
    class_sethelpsymbol(translate_tilde_class, gensym("translate~")); // links to the help patch
    
    class_addmethod(translate_tilde_class, (t_method)translate_tilde_dsp, gensym("dsp"), A_CANT, 0); // add a dsp method to data space
    CLASS_MAINSIGNALIN(translate_tilde_class, t_translate_tilde, f); // signal inlet as first inlet
}
