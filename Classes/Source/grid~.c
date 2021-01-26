#include "m_pd.h"
#include "Audio_Math.h"
#include "vec3.h"

static t_class *grid_tilde_class;

// fucking rip, this needs to be cleaned up big time

/*
 Not quite the same as oscistudio's grid.
 I don't know of a way to get a global phase and vector modifier chain to happen in pd,
 so right now this object can come anywhere in the chain, but has to recieve the original phase from the top of the chain...
 bleugh. This is really ugly, but I don't know how to make it better yet.
 */

typedef struct _grid_tilde
{
    t_object    x_obj;
    t_sample    f; // dummy arg
    t_vec3 v;
    t_inlet     *nx_in, *ny_in, *nz_in, *spread_in;
    t_outlet    *t2_out, *scale_out, *x_out, *y_out, *z_out;
} t_grid_tilde;

t_int *grid_tilde_perform(t_int *w)
{
    t_grid_tilde *x  = (t_grid_tilde *)(w[1]);
    t_sample* driver    =  (t_sample *)(w[2]);
    t_sample* nx_in     =  (t_sample *)(w[3]);
    t_sample* ny_in     =  (t_sample *)(w[4]);
    t_sample* nz_in     =  (t_sample *)(w[5]);
    t_sample* spread_in =  (t_sample *)(w[6]);
    t_sample* t2_out    =  (t_sample *)(w[7]);
    t_sample* scale_out =  (t_sample *)(w[8]);
    t_sample* x_out     =  (t_sample *)(w[9]);
    t_sample* y_out     =  (t_sample *)(w[10]);
    t_sample* z_out     =  (t_sample *)(w[11]);
    int       nblock    =         (int)(w[12]);
    
    while (nblock--)
    {
        //init everything
        t_float t = *driver++;
        t_float spread = *spread_in++;
        int nx = max(*nx_in++, 1);
        int ny = max(*ny_in++, 1);
        int nz = max(*nz_in++, 1);

        int n_total = nx * ny * nz;

        // calculate where we are
        t_float tn = n_total * t;
        int idx = tn;
        int idx_x = (idx / nz) % nx;
        int idx_y = idx / (nx * nz);
        int idx_z = idx % nz;
        t_float t2 = mod1(tn);

        // now compute the locations accordingly
        t_float space_x = 1/(t_float)nx;
        t_float space_y = 1/(t_float)ny;
        t_float space_z = 1/(t_float)nz;

        t_float off_x = -1 + space_x * (1 + idx_x * 2);
        t_float off_y = -1 + space_y * (1 + idx_y * 2);
        t_float off_z = -1 + space_z * (1 + idx_z * 2);

        t_float scale = min(space_x,space_y);
        scale = min(scale,space_z);

        //t_vec3 v = input(t2)*scale + t_vec3(off_x, off_y, off_z)*spread; <-- what oscistudio would be
        x->v = v3_multf(x->v, scale);
        x->v = v3_add(x->v, (t_vec3){off_x * spread, off_y * spread, off_z * spread} );

        *t2_out++ = t2;
        *scale_out++ = scale;
        *x_out++ = x->v.x;
        *y_out++ = x->v.y;
        *z_out++ = x->v.z;
    }

   return (w + 13);
}

void grid_tilde_dsp(t_grid_tilde *x, t_signal **sp)
{
    dsp_add(grid_tilde_perform, 12, x,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // nx in
            sp[2]->s_vec, // ny in
            sp[3]->s_vec, // nz in
            sp[4]->s_vec, // spread in
            sp[5]->s_vec, // t2_out
            sp[6]->s_vec, // scale out
            sp[7]->s_vec, // x_out
            sp[8]->s_vec, // y_out
            sp[9]->s_vec, // z_out
            sp[0]->s_n);  // block size
}

void grid_tilde_free(t_grid_tilde *x)
{
    inlet_free(x->nx_in);
    inlet_free(x->ny_in);
    inlet_free(x->nz_in);
    inlet_free(x->spread_in);

    outlet_free(x->t2_out);
    outlet_free(x->scale_out);
    outlet_free(x->x_out);
    outlet_free(x->y_out);
    outlet_free(x->z_out);
}

void *grid_tilde_new(t_floatarg nx, t_floatarg ny, t_floatarg nz, t_floatarg spread)
{
    t_grid_tilde *x = (t_grid_tilde *)pd_new(grid_tilde_class);

    x->v   = NEW_VEC3;
    
    x->nx_in     = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->ny_in     = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->nz_in     = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->spread_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

    pd_float((t_pd*)x->nx_in, nx);
    pd_float((t_pd*)x->ny_in, ny);
    pd_float((t_pd*)x->nz_in, nz);
    pd_float((t_pd*)x->spread_in, spread);

    x->t2_out = outlet_new(&x->x_obj, &s_signal);
    x->scale_out = outlet_new(&x->x_obj, &s_signal);
    x->x_out = outlet_new(&x->x_obj, &s_signal);
    x->y_out = outlet_new(&x->x_obj, &s_signal);
    x->z_out = outlet_new(&x->x_obj, &s_signal);

    return (void*)x;
}

void grid_tilde_setup(void)
{
    grid_tilde_class = class_new(gensym("grid~"),
                                (t_newmethod)grid_tilde_new, // ctor
                                (t_method)grid_tilde_free, // dtor
                                sizeof(t_grid_tilde), // data space
                                CLASS_DEFAULT, // gui appearance
                                A_DEFFLOAT, // nx
                                A_DEFFLOAT, // ny
                                A_DEFFLOAT, // nz
                                A_DEFFLOAT, // spread
                                0);
    
    class_sethelpsymbol(grid_tilde_class, gensym("grid~"));
    
    class_addmethod(grid_tilde_class, (t_method)grid_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(grid_tilde_class, t_grid_tilde, f); // dummy arg for signal into first inlet
}
