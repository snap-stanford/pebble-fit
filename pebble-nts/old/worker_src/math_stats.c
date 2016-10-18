// #include "math_stats.h"
//
// // ---------------------------------------------------------
// // ---------------- START SUPPORT FUNCTIONS ----------------
// // ---------------------------------------------------------
//
// double abs_dbl(double d){
//   return (d>=0) ? d : (-d);
// }
//
// double sum_array_dbl(double d[], uint32_t dlen){
//   double sum = 0;
//   for(uint32_t i = 0; i < dlen; i++){ sum += d[i]; }
//   return sum;
// }
//
// double mean_dbl(double d[], uint32_t dlen){
//   return (sum_array_dbl(d, dlen)/dlen);
// }
//
// double var_dbl(double d[], uint32_t dlen, double mean_in){
//   double sumsqr = 0;
//   for(uint32_t i = 0; i < dlen; i++){
//     sumsqr += (d[i] - mean_in)*((d[i] - mean_in));
//   }
//   return (sumsqr/dlen);
// }
//
// double std_dbl(double d[], uint32_t dlen, double mean_in){
//   return sqrt(var_dbl(d,dlen,mean_in));
// }
//
//
// double median_torben_dbl(double m[], int n){
//   /*
//    * The following code is public domain.
//    * Algorithm by Torben Mogensen, implementation by N. Devillard.
//    * This code in public domain. Find median in-place
//    */
//     int         i, less, greater, equal;
//     double  min, max, guess, maxltguess, mingtguess;
//
//     min = max = m[0] ;
//     for (i=1 ; i<n ; i++) {
//         if (m[i]<min) min=m[i];
//         if (m[i]>max) max=m[i];
//     }
//
//     while (1) {
//         guess = (min+max)/2;
//         less = 0; greater = 0; equal = 0;
//         maxltguess = min ;
//         mingtguess = max ;
//         for (i=0; i<n; i++) {
//             if (m[i]<guess) {
//                 less++;
//                 if (m[i]>maxltguess) maxltguess = m[i] ;
//             } else if (m[i]>guess) {
//                 greater++;
//                 if (m[i]<mingtguess) mingtguess = m[i] ;
//             } else equal++;
//         }
//         if (less <= (n+1)/2 && greater <= (n+1)/2) break ;
//         else if (less>greater) max = maxltguess ;
//         else min = mingtguess;
//     }
//     if (less >= (n+1)/2) return maxltguess;
//     else if (less+equal >= (n+1)/2) return guess;
//     else return mingtguess;
// }
//
//
// // Median absolute deviation (MAD)
// double MAD_dbl(double d[], uint32_t dlen, double scl_factor, double *mean_in){
//   // if mean != NULL, then we know that we should use that
//   double mean_approx;
//   if(mean_in != NULL) mean_approx = *mean_in;
//   else mean_approx = median_torben_dbl(d, dlen);
//
//   // get an array that will be the absolute difference between the sample
//   // and the mean. We need to copy the array because we can't modify the original
//   double *d_cpy =  calloc(dlen,sizeof(double));
//   for(uint32_t i = 0; i < dlen; i++){
//     d_cpy[i] = abs_dbl(d[i] - mean_approx);
//   }
//   double result = scl_factor * median_torben_dbl(d_cpy, dlen);
//   free(d_cpy);
//   return result;
// }
//
//
// double pdf_normal_dist(double x, double mean, double std){
//   // return ( (1/(std*sqrt(2*M_PI))) * (exp(-((x-mean)*(x-mean))/(2*std*std))) );
//   // return ( (1/(std*sqrt(2*CONST_PI))) * (exp(-((x-mean)*(x-mean))/(2*std*std))) );
//   // return ( (1/(std*sqrt_fxp(2*CONST_PI))) * (exp_fxp(-((x-mean)*(x-mean))/(2*std*std))) );
//   log(1.1234);
//   return ( (1/(std*sqrt(2*CONST_PI))) * (exp_fxp(-((x-mean)*(x-mean))/(2*std*std))) );
// }
//
// // double pdf_normal_dist(double x, double mean, double std){
// //   double t = sqrt(5.5) - mean;
// //   return (1/(std*sqrt(2*CONST_PI)))* exp(t) ;
// // }
