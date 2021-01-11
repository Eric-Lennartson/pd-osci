#include "m_pd.h"
#include "Audio_Math.h"

static t_class *icosahedron_tilde_class;

t_vec3 points[] = {{0,0,-1},
                 {0.7236,-0.52572,-0.447215},
                 {-0.276385,-0.85064,-0.447215},
                 {-0.894425,0,-0.447215},
                 {-0.276385,0.85064,-0.447215},
                 {0.7236,0.52572,-0.447215},
                 {0.276385,-0.85064,0.447215},
                 {-0.7236,-0.52572,0.447215},
                 {-0.7236,0.52572,0.447215},
                 {0.276385,0.85064,0.447215},
                 {0.894425,0,0.447215},
                 {0,0,1}
                };

typedef struct _icosahedron_tilde
{
    t_object  x_obj;
    t_sample f; // dummy arg
    t_sample xPos, yPos, zPos, xScale, yScale, zScale;
    int lines[36];
    t_vec3 v1, v2; // v1 and v2 are temp vecs
    t_inlet *interp_amt;
    t_outlet *x_out, *y_out, *z_out;
} t_icosahedron_tilde;

t_int *icosahedron_tilde_perform(t_int *w)
{
    t_icosahedron_tilde *x = (t_icosahedron_tilde *)(w[1]); // access data space
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
      double t2 = t * 36;
      int idx = t2;
      int idx_next = (idx + 1 < 36) ? idx + 1 : 0;
      
      x->v1 = points[x->lines[idx]];
      x->v2 = points[x->lines[idx_next]];
      
      *x_out++ = lerp(mod1(t2 * amt), x->v1.x, x->v2.x) * x->xScale + x->xPos;
      *y_out++ = lerp(mod1(t2 * amt), x->v1.y, x->v2.y) * x->yScale + x->yPos;
      *z_out++ = lerp(mod1(t2 * amt), x->v1.z, x->v2.z) * x->zScale + x->zPos;
  }

  return (w + 8); // num ptrs + 1
}

void icosahedron_tilde_dsp(t_icosahedron_tilde *x, t_signal **sp)
{
    dsp_add(icosahedron_tilde_perform, 7, x,
            sp[0]->s_vec, // driver_in
            sp[1]->s_vec, // interp_amt
            sp[2]->s_vec, // x_out
            sp[3]->s_vec, // y_out
            sp[4]->s_vec, // z_out
            sp[0]->s_n); // block size
}

                     // name used for creation, number of args, pointer to arg list
void *icosahedron_tilde_new(t_symbol *s, int argc, t_atom *argv) // this is because of A_GIMME
{
    t_icosahedron_tilde *x = (t_icosahedron_tilde *)pd_new(icosahedron_tilde_class);

    // here's the better way
    // one array with the points
    // one array with the order of points to lerp generating (lines)
    x->lines[0] = 11;
    x->lines[1] = 9;
    x->lines[2] = 10;
    x->lines[3] = 5;
    x->lines[4] = 1;
    x->lines[5] = 2;
    x->lines[6] = 0;
    x->lines[7] = 1;
    x->lines[8] = 10;
    x->lines[9] = 6;
    x->lines[10] = 7;
    x->lines[11] = 8;
    x->lines[12] = 9;
    x->lines[13] = 4;
    x->lines[14] = 3;
    x->lines[15] = 8;
    x->lines[16] = 4;
    x->lines[17] = 5;
    x->lines[18] = 1;
    x->lines[19] = 6;
    x->lines[20] = 2;
    x->lines[21] = 0;
    x->lines[22] = 4;
    x->lines[23] = 3;
    x->lines[24] = 7;
    x->lines[25] = 2;
    x->lines[26] = 3;
    x->lines[27] = 0;
    x->lines[28] = 5;
    x->lines[29] = 9;
    x->lines[30] = 11;
    x->lines[31] = 8;
    x->lines[32] = 7;
    x->lines[33] = 11;
    x->lines[34] = 10;
    x->lines[35] = 6;
    
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
    
    x->interp_amt = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal); // do I have to pass an argument to the inlet?
    
    x->x_out = outlet_new(&x->x_obj, &s_signal);
    x->y_out = outlet_new(&x->x_obj, &s_signal);
    x->z_out = outlet_new(&x->x_obj, &s_signal);
    
    return (void *)x;
}

void icosahedron_tilde_free(t_icosahedron_tilde *x)
{
    inlet_free(x->interp_amt);
    
    outlet_free(x->x_out);
    outlet_free(x->y_out);
    outlet_free(x->z_out);
}

void icosahedron_tilde_setup(void)
{
    icosahedron_tilde_class = class_new(gensym("icosa~"),
                                  (t_newmethod)icosahedron_tilde_new,
                                  (t_method)icosahedron_tilde_free,
                                  sizeof(t_icosahedron_tilde),
                                  CLASS_DEFAULT,
                                  A_GIMME, // xPos, yPos, zPos, xScale, yScale, zScale
                                  0);
    
    class_sethelpsymbol(icosahedron_tilde_class, gensym("icosahedron~"));
    
    class_addmethod(icosahedron_tilde_class, (t_method)icosahedron_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(icosahedron_tilde_class, t_icosahedron_tilde, f); // dummy arg for singal into first inlet
}