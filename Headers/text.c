#include "text.h"

// this should be refactored
t_float scale_lin(t_float value, t_float inputMin, t_float inputMax, t_float outputMin, t_float outputMax, bool clamp) {

   if (fabsf(inputMin - inputMax) < FLT_EPSILON){
         return outputMin;
     } else {
         float outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);

         if( clamp ){
             if(outputMax < outputMin){
                 if( outVal < outputMax )outVal = outputMax;
                 else if( outVal > outputMin )outVal = outputMin;
             }else{
                 if( outVal > outputMax )outVal = outputMax;
                 else if( outVal < outputMin )outVal = outputMin;
             }
         }
         return outVal;
     }
}

t_float tri(t_float value) {
    value = fmodf(fmodf(value,1)+1,1);
    return value<0.5?(2*value):(2-2*value);
}

t_float tri_skew(t_float value, t_float skew){
    value = fmodf(fmodf(value,1)+1,1);
    return value<skew?
        scale_lin(value,0,skew,0,1, true) :
        scale_lin(value,skew,1,1,0, true);
}

//======================LETTERS===========================
t_vec3 letter_A(float t) { // A
    t_vec3 v = NEW_VEC3;
    
    if (t < .075) {
        v.x = t/.1*.25;
        v.y = t/.1*.5;
    }
    else if (t < .525) {
        v.x = (t-.075)/.45*.625+.1875;
        v.y = tri((t-.075)/.45)*.625+.375;
    }
    else if (t < .675) {
        v.x = tri((t-.525)/.15)*.1875+.8125;
        v.y = .375-tri((t-.525)/.15)*.375;
    }
    else if (t < .925) {
        v.x = .8125-((t-.675)/(.25))*.625;
        v.y = .375;
    }
    else if (t <= 1) {
        v.x = .1875-(t-.925)/.075*.1875;
        v.y = .375-(t-.925)/.075*.375;
    }

    return v;
}

t_vec3 letter_B(float t) { // B
    t_vec3 v = NEW_VEC3;
    
    if (t < 4/16.) {
        v.y = t/.25;
    }
    else if (t < 6/16.) {
        v.x = (t-4/16.)/(2/16.)*.5;
        v.y = 1;
    }
    else if (t < 9/16.) {
        v.x = sin((t-6/16.)/(3/16.)*PI)*.25+.5;
        v.y = cos((t-6/16.)/(3/16.)*PI)*.25+.75;
    }
    else if (t < 11/16.) {
        v.x = .5-tri((t-9/16.)/(2/16.))*.5;
        v.y = .5;
    }
    else if (t < 14/16.) {
        v.x = sin((t-11/16.)/(3/16.)*PI)*.25+.5;
        v.y = cos((t-11/16.)/(3/16.)*PI)*.25+.25;
    }
    else if (t <= 1) {
        v.x = .5-(t-14/16.)/(2/16.)*.5;
    }
    v.x *= 4 / 3.;

    return v;
}

t_vec3 letter_C(float t) { // C
    t_vec3 v = NEW_VEC3;
    t = tri(1-t+1/12.);
    v.x = sin(t*1.5*PI + 3 / 4.*PI) / 2 + .5;
    v.y = cos(t*1.5*PI + 3 / 4.*PI) / 2 + .5;

    v.x /= (sin(PI / 4) / 2 + .5);

    return v;
}


t_vec3 letter_D(float t) { // D
    t_vec3 v = NEW_VEC3;
    
    t *= 6+2*PI;
    
    if (t < 4)
        v.y = t/4;
    else if (t < 5) {
        v.x = (t-4)/4;
        v.y = 1;
    }
    else if (t < 5+2*PI) {
        v.x = sin((t-5)/2)/2+.25;
        v.y = cos((t-5)/2)/2+.5;
    }
    else if (t <= 6+2*PI)
        v.x = .25-(t-5-2*PI)*.25;
    
    v.x *= 4 / 3.;

    return v;
}

t_vec3 letter_E(float t) { // E
    t_vec3 v = NEW_VEC3;
    
    if (t < 4./26) {
        v.y = t/(4./26);
    }
    else if (t < 7./26) {
        v.x = (t-(4./26))/(3./26);
        v.y = 1;
    }
    else if (t < 10./26) {
        v.x = 1-(t-(7./26))/(3./26);
        v.y = 1;
    }
    else if (t < 12./26) {
        v.y = 1-(t-10./26)/(2./26)*.5;
    }
    else if (t < 15./26) {
        v.x = (t-(12./26))/(3./26);
        v.y = .5;
    }
    else if (t < 18./26) {
        v.x = 1-(t-(15./26))/(3./26);
        v.y = .5;
    }
    else if (t < 20./26) {
        v.y = .5-(t-18./26)/(2./26)*.5;
    }
    else if (t < 1) {
        v.x = tri((t-20./26)/(6./26));
    }

    return v;
}

t_vec3 letter_F(float t) { // F
    t_vec3 v = NEW_VEC3;
    
    if (t < 4./20) {
        v.y = t/(4./20);
    }
    else if (t < 7./20) {
        v.x = (t-(4./20))/(3./20);
        v.y = 1;
    }
    else if (t < 10./20) {
        v.x = 1-(t-(7./20))/(3./20);
        v.y = 1;
    }
    else if (t < 12./20) {
        v.y = 1-(t-10./20)/(2./20)*.5;
    }
    else if (t < 15./20) {
        v.x = (t-(12./20))/(3./20);
        v.y = .5;
    }
    else if (t < 18./20) {
        v.x = 1-(t-(15./20))/(3./20);
        v.y = .5;
    }
    else if (t < 20./20) {
        v.y = .5-(t-18./20)/(2./20)*.5;
    }

    return v;
}

t_vec3 letter_G(float t) { // G
    t_vec3 v = NEW_VEC3;
    
    t = tri_skew(mod1(t+2.5/8), .5);
    
    if (t < 7./8) {
        v.x = (clamp(cos((t + 1. / 8) * 2 * PI), -1, cos(PI / 4))/2+.5)/((1+cos(PI/4))/2);
        v.y = sin((t+1./8)*2*PI)/2+.5;
    }
    else {
        v.x = 1-(t-7./8)*4;
        v.y = .5;
    }

    //v.x = clamp(v.x, 0, cos(PI * 3/16.));

    return v;
}

t_vec3 letter_H(float t) { // H
    t_vec3 v = NEW_VEC3;
    
    //t = tri(t);
    
    if (t < 6./22)
        v.y = tri(t/(8./22));
    else if (t < 9./22) {
        v.x = (t-6./22)/(3./22);
        v.y = .5;
    }
    else if (t < 17./22) {
        v.x = 1;
        v.y = tri((t-9./22)/(8./22)+.25);
    }
    else if (t < 20./22) {
        v.x = 1-((t-17./22)/(3./22));
        v.y = .5;
    }
    else
        v.y = .5-(t-20./22)/(2./22)*.5;
    
    return v;
}

t_vec3 letter_I(float t) { // I
    t_vec3 v = NEW_VEC3;
    
    if (t < 3./20) {
        v.x = t/(3./20);
    }
    else if (t < 4.5/20) {
        v.x = 1-(t-3./20)/(1.5/20)*.5;
    }
    else if (t < 8.5/20) {
        v.x = .5;
        v.y = (t-4.5/20)/(4./20);
    }
    else if (t < 10./20) {
        v.x = (t-8.5/20)/(1.5/20)*.5+.5;
        v.y = 1;
    }
    else if (t < 13./20) {
        v.x = 1-(t-1./2)/(3./20);
        v.y = 1;
    }
    else if (t < 14.5/20) {
        v.x = (t-13./20)/(1.5/20)*.5;
        v.y = 1;
    }
    else if (t < 18.5/20) {
        v.x = .5;
        v.y = 1-(t-14.5/20)/(4./20);
    }
    else if (t <= 1) {
        v.x = .5-(t-18.5/20)/(1.5/20)*.5;
    }

    return v;
}


t_vec3 letter_J(float t) { // J
    t_vec3 v = NEW_VEC3;
    t = tri(t+1.5/10);
    
    if (t < 3.0/5) {
        v.x = .5-.5*cos(PI*t/(3.0/5));
        v.y = .5-.5*sin(PI*t/(3.0/5));
    }
    else {
        v.x = 1;
        v.y = (t-3.0/5)/(2.0/5)*(1-.5)+.5;
    }

    return v;
}


t_vec3 letter_K(float t) { // K
    t_vec3 v = NEW_VEC3;
    
    if (t < 6./22)
        v.y = tri(t/(8./22));
    else if (t < 7./22) {
        v.x = (t-6./22)/(1./22)*.5;
        v.y = .5;
    }
    else if (t < 13./22) {
        v.x = .5+tri((t-7./22)/(6./22))*.5;
        v.y = .5+tri((t-7./22)/(6./22))*.5;
    }
    else if (t < 19./22) {
        v.x = .5+tri((t-13./22)/(6./22))*.5;
        v.y = .5-tri((t-13./22)/(6./22))*.5;
    }
    else if (t < 20./22) {
        v.x = .5-((t-19./22)/(1./22))*.5;
        v.y = .5;
    }
    else
        v.y = .5-(t-20./22)/(2./22)*.5;
    
    return v;
}

t_vec3 letter_L(float t) { // L
    t_vec3 v = NEW_VEC3;
    
    t = mod1(t+4.0/14);
    t = tri(t);
    
    if (t < 4.0 / 7)
        v.y = 1 - t / (4.0 / 7);
    else if (t <= 1)
        v.x = (t - 4.0 / 7) / (3.0 / 7);

    return v;
}

t_vec3 letter_M(float t) { // M
    t_vec3 v = NEW_VEC3;
    
    t = tri(t);
    
    if (t < 4./13)
        v.y = t/(4./13);
    else if (t < 6.5/13) {
        v.x = (t-4./13)/(2.5/13)*.5;
        v.y = 1-(t-4./13)/(2.5/13)*.5;
    }
    else if (t < 9./13) {
        v.x = (t-6.5/13)/(2.5/13)*.5+.5;
        v.y = .5+(t-6.5/13)/(2.5/13)*.5;
    }
    else {
        v.x = 1;
        v.y = 1-(t-9./13)/(4./13);
    }

    return v;
}

t_vec3 letter_N(float t) { // N
    t_vec3 v = NEW_VEC3;
    
    t = tri(t);
    
    if (t < 4. / 13)
        v.y = t / (4. / 13);
    else if (t < 9./13) {
        v.x = (t - 4. / 13) / (5. / 13);
        v.y = 1 - (t - 4. / 13) / (5. / 13);
    }
    else {
        v.x = 1;
        v.y = (t-9./13)/(4./13);
    }

    return v;
}

t_vec3 letter_O(float t) { // O
    t_vec3 v = NEW_VEC3;
    t = mod1(1.5-t);
    v.x = sin(t*2*PI)/2+.5;
    v.y = cos(t*2*PI)/2+.5;

    return v;
}

t_vec3 letter_P(float t) { // P
    t_vec3 v = NEW_VEC3;
    
    t = tri(t)*(8+PI);

    if (t < 4)
        v.y = t/4;
    else if (t < 6) {
        v.x = (t-4)*.25;
        v.y = 1;
    }
    else if (t < 6+PI) {
        v.x = sin(t-6)/2+.5;
        v.y = cos(t-6)/4+.75;
    }
    else if (t <= 8+PI) {
        v.x = .5-(t-6-PI)*.25;
        v.y = .5;
    }

    return v;
}

t_vec3 letter_Q(float t) { // Q
    t_vec3 v = NEW_VEC3;
    t = tri_skew(t + .75, 1);
    t *= 1.17;

    v.x = cos((clamp(t, 0, 1) - .125) * 2 * PI) / 2 + .5;
    v.y = sin((clamp(t, 0, 1) - .125) * 2 * PI) / 2 + .5;

    if (t>1) {
        float t2 = (tri((t - 1) / .17 + .25) * 2 - 1)*(1-cos(PI/4))/2;

        v.x += t2;
        v.y -= t2;
    }

    return v;
}

t_vec3 letter_R(float t) { // R
    t_vec3 v = NEW_VEC3;
    
    t = tri(t)*(10+PI);

    if (t < 4)
        v.y = t/4;
    else if (t < 6) {
        v.x = (t-4)*.25;
        v.y = 1;
    }
    else if (t < 6+PI) {
        v.x = sin(t-6)/2+.5;
        v.y = cos(t-6)/4+.75;
    }
    else if (t < 8+PI) {
        v.x = tri((t-6-PI)/2)/2+.5;
        v.y = .5-tri((t-6-PI)/2)/2;
    }
    else if (t <= 10+PI) {
        v.x = .5-(t-8-PI)*.25;
        v.y = .5;
    }

    return v;
}

 
t_vec3 letter_S(float t) { // S
    t_vec3 v = NEW_VEC3;

    t = tri(t+1/12.);

    if (t < .5) {
        v.x = sin(-t*3*PI-PI/2)/2+.5;
        v.y = cos(-t*3*PI-PI/2)/4+.25;
    }
    else {
        v.x = sin(t*3*PI-PI/2)/2+.5;
        v.y = cos(t*3*PI-PI/2)/4+.75;
    }

    return v;
}

 
t_vec3 letter_T(float t) { // T
    t_vec3 v = NEW_VEC3;
    
    if (t < 2/7.0) {
        v.x = .5;
        v.y = t/(2/7.0);
    }
    else if (t < 5/7.0) {
        v.x = tri((t-2/7.0)/(3/7.0)+.25);
        v.y = 1;
    }
    else {
        v.x = .5;
        v.y = 1-(t-5/7.0)/(2/7.0);
    }
    
    return v;
}

 
t_vec3 letter_U(float t) { // U
    t_vec3 v = NEW_VEC3;

    t = tri(t+.25)*(2+PI);
    
    if (t < 1) {
        v.y = 1-t/2;
    }
    else if (t < 1+PI) {
        v.x = sin(-PI/2-t+1)/2+.5;
        v.y = cos(-PI/2-t+1)/2+.5;
    }
    else {
        v.x = 1;
        v.y = (t-1-PI)/2+.5;
    }

    return v;
}

 
t_vec3 letter_V(float t) { // V
    t_vec3 v = NEW_VEC3;
    
    v.x = tri(t+.25);
    v.y = 1-tri(t*2+.5);

    return v;
}

 
t_vec3 letter_W(float t) { // W
    t_vec3 v = NEW_VEC3;

    v.x = tri(t+1/8.);
    v.y = tri(t*4);

    return v;
}

 
t_vec3 letter_X(float t) { // X
    t_vec3 v = NEW_VEC3;

    if (t < .25)
        v.x = tri(t*4)*.5;
    else if (t < .5)
        v.x = (t-.25)*4;
    else if (t < .75)
        v.x = 1-tri((t-.5)*4)*.5;
    else
        v.x = 1-(t-.75)*4;
    v.y = tri(t * 2);

    return v;
}

 
t_vec3 letter_Y(float t) { // Y
    t_vec3 v = NEW_VEC3;
    
    if (t < 2/12.) {
        v.x = .5;
        v.y = t/(2/12.)*.5;
    }
    else if (t < 10/12.) {
        v.x = tri((t-2/12.)/(8/12.)+.25);
        v.y = .5+tri((t-2/12.)/(8/12.)*2)*.5;
    }
    else if (t <= 1) {
        v.x = .5;
        v.y = .5-(t-10/12.)/(2/12.)*.5;
    }
    

    return v;
}

 
t_vec3 letter_Z(float t) { // Z
    t_vec3 v = NEW_VEC3;
    
    if (t < 3/14.) {
        v.x = t*14/3.;
        v.y = t*14/3.;
    }
    else if (t < 5/14.) {
        v.x = 1-(t-3/14.)*14/2.;
        v.y = 1;
    }
    else if (t < 7/14.) {
        v.x = (t-5/14.)*14/2.;
        v.y = 1;
    }
    else if (t < 10/14.) {
        v.x = 1-(t-7/14.)*14/3.;
        v.y = 1-(t-7/14.)*14/3.;
    }
    else {
        v.x = tri((t-10/14.)*14/4.);
        v.y = 0;
    }
    
    return v;
}

//======================NUMBERS===========================
t_vec3 number_0(float t) { // 0
    t_vec3 v = NEW_VEC3;
    t = mod1(1.5 - t);
    v.x = sin(t * 2 * PI) / 2 + .5;
    v.y = cos(t * 2 * PI) / 2 + .5;

    return v;
}

 
t_vec3 number_1(float t) { // 1
    t_vec3 v = NEW_VEC3;
    v.x = clamp((1 - tri(t)) * 3, 0, 1);
    v.y = tri(tri(t)*.75);
    
    return v;
}

 
t_vec3 number_2(float t) { // 2
    t = tri_skew(t+3/8., .5);
    t_vec3 v = NEW_VEC3;
    
    if (t < 2/4.) {
        v.x = .5-.5*cos(PI*2.5*t);
        v.y = .7+.3*sin(PI*2.5*t);
    }
    else if (t < 3/4.) {
        v.x = (.5+.5*cos(PI/4))*(1-(t-2/4.)*4);
        v.y = (.7-.3*sin(PI/4))*(1-(t-2/4.)*4);
    }
    else {
        v.x = (t - 3 / 4.) * 4;
        v.y = 0;
    }
    
    return v;
}

 
t_vec3 number_3(float t) { // 3
    t = 1-tri_skew(t+.08, .5);
    t_vec3 v = NEW_VEC3;
    
    if (t < .5) {
        v.x = .5-.5*cos(3*PI*t);
        v.y = .75+.25*sin(3*PI*t);
    }
    else if (t <= 1) {
        v.x = .5-.5*cos(PI/2+3*PI*(t-.5));
        v.y = .25+.25*sin(PI/2+3*PI*(t-.5));
    }
    
    return v;
}

 
t_vec3 number_4(float t) { // 4
    //t = 1-tri(t, .5);
    t_vec3 v = NEW_VEC3;
    t = tri(t);
    if (t < 1 / 3.0) {
        v.x = .75;
        v.y = t * 3;
    }
    else if (t < 2 / 3.0) {
        v.x = .75 - (t - 1 / 3.) * 3 * .75;
        v.y = 1 - (t - 1 / 3.) * 3 * .75;
    }
    else {
        v.x = (t - 2 / 3.) * 3;
        v.y = .25;
    }

    return v;
}

 
t_vec3 number_5(float t) { // 5
    t = tri_skew(t+.05, .5);
    t_vec3 v = NEW_VEC3;
    
    if (t < 3/5.) {
        v.x = .5*sin(1.25*PI-1.5*PI*t*5/3)/(.5+sin(PI/4)/2)+(.5-.25+sin(PI/4)/4);
        v.y = .375*cos(1.25*PI-1.5*PI*t*5/3)+.375;
    }
    else if (t < 3.5/5) {
        v.x = 0;
        v.y = (t-3/5.)*5*(1-(.5+sin(PI/4)/2)*.75)*2+(.5+sin(PI/4)/2)*.75;
    }
    else {
        v.x = (t-3.5/5.)/0.3;
        v.y = 1;
    }
        
    return v;
}

 
t_vec3 number_6(float t) { // 6
    t_vec3 v = NEW_VEC3;
    t = tri_skew(t + 3.5 / 6.5, 0);
    if (t > 4 / 6.5)
        t = tri((t - 4 / 6.5) / (2.5 / 6.5))*(2.5 / 6.5) + 4 / 6.5;
    if (t < 4/6.5) {
        float t2 = t / (4/6.5);
        v.x = -cos(2 * PI*t2) / 2 + .5;
        v.y = sin(2 * PI*t2)/3 + 1/3.;
    }
    else if (t < 4.5 / 6.5) {
        float t2 = (t - 4 / 6.5) / (.5/6.5);
        v.x = 0;
        v.y = 1/3. + t2 / 3;
    }
    else {
        float t2 = (t - 4.5 / 6.5) / (2 / 6.5);
        v.x = -cos(PI*t2) / 2 + .5;
        v.y = sin(PI*t2) / 3 + 2 / 3.;
    }
        
    return v;
}

 
t_vec3 number_7(float t) { // 7
    t_vec3 v = NEW_VEC3;
    
    if (t < 1/3.) {
        v.x = .25+t*2.25;
        v.y = t*3;
    }
    else if (t < 2/3.) {
        v.x = 1-tri((t-1/3.)*3);
        v.y = 1;
    }
    else {
        v.x = 1-(t-2/3.)*2.25;
        v.y = 1-(t-2/3.)*3;
    }
        
    return v;
}

 
t_vec3 number_8(float t) { // 8
    t_vec3 v = NEW_VEC3;
    v.x = sin(t*4*PI)/2+.5;
    v.y = -cos(t*2*PI)/2+.5;
        
    return v;
}

 
t_vec3 number_9(float t) { // 9
    t_vec3 v = NEW_VEC3;
    t = tri_skew(t + 1.75 / 6.5, 0);
    if (t > 4 / 6.5)
        t = tri((t - 4 / 6.5) / (2.5 / 6.5))*(2.5 / 6.5) + 4 / 6.5;
    if (t < 4 / 6.5) {
        float t2 = t / (4 / 6.5);
        v.x = -cos(2 * PI*t2) / 2 + .5;
        v.y = sin(2 * PI*t2) / 3 + 1 / 3.;
    }
    else if (t < 4.5 / 6.5) {
        float t2 = (t - 4 / 6.5) / (.5 / 6.5);
        v.x = 0;
        v.y = 1 / 3. + t2 / 3;
    }
    else {
        float t2 = (t - 4.5 / 6.5) / (2 / 6.5);
        v.x = -cos(PI*t2) / 2 + .5;
        v.y = sin(PI*t2) / 3 + 2 / 3.;
    }

    v.x = 1 - v.x;
    v.y = 1 - v.y;
    return v;
}

t_vec3 drawLetter(char letter, float t){
    switch (letter) {
    case 'a':
    case 'A': return letter_A(t); break;
    case 'b':
    case 'B': return letter_B(t); break;
    case 'c':
    case 'C': return letter_C(t); break;
    case 'd':
    case 'D': return letter_D(t); break;
    case 'e':
    case 'E': return letter_E(t); break;
    case 'f':
    case 'F': return letter_F(t); break;
    case 'g':
    case 'G': return letter_G(t); break;
    case 'h':
    case 'H': return letter_H(t); break;
    case 'i':
    case 'I': return letter_I(t); break;
    case 'j':
    case 'J': return letter_J(t); break;
    case 'k':
    case 'K': return letter_K(t); break;
    case 'l':
    case 'L': return letter_L(t); break;
    case 'm':
    case 'M': return letter_M(t); break;
    case 'n':
    case 'N': return letter_N(t); break;
    case 'o':
    case 'O': return letter_O(t); break;
    case 'p':
    case 'P': return letter_P(t); break;
    case 'q':
    case 'Q': return letter_Q(t); break;
    case 'r':
    case 'R': return letter_R(t); break;
    case 's':
    case 'S': return letter_S(t); break;
    case 't':
    case 'T': return letter_T(t); break;
    case 'u':
    case 'U': return letter_U(t); break;
    case 'v':
    case 'V': return letter_V(t); break;
    case 'w':
    case 'W': return letter_W(t); break;
    case 'x':
    case 'X': return letter_X(t); break;
    case 'y':
    case 'Y': return letter_Y(t); break;
    case 'z':
    case 'Z': return letter_Z(t); break;
    case '0': return number_0(t); break;
    case '1': return number_1(t); break;
    case '2': return number_2(t); break;
    case '3': return number_3(t); break;
    case '4': return number_4(t); break;
    case '5': return number_5(t); break;
    case '6': return number_6(t); break;
    case '7': return number_7(t); break;
    case '8': return number_8(t); break;
    case '9': return number_9(t); break;
    }

    return NEW_VEC3;
}

// These were originally private functions used to for various helpful things regarding text

// length of string
int len(OsciText *o_text){
    int i = 0;
    while(o_text->text[i] != 0)    i++;
    return i;
}


// count spaces given some text
int count_spaces(OsciText *o_text) {
    int spaces = 0;

    for(int i = 0; i < o_text->length; ++i) {
            if(o_text->text[i] == '_' || o_text->text[i] == '\n' || o_text->text[i] == ' ' || o_text->text[i] == '.')
                spaces++;
    }
    return spaces;
}

//count the lines given some text
int count_lines(OsciText *o_text) {
    int lines = 0;

    for(int i = 0; i < o_text->length; ++i) {
            if(o_text->text[i] == '\n' || o_text->text[i] == '.') // I'm using periods as the new line character for now...
                lines++;
    }
    return lines;
}

void positions(OsciText *o_text, const char *text, int *map, int *pos, int *line, int *line_length) {
    int npos = 0;
    int npos_max = 0;
    int nline = 0;
    int i = 0;
    int j = 0;
    int l = 0;
    while (text[i] != 0) {
        if (text[i] != '.') { // periods are newline characters
            if (text[i] != '_') // underscores are space characters
                map[j++] = i;
            pos[i] = npos++;
            npos_max = MAX(npos, npos_max);
            if (npos >= npos_max)
                o_text->longest_line = nline;
        }
        else {
            o_text->line_length[l] = npos;
            l++;
            npos = 0;
            nline++;
        }
        line[i] = nline;
        i++;
    }
    o_text->line_length[l] = npos;
}

int pos(OsciText *o_text, int i) {
    return o_text->letter_pos[i];
}

int line(OsciText *o_text, int i) {
    return o_text->letter_line[i];
}

int map_text(OsciText *o_text, int i) {
    return o_text->text_map[i];
}

int longestLine(OsciText *o_text) {
    return o_text->longest_line;
}

int lineLength(OsciText *o_text, int i) {
    return o_text->line_length[i];
}

void set_horz_align(OsciText *o_text, Align alignment) {
    o_text->horz_align = alignment;
}

void set_vert_align(OsciText *o_text, vAlign alignment) {
    o_text->vert_align = alignment;
}

void setText(OsciText *o_text, const char *text){
    o_text->text = text;
    
    o_text->length = len(o_text);
    o_text->numSpaces = count_spaces(o_text);
    o_text->numLines = count_lines(o_text);
    
    // calculates the positions of all the letters
    positions(o_text, text, o_text->text_map, o_text->letter_pos, o_text->letter_line, o_text->line_length);
}

//============== Gen Function

t_vec3 gen(OsciText* o_text, t_float t, t_float line_height, t_float letter_spacing, t_float ratio)
{
    if(o_text->length == 0) return NEW_VEC3;
    t = mod1(t);
    
    t_vec3 v = NEW_VEC3;
    t_float phase = mod1(t*(o_text->length - o_text->numSpaces));
    int stair = t*(o_text->length - o_text->numSpaces) - phase;

    v = drawLetter(o_text->text[map_text(o_text, stair)], phase);

    t_float val = 0;
    switch (o_text->horz_align){
    case Default:
        v.x = v.x*ratio + pos(o_text, map_text(o_text, stair))*(ratio + letter_spacing);
        v.y = v.y - line(o_text, map_text(o_text, stair))*line_height;
        break;
    case Left:
        v.x = v.x*ratio + pos(o_text, map_text(o_text, stair))*(ratio + letter_spacing);
        v.y = v.y - line(o_text, map_text(o_text, stair))*line_height;

        val = (lineLength(o_text, longestLine(o_text) )*(ratio + letter_spacing) - letter_spacing);

        v.x *= 2.f / val;
        v.y *= 2.f / val;
        v.z *= 2.f / val;

        v.x -= 1;
        break;
    case Right:
        v.x = v.x*ratio + (pos(o_text, map_text(o_text, stair)) + lineLength(o_text, longestLine(o_text))-lineLength(o_text, line(o_text, map_text(o_text, stair))))*(ratio + letter_spacing);
        v.y = v.y - line(o_text, map_text(o_text, stair)) * line_height;

        val = (lineLength(o_text, longestLine(o_text) )*(ratio + letter_spacing) - letter_spacing);
        v.x *= 2.f / val;
        v.y *= 2.f / val;
        v.z *= 2.f / val;

        v.x -= 1;
        break;
    case Center:
        v.x = v.x*ratio + (pos(o_text, map_text(o_text, stair)) + (lineLength(o_text, longestLine(o_text)) - lineLength(o_text, line(o_text, map_text(o_text, stair))))/2)*(ratio + letter_spacing);
        v.y = v.y - line(o_text, map_text(o_text, stair))*line_height;

        val = (lineLength(o_text, longestLine(o_text))*(ratio + letter_spacing) - letter_spacing);
        v.x *= 2.f / val;
        v.y *= 2.f / val;
        v.z *= 2.f / val;

        v.x -= 1;
        break;
    }

    switch (o_text->vert_align) {
    case None:
        break;
    case Top:
        v.y += 1 - 2/(lineLength(o_text, longestLine(o_text))*(ratio + letter_spacing) - letter_spacing);
        break;
    case Bottom:
        v.y += -1 +(count_lines(o_text)-1)*line_height*2 / (lineLength(o_text, longestLine(o_text))*(ratio + letter_spacing) - letter_spacing);
        break;
    case Middle:
        v.y += ((count_lines(o_text)-1)*line_height/2-.5) * 2/(lineLength(o_text, longestLine(o_text))*(ratio + letter_spacing) - letter_spacing);
        break;
    }

    return v;
}
