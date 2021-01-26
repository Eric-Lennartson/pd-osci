
#include "m_pd.h"
#include "Audio_Math.h"

static t_class *dash_class;

typedef struct _dash
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    t_inlet *num_pnts, *dash_length;
} t_dash;

static t_int *dash_perform(t_int *w)
{
    // you can't have init arguments, and sig args be the same, b/c the sig args will overwrite them
    t_sample *driver         = (t_sample *)(w[1]);
    t_sample *num_pnts_in    = (t_sample *)(w[2]);
    t_sample *dash_length_in = (t_sample *)(w[3]);
    t_sample *out            = (t_sample *)(w[4]);
    int      nblock          =        (int)(w[5]); // get block size
    
    while (nblock--)
    {
        t_sample t = *driver++;
        int pnts = *num_pnts_in++;
        t_sample dash_length = mod1(*dash_length_in++); // bounds protection
        
        pnts = pnts < 1 ? 1 : pnts; // prevent negative pnts
        
        
        t_float tn = t * pnts;
        int idx = tn;
        t_float alpha = mod1(tn);
        t_float t2 = (idx + alpha * dash_length ) / pnts;
        
        *out++ = t2;
    }
    
    return (w + 6);
}

static void dash_dsp(t_dash *x, t_signal **sp)
{
    dsp_add(dash_perform, 5,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // num_pnts
            sp[2]->s_vec, // dash_length
            sp[3]->s_vec, // out
            sp[0]->s_n); // block size
}

// FREE
static void *dash_free(t_dash *x)
{
    inlet_free(x->num_pnts);
    inlet_free(x->dash_length);
    
    return (void *)x;
}

static void *dash_new(t_floatarg num_pnts, t_floatarg dash_length)
{
    t_dash *x = (t_dash *)pd_new(dash_class);
    
    x->num_pnts    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->dash_length = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
    pd_float((t_pd*)x->num_pnts, num_pnts);
    pd_float((t_pd*)x->dash_length, dash_length);

    outlet_new(&x->x_obj, &s_signal); // default provided outlet
    
    return (x);
}

void dash_tilde_setup(void)
{
    dash_class = class_new(gensym("dash~"),
                            (t_newmethod)dash_new, //ctor
                            (t_method)dash_free, //dtor
                            sizeof(t_dash), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_DEFFLOAT, //num_pnts
                            A_DEFFLOAT, //dash_length
                            0); // no more args
    
    class_sethelpsymbol(dash_class, gensym("dash~")); // links to the help patch
    
    class_addmethod(dash_class, (t_method)dash_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(dash_class, t_dash, f); // signal inlet as first inlet
}
