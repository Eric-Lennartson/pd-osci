#include "m_pd.h"
#include "Audio_Math.h"

static t_class *ball_tilde_class;

typedef struct _ball_tilde
{
    t_object  x_obj;
    t_sample f;
    t_vec3 a, b, out;
    t_inlet  *y_in, *z_in, *amt_in, *r_in;
    t_outlet *x_out, *y_out, *z_out;
} t_ball_tilde;

t_int *ball_tilde_perform(t_int *w)
{
    t_ball_tilde  *x =    (t_ball_tilde *)(w[1]);
    t_sample *x_in   =        (t_sample *)(w[2]);
    t_sample *y_in   =        (t_sample *)(w[3]);
    t_sample *z_in   =        (t_sample *)(w[4]);
    t_sample *amt_in =        (t_sample *)(w[5]);
    t_sample *r_in   =        (t_sample *)(w[6]);
    t_sample *x_out  =        (t_sample *)(w[7]);
    t_sample *y_out  =        (t_sample *)(w[8]);
    t_sample *z_out  =        (t_sample *)(w[9]);
    int          n   =               (int)(w[10]); // block size
      
    while (n--)
    {
        t_sample r = *r_in++;
        x->a = vec3(*x_in++, *y_in++, *z_in++);

        t_float len = v3_len(x->a);
        x->b = (len > 0.f) ? v3_divf(x->a, len) : NEW_VEC3;
        x->out = blend(*amt_in++, x->a, x->b);

        *x_out++ = x->out.x * r;
        *y_out++ = x->out.y * r;
        *z_out++ = x->out.z * r;
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

void *ball_tilde_new(t_floatarg amt, t_floatarg r)
{
    t_ball_tilde *x = (t_ball_tilde *)pd_new(ball_tilde_class);

    x->a   = NEW_VEC3;
    x->b   = NEW_VEC3;
    x->out = NEW_VEC3;
    
    x->y_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->z_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->amt_in  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->r_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
    pd_float((t_pd*)x->amt_in, amt); // send creation args to inlets, converts float to signal at the inlets
    pd_float((t_pd*)x->r_in, r);
    
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
                                  A_DEFFLOAT, //amt
                                  A_DEFFLOAT, // radius
                                  0);

    class_sethelpsymbol(ball_tilde_class, gensym("ball~"));
    
    class_addmethod(ball_tilde_class, (t_method)ball_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(ball_tilde_class, t_ball_tilde, f); // dummy arg for singal into first inlet
}
