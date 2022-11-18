#include "m_pd.h"
#include <math.h>
#include <string.h>
#include <stdlib.h> // atof
#include <ctype.h> // isdigit
#include "vec3.h"
#include "stdbool.h"

/*
todo

Other random thoughts:
- what happens when the number of vertices gets really high?
- Is there a faster way to check if a node is in the odds array?
- I need to check the js code, what happens for meshes that are disconnected?
- how do I solve that particular problem?
*/

// these are global up here for now, but will later be moved to be members of t_tsp
// The final change will be to integrate an array library
t_vec3 vertices[2000];

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
    t_graph g;
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
        if(isdigit(*s) || *s == '-' || *s == '.') {
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

// find an element in the array, returns the idx
// if not found, returns -1
int arr_find(t_int_arr *arr, int elem) {
    int idx = -1;

    // later see what the average number of connections are
    // if it's alot, it could potentially be useful to rethink
    // the data structure holding the list of connections
    for(int i=0; i < arr->len; ++i) {
        if(arr->data[i] == elem) return i;
    }

    return idx;
}

void arr_swap(t_int_arr *arr, int a, int b) {
    int idx_a = arr_find(arr, a);
    int idx_b = arr_find(arr, b);
    if(idx_a != -1 && idx_b != -1) {
        int tmp = arr->data[idx_b];
        arr->data[idx_b] = arr->data[idx_a];
        arr->data[idx_a] = tmp;
    } else {
        error("[osci/tsp] error: arr_swap() at least one element doesn't exist in the array");
        error("idx_a: %d idx_b: %d", idx_a, idx_b);
    }
}

// push element to the end of the array
void arr_push(t_int_arr *arr, int elem) {
    arr->data = (int*)realloc(arr->data, ++arr->len * sizeof(int));
    arr->data[arr->len-1] = elem;
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

void add_edge(t_graph *g, int u, int v) {
    arr_push(&g->edges[u], v);
    arr_push(&g->edges[v], u);
}

void remove_edge(t_graph *g, int v, int u) {
    t_int_arr *node_v = &g->edges[v];
    t_int_arr *node_u = &g->edges[u];

    for(int i=0; i < node_v->len; ++i) {
        if(node_v->data[i] == u) {
            arr_swap(node_v, node_v->data[i], node_v->data[node_v->len-1]);
            arr_pop(node_v);
            break;
        }
    }

    for(int i=0; i < node_u->len; ++i) {
        if(node_u->data[i] == v) {
            arr_swap(node_u, node_u->data[i], node_u->data[node_u->len-1]);
            arr_pop(node_u);
            break;
        }
    }
}

int count_connected(t_graph *g, int u, bool *visited) {
    visited[u] = true;
    int count = 1;
    for(int i=0; i < g->edges[u].len; ++i) {
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

void euler_circuit(t_graph *g, int v) {
    postfloat(v);

    if(g->edges[v].len == 0) // no edges left
        return;

    if(g->edges[v].len == 1) { // only one to choose
        int u = g->edges[v].data[0];
        remove_edge(g, v, u);
        euler_circuit(g, u);
        return;
    }

    // find a non-bridge edge
    for(int i=0; i < g->edges[v].len; ++i) {
        if( is_valid_edge(g, v, g->edges[v].data[i]) ) {
            remove_edge(g, v, g->edges[v].data[i]);
            euler_circuit(g, g->edges[v].data[i]);
            return;
        }
    }
}

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

    t_graph g = graph(7);

	add_edge(&g, 0, 1);
	add_edge(&g, 0, 2);
	add_edge(&g, 0, 3);
	add_edge(&g, 0, 5);
	add_edge(&g, 1, 2);
	add_edge(&g, 1, 4);
	add_edge(&g, 1, 6);
	add_edge(&g, 4, 5);
	add_edge(&g, 5, 6);

    euler_circuit(&g, 0);

    graph_free(&g);

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
