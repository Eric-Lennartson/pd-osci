#include "m_pd.h"
#include "Audio_Functions.h"

t_vec3 v = NEW_VEC3;

static t_class *polygon_tilde_class;

typedef struct _polygon_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    //t_float xPos, yPos, size, sides;
    t_inlet *xPos_in, *yPos_in, *sides_in, *size_in; // driver default provided
    t_outlet *yChan_out; // *xChan_out default provided
} t_polygon_tilde;

static t_int *polygon_tilde_perform(t_int *w)
{
    t_sample *driver_in =      (t_sample *)(w[1]);
    t_sample *xPos_in   =      (t_sample *)(w[2]);
    t_sample *yPos_in   =      (t_sample *)(w[3]);
    t_sample *sides_in  =      (t_sample *)(w[4]);
    t_sample *size_in   =      (t_sample *)(w[5]);
    t_sample *xChan_out =      (t_sample *)(w[6]);
    t_sample *yChan_out =      (t_sample *)(w[7]);
    int     nblock      =             (int)(w[8]); // get blocksize
    
    
    while (nblock--) // dsp here
    {
        t_sample size = *size_in++; // gets it's own variable so that I don't have to remember when to increment
        
        v = polygon(mod1(*driver_in++), *sides_in++);
        
        *xChan_out++ = (v.x + *xPos_in++) * size;
        *yChan_out++ = (v.y + *yPos_in++) * size;
    }
    
    return (w + 9);
}

static void polygon_tilde_dsp(t_polygon_tilde *x, t_signal **sp)
{
    dsp_add(polygon_tilde_perform, 8,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // xPos
            sp[2]->s_vec, // yPos
            sp[3]->s_vec, // sides
            sp[4]->s_vec, // size
            sp[5]->s_vec, // xOut
            sp[6]->s_vec, // yOut
            sp[0]->s_n // block size
            );
}


// ctor
static void *polygon_tilde_new(t_floatarg xPos, t_floatarg yPos, t_floatarg sides, t_floatarg size)
{
    t_polygon_tilde *x = (t_polygon_tilde *)pd_new(polygon_tilde_class);
    
    //Init inlets and variables
//    x->xPos = xPos;
//    x->yPos = yPos;
//    x->sides = sides;
//    x->size = size;
   
    x->xPos_in  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->yPos_in  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->sides_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->size_in  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
    pd_float((t_pd*)x->xPos_in, xPos); // send creation args to inlets, allows for floats to inlets
    pd_float((t_pd*)x->yPos_in, yPos);
    pd_float((t_pd*)x->sides_in, sides);
    pd_float((t_pd*)x->size_in, size);
    
    
    
    outlet_new(&x->x_obj, &s_signal); // default provided outlet
    
    x->yChan_out = outlet_new(&x->x_obj, &s_signal); // outlet we made
    
    return (x);
}

// dtor / free
static void *polygon_tilde_free(t_polygon_tilde *x)
{
    inlet_free(x->xPos_in);
    inlet_free(x->yPos_in);
    inlet_free(x->size_in);
    inlet_free(x->sides_in);
    
    outlet_free(x->yChan_out);
    
    return (void *)x;
}

void polygon_tilde_setup(void)
{
    polygon_tilde_class = class_new(gensym("polygon~"),
                            (t_newmethod)polygon_tilde_new, //ctor
                            (t_method)polygon_tilde_free, //dtor
                            sizeof(t_polygon_tilde), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_DEFFLOAT, // xPos
                            A_DEFFLOAT, // yPos
                            A_DEFFLOAT, // sides
                            A_DEFFLOAT, // size
                            0); // no more args

    class_addcreator((t_newmethod)polygon_tilde_new,
                     gensym("gon~"),
                     A_DEFFLOAT, // xPos
                     A_DEFFLOAT, // yPos
                     A_DEFFLOAT, // sides
                     A_DEFFLOAT, // size
                     0); // no more args
    
    class_addcreator((t_newmethod)polygon_tilde_new,
                     gensym("osci/polygon~"),
                     A_DEFFLOAT, // xPos
                     A_DEFFLOAT, // yPos
                     A_DEFFLOAT, // sides
                     A_DEFFLOAT, // size
                     0); // no more args
    
    class_addcreator((t_newmethod)polygon_tilde_new,
                     gensym("osci/gon~"),
                     A_DEFFLOAT, // xPos
                     A_DEFFLOAT, // yPos
                     A_DEFFLOAT, // sides
                     A_DEFFLOAT, // size
                     0); // no more args
    
    class_sethelpsymbol(polygon_tilde_class, gensym("polygon~")); // links to the help patch
    
    class_addmethod(polygon_tilde_class, (t_method)polygon_tilde_dsp, gensym("dsp"), A_CANT, 0); // add a dsp method to data space
    CLASS_MAINSIGNALIN(polygon_tilde_class, t_polygon_tilde, f); // signal inlet as first inlet
}
