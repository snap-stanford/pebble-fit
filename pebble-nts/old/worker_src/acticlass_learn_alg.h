// #pragma once
// #include <pebble_worker.h>
// #include "constants_worker.h"
// #include "math_fxp.h"
// #include "math_stats.h"
// // #include <math.h>
// /* >>> COMPILED against GCC on Apple Macbook Pro, Apple LLVM version 6.1.0
// * using this command string:
// * $ gcc -std=c99 learn_alg1_stepc.c softfloat_wrapper.c -fPIC -o
// * learn_alg1_stepc_C -lm -Wno-strict-aliasing -DUSE_FIXED_POINT=1 -Dfloat=double
// */
//
// // #ifdef USE_FIXED_POINT
// //   static __inline__ double dbl(double x) {
// //     sll sval = dbl2sll(x);
// //     return *(double*)&sval;
// //   }
// //   // softfloat "double" to  true int32
// //   static __inline__ int sfd2i(double x) {
// //     return sll2int(*(sll*)&x);
// //   }
// //   // softfloat "double"to float
// //   static __inline__ float sfd2f(double x) {
// //     return sll2float(*(sll*)&x);
// //   }
// //   // softfloat "double" to true double
// //   static __inline__ double sfd2td(double x) {
// //     return sll2dbl(*(sll*)&x);
// //   }
// // #else
// //   #include <math.h>
// //   #define CONST_PI		3.14159265358979323846
// //   #define dbl(a) (a)
// //   #define d2i(a) ((int) a)
// //   #define d2tf(a) ((float) a)
// //   #define d2td(a) (a)
// // #endif
//
//
// // constants and variables
// static const double MAD_SCL_FACTOR_NORMAL = 1.482602218505602;
// static const uint16_t FSMP_BUF_LEN 3
//
//
// struct FeatureSample {
//   double f[NUM_ACLF];
// };
//
// struct FeatureLearnParams {
//   double lr_mean[NUM_ACLF];
//   double lr_std[NUM_ACLF];
// };
//
// struct PdfParams {
//   double f_bnds[NUM_ACLF][2];
//   double f_mean[NUM_ACLF];
//   double f_std[NUM_ACLF];
//   // double f1_f2_cov; // assume uncorrelated errors, no covariance
// };
//
// struct PdfParamBounds {
//   double f_mean_bnds[NUM_ACLF][2];
//   double f_std_bnds[NUM_ACLF][2];
// };
//
// typedef double (*ActivityClassPdf)(struct FeatureSample fsmp, struct PdfParams params);
//
//
// struct ActivityClassProbDist {
//   enum ActivityClass class;
//   ActivityClassPdf pdf;
//   struct PdfParams params;
//   double P_max_smp; // the maximum probability any sample could have;
//   double P_trun_smp;
// };
//
// struct ActivityClassLearnPrior{
//   struct ActivityClassProbDist P_dist;
//   struct PdfParamBounds param_bnds; // the bounds of what params the distriubtion can take
//   struct FeatureLearnParams fl_params;
// };
//
//
// // OUTSIDE FUNCTIONS
//
// // ---------------------------------------------------------
// // ---------------- END SUPPORT FUNCTIONS ------------------
// // ---------------------------------------------------------
//
//
// void update_fsmp_buf(struct FeatureSample new_fsmp);
//
// void update_trained_dists();
//
// enum ActivityClass classify_feature_sample(struct FeatureSample new_fsmp);
// // this function initializes all priors and posts to the same distribution
// void init_learn_alg1_stepc_posts_ps();
