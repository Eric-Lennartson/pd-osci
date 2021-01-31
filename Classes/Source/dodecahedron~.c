#include "m_pd.h"
#include "Audio_Math.h"

#define NUM_LINES 40

static t_class *dodecahedron_tilde_class;

// copied from an oscisctudio txt file
t_vec3 points[] = {{-0.401779,-0.29191,-0.620608},
                 {-0.401779,0.29191,-0.620608},
                 {0.226971,-0.748722,-0.140281},
                 {0.132124,-0.456813,-0.636912},
                 {-0.623716,-0.472315,-0.140281},
                 {0.161622,0.447228,-0.636912},
                 {-0.641941,0.447231,-0.140281},
                 {0.78222,-0.0155064,-0.140282},
                 {0.475286,-0.0155094,-0.636911},
                 {-0.256463,-0.739139,0.140281},
                 {-0.226971,0.748722,0.140281},
                 {-0.78222,-0.0155064,0.140282},
                 {-0.475286,-0.0155094,0.636911},
                 {0.240161,0.739139,-0.16666},
                 {-0.161622,0.447228,0.636912},
                 {0.393628,0.266822,0.636911},
                 {0.623716,-0.472315,0.140281},
                 {0.623716,0.472315,0.140281},
                 {-0.153469,-0.472321,0.620605},
                 {0.401779,-0.29191,0.620608},
                };

typedef struct _dodecahedron_tilde
{
    t_object  x_obj;
    t_sample f; // dummy arg
    t_sample xPos, yPos, zPos, xLen, yLen, zLen;
    int lines[NUM_LINES];
    t_vec3 v1, v2; // v1 and v2 are temp vecs
    t_inlet *xPos_in, *yPos_in, *zPos_in, *xLen_in, *yLen_in, *zLen_in,*interp_amt_in;
    t_outlet *x_out, *y_out, *z_out;
} t_dodecahedron_tilde;

t_int *dodecahedron_tilde_perform(t_int *w)
{
    t_dodecahedron_tilde *x = (t_dodecahedron_tilde *)(w[1]); // access data space
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
      t_sample t2 = t * NUM_LINES;
      int idx = t2;
      int idx_next = (idx + 1) % NUM_LINES;
      
      x->v1 = points[x->lines[idx]];
      x->v2 = points[x->lines[idx_next]];
      
      x_out[nblock] = lerp(mod1(t2 * amt), x->v1.x, x->v2.x) * x->xLen + x->xPos;
      y_out[nblock] = lerp(mod1(t2 * amt), x->v1.y, x->v2.y) * x->yLen + x->yPos;
      z_out[nblock] = lerp(mod1(t2 * amt), x->v1.z, x->v2.z) * x->zLen + x->zPos;
  }

  return (w + 14); // num ptrs + 1
}

void dodecahedron_tilde_dsp(t_dodecahedron_tilde *x, t_signal **sp)
{
    dsp_add(dodecahedron_tilde_perform, 13, x,
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

                     // name used for creation, number of args, pointer to arg list
void *dodecahedron_tilde_new(t_symbol *s, int argc, t_atom *argv) // this is because of A_GIMME
{
    t_dodecahedron_tilde *x = (t_dodecahedron_tilde *)pd_new(dodecahedron_tilde_class);

    // one array with the points
    // another array with the order of points to interpolate generating (lines)
    x->lines[0]  = 19;
    x->lines[1]  = 15;
    x->lines[2]  = 19;
    x->lines[3]  = 16;
    //
    x->lines[4]  = 2;
    x->lines[5]  = 3;
    x->lines[6]  = 0;
    x->lines[7]  = 1;
    //
    x->lines[8]  = 5;
    x->lines[9]  = 8;
    x->lines[10] = 7;
    x->lines[11] = 16;
    //
    x->lines[12] = 2;
    x->lines[13] = 9;
    x->lines[14] = 18;
    x->lines[15] = 12;
    //
    x->lines[16] = 14;
    x->lines[17] = 12;
    x->lines[18] = 11;
    x->lines[19] = 4;
    //
    x->lines[20] = 11;
    x->lines[21] = 6;
    x->lines[22] = 1;
    x->lines[23] = 6;
    //
    x->lines[24] = 10;
    x->lines[25] = 13;
    x->lines[26] = 17;
    x->lines[27] = 7;
    //
    x->lines[28] = 17;
    x->lines[29] = 15;
    x->lines[30] = 14;
    x->lines[31] = 10;
    //

    x->lines[32] = 13;
    x->lines[33] = 5;
    x->lines[34] = 8;
    x->lines[35] = 3;
    //
    x->lines[36] = 0;
    x->lines[37] = 4;
    x->lines[38] = 9;
    x->lines[39] = 18;
    
    x->xPos = argc ? atom_getfloat(argv) : 0.f;
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

void dodecahedron_tilde_free(t_dodecahedron_tilde *x)
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

void dodecahedron_tilde_setup(void)
{
    dodecahedron_tilde_class = class_new(gensym("dodecahedron~"),
                                  (t_newmethod)dodecahedron_tilde_new,
                                  (t_method)dodecahedron_tilde_free,
                                  sizeof(t_dodecahedron_tilde),
                                  CLASS_DEFAULT,
                                  A_GIMME, // xPos, yPos, zPos, xLen, yLen, zLen
                                  0);
    
    class_sethelpsymbol(dodecahedron_tilde_class, gensym("dodecahedron~"));
    
    class_addmethod(dodecahedron_tilde_class, (t_method)dodecahedron_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(dodecahedron_tilde_class, t_dodecahedron_tilde, f); // dummy arg for singal into first inlet
}
