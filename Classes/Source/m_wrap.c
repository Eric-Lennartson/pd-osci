#include "m_pd.h"
#include <math.h>

typedef struct _m_wrap
{
    t_object x_obj;
    t_float  min, max, multiple;
} t_m_wrap;

static t_class *m_wrap_class;

t_float m_wrap(t_float value,  t_float min, t_float max, t_float multiple)
{
    // bounds checks, probably a better way to do this
    // todo call clamp insteads
    min = (min <= 0) ? 0.1 : min;
    max = (max < min) ? min : max;
    multiple = (multiple <= 1) ? 1.1 : multiple;

    value = (value == 0.f) ? min : value;
    value = fabsf(value);

    while( value > max ) { value /= multiple; }
    while( value < min )
    {
        if( value * multiple > max )
            break;
        value *= multiple;
    }

    return value;
}

static void m_wrap_float(t_m_wrap *x, t_floatarg f)
{
    if(f <= 0)
        pd_error(x, "[m_wrap] warning: input must be > 0.");
    else {
        t_float result = m_wrap(f, x->min, x->max, x->multiple);
        outlet_float(x->x_obj.ob_outlet, result);
    }
}

static void *m_wrap_new(t_floatarg min, t_floatarg max, t_floatarg multiple)
{
    t_m_wrap *x = (t_m_wrap *)pd_new(m_wrap_class);

    min = (min <= 0) ? 0.1 : min;
    max = (max < min) ? min : max;
    multiple = (multiple <= 1) ? 1.1 : multiple;

    x->min = min;
    x->max = max;
    x->multiple = multiple;

    floatinlet_new(&x->x_obj, &x->min);
    floatinlet_new(&x->x_obj, &x->max);
    floatinlet_new(&x->x_obj, &x->multiple);

    outlet_new(&x->x_obj, &s_float);

    return (x);
}

static void *m_wrap_free(t_m_wrap *x)
{
    return (void *)x;
}

void m_wrap_setup(void)
{
    m_wrap_class = class_new(gensym("m_wrap"),
                (t_newmethod)m_wrap_new,
                (t_method)m_wrap_free,
                sizeof(t_m_wrap),
                CLASS_DEFAULT,
                A_DEFFLOAT, // min
                A_DEFFLOAT, // max
                A_DEFFLOAT, // multiple
                0);

    class_sethelpsymbol(m_wrap_class, gensym("m_wrap"));
    class_addfloat(m_wrap_class, (t_method)m_wrap_float);
}
