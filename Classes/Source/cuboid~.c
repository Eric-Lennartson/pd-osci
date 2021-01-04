#include "m_pd.h"
#include "Audio_Math.h"

static t_class *cuboid_tilde_class;

typedef struct _cuboid_tilde
{
    t_object  x_obj;
    t_sample f; // dummy arg
    t_sample xPos, yPos, zPos, xLen, yLen, zLen;
    vec3 points[16], v1, v2; // v1 and v2 are temp vecs
    t_inlet *interp_amt;
    t_outlet *x_out, *y_out, *z_out;
} t_cuboid_tilde;

t_int *cuboid_tilde_perform(t_int *w)
{
    t_cuboid_tilde *x   = (t_cuboid_tilde *)(w[1]); // access data space
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
      double t2 = t * 16;
      int idx = t2;
      int idx_next = (idx + 1 < 16) ? idx + 1 : 0;
      
      x->v1 = set(x->points[idx].x, x->points[idx].y, x->points[idx].z);
      x->v2 = set(x->points[idx_next].x, x->points[idx_next].y, x->points[idx_next].z);
      
      *x_out++ = lerp(mod1(t2 * amt), x->v1.x, x->v2.x) * x->xLen + x->xPos;
      *y_out++ = lerp(mod1(t2 * amt), x->v1.y, x->v2.y) * x->yLen + x->yPos;
      *z_out++ = lerp(mod1(t2 * amt), x->v1.z, x->v2.z) * x->zLen + x->zPos;
  }

  return (w + 8); // num ptrs + 1
}

void cuboid_tilde_dsp(t_cuboid_tilde *x, t_signal **sp)
{
    dsp_add(cuboid_tilde_perform, 7, x,
            sp[0]->s_vec, // driver_in
            sp[1]->s_vec, // interp_amt
            sp[2]->s_vec, // x_out
            sp[3]->s_vec, // y_out
            sp[4]->s_vec, // z_out
            sp[0]->s_n); // block size
}

void cuboid_tilde_free(t_cuboid_tilde *x)
{
    inlet_free(x->interp_amt);
    
    outlet_free(x->x_out);
    outlet_free(x->y_out);
    outlet_free(x->z_out);
}
                     // name used for creation, number of args, pointer to arg list
void *cuboid_tilde_new(t_symbol *s, int argc, t_atom *argv) // this is because of A_GIMME
{
    t_cuboid_tilde *x = (t_cuboid_tilde *)pd_new(cuboid_tilde_class);
    
    // this could be done better, but this works, and I don't need to change them later
    x->points[0]  = set(-1, -1,  1);
    x->points[1]  = set(-1,  1,  1);
    x->points[2]  = set( 1,  1,  1);
    x->points[3]  = set( 1, -1,  1);
    x->points[4]  = set(-1, -1,  1);
    x->points[5]  = set(-1, -1, -1);
    x->points[6]  = set(-1,  1, -1);
    x->points[7]  = set(-1,  1,  1);
    x->points[8]  = set(-1,  1, -1);
    x->points[9]  = set( 1,  1, -1);
    x->points[10] = set( 1,  1,  1);
    x->points[11] = set( 1,  1, -1);
    x->points[12] = set( 1, -1, -1);
    x->points[13] = set( 1, -1,  1);
    x->points[14] = set( 1, -1, -1);
    x->points[15] = set(-1, -1, -1);
    
    switch(argc)
    {
        case 6:
        {
            x->xPos = atom_getfloat(argv);
            x->yPos = atom_getfloat(argv+1);
            x->zPos = atom_getfloat(argv+2);
            x->xLen = atom_getfloat(argv+3);
            x->yLen = atom_getfloat(argv+4);
            x->zLen = atom_getfloat(argv+5);
            break;
        }
        case 5:
        {
            x->xPos = atom_getfloat(argv);
            x->yPos = atom_getfloat(argv+1);
            x->zPos = atom_getfloat(argv+2);
            x->xLen = atom_getfloat(argv+3);
            x->yLen = atom_getfloat(argv+4);
            x->zLen = 0.5;
            break;
        }
        case 4:
        {
            x->xPos = atom_getfloat(argv);
            x->yPos = atom_getfloat(argv+1);
            x->zPos = atom_getfloat(argv+2);
            x->xLen = atom_getfloat(argv+3);
            x->yLen = 0.5;
            x->zLen = 0.5;
            break;
        }
        case 3:
        {
            x->xPos = atom_getfloat(argv);
            x->yPos = atom_getfloat(argv+1);
            x->zPos = atom_getfloat(argv+2);
            x->xLen = 0.5;
            x->yLen = 0.5;
            x->zLen = 0.5;
            break;
        }
        case 2:
        {
            x->xPos = atom_getfloat(argv);
            x->yPos = atom_getfloat(argv+1);
            x->zPos = 0;
            x->xLen = 0.5;
            x->yLen = 0.5;
            x->zLen = 0.5;
            break;
        }
        case 1:
        {
            x->xPos = atom_getfloat(argv);
            x->yPos = 0;
            x->zPos = 0;
            x->xLen = 0.5;
            x->yLen = 0.5;
            x->zLen = 0.5;
            break;
        }
        default:
        {
            x->xPos = 0;
            x->yPos = 0;
            x->zPos = 0;
            x->xLen = 0.5;
            x->yLen = 0.5;
            x->zLen = 0.5;
            break;
        }
    }
    
    x->interp_amt = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
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
    
    class_addcreator((t_newmethod)cuboid_tilde_new,
                     gensym("osci/cuboid~"),
                     A_GIMME, // xPos, yPos, zPos, xLen, yLen, zLen
                     0);
    
    class_sethelpsymbol(cuboid_tilde_class, gensym("cuboid~"));
    
    class_addmethod(cuboid_tilde_class, (t_method)cuboid_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(cuboid_tilde_class, t_cuboid_tilde, f); // dummy arg for singal into first inlet
}
