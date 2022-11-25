#include "m_pd.h"
#include <math.h>
#include <string.h>
#include <stdlib.h> // atof
#include <ctype.h> // isdigit
#include "vec3.h"
#include "stdbool.h"

#define DEBUG

#ifdef DEBUG
    #include <time.h>
#endif

#define MAX 5000

/*
todo

potential points for opto
- graph_check_connection and finding in odds, maybe that can become one connection?
- add a function to put nodes we've connected to to the back of the array
    - test this to see if this is actually faster than what the current method is.
    - is more for loops faster or slower?

Other random thoughts:
- what happens when the number of vertices gets really high?
- read through that guy's null pointer check thing I found, and see if I should be fixing,
  the free_and_null method I found.
- Is there a faster way to check if a node is in the odds array?
    - each node could get a tag, called isodd, that would be faster to check than searching the entire array
- when meshes are not completely connected... how do I solve that problem?
    - check if the graph is connected (i.e. all nodes can be reached)
    - if it is it's all one graph
    - if NOT, then we can see which nodes we didn't visit and pick one of those.
    - check if this other graph is connected to the remaining nodes
    - if it is we're done
    - Otherwise keep repeating, each time building up a new graph.
*/

typedef struct _int_arr {
    size_t len;
    int *data;
} t_int_arr;

typedef struct _graph
{
    int n_vertices;
    t_int_arr *edges;
} t_graph;

static t_class *tsp_class;

typedef struct _tsp
{
    t_object obj;
    int max_verts;
    t_vec3 *vertices;
    t_outlet *xpts_out, *ypts_out, *zpts_out, *size_out;
} t_tsp;


/* Miscellanous Functions */
void free_and_null(void **ptr) {
    if(ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

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
        if(isdigit(*s) || *s == '-' || *s == '.' || *s == 'e') {
            s++;
            continue;
        }
        else
            return 0;
    }
    return 1;
}

/* Array Functions */
t_int_arr int_arr(size_t len) {
    return (t_int_arr){len, (int*)getbytes(len * sizeof(int)) };
}

void arr_free(t_int_arr *arr) {
    free_and_null(&arr->data);
    arr->data = NULL;
}

// print out the array
void arr_post(t_int_arr *arr) {
    for(size_t i=0; i < arr->len; ++i) {
        postfloat(arr->data[i]);
    }
    post(" length is %lu", arr->len);
}

void arr_resize(t_int_arr *arr, size_t newsize) {
    arr->len = newsize;
    arr->data = (int*)realloc(arr->data, arr->len * sizeof(int));
}

// remove an element from an array at the specified index and shift everything after over
void arr_remove(t_int_arr *arr, int idx) {
    if(arr == NULL || arr->len < 1) {
        error("[osci/tsp] arr_remove() passed a null pointer or arr has length 0");
        error("*arr = %p, arr->len = %lu", arr, arr ? arr->len : -1);
        return;
    }
    arr->len--;
    for(size_t i=idx; i < arr->len; ++i) {
        arr->data[i] = arr->data[i+1];
    }
    arr_resize(arr, arr->len);
}

// find an element in the array, returns the idx
// on fail or not found, returns -1
int arr_find(t_int_arr *arr, int elem) {
    if(arr == NULL || arr->len < 1) {
        error("[osci/tsp] null pointer or 0 length array passed to arr_find()");
        error("*arr = %p, arr->len = %lu", arr, arr ? arr->len : -1);
        return -1;
    }

    int idx = -1;

    // later see what the average number of connections are
    // if it's alot, it could potentially be useful to rethink
    // the data structure holding the list of connections
    for(size_t i=0; i < arr->len; ++i) {
        if(arr->data[i] == elem) return i;
    }

    return idx;
}

// swaps element with the last element in the array
void arr_swap_last(t_int_arr *arr, int elem) {
    int idx = arr_find(arr, elem);
    if(idx != -1 || idx == (int)arr->len-1) {
        int tmp = arr->data[arr->len-1];
        arr->data[arr->len-1] = arr->data[idx];
        arr->data[idx] = tmp;
    } else {
        error("[osci/tsp] error: arr_swap_last() element %d doesn't exist in the array", elem);
    }
}

// push element to the end of the array
void arr_push(t_int_arr *arr, int elem) {
    if(arr != NULL) {
        arr->data = (int*)realloc(arr->data, ++arr->len * sizeof(int));
        arr->data[arr->len-1] = elem;
    } else {
        error("arr_push() arr is null pointer: %p", arr);
    }
}

// remove last element and reduce size of array by one
void arr_pop(t_int_arr *arr) {
    arr->data = (int*)realloc(arr->data, --arr->len * sizeof(int));
}

/* Graph Functions */
t_graph graph(int N) { // when we build a graph, we will always know how many total verts there are
    t_graph g = (t_graph){N, (t_int_arr*)getbytes(N * sizeof(t_int_arr) )};

    for(int i = 0; i < N; ++i) {
        g.edges[i] = int_arr(0);
    }

    return g;
}

void graph_free(t_graph *g) {
    for(int i = 0; i < g->n_vertices; ++i) {
        arr_free(&g->edges[i]);
    }
    free_and_null(&g->edges);
}

void graph_post(t_graph *g) {
    for(int i=0; i < g->n_vertices; ++i) {
        post("Connections for node [%d]", i);
        for(size_t j=0; j < g->edges[i].len; ++j) {
            postfloat(g->edges[i].data[j]);
        }
        post("");
    }
}

// check for double connections, false for double or no connections, true for one
bool graph_check_connection(t_int_arr *xnet, int elem) {
    if(xnet == NULL) {
        error("graph_check_connection() xnet is null pointer: %p", xnet);
        return false;
    }

    int count = 0;

    for(size_t i=0; i < xnet->len; ++i) {
        if(xnet->data[i] == elem) {
            count++;
            //post("count = %d", count);
            if(count > 2) return false;
        }
    }

    return count == 1;
}

void add_edge(t_graph *g, int u, int v) {
    arr_push(&g->edges[u], v);
    arr_push(&g->edges[v], u);
}

void remove_edge(t_graph *g, int v, int u) {
    t_int_arr *node_v = &g->edges[v];
    t_int_arr *node_u = &g->edges[u];

    for(size_t i=0; i < node_v->len; ++i) {
        if(node_v->data[i] == u) {
            arr_swap_last(node_v, node_v->data[i]);
            arr_pop(node_v);
            break;
        }
    }

    for(size_t i=0; i < node_u->len; ++i) {
        if(node_u->data[i] == v) {
            arr_swap_last(node_u, node_u->data[i]);
            arr_pop(node_u);
            break;
        }
    }
}

int count_connected(t_graph *g, int u, bool *visited) {
    visited[u] = true;
    int count = 1;
    for(size_t i=0; i < g->edges[u].len; ++i) {
        if(!visited[ g->edges[u].data[i] ]){
            count += count_connected(g,  g->edges[u].data[i], visited);
        }
    }
    return count;
}

bool is_valid_edge(t_graph *g, int v, int u) {
    int c1 = 0, c2 = 0;
    bool visited[g->n_vertices];

    remove_edge(g, v, u);
    for(int i=0; i < g->n_vertices; ++i)
        visited[i] = false;
    c1 = count_connected(g, u, visited);

    add_edge(g, v, u);
    for(int i=0; i < g->n_vertices; ++i)
        visited[i] = false;
    c2 = count_connected(g, u, visited);

    return c1 == c2;
}

void euler_circuit(t_graph *g, t_int_arr *path, int v) {
    arr_push(path, v);
    //postfloat(v);

    if(g->edges[v].len == 0) {// no edges left
        return;
    }

    if(g->edges[v].len == 1) { // only one to choose
        int u = g->edges[v].data[0];
        remove_edge(g, v, u);
        euler_circuit(g, path, u);
        return;
    }

    // find a non-bridge edge
    for(size_t i=0; i < g->edges[v].len; ++i) {
        int u = g->edges[v].data[i];
        if( is_valid_edge(g, v, u) ) {
            remove_edge(g, v, u);
            euler_circuit(g, path, u);
            return;
        }
    }
}

void graph_sort(t_graph *g) {
    // sort the nodes connections based on their distance from each other
}

// turns all odd degree nodes in the graph into evens
void graph_fix_odds(t_graph *g) {
    // build an array containing all the odd nodes
    t_int_arr odds = int_arr(0);
    for(int i=0; i < g->n_vertices; ++i) {
        if(g->edges[i].len % 2 != 0) {
            arr_push(&odds, i);
        }
    }

    //post("members of odds");
    //arr_post(&odds);
    int n_switches = 0;

    // add a method to push elements to the back of the array once connected
    // this should speed the algorithm up (test this)
    while(odds.len > 0)
    {
        t_int_arr n = g->edges[ odds.data[0] ];
        bool no_odds = true;
        for(size_t i = 0; i < n.len; ++i) {
            int xnet = n.data[i];
            int idx = arr_find(&odds, xnet); // is the node we want to connect in odds?

            if( idx != -1 && graph_check_connection(&g->edges[xnet], odds.data[0]) ) {
                no_odds = false;
                add_edge(g, xnet, odds.data[0]);
                arr_remove(&odds, 0);
                arr_remove(&odds, idx-1); // idx has been shifted one to the left now
                break;
            }
        }

        // no odd nodes found, connecting to the first even node, we're not double connected to
        if(no_odds) {
            for(size_t i = 0; i < n.len; ++i) {
                int xnet = n.data[i];
                if( graph_check_connection(&g->edges[xnet], odds.data[0]) ) {
                    add_edge(g, xnet, odds.data[0]);
                    arr_remove(&odds, 0);
                    arr_push(&odds, xnet); // the even node is now odd, add it to odds
                }
            }
        }
        //arr_post(&odds);
    }
    arr_free(&odds);
}

void tsp_symbol(t_tsp *this, t_symbol *mesh_data)
{
    #ifdef DEBUG
        int start = clock();
    #endif

    if(mesh_data->s_name[0] != 'a') { // todo is the best way of doing this? we could also just check for null on vert, edge, and face char *
        pd_error(&this->obj, "[osci/tsp]: invalid format for mesh_data, check the symbol you are sending to [osci/tsp]");
        return;
    }

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
    int idx = 0;
    while(token != NULL) {
        token = strtok(NULL, ",{}");
        if(token == NULL || !isnum(token))
            break;
        x = atof(token);
        token = strtok(NULL, ",{}");
        y = atof(token);
        token = strtok(NULL, ",{}");
        z = atof(token);
        this->vertices[idx++] = vec3(x,y,z);
        if(idx > this->max_verts) {
            pd_error(this, "[osci/tsp] Too many vertices. The maximum # of vertices is currently set to %d", this->max_verts);
            goto err;
        }
    }

    post("vert count = %d", idx);

    // int len = i;
    // for(i = 0; i < len; ++i) {
    //     post("x: %.2f y: %.2f z: %.2f", this->vertices[i].x, this->vertices[i].y, this->vertices[i].z);
    // }

    //build the graph
    t_graph g = graph(idx);
    token = strtok(edges, "[");
    int u, v;
    while(token != NULL) {
        token = strtok(NULL, " {}");
        if(token == NULL || !isnum(token))
            break;
        u = atof(token);
        token = strtok(NULL, " {}");
        v = atof(token);
        //post("v1: %d, v2: %d", u, v);
        add_edge(&g, u, v);
    }

    // check to see if the graph is connected
        // might want to refactor count_connected
        // or write a similarly named function?

    // post("initial graph ===");
    // post("=================");
    // graph_post(&g);

    graph_sort(&g);

    // post("===\n===\n===\n===\n====\n");
    graph_fix_odds(&g);

    // post("fixed graph result ===");
    // post("======================");
    // graph_post(&g);

    // run fleury's algorithm and build the path
    t_int_arr path = int_arr(0);
    euler_circuit(&g, &path, 0);

    post("path length is %d", path.len);

    // take that path and build up the x, y, and z arrays
    t_atom *xpts = (t_atom*)getbytes(path.len * sizeof(t_atom));
    t_atom *ypts = (t_atom*)getbytes(path.len * sizeof(t_atom));
    t_atom *zpts = (t_atom*)getbytes(path.len * sizeof(t_atom));

    for(size_t i=0; i < path.len; ++i) {
        SETFLOAT(xpts+i, this->vertices[path.data[i]].x);
        SETFLOAT(ypts+i, this->vertices[path.data[i]].y);
        SETFLOAT(zpts+i, this->vertices[path.data[i]].z);
    }

    // return the arrays, making sure to output the size of the path first
    outlet_float(this->size_out, path.len);
    outlet_list(this->xpts_out, &s_list, path.len, xpts);
    outlet_list(this->ypts_out, &s_list, path.len, ypts);
    outlet_list(this->zpts_out, &s_list, path.len, zpts);

    arr_free(&path);
    graph_free(&g);
    free_and_null(&xpts);
    free_and_null(&ypts);
    free_and_null(&zpts);

    #ifdef DEBUG
        int end = clock();
        int ticks = end-start;
        post("\nelapsed ticks: %d\nelapsed time: %.6f ms", ticks, ((float)ticks/CLOCKS_PER_SEC)*1000 );
    #endif

    err: return;
}

static void tsp_set_max(t_tsp *this, t_floatarg max) {
    this->max_verts = (max <= 0) ? MAX :
                      (max > MAX) ? MAX : (int)max;
    this->vertices = (t_vec3*)realloc(this->vertices, this->max_verts * sizeof(t_vec3));
    post("max_verts set to %d", this->max_verts);
}

static void *tsp_new(t_floatarg max)
{
    t_tsp *this = (t_tsp *)pd_new(tsp_class);

    this->max_verts = (max <= 0) ? 2000 :
                      (max > MAX) ? MAX : (int)max;

    this->vertices = (t_vec3*)getbytes(this->max_verts * sizeof(t_vec3));

    this->xpts_out = outlet_new(&this->obj, &s_list);
    this->ypts_out = outlet_new(&this->obj, &s_list);
    this->zpts_out = outlet_new(&this->obj, &s_list);
    this->size_out = outlet_new(&this->obj, &s_float);

    return this;
}

static void *tsp_free(t_tsp *this) {
    free_and_null(&this->vertices);
    return (void *)this;
}

void tsp_setup(void)
{
    tsp_class = class_new(gensym("tsp"),
                (t_newmethod)tsp_new,
                (t_method)tsp_free,
                sizeof(t_tsp),
                CLASS_DEFAULT,
                A_DEFFLOAT, // max_verts
                0);

    class_sethelpsymbol(tsp_class, gensym("tsp"));
    class_addmethod(tsp_class, (t_method)tsp_set_max, gensym("max"), A_DEFFLOAT);

    class_addsymbol(tsp_class, (t_method)tsp_symbol);
}
