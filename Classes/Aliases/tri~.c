#include "m_pd.h"
#include "Audio_Math.h"

t_float points[3][2] = {{-0.5, -0.5},
                        { 0.0,  0.366},
                        { 0.5, -0.5}};

static t_class *tri_tilde_class;
typedef struct _tri_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    bool bypass;
    t_float xPos, yPos, size;
    t_inlet *xPos_in, *yPos_in, *size_in; // phase default provided
    t_outlet *x_out, *y_out;
} t_tri_tilde;

static t_int *tri_tilde_perform(t_int *w)
{
    t_tri_tilde *x = (t_tri_tilde *)(w[1]);
    t_sample *phase_in  = (t_sample *)(w[2]);
    t_sample *xPos_in   = (t_sample *)(w[3]);
    t_sample *yPos_in   = (t_sample *)(w[4]);
    t_sample *size_in   = (t_sample *)(w[5]);
    t_sample *x_out     = (t_sample *)(w[6]);
    t_sample *y_out     = (t_sample *)(w[7]);
    int      nblock     =        (int)(w[8]); // get blocksize

    while (nblock--) // dsp here
    {
        t_sample t    = phase_in[nblock];
        if(!x->bypass) {
            t_sample xPos = xPos_in[nblock];
            t_sample yPos = yPos_in[nblock];
            t_sample size = size_in[nblock];

            t_sample t2 = t * 3;
            int idx = t2;
            int idx_next = (idx + 1 < 3) ? idx + 1 : 0;

            // get each point and interpolate between them
            t_float x1 = points[idx][0];
            t_float y1 = points[idx][1];
            t_float x2 = points[idx_next][0];
            t_float y2 = points[idx_next][1];

            x_out[nblock] = lerp(mod1(t2), x1, x2) * size + xPos;
            y_out[nblock] = lerp(mod1(t2), y1, y2) * size + yPos;
        } else {
            x_out[nblock] = t;
            y_out[nblock] = 0;
        }
    }

    return (w + 9);
}

static void tri_tilde_dsp(t_tri_tilde *x, t_signal **sp)
{
    dsp_add(tri_tilde_perform, 8, x,
            sp[0]->s_vec, // phase
            sp[1]->s_vec, // xPos
            sp[2]->s_vec, // yPos
            sp[3]->s_vec, // size
            sp[4]->s_vec, // xOut
            sp[5]->s_vec, // yOut
            sp[0]->s_n // block size
            );
}

static void tri_tilde_bypass(t_tri_tilde *x, t_floatarg bypass) {
    x->bypass = (int)bypass;
}

static void *tri_tilde_new(t_symbol *s, int argc, t_atom* argv)
{
    t_tri_tilde *x = (t_tri_tilde *)pd_new(tri_tilde_class);

    x->bypass = 0;

    //Init inlets and variables
    t_float xpos = argc ? atom_getfloat(argv) : 0;
    t_float ypos = argc>1 ? atom_getfloat(argv+1) : 0;
    t_float size = argc>2 ? atom_getfloat(argv+2) : 1;

    x->xPos_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->yPos_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->size_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

    pd_float((t_pd*)x->xPos_in, xpos); // send creation args to inlets, allows for floats to inlets
    pd_float((t_pd*)x->yPos_in, ypos);
    pd_float((t_pd*)x->size_in, size);

    x->x_out = outlet_new(&x->x_obj, &s_signal); // outlet we made
    x->y_out = outlet_new(&x->x_obj, &s_signal); // outlet we made

    return (x);
}

// dtor / free
static void *tri_tilde_free(t_tri_tilde *x)
{
    inlet_free(x->size_in);
    inlet_free(x->xPos_in);
    inlet_free(x->yPos_in);

    outlet_free(x->y_out);

    return (void *)x;
}

void tri_tilde_setup(void)
{
    tri_tilde_class = class_new(gensym("tri~"),
                            (t_newmethod)tri_tilde_new, //ctor
                            (t_method)tri_tilde_free, //dtor
                            sizeof(t_tri_tilde), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_GIMME, // xpos, ypos, size
                            0); // no more args

    class_sethelpsymbol(tri_tilde_class, gensym("tri~")); // links to the help patch
    class_addmethod(tri_tilde_class, (t_method)tri_tilde_bypass, gensym("bypass"), A_DEFFLOAT, 0);

    class_addmethod(tri_tilde_class, (t_method)tri_tilde_dsp, gensym("dsp"), A_CANT, 0); // add a dsp method to data space
    CLASS_MAINSIGNALIN(tri_tilde_class, t_tri_tilde, f); // signal inlet as first inlet
}
