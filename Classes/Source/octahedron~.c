#include "m_pd.h"
#include "Audio_Math.h"

static t_class *octahedron_tilde_class;

typedef struct _octahedron_tilde
{
    t_object  x_obj;
    t_sample f; // dummy arg
    t_sample xPos, yPos, zPos, xScale, yScale, zScale;
    t_vec3 points[12], v1, v2; // v1 and v2 are temp vecs
    t_inlet *interp_amt;
    t_outlet *x_out, *y_out, *z_out;
} t_octahedron_tilde;

t_int *octahedron_tilde_perform(t_int *w)
{
    t_octahedron_tilde *x = (t_octahedron_tilde *)(w[1]); // access data space
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
      double t2 = t * 12;
      int idx = t2;
      int idx_next = (idx + 1 < 12) ? idx + 1 : 0;
      
      x->v1 = vec3(x->points[idx].x, x->points[idx].y, x->points[idx].z);
      x->v2 = vec3(x->points[idx_next].x, x->points[idx_next].y, x->points[idx_next].z);
      
      *x_out++ = lerp(mod1(t2 * amt), x->v1.x, x->v2.x) * x->xScale + x->xPos;
      *y_out++ = lerp(mod1(t2 * amt), x->v1.y, x->v2.y) * x->yScale + x->yPos;
      *z_out++ = lerp(mod1(t2 * amt), x->v1.z, x->v2.z) * x->zScale + x->zPos;
  }

  return (w + 8); // num ptrs + 1
}

void octahedron_tilde_dsp(t_octahedron_tilde *x, t_signal **sp)
{
    dsp_add(octahedron_tilde_perform, 7, x,
            sp[0]->s_vec, // driver_in
            sp[1]->s_vec, // interp_amt
            sp[2]->s_vec, // x_out
            sp[3]->s_vec, // y_out
            sp[4]->s_vec, // z_out
            sp[0]->s_n); // block size
}

                     // name used for creation, number of args, pointer to arg list
void *octahedron_tilde_new(t_symbol *s, int argc, t_atom *argv) // this is because of A_GIMME
{
    t_octahedron_tilde *x = (t_octahedron_tilde *)pd_new(octahedron_tilde_class);
    
    // this could be done better, but this works, and I don't need to change them later
    x->points[0]  = vec3(-0.5, -0.5,  0.0);
    x->points[1]  = vec3(-0.5,  0.5,  0.0);
    x->points[2]  = vec3( 0.0,  0.0, -0.5);
    x->points[3]  = vec3(-0.5, -0.5,  0.0);
    x->points[4]  = vec3( 0.5, -0.5,  0.0);
    x->points[5]  = vec3( 0.0,  0.0, -0.5);
    x->points[6]  = vec3( 0.5,  0.5,  0.0);
    x->points[7]  = vec3( 0.5, -0.5,  0.0);
    x->points[8]  = vec3( 0.0,  0.0,  0.5);
    x->points[9]  = vec3( 0.5,  0.5,  0.0);
    x->points[10] = vec3(-0.5,  0.5,  0.0);
    x->points[11] = vec3( 0.0,  0.0,  0.5);
        
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

void octahedron_tilde_free(t_octahedron_tilde *x)
{
    inlet_free(x->interp_amt);
    
    outlet_free(x->x_out);
    outlet_free(x->y_out);
    outlet_free(x->z_out);
}

void octahedron_tilde_setup(void)
{
    octahedron_tilde_class = class_new(gensym("octahedron~"),
                                  (t_newmethod)octahedron_tilde_new,
                                  (t_method)octahedron_tilde_free,
                                  sizeof(t_octahedron_tilde),
                                  CLASS_DEFAULT,
                                  A_GIMME, // xPos, yPos, zPos, xScale, yScale, zScale
                                  0);
    
    class_sethelpsymbol(octahedron_tilde_class, gensym("octahedron~"));
    
    class_addmethod(octahedron_tilde_class, (t_method)octahedron_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(octahedron_tilde_class, t_octahedron_tilde, f); // dummy arg for singal into first inlet
}
