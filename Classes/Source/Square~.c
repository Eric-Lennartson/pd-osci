
#include "m_pd.h"
#include "Audio_Math.h"

static t_class *square_tilde_class;

t_vec3 points[4] = { {-1, 1, 0}, {1,1,0},
                     {1,-1,0}, {-1,-1,0} };

typedef struct _square_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    t_float xPos, yPos, size;
    t_inlet *xPos_in, *yPos_in, *size_in; // driver default provided
    t_outlet *yChan_out; // *xChan_out default provided
} t_square_tilde;

static t_int *square_tilde_perform(t_int *w)
{
    t_sample *driver_in =      (t_sample *)(w[1]);
    t_sample *xPos_in   =      (t_sample *)(w[2]);
    t_sample *yPos_in   =      (t_sample *)(w[3]);
    t_sample *size_in   =      (t_sample *)(w[4]);
    t_sample *xChan_out =      (t_sample *)(w[5]);
    t_sample *yChan_out =      (t_sample *)(w[6]);
    int     nblock      =             (int)(w[7]); // get blocksize
    
    
    while (nblock--) // dsp here
    {
        t_sample t    = mod1(*driver_in++);
        t_sample xPos = *xPos_in++;
        t_sample yPos = *yPos_in++;
        t_sample size = *size_in++;
        
        t_sample t2 = t * 4;
        int  int_t2 = t2;
        
        t_float x1 = points[int_t2].x;
        t_float y1 = points[int_t2].y;
        t_float x2 = points[(int_t2 + 1 < 4) ? int_t2 + 1 : 0].x;
        t_float y2 = points[(int_t2 + 1 < 4) ? int_t2 + 1 : 0].y;
        
        *xChan_out++ = lerp( x1, x2, mod1(t2) ) * size + xPos;
        *yChan_out++ = lerp( y1, y2, mod1(t2) ) * size + yPos;
    }
    
    return (w + 8);
}

static void square_tilde_dsp(t_square_tilde *x, t_signal **sp)
{
    dsp_add(square_tilde_perform, 7,
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
static void *square_tilde_new(t_floatarg xPos, t_floatarg yPos, t_floatarg size)
{
    t_square_tilde *x = (t_square_tilde *)pd_new(square_tilde_class);
    
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
    
    
    outlet_new(&x->x_obj, &s_signal); // default provided outlet
    
    x->yChan_out = outlet_new(&x->x_obj, &s_signal); // outlet we made
    
    return (x);
}

// dtor / free
static void *square_tilde_free(t_square_tilde *x)
{
    inlet_free(x->size_in);
    inlet_free(x->xPos_in);
    inlet_free(x->yPos_in);
    
    outlet_free(x->yChan_out);
    
    return (void *)x;
}

void square_tilde_setup(void)
{
    square_tilde_class = class_new(gensym("square~"),
                            (t_newmethod)square_tilde_new, //ctor
                            (t_method)square_tilde_free, //dtor
                            sizeof(t_square_tilde), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_DEFFLOAT, // xPos
                            A_DEFFLOAT, // yPos
                            A_DEFFLOAT, // size
                            0); // no more args
    
    class_addcreator((t_newmethod)square_tilde_new,
                     gensym("osci/square~"),
                     A_DEFFLOAT, // xPos
                     A_DEFFLOAT, // yPos
                     A_DEFFLOAT, // size
                     0); // no more args
    
    class_sethelpsymbol(square_tilde_class, gensym("square~")); // links to the help patch
    
    class_addmethod(square_tilde_class, (t_method)square_tilde_dsp, gensym("dsp"), A_CANT, 0); // add a dsp method to data space
    CLASS_MAINSIGNALIN(square_tilde_class, t_square_tilde, f); // signal inlet as first inlet
}
