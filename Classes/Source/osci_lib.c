/*
This will necessary in order to load up non alphanumeric objects
- Namely the binary operators I would like to add to remove dependencies
- This is also the place to load up the phase mapping stuff, as the order really matters there
- this could also result in the master header file?
- actually this allows me to print to the console, versioning and other important info
- if people wanted this in camomile, having this be the only binary would also be pretty awesome
*/

#include "m_pd.h"
#include "math.h"

#define FLT_EPSILON 1.19209290E-07F

int isequals(t_float a, t_float b, t_float rel_epsilon)
{
    // Check if the numbers are really close -- needed when comparing numbers near zero.
    t_float diff = fabsf(a - b);
    if (diff <= FLT_EPSILON) return 1;
 
    // Otherwise fall back to Knuth's algorithm
    return (diff <= (fmaxf(fabs(a), fabs(b)) * rel_epsilon));
}

//==================== [==~] ====================//

static t_class *sig_equals_class;

typedef struct _sig_equals
{
    t_object x_obj;
    t_float f; // dummy
    t_inlet *in;
} t_sig_equals;

static t_int *sig_equals_perform(t_int *w)
{
    int nblock   =       (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--) 
    {
        out[nblock] = isequals(in1[nblock], in2[nblock], 1e-8);
    }
    return (w + 5);
}

static void sig_equals_dsp(t_sig_equals *x, t_signal **sp)
{
	dsp_add(sig_equals_perform, 4, 
        sp[0]->s_n, // nblock
		sp[0]->s_vec, // in1
        sp[1]->s_vec, // in2
        sp[2]->s_vec); // out
}

static void *sig_equals_free(t_sig_equals *x)
{
    inlet_free(x->in);
    return (void *)x;
}

static void *sig_equals_new(t_floatarg f)
{
    t_sig_equals *x = (t_sig_equals *)pd_new(sig_equals_class);
    x->in = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->in, f);
    outlet_new((t_object *)x, &s_signal);
    return (void*)x;
}

void sig_equals_setup(void)
{
    sig_equals_class = class_new( gensym("==~"),
                                (t_newmethod)sig_equals_new,
                                (t_method)sig_equals_free, // no dtor
                                sizeof(t_sig_equals),
                                CLASS_DEFAULT, // gui appearance
                                A_DEFFLOAT,
                                0); // no args

    class_sethelpsymbol(sig_equals_class, gensym("binops~"));
    
    class_addmethod(sig_equals_class, (t_method)sig_equals_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(sig_equals_class, t_sig_equals, f); // dummy arg for singal into first inlet
}

//==================== [!=~] ====================//

static t_class *sig_notequals_class;

typedef struct _sig_notequals
{
    t_object x_obj;
    t_float f; // dummy
    t_inlet *in;
} t_sig_notequals;

static t_int *sig_notequals_perform(t_int *w)
{
    int nblock   =       (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--) 
    {
        out[nblock] = !isequals(in1[nblock], in2[nblock], 1e-8);
    }
    return (w + 5);
}

static void sig_notequals_dsp(t_sig_notequals *x, t_signal **sp)
{
	dsp_add(sig_notequals_perform, 4, 
        sp[0]->s_n, // nblock
		sp[0]->s_vec, // in1
        sp[1]->s_vec, // in2
        sp[2]->s_vec); // out
}

static void *sig_notequals_free(t_sig_notequals *x)
{
    inlet_free(x->in);
    return (void *)x;
}

static void *sig_notequals_new(t_floatarg f)
{
    t_sig_notequals *x = (t_sig_notequals *)pd_new(sig_notequals_class);
    x->in = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->in, f);
    outlet_new((t_object *)x, &s_signal);
    return (void*)x;
}

void sig_notequals_setup(void)
{
    sig_notequals_class = class_new( gensym("!=~"),
                                (t_newmethod)sig_notequals_new,
                                (t_method)sig_notequals_free, // no dtor
                                sizeof(t_sig_notequals),
                                CLASS_DEFAULT, // gui appearance
                                A_DEFFLOAT,
                                0); // no args

    class_sethelpsymbol(sig_notequals_class, gensym("binops~"));
    
    class_addmethod(sig_notequals_class, (t_method)sig_notequals_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(sig_notequals_class, t_sig_notequals, f); // dummy arg for singal into first inlet
}

//==================== [<~] ====================//

static t_class *sig_less_class;

typedef struct _sig_less
{
    t_object x_obj;
    t_float f; // dummy
    t_inlet *in;
} t_sig_less;

static t_int *sig_less_perform(t_int *w)
{
    int nblock   =       (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--) 
    {
        out[nblock] = isless(in1[nblock], in2[nblock]);
    }
    return (w + 5);
}

static void sig_less_dsp(t_sig_less *x, t_signal **sp)
{
	dsp_add(sig_less_perform, 4, 
        sp[0]->s_n, // nblock
		sp[0]->s_vec, // in1
        sp[1]->s_vec, // in2
        sp[2]->s_vec); // out
}

static void *sig_less_free(t_sig_less *x)
{
    inlet_free(x->in);
    return (void *)x;
}

static void *sig_less_new(t_floatarg f)
{
    t_sig_less *x = (t_sig_less *)pd_new(sig_less_class);
    x->in = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->in, f);
    outlet_new((t_object *)x, &s_signal);
    return (void*)x;
}

void sig_less_setup(void)
{
    sig_less_class = class_new( gensym("<~"),
                                (t_newmethod)sig_less_new,
                                (t_method)sig_less_free, // no dtor
                                sizeof(t_sig_less),
                                CLASS_DEFAULT, // gui appearance
                                A_DEFFLOAT,
                                0); // no args

    class_sethelpsymbol(sig_less_class, gensym("binops~"));
    
    class_addmethod(sig_less_class, (t_method)sig_less_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(sig_less_class, t_sig_less, f); // dummy arg for singal into first inlet
}

//==================== [>~] ====================//

static t_class *sig_greater_class;

typedef struct _sig_greater
{
    t_object x_obj;
    t_float f; // dummy
    t_inlet *in;
} t_sig_greater;

static t_int *sig_greater_perform(t_int *w)
{
    int nblock   =       (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--) 
    {
        out[nblock] = isgreater(in1[nblock], in2[nblock]);
    }
    return (w + 5);
}

static void sig_greater_dsp(t_sig_greater *x, t_signal **sp)
{
	dsp_add(sig_greater_perform, 4, 
        sp[0]->s_n, // nblock
		sp[0]->s_vec, // in1
        sp[1]->s_vec, // in2
        sp[2]->s_vec); // out
}

static void *sig_greater_free(t_sig_greater *x)
{
    inlet_free(x->in);
    return (void *)x;
}

static void *sig_greater_new(t_floatarg f)
{
    t_sig_greater *x = (t_sig_greater *)pd_new(sig_greater_class);
    x->in = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->in, f);
    outlet_new((t_object *)x, &s_signal);
    return (void*)x;
}

void sig_greater_setup(void)
{
    sig_greater_class = class_new( gensym(">~"),
                                (t_newmethod)sig_greater_new,
                                (t_method)sig_greater_free, // no dtor
                                sizeof(t_sig_greater),
                                CLASS_DEFAULT, // gui appearance
                                A_DEFFLOAT,
                                0); // no args

    class_sethelpsymbol(sig_greater_class, gensym("binops~"));
    
    class_addmethod(sig_greater_class, (t_method)sig_greater_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(sig_greater_class, t_sig_greater, f); // dummy arg for singal into first inlet
}

//==================== [<=~] ====================//

static t_class *sig_lessequals_class;

typedef struct _sig_lessequals
{
    t_object x_obj;
    t_float f; // dummy
    t_inlet *in;
} t_sig_lessequals;

static t_int *sig_lessequals_perform(t_int *w)
{
    int nblock   =       (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--) 
    {
        out[nblock] = islessequal(in1[nblock], in2[nblock]);
    }
    return (w + 5);
}

static void sig_lessequals_dsp(t_sig_lessequals *x, t_signal **sp)
{
	dsp_add(sig_lessequals_perform, 4, 
        sp[0]->s_n, // nblock
		sp[0]->s_vec, // in1
        sp[1]->s_vec, // in2
        sp[2]->s_vec); // out
}

static void *sig_lessequals_free(t_sig_lessequals *x)
{
    inlet_free(x->in);
    return (void *)x;
}

static void *sig_lessequals_new(t_floatarg f)
{
    t_sig_lessequals *x = (t_sig_lessequals *)pd_new(sig_lessequals_class);
    x->in = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->in, f);
    outlet_new((t_object *)x, &s_signal);
    return (void*)x;
}

void sig_lessequals_setup(void)
{
    sig_lessequals_class = class_new( gensym("<=~"),
                                (t_newmethod)sig_lessequals_new,
                                (t_method)sig_lessequals_free, // no dtor
                                sizeof(t_sig_lessequals),
                                CLASS_DEFAULT, // gui appearance
                                A_DEFFLOAT,
                                0); // no args

    class_sethelpsymbol(sig_lessequals_class, gensym("binops~"));
    
    class_addmethod(sig_lessequals_class, (t_method)sig_lessequals_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(sig_lessequals_class, t_sig_lessequals, f); // dummy arg for singal into first inlet
}

//==================== [>=~] ====================//

static t_class *sig_greaterequals_class;

typedef struct _sig_greaterequals
{
    t_object x_obj;
    t_float f; // dummy
    t_inlet *in;
} t_sig_greaterequals;

static t_int *sig_greaterequals_perform(t_int *w)
{
    int nblock   =       (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--) 
    {
        out[nblock] = isgreaterequal(in1[nblock], in2[nblock]);
    }
    return (w + 5);
}

static void sig_greaterequals_dsp(t_sig_greaterequals *x, t_signal **sp)
{
	dsp_add(sig_greaterequals_perform, 4, 
        sp[0]->s_n, // nblock
		sp[0]->s_vec, // in1
        sp[1]->s_vec, // in2
        sp[2]->s_vec); // out
}

static void *sig_greaterequals_free(t_sig_greaterequals *x)
{
    inlet_free(x->in);
    return (void *)x;
}

static void *sig_greaterequals_new(t_floatarg f)
{
    t_sig_greaterequals *x = (t_sig_greaterequals *)pd_new(sig_greaterequals_class);
    x->in = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->in, f);
    outlet_new((t_object *)x, &s_signal);
    return (void*)x;
}

void sig_greaterequals_setup(void)
{
    sig_greaterequals_class = class_new( gensym(">=~"),
                                (t_newmethod)sig_greaterequals_new,
                                (t_method)sig_greaterequals_free, // no dtor
                                sizeof(t_sig_greaterequals),
                                CLASS_DEFAULT, // gui appearance
                                A_DEFFLOAT,
                                0); // no args

    class_sethelpsymbol(sig_greaterequals_class, gensym("binops~"));
    
    class_addmethod(sig_greaterequals_class, (t_method)sig_greaterequals_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(sig_greaterequals_class, t_sig_greaterequals, f); // dummy arg for singal into first inlet
}

//==================== [||~] ====================//

static t_class *sig_or_class;

typedef struct _sig_or
{
    t_object x_obj;
    t_float f; // dummy
    t_inlet *in;
} t_sig_or;

static t_int *sig_or_perform(t_int *w)
{
    int nblock   =       (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--) 
    {
        out[nblock] = isgreater(fabs(in1[nblock]), 0.f) || isgreater(fabs(in2[nblock]), 0.f);
    }
    return (w + 5);
}

static void sig_or_dsp(t_sig_or *x, t_signal **sp)
{
	dsp_add(sig_or_perform, 4, 
        sp[0]->s_n, // nblock
		sp[0]->s_vec, // in1
        sp[1]->s_vec, // in2
        sp[2]->s_vec); // out
}

static void *sig_or_free(t_sig_or *x)
{
    inlet_free(x->in);
    return (void *)x;
}

static void *sig_or_new(t_floatarg f)
{
    t_sig_or *x = (t_sig_or *)pd_new(sig_or_class);
    x->in = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->in, f);
    outlet_new((t_object *)x, &s_signal);
    return (void*)x;
}

void sig_or_setup(void)
{
    sig_or_class = class_new( gensym("||~"),
                                (t_newmethod)sig_or_new,
                                (t_method)sig_or_free, // no dtor
                                sizeof(t_sig_or),
                                CLASS_DEFAULT, // gui appearance
                                A_DEFFLOAT,
                                0); // no args

    class_sethelpsymbol(sig_or_class, gensym("binops~"));
    
    class_addmethod(sig_or_class, (t_method)sig_or_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(sig_or_class, t_sig_or, f); // dummy arg for singal into first inlet
}

//==================== [&&~] ====================//

static t_class *sig_and_class;

typedef struct _sig_and
{
    t_object x_obj;
    t_float f; // dummy
    t_inlet *in;
} t_sig_and;

static t_int *sig_and_perform(t_int *w)
{
    int nblock   =       (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--) 
    {
        out[nblock] = isgreater(fabs(in1[nblock]), 0.f) && isgreater(fabs(in2[nblock]), 0.f);
    }
    return (w + 5);
}

static void sig_and_dsp(t_sig_and *x, t_signal **sp)
{
	dsp_add(sig_and_perform, 4, 
        sp[0]->s_n, // nblock
		sp[0]->s_vec, // in1
        sp[1]->s_vec, // in2
        sp[2]->s_vec); // out
}

static void *sig_and_free(t_sig_and *x)
{
    inlet_free(x->in);
    return (void *)x;
}

static void *sig_and_new(t_floatarg f)
{
    t_sig_and *x = (t_sig_and *)pd_new(sig_and_class);
    x->in = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->in, f);
    outlet_new((t_object *)x, &s_signal);
    return (void*)x;
}

void sig_and_setup(void)
{
    sig_and_class = class_new( gensym("&&~"),
                                (t_newmethod)sig_and_new,
                                (t_method)sig_and_free, // no dtor
                                sizeof(t_sig_and),
                                CLASS_DEFAULT, // gui appearance
                                A_DEFFLOAT,
                                0); // no args

    class_sethelpsymbol(sig_and_class, gensym("binops~"));
    
    class_addmethod(sig_and_class, (t_method)sig_and_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(sig_and_class, t_sig_and, f); // dummy arg for singal into first inlet
}

//==================== [&&~] ====================//

static t_class *sig_modulo_class;

typedef struct _sig_modulo
{
    t_object x_obj;
    t_float f; // dummy
    t_inlet *in;
} t_sig_modulo;

static t_int *sig_modulo_perform(t_int *w)
{
    int nblock   =       (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--) 
    {
        t_float f = fmodf(in1[nblock], in2[nblock]);
        out[nblock] =  isnan(f) ? 0.f : f; 
    }
    return (w + 5);
}

static void sig_modulo_dsp(t_sig_modulo *x, t_signal **sp)
{
	dsp_add(sig_modulo_perform, 4, 
        sp[0]->s_n, // nblock
		sp[0]->s_vec, // in1
        sp[1]->s_vec, // in2
        sp[2]->s_vec); // out
}

static void *sig_modulo_free(t_sig_modulo *x)
{
    inlet_free(x->in);
    return (void *)x;
}

static void *sig_modulo_new(t_floatarg f)
{
    t_sig_modulo *x = (t_sig_modulo *)pd_new(sig_modulo_class);
    x->in = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->in, f);
    outlet_new((t_object *)x, &s_signal);
    return (void*)x;
}

void sig_modulo_setup(void)
{
    sig_modulo_class = class_new( gensym("%~"),
                                (t_newmethod)sig_modulo_new,
                                (t_method)sig_modulo_free, // no dtor
                                sizeof(t_sig_modulo),
                                CLASS_DEFAULT, // gui appearance
                                A_DEFFLOAT,
                                0); // no args

    class_addcreator((t_newmethod)sig_modulo_new,
                      gensym("mod~"),
                      A_DEFFLOAT,
                      0); // no args

    class_sethelpsymbol(sig_modulo_class, gensym("binops~"));
    
    class_addmethod(sig_modulo_class, (t_method)sig_modulo_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(sig_modulo_class, t_sig_modulo, f); // dummy arg for singal into first inlet
}

//==================== [osci] ====================//

static t_class *osci_class; // to static or not to static, that is the question
typedef struct _osci
{
    t_object x_obj;
} t_osci;

static int printed = 0;

void osci_about(t_osci* x)
{
    //int major = 0, minor = 0, bugfix = 0;
    //sys_getversion(&major, &minor, &bugfix);
    post("");
    post("|---------------------------------------------------------|");
    post("| OSCI - Pure Data externals for oscilloscope music & art  ");
    post("|---------------------------------------------------------|");
    post("| Author: Eric Lennartson                                  ");
    post("| Version: 0.1.0 Released January 26th 2021                ");
    post("| Repository: https://github.com/Eric-Lennartson/pd-osci   ");
    post("| License: GPL-3.0                                         ");
    post("| Build Date: January 26th 2021                            ");
    //if(major >= min_major && minor >= min_minor && bugfix >= min_bugfix)
    //     post("- OSCI 1.0.-0 %s-%d needs at least Pd %d.%d-%d (you have %d.%d-%d, you're good!)",
    //          STATUS, status_number, min_major, min_minor, min_bugfix, major, minor, bugfix);
    // else
    //     pd_error(x, "- OSCI 1.0-0 %s-%d needs at least Pd %d.%d-%d (you have %d.%d-%d, please upgrade!)",
    //         STATUS, status_number, min_major, min_minor, min_bugfix, major, minor, bugfix);
    post("|---------------------------------------------------------|");
    post("");
}

static void* osci_new(void)
{
    t_osci *x = (t_osci*)pd_new(osci_class);

    if(!printed) 
    {
        osci_about(x);
        printed = 1;
    }
    return(x);
}

void osci_setup(void)
{
    osci_class = class_new( gensym("osci"),
                            (t_newmethod)osci_new,
                            NULL, // no dtor
                            sizeof(t_osci),
                            CLASS_NOINLET, // gui appearance
                            0); // no args

    t_osci *x = (t_osci*)pd_new(osci_class);
    if(!printed)
    {
        osci_about(x);
        printed = 1;
    }

    sig_equals_setup();
    sig_notequals_setup();
    sig_less_setup();
    sig_greater_setup();
    sig_lessequals_setup();
    sig_greaterequals_setup();
    sig_or_setup();
    sig_and_setup();
    sig_modulo_setup();
}
