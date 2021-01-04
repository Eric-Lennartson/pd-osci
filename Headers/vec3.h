//
//  vec3.h
//  
//
//  copied from somewhere on github 
//

#ifndef vec3_h
#define vec3_h

#include <math.h>
#include <stdbool.h>
#include "m_pd.h"

typedef struct { t_float x, y, z; } t_vec3;

static inline t_vec3 vec3(t_float x, t_float y, t_float z) { return (t_vec3){x, y, z}; }

// basic operations
static inline t_vec3  v3_add   (t_vec3 a, t_vec3 b)          { return (t_vec3){ a.x + b.x, a.y + b.y, a.z + b.z }; }
static inline t_vec3  v3_addf  (t_vec3 a, t_float f)         { return (t_vec3){ a.x + f,   a.y + f,   a.z + f   }; }
static inline t_vec3  v3_sub   (t_vec3 a, t_vec3 b)          { return (t_vec3){ a.x - b.x, a.y - b.y, a.z - b.z }; }
static inline t_vec3  v3_subf  (t_vec3 a, t_float f)         { return (t_vec3){ a.x - f,   a.y - f,   a.z - f   }; }
static inline t_vec3  v3_mul   (t_vec3 a, t_vec3 b)          { return (t_vec3){ a.x * b.x, a.y * b.y, a.z * b.z }; }
static inline t_vec3  v3_mulf  (t_vec3 a, t_float f)         { return (t_vec3){ a.x * f,   a.y * f,   a.z * f   }; }
static inline t_vec3  v3_div   (t_vec3 a, t_vec3 b)          { return (t_vec3){ a.x / b.x, a.y / b.y, a.z / b.z }; }
static inline t_vec3  v3_divf  (t_vec3 a, t_float f)         { return (t_vec3){ a.x / f,   a.y /f,    a.z / f   }; }
static inline t_float v3_len   (t_vec3 v)                    { return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);          }

// boolean comparison
static inline bool v3_equal (t_vec3 a, t_vec3 b)   { return (a.x == b.x) && (a.y == b.y) && (a.z == b.z); }
static inline bool v3_unequal (t_vec3 a, t_vec3 b) { return (a.x != b.x) || (a.y != b.y) || (a.z != b.z); }
// basic transformations
#endif /* vec3_h */
