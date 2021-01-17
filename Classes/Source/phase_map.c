#include "m_pd.h"
#include "Phase_Map.h"

static t_class *phase_map_class;

typedef struct _phase_map
{
    t_object x_obj;
    t_pmap map;
    t_symbol* name;
    int size;
} t_phase_map;

// FREE
static void *phase_map_free(t_phase_map *x)
{   
    free_pmap(&x->map);
    return (void *)x;
}

static void *phase_map_new(t_symbol* name, t_floatarg size)
{
    t_phase_map *x = (t_phase_map *)pd_new(phase_map_class);
    
    x->name = name;
    x->size = (size > 0) ? (int)size : 1;

    x->map = pmap(size);

    return (x);
}

void phase_map_setup(void)
{
    phase_map_class = class_new(gensym("phase_map"),
                            (t_newmethod)phase_map_new, //ctor
                            (t_method)phase_map_free, //dtor
                            sizeof(t_phase_map), // data space
                            CLASS_NOINLET, // supress the default inlet
                            A_DEFSYMBOL, // name
                            A_DEFFLOAT, // size
                            0); // no more args
    
    class_sethelpsymbol(phase_map_class, gensym("phase_map")); // links to the help patch
}
