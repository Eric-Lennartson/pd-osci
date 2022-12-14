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

it works... mostly, now for optomizing and crash reducing
    - go through and get all the freeing methods worked out
    - make sure that every instance of a pointer is checked for null
        - make sure that a good option for when null happens is implemented

potential points for opto
- when completely finished check to see if the speed is good enough for me.
- graph_check_connection and finding in odds, maybe that can become one connection?
- add a function to put nodes we've connected to to the back of the array
    - test this to see if this is actually faster than what the current method is.
    - is more for loops faster or slower?

Other random thoughts:
- what happens when the number of vertices gets really high?
- read through that guy's null pointer check thing I found, and see if I should be fixing, the free_and_null method I found.
- Is there a faster way to check if a node is in the odds array?
    - each node could get a tag, called isodd, that would be faster to check than searching the entire array

After opto and freeing of arrays is all fixed up
    - implement a save feature where pd remembers the data for the graph

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

typedef struct _graph_arr {
    size_t len;
    t_graph *graphs;
} t_graph_arr;

static t_class *tsp_class;

typedef struct _tsp
{
    t_object obj;
    int max_verts;
    t_vec3 *vertices;
    t_outlet *xpts_out, *ypts_out, *zpts_out, *interp_out, *size_out;
} t_tsp;


/* Miscellanous Functions */
void free_and_null(void **ptr) {
    if(ptr != NULL && *ptr != NULL) {
        free(*ptr);
        *ptr = NULL;
    }
}

// count occurrences of a substring in a string
int count_substring(const char *s, char *sub) {
    int found, count = 0;
    int slen = strlen(s), sublen = strlen(sub);

    for(int i=0; i <= slen - sublen; ++i) {
        found = 1;
        for(int j=0; j < sublen; ++j) {
            if(s[i+j] != sub[j]) {
                found = 0;
                break;
            }
        }
        if(found) count++;
    }

    return count;
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
}

// print out the array
void arr_post(t_int_arr *arr) {
    for(size_t i=0; i < arr->len; ++i) {
        postfloat(arr->data[i]);
    }
    post("length is %lu", arr->len);
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

static void arr_swap_idx(t_int_arr *arr, int a, int b) {
    int tmp = arr->data[a];
    arr->data[a] = arr->data[b];
    arr->data[b] = tmp;
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
    if(arr == NULL) {
        error("arr_push() arr is null pointer: %p", arr);
        return;
    }
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

// push a new node onto the edges array in t_graph
void graph_add_node(t_graph *g, t_int_arr *node) {
    if(g == NULL || node == NULL) {
        error("osci/tsp: graph_add_node() bad pointers: g %p | node %p", g, node);
        return;
    }
    g->edges = (t_int_arr*)realloc(g->edges, ++g->n_vertices * sizeof(t_int_arr));
    g->edges[g->n_vertices-1] = *node;
}

void graph_resize(t_graph *g, int new_size) {
    int old_size = g->n_vertices;
    g->n_vertices = new_size;
    g->edges = (t_int_arr*)realloc(g->edges, g->n_vertices * sizeof(t_int_arr));
    // make sure that any new nodes are initalized
    for(int i = old_size; i < g->n_vertices; ++i)
        g->edges[i] = int_arr(0);
}

/*
adjust node connections since node indices will be off
when we build the new graph
*/
void graph_fix_indices(t_graph *g, int amt) {
    for(int i=0; i < g->n_vertices; ++i) {
        t_int_arr* node = &g->edges[i];
        for(size_t j=0; j < node->len; ++j) {
            // post("%d - %d = %d", node->data[j], amt, node->data[j] - amt);
            node->data[j] -= amt;
        }
    }
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

bool is_connected(t_graph *g, bool *visited) {
    int c = count_connected(g, 0, visited);
    return c == g->n_vertices;
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
    if(g == NULL) {
        error("[osci/tsp]: bad graph pointer");
        return;
    }
    if(path == NULL) {
        error("[osci/tsp]: bad path pointer");
        return;
    }

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

void graph_sort(t_graph *g, t_vec3 *verts) {
    // sort the nodes connections based on their distance from each other
    for(int i=0; i < g->n_vertices; ++i) {
        t_int_arr *n = &g->edges[i];
        t_vec3 v = verts[i];
        // iterate over the connections and sort them here

        for(size_t j=0; j < n->len; ++j) {
            for(size_t k = j+1; k < n->len; ++k) {
                t_float d1 = v3_dist_sqrd(v, verts[n->data[j]]);
                t_float d2 = v3_dist_sqrd(v, verts[n->data[k]]);
                if(d1 > d2) { arr_swap_idx(n, j, k); }
            }
        }
    }
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

/* Graph Array Functions */

// create a graph arr
t_graph_arr graph_arr() {
    return (t_graph_arr){0, (t_graph*)getbytes(0 * sizeof(t_graph)) };
}

void ga_free(t_graph_arr *ga) {
    for(size_t i=0; i < ga->len; ++i) {
        graph_free(&ga->graphs[i]);
    }
    free_and_null(&ga->graphs);
}

// add a copy of g to the ga
void ga_push(t_graph_arr *ga, t_graph *g) {
    if(ga == NULL || g == NULL) { return; }

    ga->graphs = (t_graph *)realloc(ga->graphs, ++ga->len * sizeof(t_graph));
    // post("adding graph to position %lu", ga->len-1);
    ga->graphs[ga->len-1] = *g; // maybe it should hold pointers to graphs instead?
}

void ga_post(t_graph_arr *ga) {
    for(size_t i=0; i < ga->len; ++i) {
        post("graph [%d]", i);
        graph_post(&ga->graphs[i]);
    }
}

//take an input graph, and return a graph_array where each graph is a connected graph
t_graph_arr ga_generate_connected_graphs(t_graph g) {
    t_graph_arr ga = graph_arr();

    bool doit = true;
    while(doit) {
        bool visited[g.n_vertices];
        for(int i=0; i < g.n_vertices; ++i) {
            visited[i] = false;
        }

        doit = !is_connected(&g, visited);
        if(doit) {
            // post("not connected, generating a new graph");
            t_graph tmp = graph(0);
            // fill new graph with nodes from old graph that weren't visited
            for(int i=0; i < g.n_vertices; ++i) {
                if( !visited[i] ) {
                    // later: adjust the verts array
                    graph_add_node(&tmp, &g.edges[i]);
                    /* ERROR: node indices in the graph don't match the index of the vertex they refer to now!
                        Solution: change the vertices array to be a jagged 2d array. Then I can chop of vertices and drop
                        them in new arrays as needed to keep things making sense index wise.
                    */
                }
            }

            // must be first otherwise g.n_vertices will be wrong
            graph_fix_indices(&tmp, g.n_vertices - tmp.n_vertices);
            graph_resize(&g, g.n_vertices - tmp.n_vertices);
            ga_push(&ga, &g);
            g = tmp;
        }
    }

    ga_push(&ga, &g); // push the final graph onto the array

    // ga_post(&ga);
    return ga;
}

// create a path and it's interp array
void ga_create_path(t_graph_arr *ga, t_int_arr *p, t_int_arr *interp) {
    t_int_arr *paths = (t_int_arr*)getbytes(ga->len * sizeof(t_int_arr));

    // generate the paths
    for(size_t i=0; i < ga->len; ++i) {
        paths[i] = int_arr(0);
        euler_circuit(&ga->graphs[i], &paths[i], 0);
    }

    // flatten the paths
    int offset = 0;
    for(size_t i=0; i < ga->len; ++i) {
        if(i > 0) offset += ga->graphs[i-1].n_vertices;
        for(size_t j=0; j < paths[i].len; ++j) {
            arr_push(p, paths[i].data[j] + offset);
            arr_push(interp, (j < paths[i].len-1) ? 1 : 0);
        }
    }
}

void ga_fix_odds(t_graph_arr *ga) {
    for(size_t i=0; i < ga->len; ++i) {
        graph_fix_odds(&ga->graphs[i]);
    }
}

/*
* Parses the mesh data being sent in from blender.
* Data takes the following form
*   1. Begins with the string "animation frames"
*   2. Then comes the name of the mesh (skipping some non important pieces)
*   3. Then comes Vertices, followed by Edges, and it ends with Faces
*       a. Vertices are displayed like this {1,1,1}
*       b. Edges are displayed like this {1, 0}
*       c. We don't care about faces, we just use that to mark the end of the data we care about
*   7. If there is multiple meshes the next mesh will then follow after faces
*   8. Sometimes we get multiple set of animation frames, so make sure to only read in the last
*      one using the function rev_strstr()
*/
t_graph tsp_parse_mesh_data(t_tsp *this, t_symbol *mesh_data) {
    t_graph g = graph(0);
    int idx = 0, offset = 0;

    if(mesh_data == NULL) {
        pd_error(&this->obj, "[osci/tsp]: bad symbol pointer to mesh_data | %p", mesh_data);
        return g;
    } else if( strlen(mesh_data->s_name) < 100) {
        pd_error(&this->obj, "[osci/tsp]: Blender has no meshes!");
        return g;
    }

    // remove duplicate sets of mesh_data
    char *data = rev_strstr(mesh_data->s_name, "frames");

    int n_meshes = count_substring(data, "mesh");
    // post("n_meshes = %d", n_meshes);

    for(int i=0; i < n_meshes; ++i) {
        if(data == NULL) return g;
        // post("round %d", i);

        char *tmp_v = strstr(data, "vert");
        char *tmp_e = strstr(data, "edge");
        char *faces = strstr(data, "face");

        if(tmp_v == NULL || tmp_e == NULL || faces == NULL) {
            pd_error(&this->obj, "[osci/tsp]: invalid format for mesh data");
            error("mesh data: %s", data);
            return g;
        }

        int verts_len = (int)(tmp_e-2-tmp_v);
        int edges_len = (int)(faces-2-tmp_e);

        // post("verts len %d", verts_len);
        // post("edges len %d", edges_len);

        char verts[verts_len];
        char edges[edges_len];

        memset(verts, '\0', sizeof(verts));
        memset(edges, '\0', sizeof(edges));

        strncpy(verts, tmp_v, verts_len);
        strncpy(edges, tmp_e, edges_len);

        // post("verts string:\n%s", verts);
        // post("edges string:\n%s", edges);
        // post("faces string:\n%s", faces);
        // post("");
        // post("data string:\n%s", data);

        char *token = strtok(verts, "[");

        //get the vertices
        t_float x, y, z;
        while(token != NULL) {
            if(idx > this->max_verts) {
                pd_error(this, "[osci/tsp] Too many vertices. The maximum # of vertices is currently set to %d", this->max_verts);
                return g;
            }

            token = strtok(NULL, ",{}");
            if(token == NULL || !isnum(token))
                break;

            x = atof(token);
            token = strtok(NULL, ",{}");
            y = atof(token);
            token = strtok(NULL, ",{}");
            z = atof(token);
            this->vertices[idx++] = vec3(x,y,z);
        }

        offset = (i < 1) ? 0 : g.n_vertices;
        // post("offset: %d", offset);
        graph_resize(&g, idx);
        // add the edges to graph
        token = strtok(edges, "[");
        int u, v;
        while(token != NULL) {
            token = strtok(NULL, " {}");
            if(token == NULL || !isnum(token))
                break;
            u = atof(token);
            token = strtok(NULL, " {}");
            v = atof(token);

            //post("u: %d, v: %d", u, v);
            //post("u+off: %d, v+off: %d", u+offset, v+offset);
            add_edge(&g, u+offset, v+offset);
        }

        // from here faces is the only string that isn't mangled
        // we reset data to start from the next of vertices
        data = strstr(faces, "vert");
    }

    return g;
}

void tsp_symbol(t_tsp *this, t_symbol *mesh_data)
{
    #ifdef DEBUG
        int start = clock();
    #endif

    t_graph g = tsp_parse_mesh_data(this, mesh_data);

    if(g.n_vertices == 0) { return; }
    // graph_post(&g);

    graph_sort(&g, this->vertices);

    //graph_post(&g);

    t_graph_arr ga = ga_generate_connected_graphs(g);

    // post("===\n===\n===\n===\n====\n");
    ga_fix_odds(&ga);

    // post("fixed graph result ===");
    // post("======================");
    // graph_post(&g);

    // run fleury's algorithm and build the path(s)
    t_int_arr path = int_arr(0);
    t_int_arr interp = int_arr(0);
    ga_create_path(&ga, &path, &interp);
    // post("path length is %d", path.len);
    // post("interp lengh is %d", interp.len);

    // take that path and build up the x, y, and z arrays
    t_atom *xpts = (t_atom*)getbytes(path.len * sizeof(t_atom));
    t_atom *ypts = (t_atom*)getbytes(path.len * sizeof(t_atom));
    t_atom *zpts = (t_atom*)getbytes(path.len * sizeof(t_atom));
    t_atom *interp_vals = (t_atom*)getbytes(path.len * sizeof(t_atom));

    for(size_t i=0; i < path.len; ++i) {
        // postfloat(path.data[i]);
        SETFLOAT(xpts+i, this->vertices[path.data[i]].x);
        SETFLOAT(ypts+i, this->vertices[path.data[i]].y);
        SETFLOAT(zpts+i, this->vertices[path.data[i]].z);
        SETFLOAT(interp_vals+i, interp.data[i]);
    }

    // return the arrays, making sure to output the size of the path first
    outlet_float(this->size_out, path.len);
    outlet_list(this->xpts_out, &s_list, path.len, xpts);
    outlet_list(this->ypts_out, &s_list, path.len, ypts);
    outlet_list(this->zpts_out, &s_list, path.len, zpts);
    outlet_list(this->interp_out, &s_list, path.len, interp_vals);

    // cleanup
    free_and_null(&xpts);
    free_and_null(&ypts);
    free_and_null(&zpts);
    free_and_null(&interp_vals);

    arr_free(&path);
    arr_free(&interp);

    //graph_free(&g); todo take a look at graph and ga free methods something is causing them to crash

    #ifdef DEBUG
        int end = clock();
        int ticks = end-start;
        post("\nelapsed ticks: %d\nelapsed time: %.6f ms", ticks, ((float)ticks/CLOCKS_PER_SEC)*1000 );
    #endif
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
    this->interp_out = outlet_new(&this->obj, &s_list);
    this->size_out = outlet_new(&this->obj, &s_float);

    return (void *)this;
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
