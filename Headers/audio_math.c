#include "Audio_Math.h"

const double PI      = 3.14159265358979323846;
const double TWO_PI  = 6.28318530717958647693;
const double HALF_PI = 1.57079632679489661923;

int gcd(int a, int b) 
{
    return (b == 0) ? a : gcd(b, a % b);
}

t_float map_lin( t_float value,  t_float inputMin,  t_float inputMax,  t_float outputMin,  t_float outputMax, bool clamp)
{
    if(fabs(inputMin - inputMax) < FLT_EPSILON) // check if distance is basically zero
    {
        return outputMin;
    }
    else
    {
        t_float outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
        if( clamp )
        {
            if(outputMax < outputMin)
            {
                if( outVal < outputMax ) outVal = outputMax;
                else if( outVal > outputMin )outVal = outputMin;
                
            }
            else
            {
                if( outVal > outputMax )outVal = outputMax;
                else if( outVal < outputMin )outVal = outputMin;
            }
      }
      return outVal;
   }
}
 
t_float map_to_unit(t_float value, t_float inputMin, t_float inputMax, bool clamp)
{
    return map_lin(value,inputMin,inputMax,0.f,1.f,clamp);
}


t_float lerp(const t_float t, const t_float a, const t_float b)
{
    return a + (t * (b - a));
}

t_vec3 blend(float t, t_vec3 a, t_vec3 b)
{
    return (t_vec3) { lerp(t, a.x, b.x),
                    lerp(t, a.y, b.y),
                    lerp(t, a.z, b.z)
                    };
}

t_float mod1(const t_float value)
{
    return value - (int)value;
}
