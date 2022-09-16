#include "m_pd.h"
#include "Audio_Math.h"

static t_class *ball_tilde_class;

typedef struct _ball_tilde
{
    t_object  x_obj;
    t_sample f;
    t_vec3 a, b, out;
    bool bypass;
    t_inlet  *y_in, *z_in, *amt_in, *r_in;
    t_outlet *x_out, *y_out, *z_out;
} t_ball_tilde;

t_int *ball_tilde_perform(t_int *w)
{
    t_ball_tilde  *x = (t_ball_tilde *)(w[1]);
    t_sample *x_in   = (t_sample *)(w[2]);
    t_sample *y_in   = (t_sample *)(w[3]);
    t_sample *z_in   = (t_sample *)(w[4]);
    t_sample *amt_in = (t_sample *)(w[5]);
    t_sample *r_in   = (t_sample *)(w[6]);
    t_sample *x_out  = (t_sample *)(w[7]);
    t_sample *y_out  = (t_sample *)(w[8]);
    t_sample *z_out  = (t_sample *)(w[9]);
    int      nblock  = (int)(w[10]); // block size

    while (nblock--)
    {
        if(!x->bypass)
        {
            t_sample r = r_in[nblock];
            x->a = vec3(x_in[nblock], y_in[nblock], z_in[nblock]);

            t_float len = v3_len(x->a);
            x->b = (len > 0.f) ? v3_divf(x->a, len) : vec3(0,0,0);
            x->out = blend(amt_in[nblock], x->a, x->b);

            x_out[nblock] = x->out.x * r;
            y_out[nblock] = x->out.y * r;
            z_out[nblock] = x->out.z * r;
        }
        else {
            x_out[nblock] = x_in[nblock];
            y_out[nblock] = y_in[nblock];
            z_out[nblock] = z_in[nblock];
        }

    }

    return (w + 11); // end of the s_vec
}

void ball_tilde_dsp(t_ball_tilde *x, t_signal **sp)
{
    dsp_add(ball_tilde_perform, 10, x,
            sp[0]->s_vec, // x_in
            sp[1]->s_vec, // y_in
            sp[2]->s_vec, // z_in
            sp[3]->s_vec, // amt_in
            sp[4]->s_vec, // r_in
            sp[5]->s_vec, // x_out
            sp[6]->s_vec, // y_out
            sp[7]->s_vec, // z_out
            sp[0]->s_n); // block size
}

static void bypass(t_ball_tilde *x, t_floatarg f)
{
    f = floorf(f);
    x->bypass = f != 0;
}

void ball_tilde_free(t_ball_tilde *x)
{
    inlet_free(x->y_in);
    inlet_free(x->z_in);
    inlet_free(x->amt_in);
    inlet_free(x->r_in);

    outlet_free(x->x_out);
    outlet_free(x->y_out);
    outlet_free(x->z_out);
}

void *ball_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_ball_tilde *x = (t_ball_tilde *)pd_new(ball_tilde_class);

    t_float amt = argc ? atom_getfloat(argv) : 0.f;
    t_float radius = argc > 1 ? atom_getfloat(argv+1) : 1.f;
    x->bypass = argc > 2 ? atom_getfloat(argv+2) : false;

    x->y_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->z_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->amt_in  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->r_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

    pd_float((t_pd*)x->amt_in, amt); // send creation args to inlets, converts float to signal at the inlets
    pd_float((t_pd*)x->r_in, radius);

    x->x_out = outlet_new(&x->x_obj, &s_signal);
    x->y_out = outlet_new(&x->x_obj, &s_signal);
    x->z_out = outlet_new(&x->x_obj, &s_signal);

    return (void *)x;
}

void ball_tilde_setup(void)
{
    ball_tilde_class = class_new(gensym("ball~"),
                                  (t_newmethod)ball_tilde_new,
                                  (t_method)ball_tilde_free,
                                  sizeof(t_ball_tilde),
                                  CLASS_DEFAULT,
                                  A_GIMME, //amt and radius
                                  0);

    class_sethelpsymbol(ball_tilde_class, gensym("ball~"));

    class_addmethod(ball_tilde_class, (t_method)bypass, gensym("bypass"), A_DEFFLOAT, 0);

    class_addmethod(ball_tilde_class, (t_method)ball_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(ball_tilde_class, t_ball_tilde, f); // dummy arg for singal into first inlet
}
