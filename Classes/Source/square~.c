
#include "m_pd.h"
#include "Audio_Math.h"

static t_class *square_tilde_class;

int x_pts[4] = {-1, -1, 1, 1};
int y_pts[4] = {-1, 1, 1, -1};

typedef struct _square_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    int bypass;
    t_float xPos, yPos, size;
    t_inlet *xPos_in, *yPos_in, *size_in; // driver default provided
    t_outlet *y_out;                  // *x_out default provided
} t_square_tilde;

static t_int *square_tilde_perform(t_int *w)
{
    t_square_tilde *x   = (t_square_tilde *)(w[1]);
    t_sample *driver_in = (t_sample *)(w[2]);
    t_sample *xPos_in   = (t_sample *)(w[3]);
    t_sample *yPos_in   = (t_sample *)(w[4]);
    t_sample *size_in   = (t_sample *)(w[5]);
    t_sample *x_out     = (t_sample *)(w[6]);
    t_sample *y_out     = (t_sample *)(w[7]);
    int nblock          = (int)(w[8]); // get blocksize

    while (nblock--) // dsp here
    {
        t_sample t = mod1(driver_in[nblock]);

        if(!x->bypass) {
            t_sample xPos = xPos_in[nblock];
            t_sample yPos = yPos_in[nblock];
            t_sample size = size_in[nblock];

            t_sample t2 = t * 4;
            int idx = t2;
            int idx_next = (idx + 1) % 4;

            int x1 = x_pts[idx];
            int y1 = y_pts[idx];
            int x2 = x_pts[idx_next];
            int y2 = y_pts[idx_next];

            x_out[nblock] = lerp(mod1(t2), x1, x2) * size + xPos;
            y_out[nblock] = lerp(mod1(t2), y1, y2) * size + yPos;
        } else {
            x_out[nblock] = t;
            y_out[nblock] = 0;
        }
    }

    return (w + 9);
}

static void square_tilde_dsp(t_square_tilde *x, t_signal **sp)
{
    dsp_add(square_tilde_perform, 8, x,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // xPos
            sp[2]->s_vec, // yPos
            sp[3]->s_vec, // size
            sp[4]->s_vec, // xOut
            sp[5]->s_vec, // yOut
            sp[0]->s_n    // block size
    );
}

static void square_tilde_bypass(t_square_tilde *x, t_floatarg bypass) {
    x->bypass = bypass;
}

// ctor
static void *square_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_square_tilde *x = (t_square_tilde *)pd_new(square_tilde_class);

    x->bypass = 0;

    //Init inlets and variables
    t_float xpos = argc ? atom_getfloat(argv) : 0;
    t_float ypos = argc>1 ? atom_getfloat(argv+1) : 0;
    t_float size = argc>2 ? atom_getfloat(argv+2) : 1;

    x->xPos_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->yPos_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->size_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

    pd_float((t_pd *)x->xPos_in, xpos); // send creation args to inlets, allows for floats to inlets
    pd_float((t_pd *)x->yPos_in, ypos);
    pd_float((t_pd *)x->size_in, size);

    outlet_new(&x->x_obj, &s_signal); // default provided outlet

    x->y_out = outlet_new(&x->x_obj, &s_signal); // outlet we made

    return (x);
}

// dtor / free
static void *square_tilde_free(t_square_tilde *x)
{
    inlet_free(x->size_in);
    inlet_free(x->xPos_in);
    inlet_free(x->yPos_in);

    outlet_free(x->y_out);

    return (void *)x;
}

void square_tilde_setup(void)
{
    square_tilde_class = class_new(gensym("square~"),
                                   (t_newmethod)square_tilde_new, //ctor
                                   (t_method)square_tilde_free,   //dtor
                                   sizeof(t_square_tilde),        // data space
                                   CLASS_DEFAULT,                 // gui apperance
                                   A_GIMME, // xpos, ypos, size
                                   0);                            // no more args

    class_sethelpsymbol(square_tilde_class, gensym("square~")); // links to the help patch
    class_addmethod(square_tilde_class, (t_method)square_tilde_bypass, gensym("bypass"), A_DEFFLOAT, 0);

    class_addmethod(square_tilde_class, (t_method)square_tilde_dsp, gensym("dsp"), A_CANT, 0); // add a dsp method to data space
    CLASS_MAINSIGNALIN(square_tilde_class, t_square_tilde, f);                                 // signal inlet as first inlet
}
