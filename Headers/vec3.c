#include "vec3.h"

t_vec3 vec3(t_float x, t_float y, t_float z) { return (t_vec3){x, y, z}; }

// basic operations
t_vec3  v3_add    (t_vec3 a, t_vec3 b)          { return (t_vec3){ a.x + b.x, a.y + b.y, a.z + b.z }; }
t_vec3  v3_addf   (t_vec3 a, t_float f)         { return (t_vec3){ a.x + f,   a.y + f,   a.z + f   }; }
t_vec3  v3_sub    (t_vec3 a, t_vec3 b)          { return (t_vec3){ a.x - b.x, a.y - b.y, a.z - b.z }; }
t_vec3  v3_subf   (t_vec3 a, t_float f)         { return (t_vec3){ a.x - f,   a.y - f,   a.z - f   }; }
t_vec3  v3_mult   (t_vec3 a, t_vec3 b)          { return (t_vec3){ a.x * b.x, a.y * b.y, a.z * b.z }; }
t_vec3  v3_multf  (t_vec3 a, t_float f)         { return (t_vec3){ a.x * f,   a.y * f,   a.z * f   }; }
t_vec3  v3_div    (t_vec3 a, t_vec3 b)          { return (t_vec3){ a.x / b.x, a.y / b.y, a.z / b.z }; }
t_vec3  v3_divf   (t_vec3 a, t_float f)         { return (t_vec3){ a.x / f,   a.y /f,    a.z / f   }; }
t_float v3_len    (t_vec3 v)                    { return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);          }

// boolean comparison
bool v3_equal (t_vec3 a, t_vec3 b)   { return (a.x == b.x) && (a.y == b.y) && (a.z == b.z); }
bool v3_unequal (t_vec3 a, t_vec3 b) { return (a.x != b.x) || (a.y != b.y) || (a.z != b.z); }

// transformations
void v3_shear(t_vec3 *v, t_float angle, const char* axis); 

// normalize
t_vec3 v3_norm(t_vec3 v) 
{
	float len = v3_len(v);
	if (len > 0)
		return (t_vec3){ v.x / len, v.y / len, v.z / len };
	else
		return (t_vec3){ 0, 0, 0};
}

// basic transformations
// char* because of t_symbol
void v3_shear(t_vec3 *v, t_float angle, const char* axis)
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

// doesn't avoid gimbal lock problems
// this should probably be in vec3.h
void v3_rotate(t_vec3* v, t_float ax, t_float ay, t_float az)
{
    t_sample a = cosf(ax);
    t_sample b = sinf(ax);
    t_sample c = cosf(ay);
    t_sample d = sinf(ay);
    t_sample e = cosf(az);
    t_sample f = sinf(az);

    t_sample nx = c * e * v->x - c * f * v->y + d * v->z;
    t_sample ny = (a * f + b * d * e) * v->x + (a * e - b * d * f) * v->y - b * c * v->z;
    t_sample nz = (b * f - a * d * e) * v->x + (a * d * f + b * e) * v->y + a * c * v->z;

    v->x = nx;
    v->y = ny;
    v->z = nz;
}