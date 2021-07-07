#include "m_pd.h"
#include "Audio_Math.h"
#include "vec3.h"

static t_class *rotate_tilde_class;

typedef struct _rotate_tilde
{
    t_object  x_obj;
    t_sample f; // dummy arg
    t_vec3 rot, v; // rotation vector, in vector
    t_inlet /* *x_in, */ *y_in, *z_in, *xRot_in, *yRot_in, *zRot_in;
    t_outlet *x_out, *y_out, *z_out;
} t_rotate_tilde;

t_int *rotate_tilde_perform(t_int *w)
{
    t_rotate_tilde *x   = (t_rotate_tilde *)(w[1]); // access data space
    t_sample *x_in      =       (t_sample *)(w[2]);
    t_sample *y_in      =       (t_sample *)(w[3]);
    t_sample *z_in      =       (t_sample *)(w[4]);
    t_sample *xRot_in   =       (t_sample *)(w[5]);
    t_sample *yRot_in   =       (t_sample *)(w[6]);
    t_sample *zRot_in   =       (t_sample *)(w[7]);
    t_sample *x_out     =       (t_sample *)(w[8]);
    t_sample *y_out     =       (t_sample *)(w[9]);
    t_sample *z_out     =       (t_sample *)(w[10]);
    int       n         =              (int)(w[11]); // block size
    
  while (n--)
  {
      x->v = vec3(*x_in++, *y_in++, *z_in++);
      x->rot = vec3(*xRot_in++, *yRot_in++, *zRot_in++);
      
      rotate(&x->v, &x->rot);
      
      *x_out++ = x->v.x;
      *y_out++ = x->v.y;
      *z_out++ = x->v.z;
  }

  return (w + 12); // num ptrs + 1
}

void rotate_tilde_dsp(t_rotate_tilde *x, t_signal **sp)
{
    dsp_add(rotate_tilde_perform, 11, x,
            sp[0]->s_vec, // x_in
            sp[1]->s_vec, // y_in
            sp[2]->s_vec, // z_in
            sp[3]->s_vec, // xRot_in
            sp[4]->s_vec, // yRot_in
            sp[5]->s_vec, // zRot_in
            sp[6]->s_vec, // x_out
            sp[7]->s_vec, // y_out
            sp[8]->s_vec, // z_out
            sp[0]->s_n); // block size
}

void rotate_tilde_free(t_rotate_tilde *x)
{
//    inlet_free(x->x_in);
    inlet_free(x->y_in);
    inlet_free(x->z_in);
    inlet_free(x->xRot_in);
    inlet_free(x->yRot_in);
    inlet_free(x->zRot_in);
    
    outlet_free(x->x_out);
    outlet_free(x->y_out);
    outlet_free(x->z_out);
}

void *rotate_tilde_new(t_floatarg ax, t_floatarg ay, t_floatarg az)
{
    t_rotate_tilde *x = (t_rotate_tilde *)pd_new(rotate_tilde_class);

    x->rot.x = ax;
    x->rot.y = ay;
    x->rot.z = az;
    x->v = NEW_VEC3;
    
//    x->x_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->y_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->z_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->xRot_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->yRot_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->zRot_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
    pd_float((t_pd*)x->xRot_in, ax);
    pd_float((t_pd*)x->yRot_in, ay);
    pd_float((t_pd*)x->zRot_in, az);
    
    x->x_out = outlet_new(&x->x_obj, &s_signal);
    x->y_out = outlet_new(&x->x_obj, &s_signal);
    x->z_out = outlet_new(&x->x_obj, &s_signal);
    
    return (void *)x;
}

void rotate_tilde_setup(void)
{
    rotate_tilde_class = class_new(gensym("rotate_axis~"),
                                  (t_newmethod)rotate_tilde_new,
                                  (t_method)rotate_tilde_free,
                                  sizeof(t_rotate_tilde),
                                  CLASS_DEFAULT,
                                  A_DEFFLOAT, //xRot
                                  A_DEFFLOAT, //yRot
                                  A_DEFFLOAT, //zRot
                                  0);

    class_addcreator((t_newmethod)rotate_tilde_new,
                      gensym("rotate2~"),
                      A_DEFFLOAT, //xRot
                      A_DEFFLOAT, //yRot
                      A_DEFFLOAT, //zRot
                      0); // no args


    class_addmethod(rotate_tilde_class, (t_method)rotate_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(rotate_tilde_class, t_rotate_tilde, f); // dummy arg for singal into first inlet
}
