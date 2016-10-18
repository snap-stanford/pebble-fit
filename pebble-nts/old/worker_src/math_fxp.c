//
// #include "math_fxp.h"
//
// #ifndef USE_FIXED_POINT
// 
//   // BASIC MATHEMATICAL FUNCTIONS
//   double cos_fxp(double x) {
//     return cos(x);
//   }
//
//   double sin_fxp(double x) {
//     return sin(x);
//   }
//
//   double tan_fxp(double x) {
//     return tan(x);
//   }
//
//   double exp_fxp(double x) {
//     // return exp(x);
//     return exp_t(x);
//   }
//
//   double log_fxp(double x) {
//     return log(x);
//   }
//
//   double sqrt_fxp(double x) {
//     return sqrt(x);
//   }
//
//   double pow_fxp(double x, double y) {
//     return pow(x,y);
//     // return pow_t(x,y);
//   }
//
//   double pow_dbl_int_fxp(double x, uint32_t y){
//     double r = 1;
//     for( uint32_t i = 0; i < y; i++){ r*= x;}
//     return r;
//   }
//
//
// #else
//   // BASIC MATHEMATICAL FUNCTIONS
//   double cos_fxp(double x) {
//     sll a = *(sll*)&x;
//     sll result = sllcos(a);
//     return *(double*)&result;
//   }
//
//   double sin_fxp(double x) {
//     sll a = *(sll*)&x;
//     sll result = sllsin(a);
//     return *(double*)&result;
//   }
//
//   double tan_fxp(double x) {
//     sll a = *(sll*)&x;
//     sll result = slltan(a);
//     return *(double*)&result;
//   }
//
//
//   double exp_fxp(double x) {
//     sll a = *(sll*)&x;
//     sll result = sllexp(a);
//     return *(double*)&result;
//   }
//
//   double exp_fxp(double x) {
//     sll a = *(sll*)&x;
//     sll result = slllog(a);
//     return *(double*)&result;
//   }
//
//   double sqrt_fxp(double x) {
//     sll a = *(sll*)&x;
//     sll result = sllsqrt(a);
//     return *(double*)&result;
//   }
//
//   double pow_fxp(double x, double y) {
//     sll a = *(sll*)&x;
//     sll b = *(sll*)&y;
//
//     sll result = sllpow(a, b);
//     return *(double*)&result;
//   }
//
//
// #endif
//
//
//
// /// SPECIAL FUNCTIONS
// //
// static __inline__ double _exp_t(double x)
// {
// 	double retval = 1;
//   retval = 1 + retval*x*(1/11);
//   retval = 1 + retval*x*(1/10);
//   retval = 1 + retval*x*(1/9);
//   retval = 1 + retval*x*(1/8);
//   retval = 1 + retval*x*(1/7);
//   retval = 1 + retval*x*(1/6);
//   retval = 1 + retval*x*(1/5);
//   retval = 1 + retval*x*(1/4);
//   retval = 1 + retval*x*(1/3);
//   retval = 1 + retval*x*(1/2);
//   retval = 1 + retval*x*(1);
// 	return retval;
// }
//
//
// /*
//  * Calculate e^x where x is arbitrary
//  */
//
// double exp_t(double x)
// {
// 	int i;
// 	double e, retval;
//
// 	e = 2.7182818284;
//
// 	/* -0.5 <= x <= 0.5  */
// 	i =  (int)(x + 0.5) ;
// 	retval = _exp_t(x - ((double)i) ) ;
//
// 	/* i >= 0 */
// 	if (i < 0) {
// 		i = -i;
// 		e = 1/(2.7182818284);
// 	}
//
// 	/* Scale the result */
// 	for (;i; i >>= 1) {
// 		if (i & 1)
// 			retval = retval*e;
// 		e = e*e;
// 	}
// 	return retval;
// }
