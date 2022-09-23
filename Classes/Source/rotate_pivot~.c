#include "m_pd.h"
#include "Audio_Math.h"
#include "vec3.h"

// should this header be somewhere else?
#include "string.h"

static t_class *rotate_pivot_tilde_class;

typedef struct _rotate_pivot_tilde
{
  t_object x_obj;
  t_sample f; // dummy arg
  int angle_mode, bypass;
  t_vec3 v, pivot, axis;
  t_inlet /* *x_in, */ *y_in, *z_in, *angle_in, *px_in, *py_in, *pz_in, *ax_in, *ay_in, *az_in;
  t_outlet *x_out, *y_out, *z_out;
} t_rotate_pivot_tilde;

t_int *rotate_pivot_tilde_perform(t_int *w)
{
  t_rotate_pivot_tilde *x = (t_rotate_pivot_tilde *)(w[1]); // access data space
  t_sample *x_in          = (t_sample *)(w[2]);
  t_sample *y_in          = (t_sample *)(w[3]);
  t_sample *z_in          = (t_sample *)(w[4]);
  t_sample *angle_in      = (t_sample *)(w[5]);
  t_sample *px_in         = (t_sample *)(w[6]);
  t_sample *py_in         = (t_sample *)(w[7]);
  t_sample *pz_in         = (t_sample *)(w[8]);
  t_sample *ax_in         = (t_sample *)(w[9]);
  t_sample *ay_in         = (t_sample *)(w[10]);
  t_sample *az_in         = (t_sample *)(w[11]);
  t_sample *x_out         = (t_sample *)(w[12]);
  t_sample *y_out         = (t_sample *)(w[13]);
  t_sample *z_out         = (t_sample *)(w[14]);
  int nblock              = (int)(w[15]); // block size

  while (nblock--)
  {
    if(!x->bypass)
    {
      x->v = vec3(x_in[nblock], y_in[nblock], z_in[nblock]);
      t_sample angle = angle_in[nblock];
      x->pivot = vec3(px_in[nblock], py_in[nblock], pz_in[nblock]);
      x->axis = vec3(ax_in[nblock], ay_in[nblock], az_in[nblock]);

      if (x->angle_mode)
      {
        angle *= DEG_TO_RAD;
      }

      v3_rotate_pivot(&x->v, angle, &x->pivot, &x->axis);

      x_out[nblock] = x->v.x;
      y_out[nblock] = x->v.y;
      z_out[nblock] = x->v.z;
    } else {
      x_out[nblock] = x_in[nblock];
      y_out[nblock] = y_in[nblock];
      z_out[nblock] = z_in[nblock];
    }
  }

  return (w + 16); // num ptrs + 1
}

void rotate_pivot_tilde_dsp(t_rotate_pivot_tilde *x, t_signal **sp)
{
  dsp_add(rotate_pivot_tilde_perform, 15, x,
          sp[0]->s_vec, // x_in
          sp[1]->s_vec, // y_in
          sp[2]->s_vec, // z_in
          sp[3]->s_vec, // angle
          sp[4]->s_vec, // px_in
          sp[5]->s_vec, // py_in
          sp[6]->s_vec, // pz_in
          sp[7]->s_vec, // ax_in
          sp[8]->s_vec, // ay_in
          sp[9]->s_vec, // az_in
          sp[10]->s_vec, // x_out
          sp[11]->s_vec, // y_out
          sp[12]->s_vec, // z_out
          sp[0]->s_n);  // block size
}

void angle_mode(t_rotate_pivot_tilde *x, t_symbol* mode)
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

static void bypass(t_rotate_pivot_tilde *x, t_floatarg bypass) {
  x->bypass = (int)bypass;
}

void rotate_pivot_tilde_free(t_rotate_pivot_tilde *x)
{
  // inlet_free(x->x_in);
  inlet_free(x->y_in);
  inlet_free(x->z_in);
  inlet_free(x->angle_in);
  inlet_free(x->px_in);
  inlet_free(x->py_in);
  inlet_free(x->pz_in);
  inlet_free(x->ax_in);
  inlet_free(x->ay_in);
  inlet_free(x->az_in);

  outlet_free(x->x_out);
  outlet_free(x->y_out);
  outlet_free(x->z_out);
}

void *rotate_pivot_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
  t_rotate_pivot_tilde *x = (t_rotate_pivot_tilde *)pd_new(rotate_pivot_tilde_class);

  x->v = vec3(0, 0, 0);
  x->pivot = vec3(0, 0, 0);
  x->axis = vec3(0, 0, 0);
  x->angle_mode = 1;
  x->bypass = 0;

  //    x->x_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->y_in     = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->z_in     = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->angle_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->px_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->py_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->pz_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->ax_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->ay_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->az_in    = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

  // argument parsing
  t_float angle = argc ? atom_getfloat(argv) : 0.f;
  t_float px    = argc > 1 ? atom_getfloat(argv+1) : 0.f;
  t_float py    = argc > 2 ? atom_getfloat(argv+2) : 0.f;
  t_float pz    = argc > 3 ? atom_getfloat(argv+3) : 0.f;
  t_float ax    = argc > 4 ? atom_getfloat(argv+4) : 0.f;
  t_float ay    = argc > 5 ? atom_getfloat(argv+5) : 0.f;
  t_float az    = argc > 6 ? atom_getfloat(argv+6) : 0.f;

  // pass args to inlets
  pd_float((t_pd *)x->angle_in, angle);
  pd_float((t_pd *)x->px_in, px);
  pd_float((t_pd *)x->py_in, py);
  pd_float((t_pd *)x->pz_in, pz);
  pd_float((t_pd *)x->ax_in, ax);
  pd_float((t_pd *)x->ay_in, ay);
  pd_float((t_pd *)x->az_in, az);

  x->x_out = outlet_new(&x->x_obj, &s_signal);
  x->y_out = outlet_new(&x->x_obj, &s_signal);
  x->z_out = outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

void rotate_pivot_tilde_setup(void)
{
  rotate_pivot_tilde_class = class_new(gensym("rotate_pivot~"),
                                       (t_newmethod)rotate_pivot_tilde_new,
                                       (t_method)rotate_pivot_tilde_free,
                                       sizeof(t_rotate_pivot_tilde),
                                       CLASS_DEFAULT,
                                       A_GIMME, // angle, pivot, axis
                                       0);

  class_sethelpsymbol(rotate_pivot_tilde_class, gensym("rotate~")); // links to the help patch

  class_addmethod(rotate_pivot_tilde_class, (t_method)angle_mode, gensym("mode"), A_DEFSYMBOL, 0);
  class_addmethod(rotate_pivot_tilde_class, (t_method)bypass, gensym("bypass"), A_DEFFLOAT, 0);

  class_addmethod(rotate_pivot_tilde_class, (t_method)rotate_pivot_tilde_dsp, gensym("dsp"), A_CANT, 0);
  CLASS_MAINSIGNALIN(rotate_pivot_tilde_class, t_rotate_pivot_tilde, f); // dummy arg for singal into first inlet
}
