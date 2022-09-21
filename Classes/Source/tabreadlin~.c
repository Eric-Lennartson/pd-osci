#include "m_pd.h"
#include "Audio_Math.h"

static t_class *tabreadlin_tilde_class;

typedef struct _tabreadlin_tilde
{
    t_object obj;
    t_float f; // dummy
    int n_points, n_dimensions;
    t_word *x_vec, *y_vec, *z_vec;
    t_symbol *x_pts_array, *y_pts_array, *z_pts_array;
    // no inlet bc first in is default provided
    t_outlet *y_out, *z_out; // x_out default provided
} t_tabreadlin_tilde;

static void tabreadlin_tilde_set(t_tabreadlin_tilde *this, t_floatarg which, t_symbol *array_name)
{
    t_garray *array = NULL;

    // there's lots of duplicate code here, but I can't figure out
    // how to make it just call a function that does all this.
    switch( (int)which )
    {
        case 0:
            this->x_pts_array = array_name;
            if (!(array = (t_garray *)pd_findbyclass(this->x_pts_array, garray_class)))
            {
                if (*array_name->s_name)
                    pd_error(this, "tabread~: %s: no such array", this->x_pts_array->s_name);
                this->x_vec = 0;
            }
            else if (!garray_getfloatwords(array, &this->n_points, &this->x_vec))
            {
                pd_error(this, "%s: bad template for tabread~", this->x_pts_array->s_name);
                this->x_vec = 0;
            }
            else garray_usedindsp(array);
            break;
        case 1:
            this->y_pts_array = array_name;
            if (!(array = (t_garray *)pd_findbyclass(this->y_pts_array, garray_class)))
            {
                if (*array_name->s_name)
                    pd_error(this, "tabread~: %s: no such array", this->y_pts_array->s_name);
                this->y_vec = 0;
            }
            else if (!garray_getfloatwords(array, &this->n_points, &this->y_vec))
            {
                pd_error(this, "%s: bad template for tabread~", this->y_pts_array->s_name);
                this->y_vec = 0;
            }
            else garray_usedindsp(array);
            break;
        case 2:
            this->z_pts_array = array_name;
            if (!(array = (t_garray *)pd_findbyclass(this->z_pts_array, garray_class)))
            {
                if (*array_name->s_name)
                    pd_error(this, "tabread~: %s: no such array", this->z_pts_array->s_name);
                this->z_vec = 0;
            }
            else if (!garray_getfloatwords(array, &this->n_points, &this->z_vec))
            {
                pd_error(this, "%s: bad template for tabread~", this->z_pts_array->s_name);
                this->z_vec = 0;
            }
            else garray_usedindsp(array);
            break;
        default:
            this->x_pts_array = array_name;
            array = (t_garray *)pd_findbyclass(this->x_pts_array, garray_class);
            if (!(array = (t_garray *)pd_findbyclass(this->x_pts_array, garray_class)))
            {
                if (*array_name->s_name)
                    pd_error(this, "tabread~: %s: no such array", this->x_pts_array->s_name);
                this->x_vec = 0;
            }
            else if (!garray_getfloatwords(array, &this->n_points, &this->x_vec))
            {
                pd_error(this, "%s: bad template for tabread~", this->x_pts_array->s_name);
                this->x_vec = 0;
            }
            else garray_usedindsp(array);
            break;
    }

}

static t_int *tabreadlin_tilde_perform(t_int *w)
{
    t_tabreadlin_tilde *this = (t_tabreadlin_tilde *)(w[1]);
    t_sample *idx_in         = (t_sample *)(w[2]);
    t_sample *x_out          = (t_sample *)(w[3]);
    t_sample *y_out          = (t_sample *)(w[4]);
    t_sample *z_out          = (t_sample *)(w[5]);
    int nblock               = (int)(w[6]);

    int max_idx = this->n_points - 1;
    t_word *x_buf = this->x_vec;
    t_word *y_buf = this->y_vec;
    t_word *z_buf = this->z_vec;

    if(max_idx < 0 || x_buf == NULL || y_buf == NULL || z_buf == NULL)
    {
        while(nblock--)
        {
            x_out[nblock] = 0;
            y_out[nblock] = 0;
            z_out[nblock] = 0;
        }
        return (w+7);
    }
    else
    {
        while(nblock--)
        {
            // reading of proper range for table is controlled
            // outside the object.
            t_float t = idx_in[nblock]; // phasor controls lerp and idx
            int idx = CLAMP(idx_in[nblock], 0, max_idx);
            int idx_next = (idx + 1) % this->n_points;

            x_out[nblock] = lerp(mod1(t), x_buf[idx].w_float, x_buf[idx_next].w_float);
            y_out[nblock] = lerp(mod1(t), y_buf[idx].w_float, y_buf[idx_next].w_float);
            z_out[nblock] = lerp(mod1(t), z_buf[idx].w_float, z_buf[idx_next].w_float);
        }
    }

    return (w+7);
}

static void tabreadlin_tilde_dsp(t_tabreadlin_tilde *this, t_signal **sp)
{
    tabreadlin_tilde_set(this, 0, this->x_pts_array);
    tabreadlin_tilde_set(this, 1, this->y_pts_array);
    tabreadlin_tilde_set(this, 2, this->z_pts_array);

    dsp_add(tabreadlin_tilde_perform, 6, this,
            sp[0]->s_vec, // phasor input for idx
            sp[1]->s_vec, // x_out
            sp[2]->s_vec, // y_out
            sp[3]->s_vec, // z_out
            sp[0]->s_n); // block size
}

static void *tabreadlin_tilde_new(t_floatarg n_dimensions, t_symbol *x_array, t_symbol *y_array, t_symbol *z_array)
{
    t_tabreadlin_tilde *this = (t_tabreadlin_tilde *)pd_new(tabreadlin_tilde_class);

    // figure out what to do when there isn't a need for a third dimension
    this->n_dimensions = (n_dimensions <= 1) ? 2 : n_dimensions;
    this->x_pts_array = x_array;
    this->y_pts_array = y_array;
    this->z_pts_array = z_array;

    this->x_vec = NULL;
    this->y_vec = NULL;
    this->z_vec = NULL;
    this->f = 0;

    outlet_new(&this->obj, gensym("signal")); // default outlet
    this->y_out = outlet_new(&this->obj, &s_signal);
    this->z_out = outlet_new(&this->obj, &s_signal);

    return this;
}

static void tabreadlin_tilde_free(t_tabreadlin_tilde *this) {}

void tabreadlin_tilde_setup(void)
{
    tabreadlin_tilde_class = class_new(gensym("tabreadlin~"),
                                        (t_newmethod)tabreadlin_tilde_new,
                                        (t_method)tabreadlin_tilde_free,
                                        sizeof(t_tabreadlin_tilde),
                                        CLASS_DEFAULT,
                                        A_DEFFLOAT,
                                        A_DEFSYMBOL,
                                        A_DEFSYMBOL,
                                        A_DEFSYMBOL, 0);
    CLASS_MAINSIGNALIN(tabreadlin_tilde_class, t_tabreadlin_tilde, f);
    class_addmethod(tabreadlin_tilde_class, (t_method)tabreadlin_tilde_dsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(tabreadlin_tilde_class, (t_method)tabreadlin_tilde_set, gensym("set"), A_FLOAT, A_SYMBOL, 0);
}