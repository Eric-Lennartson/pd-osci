//
//  Square~.c
//  Square~
//
//  Created by Eric Lennartson on 4/08/20.


#include "m_pd.h"
#include "Audio_Math.h"

t_float points[3][2] = {{-0.5, -0.5},
                        { 0.0,  0.366},
                        { 0.5, -0.5}};
// there's some problem with using the class dataspace in the perform and dsp_add functions

static t_class *triangle_tilde_class;
typedef struct _triangle_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    t_float xPos, yPos, size;
    //t_sample points[3][2]; // 3 xy (2) points
    t_inlet *xPos_in, *yPos_in, *size_in; // driver default provided
    t_outlet *x_out, *y_out;
} t_triangle_tilde;

// something weird happens when I try to add member variables to the data space
static t_int *triangle_tilde_perform(t_int *w)
{
//    t_triangle_tilde *x = (t_triangle_tilde*)(w[1]);
    t_sample *driver_in =        (t_sample *)(w[1]);
    t_sample *xPos_in   =        (t_sample *)(w[2]);
    t_sample *yPos_in   =        (t_sample *)(w[3]);
    t_sample *size_in   =        (t_sample *)(w[4]);
    t_sample *x_out     =        (t_sample *)(w[5]);
    t_sample *y_out     =        (t_sample *)(w[6]);
    int      nblock     =               (int)(w[7]); // get blocksize
    
    while (nblock--) // dsp here
    {
        t_sample t    = *driver_in++;
        t_sample xPos = *xPos_in++;
        t_sample yPos = *yPos_in++;
        t_sample size = *size_in++;
        
        t_sample t2 = t * 3;
        int  int_t2 = t2;
        
        // get each point and interpolate between them
//        t_float x1 = x->points[int_t2][0];
//        t_float x2 = x->points[(int_t2 + 1 < 3) ? int_t2 + 1 : 0][0];
//        t_float y1 = x->points[int_t2][1];
        t_float x1 = points[int_t2][0];
        t_float y1 = points[int_t2][1];
        t_float x2 = points[(int_t2 + 1 < 3) ? int_t2 + 1 : 0][0];
        t_float y2 = points[(int_t2 + 1 < 3) ? int_t2 + 1 : 0][1];
        
        *x_out++ = lerp( x1, x2, mod1(t2) ) * size + xPos;
        *y_out++ = lerp( y1, y2, mod1(t2) ) * size + yPos;
    }
    
    return (w + 8);
}

static void triangle_tilde_dsp(t_triangle_tilde *x, t_signal **sp)
{
    dsp_add(triangle_tilde_perform, 7,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // xPos
            sp[2]->s_vec, // yPos
            sp[3]->s_vec, // size
            sp[4]->s_vec, // xOut
            sp[5]->s_vec, // yOut
            sp[0]->s_n // block size
            );
}


// ctor
static void *triangle_tilde_new(t_floatarg xPos, t_floatarg yPos, t_floatarg size)
{
    t_triangle_tilde *x = (t_triangle_tilde *)pd_new(triangle_tilde_class);
    
    //Init inlets and variables
    x->xPos = xPos;
    x->yPos = yPos;
    x->size = size;
   
    x->xPos_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->yPos_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->size_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
    pd_float((t_pd*)x->xPos_in, xPos); // send creation args to inlets, allows for floats to inlets
    pd_float((t_pd*)x->yPos_in, yPos);
    pd_float((t_pd*)x->size_in, size);
    
//    x->points[0][0] = -0.5; // bottom left
//    x->points[0][1] = -0.5;
//    x->points[1][0] =  0.0; // top
//    x->points[1][1] =  0.366;
//    x->points[2][0] =  0.5; // bottom right
//    x->points[2][1] = -0.5;
    
    x->x_out = outlet_new(&x->x_obj, &s_signal); // outlet we made
    x->y_out = outlet_new(&x->x_obj, &s_signal); // outlet we made
    
    return (x);
}

// dtor / free
static void *triangle_tilde_free(t_triangle_tilde *x)
{
    inlet_free(x->size_in);
    inlet_free(x->xPos_in);
    inlet_free(x->yPos_in);
    
    outlet_free(x->y_out);
    
    return (void *)x;
}

void triangle_tilde_setup(void)
{
    triangle_tilde_class = class_new(gensym("triangle~"),
                            (t_newmethod)triangle_tilde_new, //ctor
                            (t_method)triangle_tilde_free, //dtor
                            sizeof(t_triangle_tilde), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_DEFFLOAT, // xPos
                            A_DEFFLOAT, // yPos
                            A_DEFFLOAT, // size
                            0); // no more args
    
    class_addcreator((t_newmethod)triangle_tilde_new,
                     gensym("osci/triangle~"),
                     A_DEFFLOAT, // xPos
                     A_DEFFLOAT, // yPos
                     A_DEFFLOAT, // size
                     0); // no more args
    
    class_addcreator((t_newmethod)triangle_tilde_new,
                     gensym("tri~"),
                     A_DEFFLOAT, // xPos
                     A_DEFFLOAT, // yPos
                     A_DEFFLOAT, // size
                     0); // no more args
    
    class_addcreator((t_newmethod)triangle_tilde_new,
                     gensym("osci/tri~"),
                     A_DEFFLOAT, // xPos
                     A_DEFFLOAT, // yPos
                     A_DEFFLOAT, // size
                     0); // no more args
    
    class_sethelpsymbol(triangle_tilde_class, gensym("triangle~")); // links to the help patch
    
    class_addmethod(triangle_tilde_class, (t_method)triangle_tilde_dsp, gensym("dsp"), A_CANT, 0); // add a dsp method to data space
    CLASS_MAINSIGNALIN(triangle_tilde_class, t_triangle_tilde, f); // signal inlet as first inlet
}
