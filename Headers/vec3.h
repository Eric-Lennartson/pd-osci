

#ifndef vec3_h
#define vec3_h

#include <math.h>
#include <stdbool.h>
#include "m_pd.h"

typedef struct { t_float x, y, z; } t_vec3;

// Is this macro a good idea?
#define NEW_VEC3 (t_vec3){0, 0, 0}
static inline t_vec3 vec3(t_float x, t_float y, t_float z) { return (t_vec3){x, y, z}; }

// basic operations
static inline t_vec3  v3_add    (t_vec3 a, t_vec3 b)          { return (t_vec3){ a.x + b.x, a.y + b.y, a.z + b.z }; }
static inline t_vec3  v3_addf   (t_vec3 a, t_float f)         { return (t_vec3){ a.x + f,   a.y + f,   a.z + f   }; }
static inline t_vec3  v3_sub    (t_vec3 a, t_vec3 b)          { return (t_vec3){ a.x - b.x, a.y - b.y, a.z - b.z }; }
static inline t_vec3  v3_subf   (t_vec3 a, t_float f)         { return (t_vec3){ a.x - f,   a.y - f,   a.z - f   }; }
static inline t_vec3  v3_mult   (t_vec3 a, t_vec3 b)          { return (t_vec3){ a.x * b.x, a.y * b.y, a.z * b.z }; }
static inline t_vec3  v3_multf  (t_vec3 a, t_float f)         { return (t_vec3){ a.x * f,   a.y * f,   a.z * f   }; }
static inline t_vec3  v3_div    (t_vec3 a, t_vec3 b)          { return (t_vec3){ a.x / b.x, a.y / b.y, a.z / b.z }; }
static inline t_vec3  v3_divf   (t_vec3 a, t_float f)         { return (t_vec3){ a.x / f,   a.y /f,    a.z / f   }; }
static inline t_float v3_len    (t_vec3 v)                    { return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);          }
static inline t_vec3  v3_norm   (t_vec3 v) ;

// boolean comparison
static inline bool v3_equal (t_vec3 a, t_vec3 b)   { return (a.x == b.x) && (a.y == b.y) && (a.z == b.z); }
static inline bool v3_unequal (t_vec3 a, t_vec3 b) { return (a.x != b.x) || (a.y != b.y) || (a.z != b.z); }

// transformations
void v3_shear(t_vec3 *v, t_float angle, const char* axis); // why char ptr? it's a single char?

// normalize
static inline t_vec3 v3_norm(t_vec3 v) 
{
	float len = v3_length(v);
	if (len > 0)
		return (t_vec3){ v.x / len, v.y / len, v.z / len };
	else
		return (t_vec3){ 0, 0, 0};
}

// basic transformations
// isn't translate covered by addition?
// isn't scale covered by mult?
void v3_shear(t_vec3 *v, t_float angle, const char* axis) // possibly change to a regular char
{
    
    if(axis && v) // check for nullptr
    {
        t_float x = v->x;
        t_float y = v->y;
        t_float z = v->z;
        if(*axis == 'x')
        {
            v->x = x + (angle * y) + (angle * z);
        }
        else if(*axis == 'y')
        {
            v->y = (angle * x) + y + (angle * z);
        }
        else // must be the z axis
        {
            v->z = (angle * x) + (angle * y) + z;
        }
    }
}

#endif /* vec3_h */
