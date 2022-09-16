// bare bones from else mtx~

#include "m_pd.h"
#include "Phase_Cut.h"

#define MININLETS 1
#define MAXINLETS 100
#define MINOUTLETS 1
#define MAXOUTLETS 100

static t_class *cut_mix_class = NULL;

typedef struct cut_mix
{
    t_object     x_obj;
    int        n_outs, n_cuts;
    int        nblock;
    int        maxblock;
    t_inlet  *which, *fadeRatio;
    t_float  **out_vecs; // stores pointers to outlets
    t_float  **osums; // needs a better name sticking with this for now
    PhaseCut cut;
} t_cut_mix;

// prevents float messages
static void cut_mix_float(t_cut_mix *x, t_float f)
{
    pd_error(x, "cut_mix~: no method for float");
}

static t_int *cut_mix_perform(t_int *w)
{
    t_cut_mix *x = (t_cut_mix *)(w[1]); // data space
    t_float *driver_in = (t_float*)(w[2]);
    t_float *which     = (t_float*)(w[3]);
    t_float *fadeRatio = (t_float*)(w[4]);
    int nblock = (int)(w[5]);
    t_float **out_vecs = x->out_vecs;
    t_float **osums = x->osums; // temporary to read the info from the inlets

    t_float **ovecp = osums; // osum holds all zeros

    int ondx = x->n_outs; // iterate over the outlets
    while (ondx--) // from top to bottom
    {
        t_float *in = driver_in; // get the inlet
        t_float *out = *ovecp; // this is a temporary, not the final outlet we're sending to

        int sndx = nblock; // sample index
        while (sndx--)
        {
            x->cut = cut_mix(in[sndx], x->n_cuts, which[sndx], fadeRatio[sndx]);
            if(x->cut.idx == ( (x->n_outs - 1) - ondx) )
                out[sndx] = x->cut.t;
            else
                out[sndx] = 0.f;
        }
        ovecp++; // get next osum
    }

    // write to the outlets
    int indx = x->n_outs;
    while (indx--)
    {
        t_float *in = osums[indx]; // osums is the temporary that has all the information
        t_float *out = out_vecs[indx];
        int sndx = nblock;
        while (sndx--) // s = sample index
        {
            out[sndx] = in[sndx];
        }
    }

    return(w + 6);
}

static void cut_mix_dsp(t_cut_mix *x, t_signal **sp)
{
    int i = x->n_outs, nblock = sp[0]->s_n;

    // dummy vector bs, needed for inlets and outlets
    t_float **vecp = x->out_vecs;

    for(int n = 3; n < x->n_outs + 3; ++n)
    {
        *vecp++ = sp[n]->s_vec;
    }

    // check the block size
    if(nblock != x->nblock)
    {
        if(nblock > x->maxblock)
        {
            size_t oldsize = x->maxblock * sizeof(*x->osums[i]);
            size_t newsize = nblock * sizeof(*x->osums[i]);

            for(i = 0; i < x->n_outs; i++)
                x->osums[i] = resizebytes(x->osums[i], oldsize, newsize);
            x->maxblock = nblock;
        }

        x->nblock = nblock;
    }

    dsp_add(cut_mix_perform, 5, x,
            sp[0]->s_vec, // driver_in
            sp[1]->s_vec, // which
            sp[2]->s_vec, // fadeRatio
            nblock); // block size
}


static void *cut_mix_free(t_cut_mix *x)
{
    if (x->out_vecs)
        freebytes(x->out_vecs, x->n_outs * sizeof(*x->out_vecs));

    if (x->osums)
    {
        int i;
        for (i = 0; i < x->n_outs; i++)
            freebytes(x->osums[i], x->maxblock * sizeof(*x->osums[i]));
        freebytes(x->osums, x->n_outs * sizeof(*x->osums));
    }

    return (void *)x;
}

static void *cut_mix_new(t_floatarg n_outs)
{
    t_cut_mix *x = (t_cut_mix *)pd_new(cut_mix_class);

    if(n_outs <= 0) n_outs = MINOUTLETS;
    if(n_outs > MAXOUTLETS) n_outs = MAXOUTLETS;

    x->n_outs = n_outs;
    x->n_cuts = n_outs;

    // allocate the memory for all our ptrs
    x->out_vecs = getbytes(x->n_outs * sizeof(*x->out_vecs));
    x->nblock = x->maxblock = sys_getblksize();
    x->osums = getbytes(x->n_outs * sizeof(*x->osums));

    for (int i = 0; i < x->n_outs; i++)
        x->osums[i] = getbytes(x->maxblock * sizeof(*x->osums[i]));

    // our 1 inlet is default provided
    x->which     = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->fadeRatio = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

    //create the outlets
    for (int i = 0; i < x->n_outs; i++)
    {
         outlet_new(&x->x_obj, &s_signal);
    }

    int i = x->n_outs;
    while(i--)
        x->out_vecs[i] = 0;

    return (x);
}

void cut_mix_tilde_setup(void){
    cut_mix_class = class_new(gensym("cut_mix~"),
                             (t_newmethod)cut_mix_new,
                             (t_method)cut_mix_free,
                             sizeof(t_cut_mix),
                             CLASS_DEFAULT,
                             A_DEFFLOAT,
                             0);

    // handles float messages, just prevents them from becoming dsp
    class_addfloat(cut_mix_class, cut_mix_float);

    class_sethelpsymbol(cut_mix_class, gensym("cut_mix~"));

    // first inlet to dsp
    class_addmethod(cut_mix_class, nullfn, gensym("signal"), 0);

    // declaring the dsp method
    class_addmethod(cut_mix_class, (t_method)cut_mix_dsp, gensym("dsp"), A_CANT, 0);
}
