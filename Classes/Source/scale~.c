#include "m_pd.h"
#include "vec3.h"

static t_class *scale_tilde_class;

typedef struct _scale_tilde
{
    t_object  x_obj;
    t_sample f; // dummy arg
    int bypass;
    t_vec3 v, v2; // in vector, mult vector
    t_inlet /* *x_in, */ *y_in, *z_in, *x_scalar_in, *y_scalar_in, *z_scalar_in;
    t_outlet *x_out, *y_out, *z_out;
} t_scale_tilde;

t_int *scale_tilde_perform(t_int *w)
{
    t_scale_tilde *x      = (t_scale_tilde *)(w[1]); // access data space
    t_sample *x_in        = (t_sample *)(w[2]);
    t_sample *y_in        = (t_sample *)(w[3]);
    t_sample *z_in        = (t_sample *)(w[4]);
    t_sample *x_scalar_in = (t_sample *)(w[5]);
    t_sample *y_scalar_in = (t_sample *)(w[6]);
    t_sample *z_scalar_in = (t_sample *)(w[7]);
    t_sample *x_out       = (t_sample *)(w[8]);
    t_sample *y_out       = (t_sample *)(w[9]);
    t_sample *z_out       = (t_sample *)(w[10]);
    int nblock            = (int)(w[11]); // block size

    while (nblock--)
    {
        if(!x->bypass)
        {
            x->v = vec3(x_in[nblock], y_in[nblock], z_in[nblock]);
            x->v2 = vec3(x_scalar_in[nblock], y_scalar_in[nblock], z_scalar_in[nblock]);

            x->v = v3_mult(x->v, x->v2);

            x_out[nblock] = x->v.x;
            y_out[nblock] = x->v.y;
            z_out[nblock] = x->v.z;
        } else {
            x_out[nblock] = x_in[nblock];
            y_out[nblock] = y_in[nblock];
            z_out[nblock] = z_in[nblock];
        }

    }

  return (w + 12); // num ptrs + 1
}

void scale_tilde_dsp(t_scale_tilde *x, t_signal **sp)
{
    dsp_add(scale_tilde_perform, 11, x,
            sp[0]->s_vec, // x_in
            sp[1]->s_vec, // y_in
            sp[2]->s_vec, // z_in
            sp[3]->s_vec, // x_scalar_in
            sp[4]->s_vec, // y_scalar_in
            sp[5]->s_vec, // z_scalar_in
            sp[6]->s_vec, // x_out
            sp[7]->s_vec, // y_out
            sp[8]->s_vec, // z_out
            sp[0]->s_n); // block size
}

static void scale_bypass(t_scale_tilde *x, t_floatarg bypass) {
  x->bypass = (int)bypass;
}

void scale_tilde_free(t_scale_tilde *x)
{
//    inlet_free(x->x_in);
    inlet_free(x->y_in);
    inlet_free(x->z_in);
    inlet_free(x->x_scalar_in);
    inlet_free(x->y_scalar_in);
    inlet_free(x->z_scalar_in);

    outlet_free(x->x_out);
    outlet_free(x->y_out);
    outlet_free(x->z_out);
}

void *scale_tilde_new(t_floatarg x_mult, t_floatarg y_mult, t_floatarg z_mult)
{
    t_scale_tilde *x = (t_scale_tilde *)pd_new(scale_tilde_class);

    x->bypass = 0;

    if(x_mult == 0 && y_mult == 0 && z_mult == 0) {
        x_mult = y_mult = z_mult = 1; // default to 1 if none are set
    }

    x->y_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->z_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->x_scalar_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->y_scalar_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->z_scalar_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

    pd_float((t_pd*)x->x_scalar_in, x_mult);
    pd_float((t_pd*)x->y_scalar_in, y_mult);
    pd_float((t_pd*)x->z_scalar_in, z_mult);

    x->x_out = outlet_new(&x->x_obj, &s_signal);
    x->y_out = outlet_new(&x->x_obj, &s_signal);
    x->z_out = outlet_new(&x->x_obj, &s_signal);

    return (void *)x;
}

void scale_tilde_setup(void)
{
    scale_tilde_class = class_new(gensym("scale~"),
                                  (t_newmethod)scale_tilde_new,
                                  (t_method)scale_tilde_free,
                                  sizeof(t_scale_tilde),
                                  CLASS_DEFAULT,
                                  A_DEFFLOAT, //x_mult
                                  A_DEFFLOAT, //y_mult
                                  A_DEFFLOAT, //z_mult
                                  0);

    class_addmethod(scale_tilde_class, (t_method)scale_bypass, gensym("bypass"), A_DEFFLOAT, 0);

    class_addmethod(scale_tilde_class, (t_method)scale_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(scale_tilde_class, t_scale_tilde, f); // dummy arg for singal into first inlet
}
