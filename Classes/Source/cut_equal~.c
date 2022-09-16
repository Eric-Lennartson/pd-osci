// bare bones from else mtx~

#include "m_pd.h"
#include "Phase_Cut.h"

#define MININLETS 1
#define MAXINLETS 100
#define MINOUTLETS 1
#define MAXOUTLETS 100

static t_class *cut_equal_class = NULL;

typedef struct cut_equal
{
    t_object     x_obj;
    int        n_outs;
    int        nblock;
    int        maxblock;
    t_float  **out_vecs; // stores pointers to outlets
    t_float  **osums; // needs a better name sticking with this for now
    PhaseCut cut;
} t_cut_equal;

// prevents float messages
static void cut_equal_float(t_cut_equal *x, t_float f)
{
    pd_error(x, "cut_equal~: no method for float");
}


static t_int *cut_equal_perform(t_int *w)
{
    t_cut_equal *x = (t_cut_equal *)(w[1]); // data space
    t_float *driver_in = (t_float*)(w[2]);
    int nblock = (int)(w[3]); // block size
    t_float **out_vecs = x->out_vecs; // outlets
    t_float **osums = x->osums; // copied inlets??

    /*
     For each inlet we iterate over all the outlets
     If that cell is on, then write the data in the inlet to a temporary.
     This way we can read all the inlets BEFORE we write to the outlets.
     Pd will overwrite the data to optimize the dsp tree, and this creates problems.
     we have to read everything before we write.
     */

     t_float **ovecp = osums; // osum holds all zeros

    int ondx = x->n_outs; // iterate over the outlets
    while (ondx--) // from top to bottom
    {
        t_float *in = driver_in; // get the inlet
        t_float *out = *ovecp; // this is a temporary, not the final outlet we're sending to

        int sndx = nblock; // sample index
            while (sndx--)
            {
                x->cut = cut_equal(in[sndx], x->n_outs);
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

    return(w + 4);
}

static void cut_equal_dsp(t_cut_equal *x, t_signal **sp)
{
    int i = x->n_outs, nblock = sp[0]->s_n;

    // dummy vector bs, needed for inlets and outlets
    t_float **vecp = x->out_vecs;

    while(i--)
        *vecp++ = sp[x->n_outs - i]->s_vec;


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

    dsp_add(cut_equal_perform, 3, x,
            sp[0]->s_vec, // driver_in
            nblock); // block size
}




static void *cut_equal_free(t_cut_equal *x)
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

static void *cut_equal_new(t_floatarg n_outs)
{
    t_cut_equal *x = (t_cut_equal *)pd_new(cut_equal_class);

    if(n_outs <= 0) n_outs = MINOUTLETS;
    if(n_outs > MAXOUTLETS) n_outs = MAXOUTLETS;

    x->n_outs = n_outs;

    // allocate the memory for all our ptrs
    x->out_vecs = getbytes(x->n_outs * sizeof(*x->out_vecs));
    x->nblock = x->maxblock = sys_getblksize();
    x->osums = getbytes(x->n_outs * sizeof(*x->osums));

    for (int i = 0; i < x->n_outs; i++)
        x->osums[i] = getbytes(x->maxblock * sizeof(*x->osums[i]));


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

void cut_equal_tilde_setup(void){
    cut_equal_class = class_new(gensym("cut_equal~"),
                             (t_newmethod)cut_equal_new,
                             (t_method)cut_equal_free,
                             sizeof(t_cut_equal),
                             CLASS_DEFAULT,
                             A_DEFFLOAT,
                             0);

    // handles float messages, just prevents them from becoming dsp
    class_addfloat(cut_equal_class, cut_equal_float);

    class_sethelpsymbol(cut_equal_class, gensym("cut_equal~"));

    // first inlet to dsp
    class_addmethod(cut_equal_class, nullfn, gensym("signal"), 0);

    // declaring the dsp method
    class_addmethod(cut_equal_class, (t_method)cut_equal_dsp, gensym("dsp"), A_CANT, 0);
}
