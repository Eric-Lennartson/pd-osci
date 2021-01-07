#include "utils.h"

// copied from geeksforgeeks
static inline int gcd(int a, int b) {
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

double va_minf(double num_args, ...)
{
    double min;
    
    va_list values;
    va_start(values, num_args);
    
    min = va_arg(values, double);
    
    for(double i = 1; i < num_args; ++i)
    {
        double temp =  va_arg(values, double);
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

double va_maxf(double num_args, ...)
{
    double max;
    
    va_list values;
    va_start(values, num_args);
    
    max = va_arg(values, double);
    
    for(double i = 1; i < num_args; ++i)
    {
        double temp =  va_arg(values, double);
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
