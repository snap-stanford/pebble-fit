//
// #pragma once
// #include <pebble_worker.h>
// // #include "constants_worker.h"
//
// #ifndef USE_FIXED_POINT
// 
//   #include <math.h>
//   // #define CONST_PI		3.14159265358979323846
//   #define CONST_PI		3.14159265358979323846
//   #define dbl(a) (a)
//   #define d2i(a) ((int) a)
//   #define d2tf(a) ((float) a)
//   #define d2td(a) (a)
//
// #else
//
//   // #include "softfloat_wrapper.h"
//   #include "math-sll.h"
//   // #include "math_fxp.h"
//   static __inline__ double dbl(double x) {
//     sll sval = dbl2sll(x);
//     return *(double*)&sval;
//   }
//   // softfloat "double" to  true int32
//   static __inline__ int d2i(double x) {
//     return sll2int(*(sll*)&x);
//   }
//   // softfloat "double"to float
//   static __inline__ float d2f(double x) {
//     return sll2float(*(sll*)&x);
//   }
//   // softfloat "double" to true double
//   static __inline__ double d2td(double x) {
//     return sll2dbl(*(sll*)&x);
//   }
//
// #endif
//
// double exp_t(double x);
//
// // BASIC MATHEMATICAL FUNCTIONS
// double cos_fxp(double x);
//
// double sin_fxp(double x);
//
// double tan_fxp(double x);
//
// double exp_fxp(double x);
//
// double log_fxp(double x);
//
// double sqrt_fxp(double x);
//
// double pow_fxp(double x, double y);
//
// double pow_dbl_int_fxp(double x, uint32_t y);
