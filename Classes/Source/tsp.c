#include "m_pd.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "vec3.h"
#include <ctype.h> // isnumber

// these are global up here for now, but will later be moved to be members of t_tsp
// The final change will be to integrate an array library
t_vec3 vertices[2000];

// reverse strstr
char *rev_strstr(const char *s, const char *m)
{
    char *ptr = s + strlen(s);
    size_t mlen = strlen(m);
    while(ptr-- != s) { // until ptr reaches the beginning of s
        if(*ptr == *m) // single char match first
            if(strncmp(ptr, m, mlen) == 0) // check the remainder
                return ptr;
    }
    return NULL;
}

// this has issues, BUT this should work for the specific use case I have for it.
// assumptions I've made:
// strings don't have e in them, . only occurs once,
int isnum(const char *s) {
    while(*s != '\0') {
        if(isdigit(*s) || *s == '-' || *s == '.') {
            s++;
            continue;
        }
        else
            return 0;
    }
    return 1;
}

static t_class *tsp_class;

typedef struct _tsp
{
    t_object obj;
    t_symbol *mesh_data;
    t_outlet *xpts_out, *ypts_out, *zpts_out, *size_out;
} t_tsp;

/*
todo

have a good think about what sort of data structure makes sense for the graph, I'd like to avoid the craziness of the js code, it is ugly and hard to read.

Other random thoughts:
- what happens when the number of vertices gets really high?
- Is there a faster way to check if a node is in the odds array?
- I need to check the js code, what happens for meshes that are disconnected?
- how do I solve that particular problem?
*/

void tsp_symbol(t_tsp *this, t_symbol *mesh_data)
{
    // post("calling tsp_symbol method\n");

    // read in the data, chopping off all the earlier bits
    char *tmp_v = rev_strstr(mesh_data->s_name, "vert");
    char *tmp_e = rev_strstr(mesh_data->s_name, "edge");
    char *faces = rev_strstr(mesh_data->s_name, "face");

    int verts_len = (int)(tmp_e-2-tmp_v);
    int edges_len = (int)(faces-2-tmp_e);

    char verts[verts_len];
    char edges[edges_len];

    memset(verts, '\0', sizeof(verts));
    memset(edges, '\0', sizeof(edges));

    strncpy(verts, tmp_v, verts_len);
    strncpy(edges, tmp_e, edges_len);

    // post("verts string: %s", verts);
    // post("edges string: %s", edges);

    char *token = strtok(verts, "[");

    //get the vertices
    t_float x, y, z;
    int i = 0; // later change all arrays to be something from a library
    while(token != NULL) {
        token = strtok(NULL, ",{}");
        if(token == NULL || !isnum(token))
            break;
        x = atof(token);
        token = strtok(NULL, ",{}");
        y = atof(token);
        token = strtok(NULL, ",{}");
        z = atof(token);
        vertices[i++] = vec3(x,y,z);
    }

    int len = i;
    for(i = 0; i < len; ++i) {
        post("x: %.2f y: %.2f z: %.2f", vertices[i].x, vertices[i].y, vertices[i].z);
    }

    // get the connections
    token = strtok(edges, "[");
    int v1, v2;
    while(token != NULL) {
        token = strtok(NULL, " {}");
        if(token == NULL || !isnum(token))
            break;
        v1 = atof(token);
        token = strtok(NULL, " {}");
        v2 = atof(token);
        post("v1: %d, v2: %d", v1, v2);
    }

    // sort nodes connections by distance
    // build an array containing all the odd nodes
    // turn all odd nodes into even nodes
    // build the graph (remove this step and have the graph already be built at this point)
    // run fleury's algorithm and build the path
    // take that path and build up the x, y, and z arrays
    // return the arrays, making sure to output the size of the path first

    //outlet_list()
}

static void *tsp_new()
{
    t_tsp *this = (t_tsp *)pd_new(tsp_class);

    this->mesh_data = NULL;

    this->xpts_out = outlet_new(&this->obj, &s_list);
    this->ypts_out = outlet_new(&this->obj, &s_list);
    this->zpts_out = outlet_new(&this->obj, &s_list);
    this->size_out = outlet_new(&this->obj, &s_float);

    return this;
}

static void *tsp_free(t_tsp *this) {
    return (void *)this;
}

void tsp_setup(void)
{
    tsp_class = class_new(gensym("tsp"),
                (t_newmethod)tsp_new,
                (t_method)tsp_free,
                sizeof(t_tsp),
                CLASS_DEFAULT,
                0);

    class_sethelpsymbol(tsp_class, gensym("tsp"));

    class_addsymbol(tsp_class, (t_method)tsp_symbol);
}
