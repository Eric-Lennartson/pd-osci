#include "m_pd.h"
#include "vec3.h"

static t_class *shear_tilde_class;

typedef struct _shear_tilde
{
    t_object  x_obj;
    t_sample f; // dummy arg
    vec3 v;
    t_symbol axis;
    t_inlet  *y_in, *z_in, *angle_in;
    t_outlet *x_out, *y_out, *z_out;
} t_shear_tilde;

t_int *shear_tilde_perform(t_int *w)
{
    t_shear_tilde *x   =  (t_shear_tilde *)(w[1]); // access data space
    t_sample *x_in     =       (t_sample *)(w[2]);
    t_sample *y_in     =       (t_sample *)(w[3]);
    t_sample *z_in     =       (t_sample *)(w[4]);
    t_sample *angle_in =       (t_sample *)(w[5]);
    t_sample *x_out    =       (t_sample *)(w[6]);
    t_sample *y_out    =       (t_sample *)(w[7]);
    t_sample *z_out    =       (t_sample *)(w[8]);
    int       n        =              (int)(w[9]); // block size
    
    while (n--)
    {
        x->v = set(*x_in++, *y_in++, *z_in++);
        shear(&x->v, *angle_in++, x->axis.s_name);
        
        *x_out++ = x->v.x;
        *y_out++ = x->v.y;
        *z_out++ = x->v.z;
    }

    return (w + 10); // num ptrs + 1
}

void shear_tilde_dsp(t_shear_tilde *x, t_signal **sp)
{
    dsp_add(shear_tilde_perform, 9, x,
            sp[0]->s_vec, // x_in
            sp[1]->s_vec, // y_in
            sp[2]->s_vec, // z_in
            sp[3]->s_vec, // angle_in
            sp[4]->s_vec, // x_out
            sp[5]->s_vec, // y_out
            sp[6]->s_vec, // z_out
            sp[0]->s_n);  // block size
}

void set_axis(t_shear_tilde *x, t_symbol *s)
{
    if(*s->s_name != 'x' && *s->s_name != 'y' && *s->s_name != 'z')
        x->axis.s_name = "x"; // if no valid axis is provided we default to x
    else
        x->axis.s_name = s->s_name;
    
    //post(x->axis.s_name);
}

void shear_tilde_free(t_shear_tilde *x)
{
    inlet_free(x->y_in);
    inlet_free(x->z_in);
    inlet_free(x->angle_in);
    
    outlet_free(x->x_out);
    outlet_free(x->y_out);
    outlet_free(x->z_out);
}

void *shear_tilde_new(t_floatarg angle, t_symbol *axis)
{
    t_shear_tilde *x = (t_shear_tilde *)pd_new(shear_tilde_class);
    
    set_axis(x, axis);
    x->v = NEW_VEC3;
    
    x->y_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->z_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->angle_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
    pd_float((t_pd*)x->angle_in, angle);
    
    x->x_out = outlet_new(&x->x_obj, &s_signal);
    x->y_out = outlet_new(&x->x_obj, &s_signal);
    x->z_out = outlet_new(&x->x_obj, &s_signal);
    
    return (void *)x;
}

void shear_tilde_setup(void)
{
    shear_tilde_class = class_new(gensym("shear~"),
                                  (t_newmethod)shear_tilde_new,
                                  (t_method)shear_tilde_free,
                                  sizeof(t_shear_tilde),
                                  CLASS_DEFAULT,
                                  A_DEFFLOAT, //angle
                                  A_DEFSYM, // axis
                                  0);
    
    class_addcreator((t_newmethod)shear_tilde_new,
                     gensym("osci/shear~"),
                     A_DEFFLOAT, //angle
                     A_DEFSYM, // axis
                     0);
    
    class_addsymbol(shear_tilde_class, set_axis);
    
    class_sethelpsymbol(shear_tilde_class, gensym("shear~"));
    class_addmethod(shear_tilde_class, (t_method)shear_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(shear_tilde_class, t_shear_tilde, f); // dummy arg for singal into first inlet
}
