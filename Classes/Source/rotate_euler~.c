#include "m_pd.h"
#include "Audio_Math.h"
#include "vec3.h"

// should this header be somewhere else?
#include "string.h"

static t_class *rotate_euler_tilde_class;

typedef struct _rotate_euler_tilde
{
  t_object x_obj;
  t_sample f; // dummy arg
  int angle_mode;
  t_vec3 v;
  t_inlet /* *x_in, */ *y_in, *z_in, *xRot_in, *yRot_in, *zRot_in;
  t_outlet *x_out, *y_out, *z_out;
} t_rotate_euler_tilde;

t_int *rotate_euler_tilde_perform(t_int *w)
{
  t_rotate_euler_tilde *x = (t_rotate_euler_tilde *)(w[1]); // access data space
  t_sample *x_in = (t_sample *)(w[2]);
  t_sample *y_in = (t_sample *)(w[3]);
  t_sample *z_in = (t_sample *)(w[4]);
  t_sample *xRot_in = (t_sample *)(w[5]);
  t_sample *yRot_in = (t_sample *)(w[6]);
  t_sample *zRot_in = (t_sample *)(w[7]);
  t_sample *x_out = (t_sample *)(w[8]);
  t_sample *y_out = (t_sample *)(w[9]);
  t_sample *z_out = (t_sample *)(w[10]);
  int nblock = (int)(w[11]); // block size

  while (nblock--)
  {
    x->v = vec3(x_in[nblock], y_in[nblock], z_in[nblock]);
    t_sample ax = xRot_in[nblock];
    t_sample ay = yRot_in[nblock];
    t_sample az = zRot_in[nblock];

    if (x->angle_mode)
    {
      ax *= DEG_TO_RAD;
      ay *= DEG_TO_RAD;
      az *= DEG_TO_RAD;
    }

    v3_rotate(&x->v, ax, ay, az);

    x_out[nblock] = x->v.x;
    y_out[nblock] = x->v.y;
    z_out[nblock] = x->v.z;
  }

  return (w + 12); // num ptrs + 1
}

void rotate_euler_tilde_dsp(t_rotate_euler_tilde *x, t_signal **sp)
{
  dsp_add(rotate_euler_tilde_perform, 11, x,
          sp[0]->s_vec, // x_in
          sp[1]->s_vec, // y_in
          sp[2]->s_vec, // z_in
          sp[3]->s_vec, // xRot_in
          sp[4]->s_vec, // yRot_in
          sp[5]->s_vec, // zRot_in
          sp[6]->s_vec, // x_out
          sp[7]->s_vec, // y_out
          sp[8]->s_vec, // z_out
          sp[0]->s_n);  // block size
}

void angle_mode(t_rotate_euler_tilde *x, t_symbol* mode)
{
  if (strcmp(mode->s_name, "degrees") == 0)
  {
    x->angle_mode = 1;
  }
  else if (strcmp(mode->s_name, "radians") == 0)
  {
    x->angle_mode = 0;
  }
}

void rotate_euler_tilde_free(t_rotate_euler_tilde *x)
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

void *rotate_euler_tilde_new(t_floatarg ax, t_floatarg ay, t_floatarg az)
{
  t_rotate_euler_tilde *x = (t_rotate_euler_tilde *)pd_new(rotate_euler_tilde_class);

  x->v = vec3(0, 0, 0);
  x->angle_mode = 1;

  //    x->x_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->y_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->z_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->xRot_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->yRot_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->zRot_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

  pd_float((t_pd *)x->xRot_in, ax);
  pd_float((t_pd *)x->yRot_in, ay);
  pd_float((t_pd *)x->zRot_in, az);

  x->x_out = outlet_new(&x->x_obj, &s_signal);
  x->y_out = outlet_new(&x->x_obj, &s_signal);
  x->z_out = outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

void rotate_euler_tilde_setup(void)
{
  rotate_euler_tilde_class = class_new(gensym("rotate_euler~"),
                                       (t_newmethod)rotate_euler_tilde_new,
                                       (t_method)rotate_euler_tilde_free,
                                       sizeof(t_rotate_euler_tilde),
                                       CLASS_DEFAULT,
                                       A_DEFFLOAT, //xRot
                                       A_DEFFLOAT, //yRot
                                       A_DEFFLOAT, //zRot
                                       0);

  class_sethelpsymbol(rotate_euler_tilde_class, gensym("rotate~")); // links to the help patch

  class_addmethod(rotate_euler_tilde_class, (t_method)angle_mode, gensym("mode"), A_DEFSYMBOL, 0);
  class_addmethod(rotate_euler_tilde_class, (t_method)rotate_euler_tilde_dsp, gensym("dsp"), A_CANT, 0);
  CLASS_MAINSIGNALIN(rotate_euler_tilde_class, t_rotate_euler_tilde, f); // dummy arg for singal into first inlet
}
