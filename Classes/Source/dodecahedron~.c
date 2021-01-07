#include "m_pd.h"
#include "Audio_Math.h"

// do I really need this macro?
// It makes things clearer, but this number will never change either
// maybe a comment would do better?
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
    t_sample xPos, yPos, zPos, xScale, yScale, zScale;
    int lines[NUM_LINES];
    t_vec3 v1, v2; // v1 and v2 are temp vecs
    t_inlet *interp_amt;
    t_outlet *x_out, *y_out, *z_out;
} t_dodecahedron_tilde;

t_int *dodecahedron_tilde_perform(t_int *w)
{
    t_dodecahedron_tilde *x = (t_dodecahedron_tilde *)(w[1]); // access data space
    t_sample *driver_in   =           (t_sample *)(w[2]);
    t_sample *interp_amt  =           (t_sample *)(w[3]);
    t_sample *x_out       =           (t_sample *)(w[4]);
    t_sample *y_out       =           (t_sample *)(w[5]);
    t_sample *z_out       =           (t_sample *)(w[6]);
    int       n           =                  (int)(w[7]); // block size
    
  while (n--)
  {
      t_float amt = *interp_amt++;
      double t = mod1(*driver_in++);
      double t2 = t * NUM_LINES;
      int idx = t2;
      int idx_next = (idx + 1 < NUM_LINES) ? idx + 1 : 0;
      
      x->v1 = points[x->lines[idx]];
      x->v2 = points[x->lines[idx_next]];
      
      *x_out++ = lerp(mod1(t2 * amt), x->v1.x, x->v2.x) * x->xScale + x->xPos;
      *y_out++ = lerp(mod1(t2 * amt), x->v1.y, x->v2.y) * x->yScale + x->yPos;
      *z_out++ = lerp(mod1(t2 * amt), x->v1.z, x->v2.z) * x->zScale + x->zPos;
  }

  return (w + 8); // num ptrs + 1
}

void dodecahedron_tilde_dsp(t_dodecahedron_tilde *x, t_signal **sp)
{
    dsp_add(dodecahedron_tilde_perform, 7, x,
            sp[0]->s_vec, // driver_in
            sp[1]->s_vec, // interp_amt
            sp[2]->s_vec, // x_out
            sp[3]->s_vec, // y_out
            sp[4]->s_vec, // z_out
            sp[0]->s_n); // block size
}

                     // name used for creation, number of args, pointer to arg list
void *dodecahedron_tilde_new(t_symbol *s, int argc, t_atom *argv) // this is because of A_GIMME
{
    t_dodecahedron_tilde *x = (t_dodecahedron_tilde *)pd_new(dodecahedron_tilde_class);

    // one array with the points
    // one array with the order of points to lerp generating (lines)
    
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
    
    switch(argc)
    {
        case 6:
        {
            x->xPos = atom_getfloat(argv);
            x->yPos = atom_getfloat(argv+1);
            x->zPos = atom_getfloat(argv+2);
            x->xScale = atom_getfloat(argv+3);
            x->yScale = atom_getfloat(argv+4);
            x->zScale = atom_getfloat(argv+5);
            break;
        }
        case 5:
        {
            x->xPos = atom_getfloat(argv);
            x->yPos = atom_getfloat(argv+1);
            x->zPos = atom_getfloat(argv+2);
            x->xScale = atom_getfloat(argv+3);
            x->yScale = atom_getfloat(argv+4);
            x->zScale = 1;
            break;
        }
        case 4:
        {
            x->xPos = atom_getfloat(argv);
            x->yPos = atom_getfloat(argv+1);
            x->zPos = atom_getfloat(argv+2);
            x->xScale = atom_getfloat(argv+3);
            x->yScale = 1;
            x->zScale = 1;
            break;
        }
        case 3:
        {
            x->xPos = atom_getfloat(argv);
            x->yPos = atom_getfloat(argv+1);
            x->zPos = atom_getfloat(argv+2);
            x->xScale = 1;
            x->yScale = 1;
            x->zScale = 1;
            break;
        }
        case 2:
        {
            x->xPos = atom_getfloat(argv);
            x->yPos = atom_getfloat(argv+1);
            x->zPos = 0;
            x->xScale = 1;
            x->yScale = 1;
            x->zScale = 1;
            break;
        }
        case 1:
        {
            x->xPos = atom_getfloat(argv);
            x->yPos = 0;
            x->zPos = 0;
            x->xScale = 1;
            x->yScale = 1;
            x->zScale = 1;
            break;
        }
        default:
        {
            x->xPos = 0;
            x->yPos = 0;
            x->zPos = 0;
            x->xScale = 1;
            x->yScale = 1;
            x->zScale = 1;
            break;
        }
    }
    
    x->interp_amt = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
    x->x_out = outlet_new(&x->x_obj, &s_signal);
    x->y_out = outlet_new(&x->x_obj, &s_signal);
    x->z_out = outlet_new(&x->x_obj, &s_signal);
    
    return (void *)x;
}

void dodecahedron_tilde_free(t_dodecahedron_tilde *x)
{
    inlet_free(x->interp_amt);
    
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
                                  A_GIMME, // xPos, yPos, zPos, xScale, yScale, zScale
                                  0);
    
    class_addcreator((t_newmethod)dodecahedron_tilde_new,
                     gensym("dodeca~"),
                     A_GIMME, // xPos, yPos, zPos, xScale, yScale, zScale
                     0);
    
    class_sethelpsymbol(dodecahedron_tilde_class, gensym("dodecahedron~"));
    
    class_addmethod(dodecahedron_tilde_class, (t_method)dodecahedron_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(dodecahedron_tilde_class, t_dodecahedron_tilde, f); // dummy arg for singal into first inlet
}
