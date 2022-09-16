#include "m_pd.h"
#include "Audio_Math.h"

static t_class *cuboid_tilde_class;

typedef struct _cuboid_tilde
{
    t_object  x_obj;
    t_sample f; // dummy arg
    t_sample xPos, yPos, zPos, xLen, yLen, zLen;
    t_vec3 points[16], v1, v2; // v1 and v2 are temp vecs
    t_inlet *xPos_in, *yPos_in, *zPos_in, *xLen_in, *yLen_in, *zLen_in, *interp_amt_in;
    t_outlet *x_out, *y_out, *z_out;
} t_cuboid_tilde;

t_int *cuboid_tilde_perform(t_int *w)
{
    t_cuboid_tilde *x  = (t_cuboid_tilde *)(w[1]);
    t_sample *phase_in       = (t_sample *)(w[2]);
    t_sample *xPos_in        = (t_sample *)(w[3]);
    t_sample *yPos_in        = (t_sample *)(w[4]);
    t_sample *zPos_in        = (t_sample *)(w[5]);
    t_sample *xLen_in        = (t_sample *)(w[6]);
    t_sample *yLen_in        = (t_sample *)(w[7]);
    t_sample *zLen_in        = (t_sample *)(w[8]);
    t_sample *interp_amt_in  = (t_sample *)(w[9]);
    t_sample *x_out          = (t_sample *)(w[10]);
    t_sample *y_out          = (t_sample *)(w[11]);
    t_sample *z_out          = (t_sample *)(w[12]);
    int       nblock         = (int)(w[13]); // block size

  while (nblock--)
  {
      t_sample t = mod1(phase_in[nblock]);
      x->xPos = xPos_in[nblock];
      x->yPos = yPos_in[nblock];
      x->zPos = zPos_in[nblock];
      x->xLen = xLen_in[nblock];
      x->yLen = yLen_in[nblock];
      x->zLen = zLen_in[nblock];
      t_sample amt = interp_amt_in[nblock];

      t_sample t2 = t * 16;
      int idx = t2;
      int idx_next = (idx + 1) % 16;

      x->v1 = vec3(x->points[idx].x, x->points[idx].y, x->points[idx].z);
      x->v2 = vec3(x->points[idx_next].x, x->points[idx_next].y, x->points[idx_next].z);

      x_out[nblock] = lerp(mod1(t2 * amt), x->v1.x, x->v2.x) * x->xLen + x->xPos;
      y_out[nblock] = lerp(mod1(t2 * amt), x->v1.y, x->v2.y) * x->yLen + x->yPos;
      z_out[nblock] = lerp(mod1(t2 * amt), x->v1.z, x->v2.z) * x->zLen + x->zPos;
  }

  return (w + 14); // num ptrs + 1
}

void cuboid_tilde_dsp(t_cuboid_tilde *x, t_signal **sp)
{
    dsp_add(cuboid_tilde_perform, 13, x,
            sp[0]->s_vec, // phase_in
            sp[1]->s_vec, // xPos_in
            sp[2]->s_vec, // yPos_in
            sp[3]->s_vec, // zPos_in
            sp[4]->s_vec, // xLen_in
            sp[5]->s_vec, // yLen_in
            sp[6]->s_vec, // zLen_in
            sp[7]->s_vec, // interp_amt_in
            sp[8]->s_vec, // x_out
            sp[9]->s_vec, // y_out
            sp[10]->s_vec, // z_out
            sp[0]->s_n); // block size
}

void cuboid_tilde_free(t_cuboid_tilde *x)
{
    inlet_free(x->xPos_in);
    inlet_free(x->yPos_in);
    inlet_free(x->zPos_in);
    inlet_free(x->xLen_in);
    inlet_free(x->yLen_in);
    inlet_free(x->zLen_in);
    inlet_free(x->interp_amt_in);

    outlet_free(x->x_out);
    outlet_free(x->y_out);
    outlet_free(x->z_out);
}
                     // name used for creation, number of args, pointer to arg list
void *cuboid_tilde_new(t_symbol *s, int argc, t_atom *argv) // this is because of A_GIMME
{
    t_cuboid_tilde *x = (t_cuboid_tilde *)pd_new(cuboid_tilde_class);

    // this could be done better, but this works, and I don't need to change them later
    x->points[0]  = vec3(-1, -1,  1);
    x->points[1]  = vec3(-1,  1,  1);
    x->points[2]  = vec3( 1,  1,  1);
    x->points[3]  = vec3( 1, -1,  1);
    x->points[4]  = vec3(-1, -1,  1);
    x->points[5]  = vec3(-1, -1, -1);
    x->points[6]  = vec3(-1,  1, -1);
    x->points[7]  = vec3(-1,  1,  1);
    x->points[8]  = vec3(-1,  1, -1);
    x->points[9]  = vec3( 1,  1, -1);
    x->points[10] = vec3( 1,  1,  1);
    x->points[11] = vec3( 1,  1, -1);
    x->points[12] = vec3( 1, -1, -1);
    x->points[13] = vec3( 1, -1,  1);
    x->points[14] = vec3( 1, -1, -1);
    x->points[15] = vec3(-1, -1, -1);

    x->xPos = argc ? atom_getfloat(argv)   : 0.f;
    x->yPos = argc > 1 ? atom_getfloat(argv+1) : 0.f;
    x->zPos = argc > 2 ? atom_getfloat(argv+2) : 0.f;
    x->xLen = argc > 3 ? atom_getfloat(argv+3) : 0.5;
    x->yLen = argc > 4 ? atom_getfloat(argv+4) : 0.5;
    x->zLen = argc > 5 ? atom_getfloat(argv+5) : 0.5;

    x->xPos_in       = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->yPos_in       = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->zPos_in       = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->xLen_in       = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->yLen_in       = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->zLen_in       = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->interp_amt_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

    pd_float((t_pd*)x->xPos_in, x->xPos);
    pd_float((t_pd*)x->yPos_in, x->yPos);
    pd_float((t_pd*)x->zPos_in, x->zPos);
    pd_float((t_pd*)x->xLen_in, x->xLen);
    pd_float((t_pd*)x->yLen_in, x->yLen);
    pd_float((t_pd*)x->zLen_in, x->zLen);
    pd_float((t_pd*)x->interp_amt_in, 1);

    x->x_out = outlet_new(&x->x_obj, &s_signal);
    x->y_out = outlet_new(&x->x_obj, &s_signal);
    x->z_out = outlet_new(&x->x_obj, &s_signal);

    return (void *)x;
}

void cuboid_tilde_setup(void)
{
    cuboid_tilde_class = class_new(gensym("cuboid~"),
                                  (t_newmethod)cuboid_tilde_new,
                                  (t_method)cuboid_tilde_free,
                                  sizeof(t_cuboid_tilde),
                                  CLASS_DEFAULT,
                                  A_GIMME, // xPos, yPos, zPos, xLen, yLen, zLen
                                   // A_GIMME used for six or more args
                                  0);

    class_sethelpsymbol(cuboid_tilde_class, gensym("cuboid~"));

    class_addmethod(cuboid_tilde_class, (t_method)cuboid_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(cuboid_tilde_class, t_cuboid_tilde, f); // dummy arg for singal into first inlet
}
