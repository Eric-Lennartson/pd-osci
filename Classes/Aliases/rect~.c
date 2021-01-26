#include "m_pd.h"
#include "Audio_Math.h"

static t_class *rect_tilde_class;

t_float points[4][2] = {{-1, -1},
                        {-1,  1},
                        { 1,  1},
                        { 1, -1}
                       };

typedef struct _rect_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    t_inlet *xPos_in, *yPos_in, *height_in, *width_in; // driver default provided
    t_outlet *yChan_out; // *xChan_out default provided
} t_rect_tilde;

static t_int *rect_tilde_perform(t_int *w)
{
    t_sample *driver_in = (t_sample *)(w[1]);
    t_sample *xPos_in   = (t_sample *)(w[2]);
    t_sample *yPos_in   = (t_sample *)(w[3]);
    t_sample *height_in = (t_sample *)(w[4]);
    t_sample *width_in  = (t_sample *)(w[5]);
    t_sample *xChan_out = (t_sample *)(w[6]);
    t_sample *yChan_out = (t_sample *)(w[7]);
    int      nblock     =        (int)(w[8]); // get blocksize
    
    
    while (nblock--) // dsp here
    {
        t_sample t      = *driver_in++;
        t_sample xPos   = *xPos_in++;
        t_sample yPos   = *yPos_in++;
        t_sample height = *height_in++;
        t_sample width  = *width_in++;
        
        t_sample tn = t * 4;
        int idx = tn;
        int idx_next = (idx + 1) % 4;
        
        t_float x1 = points[idx][0];
        t_float x2 = points[idx_next][0];
        t_float y1 = points[idx][1];
        t_float y2 = points[idx_next][1];
        
        *xChan_out++ = lerp(mod1(tn), x1, x2) * height + xPos;
        *yChan_out++ = lerp(mod1(tn), y1, y2) * width + yPos;
    }
    
    return (w + 9);
}

static void rect_tilde_dsp(t_rect_tilde *x, t_signal **sp)
{
    dsp_add(rect_tilde_perform, 8,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // xPos
            sp[2]->s_vec, // yPos
            sp[3]->s_vec, // height
            sp[4]->s_vec, // width
            sp[5]->s_vec, // xOut
            sp[6]->s_vec, // yOut
            sp[0]->s_n // block size
            );
}


// ctor
static void *rect_tilde_new(t_floatarg xPos, t_floatarg yPos, t_floatarg height, t_floatarg width)
{
    t_rect_tilde *x = (t_rect_tilde *)pd_new(rect_tilde_class);
   
    x->xPos_in   = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->yPos_in   = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->height_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->width_in  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
    pd_float((t_pd*)x->xPos_in, xPos); // send creation args to inlets, allows for floats to inlets
    pd_float((t_pd*)x->yPos_in, yPos);
    pd_float((t_pd*)x->height_in, height);
    pd_float((t_pd*)x->width_in, width);
    
    outlet_new(&x->x_obj, &s_signal); // default provided outlet
    x->yChan_out = outlet_new(&x->x_obj, &s_signal); // outlet we made
    
    return (x);
}

// dtor / free
static void *rect_tilde_free(t_rect_tilde *x)
{
    inlet_free(x->xPos_in);
    inlet_free(x->yPos_in);
    inlet_free(x->height_in);
    inlet_free(x->width_in);
    
    outlet_free(x->yChan_out);
    
    return (void *)x;
}

void rect_tilde_setup(void)
{
    rect_tilde_class = class_new(gensym("rect~"),
                            (t_newmethod)rect_tilde_new, //ctor
                            (t_method)rect_tilde_free, //dtor
                            sizeof(t_rect_tilde), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_DEFFLOAT, // xPos
                            A_DEFFLOAT, // yPos
                            A_DEFFLOAT, // height
                            A_DEFFLOAT, // width
                            0); // no more args
    
    class_sethelpsymbol(rect_tilde_class, gensym("rectangle~")); // links to the help patch
    
    class_addmethod(rect_tilde_class, (t_method)rect_tilde_dsp, gensym("dsp"), A_CANT, 0); // add a dsp method to data space
    CLASS_MAINSIGNALIN(rect_tilde_class, t_rect_tilde, f); // signal inlet as first inlet
}
