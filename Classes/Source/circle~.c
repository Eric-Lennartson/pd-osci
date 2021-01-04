//
//  Circle~.c
//  Circle~
//
//  Created by Eric Lennartson on 3/26/20.

/*
 Maybe add?
 - list msgs to change creation args?
 */

#include "m_pd.h"
#include <math.h>

static t_class *circle_class;

typedef struct _circle
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    t_float xPos, yPos, radius;
    t_inlet *xPos_in, *yPos_in, *radius_in;
    t_outlet *yChan_out;
    // in1 and out1 are automatically provided
} t_circle;

static t_int *circle_perform(t_int *w)
{
    // you can't have init arguments, and sig args be the same, b/c the sig args will overwrite them
    t_sample *driver     = (t_sample *)(w[1]);
    t_sample *xPos_in    = (t_sample *)(w[2]);
    t_sample *yPos_in    = (t_sample *)(w[3]);
    t_sample *radius_in  = (t_sample *)(w[4]);
    t_sample *xChan_out  = (t_sample *)(w[5]);
    t_sample *yChan_out  = (t_sample *)(w[6]);
    int      nblock      =        (int)(w[7]); // get block size
    
    while (nblock--)
    {
        t_sample t       = *driver++;
        t_sample xPos    = *xPos_in++;
        t_sample yPos    = *yPos_in++;
        t_sample radius  = *radius_in++;
        
        t_sample angle = 2 * 3.14159 * t;
        
        *xChan_out++ = (radius * cosf(angle)) + xPos;
        *yChan_out++ = (radius * sinf(angle)) + yPos;
    }
    
    return (w + 8);
}

static void circle_dsp(t_circle *x, t_signal **sp)
{
    dsp_add(circle_perform, 7,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // xPos
            sp[2]->s_vec, // yPos
            sp[3]->s_vec, // radius
            sp[4]->s_vec, // xOut
            sp[5]->s_vec, // yOut
            sp[0]->s_n); // block size
}

// FREE
static void *circle_free(t_circle *x)
{
    inlet_free(x->xPos_in);
    inlet_free(x->yPos_in);
    inlet_free(x->radius_in);
    
    outlet_free(x->yChan_out);
    
    return (void *)x;
}

static void *circle_new(t_floatarg xPos, t_floatarg yPos, t_floatarg radius)
{
    t_circle *x = (t_circle *)pd_new(circle_class);
    
    //Init inlets and variables
    x->xPos     = xPos;
    x->yPos     = yPos;
    x->radius   = radius;
    
    x->xPos_in      = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->yPos_in      = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->radius_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
    pd_float((t_pd*)x->xPos_in, xPos); // send creation args to inlets
    pd_float((t_pd*)x->yPos_in, yPos);
    pd_float((t_pd*)x->radius_in, radius);

    
    outlet_new(&x->x_obj, &s_signal); // default provided outlet
    
    x->yChan_out = outlet_new(&x->x_obj, &s_signal); // outlet we made
    
    return (x);
}

void circle_tilde_setup(void)
{
    circle_class = class_new(gensym("circle~"),
                            (t_newmethod)circle_new, //ctor
                            (t_method)circle_free, //dtor
                            sizeof(t_circle), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_DEFFLOAT, // xPos
                            A_DEFFLOAT, // yPos
                            A_DEFFLOAT, // radius
                            0); // no more args
    
    class_addcreator((t_newmethod)circle_new,
                     gensym("osci/circle~"),
                     A_DEFFLOAT, // xPos
                     A_DEFFLOAT, // yPos
                     A_DEFFLOAT, // radius
                     0); // no more args
    
    class_sethelpsymbol(circle_class, gensym("circle~")); // links to the help patch
    
    class_addmethod(circle_class, (t_method)circle_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(circle_class, t_circle, f); // signal inlet as first inlet
}
