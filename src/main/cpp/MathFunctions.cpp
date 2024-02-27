#include "MathFunctions.h"

double MathFunctions::joystickCurve(double x, double a) {
    bool b = std::signbit(x);
    double sign = b ? -1.0 : 1.0;
    double abs_x = std::abs(x);
    return (abs_x / (1 + a * (1 - abs_x)))*sign;
}

double MathFunctions::negativeSqrt(double x) {
    bool b = std::signbit(x);
    double sign = b ? -1.0 : 1.0;
    double abs_x = std::abs(x);
    return (sqrt(abs_x))*sign;
}

double MathFunctions::lerpPoints( std::vector<double> &xData, std::vector<double> &yData, double x ) { // Assumes that xData has at least two elements, is sorted, and is increasing
   int i = 0; while ( x > xData[i] ) i++; // create a variable i and increment it until it is the index of the largest x cord under x
   return std::lerp(yData[i-1], yData[i], (x-xData[i])/(x-xData[i+1])); // use lerp w/ t = (distance from x to xLeft)/(distance from x to xRight)
}

// double MathFunctions::getPointed(std::vector<std::vector<double>>){
//     minVal = 
// }

// double MathFunctions::POVDistance(std::vector<double> origin, std::vector<double> angle, std::vector<double> target){
//     centered = 
// }