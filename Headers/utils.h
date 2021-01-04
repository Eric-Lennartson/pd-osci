//
//  utils.h
//  
//
//  Created by Eric Lennartson on 5/27/20.
//

#ifndef utils_h
#define utils_h

#include "stdarg.h"
#include "m_pd.h"

#define min(a, b) a < b ? a : b
#define max(a, b) a > b ? a : b
#define clamp(val,min,max) ((val) < (min) ? (min) : ((val > max) ? (max) : (val)))

#define FLT_EPSILON 1.19209290E-07F

t_float va_minf(t_float num_args, ...)
{
    t_float min;
    
    va_list values;
    va_start(values, num_args);
    
    min = va_arg(values, t_float);
    
    for(t_float i = 1; i < num_args; ++i)
    {
        t_float temp =  va_arg(values, t_float);
        min = min < temp ? min : temp;
    }
    
    va_end(values);
    
    return min;
}

int va_mini(int num_args, ...)
{
    int min;
    
    va_list values;
    va_start(values, num_args);
    
    min = va_arg(values, int);
    
    for(int i = 1; i < num_args; ++i)
    {
        int temp =  va_arg(values, int);
        min = min < temp ? min : temp;
    }
    
    va_end(values);
    
    return min;
}

t_float va_maxf(t_float num_args, ...)
{
    t_float max;
    
    va_list values;
    va_start(values, num_args);
    
    max = va_arg(values, t_float);
    
    for(t_float i = 1; i < num_args; ++i)
    {
        t_float temp =  va_arg(values, t_float);
        max = max > temp ? max : temp;
    }
    
    va_end(values);
    
    return max;
}

int va_maxi(int num_args, ...)
{
    int max;
    
    va_list values;
    va_start(values, num_args);
    
    max = va_arg(values, int);
    
    for(int i = 1; i < num_args; ++i)
    {
        int temp =  va_arg(values, int);
        max = max > temp ? max : temp;
    }
    
    va_end(values);
    
    return max;
}

// basically the map function, should I rename it map and map to unit?
double map_lin( double value,  double inputMin,  double inputMax,  double outputMin,  double outputMax, bool clamp)
{
    if(fabs(inputMin - inputMax) < FLT_EPSILON) // check if distance is basically zero
    {
        return outputMin;
    }
    else
    {
        double outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
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
 
double map_to_unit(double value, double inputMin, double inputMax, double clamp)
{
    return map_lin(value,inputMin,inputMax,0.f,1.f,clamp);
}

float mod1(float val) { return val - (int)val; }
    
#endif /* utils_h */
