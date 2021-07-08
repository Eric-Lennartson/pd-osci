#include "m_pd.h"
#include "Audio_Math.h"
#include "vec3.h"

// should this header be somewhere else?
#include "string.h"

// rotate1, rotate2, and rotate3 are aliases for euler, axis, and pivot

static t_class *rotate2_tilde_class;

typedef struct _rotate2_tilde
{
  t_object x_obj;
  t_sample f; // dummy arg
  int angle_mode;
  t_vec3 v, axis;
  t_inlet /* *x_in, */ *y_in, *z_in, *angle_in, *ax_in, *ay_in, *az_in;
  t_outlet *x_out, *y_out, *z_out;
} t_rotate2_tilde;

t_int *rotate2_tilde_perform(t_int *w)
{
  t_rotate2_tilde *x = (t_rotate2_tilde *)(w[1]); // access data space
  t_sample *x_in         = (t_sample *)(w[2]);
  t_sample *y_in         = (t_sample *)(w[3]);
  t_sample *z_in         = (t_sample *)(w[4]);
  t_sample *angle_in     = (t_sample *)(w[5]);
  t_sample *ax_in        = (t_sample *)(w[6]);
  t_sample *ay_in        = (t_sample *)(w[7]);
  t_sample *az_in        = (t_sample *)(w[8]);
  t_sample *x_out        = (t_sample *)(w[9]);
  t_sample *y_out        = (t_sample *)(w[10]);
  t_sample *z_out        = (t_sample *)(w[11]);
  int nblock             = (int)(w[12]); // block size

  while (nblock--)
  {
    x->v = vec3(x_in[nblock], y_in[nblock], z_in[nblock]);
    t_sample angle = angle_in[nblock];
    x->axis = vec3(ax_in[nblock], ay_in[nblock], az_in[nblock]);

    if (x->angle_mode)
      angle *= DEG_TO_RAD;

    v3_rotate_axis(&x->v, angle, &x->axis);

    x_out[nblock] = x->v.x;
    y_out[nblock] = x->v.y;
    z_out[nblock] = x->v.z;
  }

  return (w + 13); // num ptrs + 1
}

void rotate2_tilde_dsp(t_rotate2_tilde *x, t_signal **sp)
{
  dsp_add(rotate2_tilde_perform, 12, x,
          sp[0]->s_vec, // x_in
          sp[1]->s_vec, // y_in
          sp[2]->s_vec, // z_in
          sp[3]->s_vec, // angle_in
          sp[4]->s_vec, // ax_in
          sp[5]->s_vec, // ay_in
          sp[6]->s_vec, // az_in
          sp[7]->s_vec, // x_out
          sp[8]->s_vec, // y_out
          sp[9]->s_vec, // z_out
          sp[0]->s_n);  // block size
}

void angle_mode(t_rotate2_tilde *x, t_symbol *mode)
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

void rotate2_tilde_free(t_rotate2_tilde *x)
{
  //    inlet_free(x->x_in);
  inlet_free(x->y_in);
  inlet_free(x->z_in);
  inlet_free(x->angle_in);
  inlet_free(x->ax_in);
  inlet_free(x->ay_in);
  inlet_free(x->az_in);

  outlet_free(x->x_out);
  outlet_free(x->y_out);
  outlet_free(x->z_out);
}

void *rotate2_tilde_new(t_floatarg angle, t_floatarg ax, t_floatarg ay, t_floatarg az)
{
  t_rotate2_tilde *x = (t_rotate2_tilde *)pd_new(rotate2_tilde_class);

  x->v = vec3(0, 0, 0);
  x->axis = vec3(0,0,0);
  x->angle_mode = 1;

  //    x->x_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->y_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->z_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->angle_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->ax_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->ay_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->az_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

  pd_float((t_pd *)x->angle_in, angle);
  pd_float((t_pd *)x->ax_in, ax);
  pd_float((t_pd *)x->ay_in, ay);
  pd_float((t_pd *)x->az_in, az);

  x->x_out = outlet_new(&x->x_obj, &s_signal);
  x->y_out = outlet_new(&x->x_obj, &s_signal);
  x->z_out = outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

void rotate2_tilde_setup(void)
{
  rotate2_tilde_class = class_new(gensym("rotate2~"),
                                      (t_newmethod)rotate2_tilde_new,
                                      (t_method)rotate2_tilde_free,
                                      sizeof(t_rotate2_tilde),
                                      CLASS_DEFAULT,
                                      A_DEFFLOAT, //angle
                                      A_DEFFLOAT, //axis X
                                      A_DEFFLOAT, //axis Y
                                      A_DEFFLOAT, //axis Z
                                      0);

  class_addcreator((t_newmethod)rotate2_tilde_new,
                   gensym("rotate2~"),
                   A_DEFFLOAT, //angle
                   A_DEFFLOAT, //axis X
                   A_DEFFLOAT, //axis Y
                   A_DEFFLOAT, //axis Z
                  0);

  class_sethelpsymbol(rotate2_tilde_class, gensym("rotate~")); // links to the help patch

  class_addmethod(rotate2_tilde_class, (t_method)angle_mode, gensym("mode"), A_DEFSYMBOL, 0);
  class_addmethod(rotate2_tilde_class, (t_method)rotate2_tilde_dsp, gensym("dsp"), A_CANT, 0);
  CLASS_MAINSIGNALIN(rotate2_tilde_class, t_rotate2_tilde, f); // dummy arg for singal into first inlet
}
