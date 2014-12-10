/*
A CORDIC implementation of sine and cosine.

INPUT:
    double theta: Input angle 
    
OUTPUT:
    double &s: Sin output
    double &c: Cos output
*/
#include "cordic.h"


void cordic(theta_type theta, cos_sin_type &s, cos_sin_type &c)
{ 
   // Please insert your code here
double X, Y, T;              // Declare temporary variables

theta_type current;
unsigned step;
X= 0.60725293510314;         // Initialize value for X =1/sqrt(1+2^-42)
Y=0;
current = 0;
for(step=0;step<20;step++)           // Angle vector goes through a set of iterations. After every iteration it is compared with the target angle theta and based on 
{                                    //comparison it is rotated clockwise or anticlockwise 
if(theta>current)
{

T=X-(Y/(double)(1ULL<<step));

Y=(X/(double)(1ULL<<step))+Y;

X = T;
current += cordic_ctab[step];
}
else
{   

T= X +(Y/(double)(1ULL<<step));

Y = -(X/(double)(1ULL<<step)) + Y;

X = T;
current -= cordic_ctab[step];
}
s = Y;
c = X;
}
}

