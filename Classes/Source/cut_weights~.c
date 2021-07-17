// bare bones from else mtx~

#include "m_pd.h"
#include "Phase_Cut.h"

#define MIN_CUTS 1
#define MAX_CUTS 25

static t_class *cut_weights_class = NULL;

typedef struct cut_weights
{
    t_object     x_obj;
    int        n_ins;
    int        n_outs;
    int        nblock;
    int        maxblock;
    t_float  **in_vecs; // stores pointers to the inlets
    t_float  **out_vecs; // stores pointers to outlets
    t_float  **x_osums;
    int        n_cuts;
    double      *weights;
    PhaseCut   cut;
} t_cut_weights;

// prevents float messages
static void cut_weights_float(t_cut_weights *x, t_float f)
{
    pd_error(x, "cut_weights~: no method for float");
}

static t_int *cut_weights_perform(t_int *w)
{
    t_cut_weights *x = (t_cut_weights *)(w[1]); // data space
    int nblock = (int)(w[2]); // block size
    t_float **in_vecs = x->in_vecs; // inlets
    t_float *driver_in = in_vecs[0];
    t_float **out_vecs = x->out_vecs; // outlets
    t_float **osums = x->x_osums; // copied inlets??

    t_float **ivecp = in_vecs+1;
    t_float **ovecp = osums; // osum holds all zeros
    
    int out_idx = x->n_outs;
    while (out_idx--) // from top to bottom
    {
        t_float *out = *ovecp; // this is a temporary, not the final outlet we're sending to
        t_float *in = driver_in;

        int s_idx = nblock; // sample index
        while (s_idx--)
        {
            int in_idx = x->n_ins - 1;
            while(in_idx--)
            {
                t_float f = ivecp[in_idx][s_idx];
                x->weights[in_idx] = MAX(f, 0.f); // prevent negative weights
            }
            
            x->cut = cut_weights(x->n_cuts, in[s_idx], x->weights);

            if(x->cut.idx == ( (x->n_outs - 1) - out_idx) )
                out[s_idx] = x->cut.t;
            else
                out[s_idx] = 0.f;
        }
        ovecp++; // get next osum
    }

    // write to the outlets
    int idx = x->n_outs;
    while (idx--)
    {
        t_float *in = osums[idx]; // osums is the temporary that has all the information
        t_float *out = out_vecs[idx];
        int s_idx = nblock;
        while (s_idx--)
        {
            out[s_idx] = in[s_idx];
        }
    }
    return(w + 3);
}

static void cut_weights_dsp(t_cut_weights *x, t_signal **sp)
{
    int i, nblock = sp[0]->s_n;
    
    // dummy vector bs, needed for inlets and outlets
    t_float **vecp = x->in_vecs;
    t_signal **sigp = sp;
    
    for(i = 0; i < x->n_ins; i++)
        *vecp++ = (*sigp++)->s_vec;
    
    vecp = x->out_vecs;
    for(i = 0; i < x->n_outs; i++)
        *vecp++ = (*sigp++)->s_vec;
    
    // is nblock the same size?
    if(nblock != x->nblock)
    {
        if(nblock > x->maxblock)
        {
            size_t oldsize = x->maxblock * sizeof(*x->x_osums[i]);
            size_t newsize = nblock * sizeof(*x->x_osums[i]);
            
            for(i = 0; i < x->n_outs; i++)
                x->x_osums[i] = resizebytes(x->x_osums[i], oldsize, newsize);
            x->maxblock = nblock;
        }
        
        x->nblock = nblock;
    }
    
    dsp_add(cut_weights_perform, 2, x, nblock);
}

static void *cut_weights_free(t_cut_weights *x){
    if (x->in_vecs)
        freebytes(x->in_vecs, x->n_ins * sizeof(*x->in_vecs));
    if (x->out_vecs)
        freebytes(x->out_vecs, x->n_outs * sizeof(*x->out_vecs));
    if (x->x_osums)
    {
        int i;
        for (i = 0; i < x->n_outs; i++)
            freebytes(x->x_osums[i], x->maxblock * sizeof(*x->x_osums[i]));
        freebytes(x->x_osums, x->n_outs * sizeof(*x->x_osums));
    }
    if(x->weights)
        freebytes(x->weights, x->n_cuts * sizeof(double));

    return (void *)x;
}

static void *cut_weights_new(t_floatarg n_cuts)
{
    t_cut_weights *x = (t_cut_weights *)pd_new(cut_weights_class);

    if(n_cuts < MIN_CUTS)
        n_cuts = (int)MIN_CUTS;
    else if(n_cuts > MAX_CUTS)
        n_cuts = (int)MAX_CUTS;
    
    x->n_ins = (int)n_cuts + 1; // +1 b/c first is driver
    x->n_outs = x->n_cuts = (int)n_cuts;
    
    // allocate the memory for all our ptrs, aka arrays

    x->in_vecs = getbytes(x->n_ins * sizeof(*x->in_vecs));
    x->out_vecs = getbytes(x->n_outs * sizeof(*x->out_vecs));
    x->weights = getbytes(x->n_cuts * sizeof(double));
    x->nblock = x->maxblock = sys_getblksize();
    x->x_osums = getbytes(x->n_outs * sizeof(*x->x_osums));
    
    for (int i = 0; i < x->n_outs; i++)
        x->x_osums[i] = getbytes(x->maxblock * sizeof(*x->x_osums[i]));
    
    // create the inlets
    for (int i = 1; i < x->n_ins; i++)
    {
        inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    }
    
    //create the outlets
    for (int i = 0; i < x->n_outs; i++)
    {
         outlet_new(&x->x_obj, &s_signal);
    }
    
    // init weights with 1
    for(int i = 0; i < x->n_cuts; ++i)
    {
        x->weights[i] = 1.0;
    }
    
    return (x);
}

void cut_weights_tilde_setup(void){
    cut_weights_class = class_new(gensym("cut_weights~"),
                             (t_newmethod)cut_weights_new,
                             (t_method)cut_weights_free,
                             sizeof(t_cut_weights),
                             CLASS_DEFAULT,
                             A_DEFFLOAT, // n_cuts
                             0);
    
    // handles float messages, just prevents them from becoming dsp
    class_addfloat(cut_weights_class, cut_weights_float);
    
    class_sethelpsymbol(cut_weights_class, gensym("cut_weights~"));

    class_addmethod(cut_weights_class, nullfn, gensym("signal"), 0);
    class_addmethod(cut_weights_class, (t_method)cut_weights_dsp, gensym("dsp"), A_CANT, 0);
}
