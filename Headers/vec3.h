

#ifndef vec3_h
#define vec3_h

#include <math.h>
#include <stdbool.h>
#include "m_pd.h"

typedef struct { t_float x, y, z; } t_vec3;

// TODO rm this macro 
#define NEW_VEC3 (t_vec3){0, 0, 0}
t_vec3 vec3(t_float x, t_float y, t_float z);

// basic operations
t_vec3  v3_add    (t_vec3 a, t_vec3 b);
t_vec3  v3_addf   (t_vec3 a, t_float f);
t_vec3  v3_sub    (t_vec3 a, t_vec3 b);
t_vec3  v3_subf   (t_vec3 a, t_float f);
t_vec3  v3_mult   (t_vec3 a, t_vec3 b);
t_vec3  v3_multf  (t_vec3 a, t_float f);
t_vec3  v3_div    (t_vec3 a, t_vec3 b);
t_vec3  v3_divf   (t_vec3 a, t_float f);
t_float v3_len    (t_vec3 v);
t_vec3  v3_norm   (t_vec3 v);

// boolean comparison
bool v3_equal (t_vec3 a, t_vec3 b);
bool v3_unequal (t_vec3 a, t_vec3 b);

// basic transformations
// char* because of t_symbol
void v3_shear(t_vec3 *v, t_float angle, const char* axis);
// doesn't avoid gimbal lock problems
void v3_rotate(t_vec3* v, t_float ax, t_float ay, t_float az);

#endif /* vec3_h */
