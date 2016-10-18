// /* this file is a simple port over from julia of the learning algorithm
// *
// *
// * >>> COMPILED against GCC on Apple Macbook Pro, Apple LLVM version 6.1.0
// * using this command string:
// * $ gcc -std=c99 sll_math_new_func_demo.c softfloat_wrapper.c -fPIC -o
// * new_func_demo -lm -Wno-strict-aliasing -DUSE_FIXED_POINT=1 -Dfloat=double
// *
// *
// */
//
// #include "acticlass_learn_alg.h"
//
// // we define the relevant elements of the algorithm, and its structure
// // 1. seperate the learning and classification decision steps
// //    this makes debugging and implementation easier, and allows for easier
// //    swapping of code. Indeed, we might want to have more than just step
// //    classification
// // 2. distributions with their classifications
// //      a. permenant learning priors
// //      b. classification distributions
// //    This implies we need some structure that is easier extensible, ie: some
// //    form that we can add new features on as we desire. Perhaps we want to make
// //    some priors to be multivariate guassians, other simple bounded by VMC.
// // 3.
//
// // Distributions data structure
// // 1. learning priors :
// //    - need a way to add arbitrary feature fields
// //    - each field needs a permanent parameter for the learning prior
// //    - each field needs a set of bounds for the learned Distributions
// //    - each field needs a learning rate parameter, permanent
// //    - also, need truncation factors for each field, for when we set the P to
// //      zero to prevent long term effects.
// //    -> sounds like each field itself needs a structure.
// // 2. learned Distributions
// //    - need a way to add arbitrary feature fields
// //    - need the current parameters of that field, and thats it.
// //    - note, while we assume guassians now, we note that some fields might need
// //      two parameters (guassians, univariate bounds), others might need one,
// //      and still others might need more than one.
//
// static struct FeatureSample fsmp_buf[FSMP_BUF_LEN];
//
//
// // ---------------------------------------------------------
// // ---------------- END SUPPORT FUNCTIONS ------------------
// // ---------------------------------------------------------
// // ----------------- START PDF FUNCTIONS -------------------
// // ---------------------------------------------------------
//
//
// double f1_bnd_f2_normal_dist(struct FeatureSample fsmp, struct PdfParams params){
//   if( (params.f_bnds[0][0] <= fsmp.f[0] )&& (fsmp.f[0] <= params.f_bnds[0][1])){
//     // get the Mahalanobis distance.
//     double mah_D = ( fsmp.f[1] - params.f_mean[1])/params.f_std[1];
//     // return the probability that it is a given standard deviation away
//     return pdf_normal_dist(mah_D, 0, 1);
//   }else{
//     // if the sample is not in the vmc bin, return 0
//     return 0;
//   }
// }
//
// double f1_f2_0cov_bi_normal_dist(struct FeatureSample fsmp, struct PdfParams params){
//   // to be filled in
//   return 1.0;
// }
//
//
// // double biv_normal_dist_pdf(double x1, double x2, double m1, double m2, double sd1, double sd2, double cv12 ){
// //   // This function simply
// //   double cor12 = cv12/( sd1*sd2);
// //
// //   double A = 1/(2*CONST_PI*sqrt( 1 - pow(cor12,2) ));
// //   double B = dbl(-1.0)/(2*( 1 - pow(cor12,2) ));
// //   double C1 = pow((x1-m1)/sd1,2);
// //   double C2 = pow((x2-m2)/sd2,2);
// //   double C3 = (2*cor12*(x1-m1)*(x2-m2))/(sd1*sd2);
// //   return A*exp(B*(C1+C2-C3));
// // }
//
//
// // ---------------------------------------------------------
// // ------------------ END PDF FUNCTIONS --------------------
// // ---------------------------------------------------------
// // ---------------------------------------------------------
// // --------------- START CONVIENCE FUNCTIONS ---------------
// // ---------------------------------------------------------
// // we need to have a method that read and updates the post persistant storage
// // at each interaction. AND, we dont want to read the parameters of the post
// // and prior into storage except as needed, cause otherwise they take up too
// // much RAM space, even with the worker available
//
//
//
// // NOTE!!!, these are only acting as the backing structures, they will be
// // replaced with persistant storage in the actual PK implementation
//
// struct ActivityClassLearnPrior get_activityclass_prior(uint16_t ac_i){
//   // hard code the priors here
//   // NOTE : we only define the parameters that are used, otherwise undefined
//   // they are assumed to be 0
//   // struct ActivityClassLearnPrior aclp = {0};
//   struct ActivityClassLearnPrior aclp;
//
//   switch(ac_i){
//     case NO_ACTICLASS :
//     case SLOW_WALK :
//
//       // initialize the priors probability dist
//       aclp.P_dist.class = SLOW_WALK;
//       aclp.P_dist.pdf = f1_bnd_f2_normal_dist;
//       aclp.P_dist.params.f_bnds[0][0] = dbl(200); // = acpd.params.f_bnds
//       aclp.P_dist.params.f_bnds[0][1] = dbl(500);
//       // aclp.P_dist.params.f_bnds[1][0] = dbl(0);
//       // aclp.P_dist.params.f_bnds[1][1] = dbl(0);
//       // aclp.P_dist.params.f_mean[0] = dbl(0);
//       aclp.P_dist.params.f_mean[1] = dbl(2500);
//       // aclp.P_dist.params.f_std[0] = dbl(0);
//       aclp.P_dist.params.f_std[1] = dbl(750);
//
//       // adjust the acceptance conditions
//       aclp.P_dist.P_max_smp = dbl(0.4);
//       aclp.P_dist.P_trun_smp = dbl(0.05);
//
//       // set the learning limits
//       // aclp.param_bnds.f_mean_bnds[0][0] = dbl(0);
//       // aclp.param_bnds.f_mean_bnds[0][1] = dbl(0);
//       aclp.param_bnds.f_mean_bnds[1][0] = dbl(1000);
//       aclp.param_bnds.f_mean_bnds[1][1] = dbl(4000);
//       // aclp.param_bnds.f_std_bnds[0][0] = dbl(0);
//       // aclp.param_bnds.f_std_bnds[0][1] = dbl(0);
//       aclp.param_bnds.f_std_bnds[1][0] = dbl(300);
//       aclp.param_bnds.f_std_bnds[1][1] = dbl(900);
//       // aclp.fl_params.lr_mean[0] = dbl(0);
//       aclp.fl_params.lr_mean[1] = dbl(0.1);
//       // aclp.fl_params.lr_std[0] = dbl(0);
//       aclp.fl_params.lr_std[1] = dbl(0.1);
//
//     case WALK :
//       // initialize the priors probability dist
//       aclp.P_dist.class = WALK;
//       aclp.P_dist.pdf = f1_bnd_f2_normal_dist;
//       aclp.P_dist.params.f_bnds[0][0] = dbl(400);
//       aclp.P_dist.params.f_bnds[0][1] = dbl(1500);
//       // aclp.P_dist.params.f_bnds[1][0] = dbl(0);
//       // aclp.P_dist.params.f_bnds[1][1] = dbl(0);
//       // aclp.P_dist.params.f_mean[0] = dbl(0);
//       aclp.P_dist.params.f_mean[1] = dbl(2500);
//       // aclp.P_dist.params.f_std[0] = dbl(0);
//       aclp.P_dist.params.f_std[1] = dbl(750);
//
//       // set the
//       aclp.P_dist.P_max_smp = dbl(0.4);
//       aclp.P_dist.P_trun_smp = dbl(0.05);
//
//       // set the learning limits
//       // aclp.param_bnds.f_mean_bnds[0][0] = dbl(0);
//       // aclp.param_bnds.f_mean_bnds[0][1] = dbl(0);
//       aclp.param_bnds.f_mean_bnds[1][0] = dbl(1000);
//       aclp.param_bnds.f_mean_bnds[1][1] = dbl(4000);
//       // aclp.param_bnds.f_std_bnds[0][0] = dbl(0);
//       // aclp.param_bnds.f_std_bnds[0][1] = dbl(0);
//       aclp.param_bnds.f_std_bnds[1][0] = dbl(300);
//       aclp.param_bnds.f_std_bnds[1][1] = dbl(900);
//       // aclp.fl_params.lr_mean[0] = dbl(0);
//       aclp.fl_params.lr_mean[1] = dbl(0.5);
//       // aclp.fl_params.lr_std[0] = dbl(0);
//       aclp.fl_params.lr_std[1] = dbl(0.5);
//
//     case FAST_WALK :
//       aclp.P_dist.class = FAST_WALK;
//       aclp.P_dist.pdf = f1_bnd_f2_normal_dist;
//       aclp.P_dist.params.f_bnds[0][0] = dbl(1300);
//       aclp.P_dist.params.f_bnds[0][1] = dbl(2500);
//       // aclp.P_dist.params.f_bnds[1][0] = dbl(0);
//       // aclp.P_dist.params.f_bnds[1][1] = dbl(0);
//       // aclp.P_dist.params.f_mean[0] = dbl(0);
//       aclp.P_dist.params.f_mean[1] = dbl(2500);
//       // aclp.P_dist.params.f_std[0] = dbl(0);
//       aclp.P_dist.params.f_std[1] = dbl(750);
//
//       aclp.P_dist.P_max_smp = dbl(0.4);
//       aclp.P_dist.P_trun_smp = dbl(0.05);
//
//       // aclp.param_bnds.f_mean_bnds[0][0] = dbl(0);
//       // aclp.param_bnds.f_mean_bnds[0][1] = dbl(0);
//       aclp.param_bnds.f_mean_bnds[1][0] = dbl(1000);
//       aclp.param_bnds.f_mean_bnds[1][1] = dbl(4000);
//       // aclp.param_bnds.f_std_bnds[0][0] = dbl(0);
//       // aclp.param_bnds.f_std_bnds[0][1] = dbl(0);
//       aclp.param_bnds.f_std_bnds[1][0] = dbl(300);
//       aclp.param_bnds.f_std_bnds[1][1] = dbl(900);
//       // aclp.fl_params.lr_mean[0] = dbl(0);
//       aclp.fl_params.lr_mean[1] = dbl(0.1);
//       // aclp.fl_params.lr_std[0] = dbl(0);
//       aclp.fl_params.lr_std[1] = dbl(0.1);
//
//     case RUN :
//       aclp.P_dist.class = RUN;
//       aclp.P_dist.pdf = f1_bnd_f2_normal_dist;
//       aclp.P_dist.params.f_bnds[0][0] = dbl(2300);
//       aclp.P_dist.params.f_bnds[0][1] = dbl(5000);
//       // aclp.P_dist.params.f_bnds[1][0] = dbl(0);
//       // aclp.P_dist.params.f_bnds[1][1] = dbl(0);
//       // aclp.P_dist.params.f_mean[0] = dbl(0);
//       aclp.P_dist.params.f_mean[1] = dbl(2500);
//       // aclp.P_dist.params.f_std[0] = dbl(0);
//       aclp.P_dist.params.f_std[1] = dbl(750);
//
//       aclp.P_dist.P_max_smp = dbl(0.4);
//       aclp.P_dist.P_trun_smp = dbl(0.05);
//
//       // aclp.param_bnds.f_mean_bnds[0][0] = dbl(0);
//       // aclp.param_bnds.f_mean_bnds[0][1] = dbl(0);
//       aclp.param_bnds.f_mean_bnds[1][0] = dbl(1000);
//       aclp.param_bnds.f_mean_bnds[1][1] = dbl(4000);
//       // aclp.param_bnds.f_std_bnds[0][0] = dbl(0);
//       // aclp.param_bnds.f_std_bnds[0][1] = dbl(0);
//       aclp.param_bnds.f_std_bnds[1][0] = dbl(300);
//       aclp.param_bnds.f_std_bnds[1][1] = dbl(900);
//       // aclp.fl_params.lr_mean[0] = dbl(0);
//       aclp.fl_params.lr_mean[1] = dbl(0.1);
//       // aclp.fl_params.lr_std[0] = dbl(0);
//       aclp.fl_params.lr_std[1] = dbl(0.1);
//
//     case FAST_RUN :
//       aclp.P_dist.class = FAST_RUN;
//       aclp.P_dist.pdf = f1_bnd_f2_normal_dist;
//       aclp.P_dist.params.f_bnds[0][0] = dbl(4500);
//       aclp.P_dist.params.f_bnds[0][1] = dbl(10000);
//       // aclp.P_dist.params.f_bnds[1][0] = dbl(0);
//       // aclp.P_dist.params.f_bnds[1][1] = dbl(0);
//       // aclp.P_dist.params.f_mean[0] = dbl(0);
//       aclp.P_dist.params.f_mean[1] = dbl(2500);
//       // aclp.P_dist.params.f_std[0] = dbl(0);
//       aclp.P_dist.params.f_std[1] = dbl(750);
//
//       aclp.P_dist.P_max_smp = dbl(0.4);
//       aclp.P_dist.P_trun_smp = dbl(0.05);
//
//       // aclp.param_bnds.f_mean_bnds[0][0] = dbl(0);
//       // aclp.param_bnds.f_mean_bnds[0][1] = dbl(0);
//       aclp.param_bnds.f_mean_bnds[1][0] = dbl(1000);
//       aclp.param_bnds.f_mean_bnds[1][1] = dbl(4000);
//       // aclp.param_bnds.f_std_bnds[0][0] = dbl(0);
//       // aclp.param_bnds.f_std_bnds[0][1] = dbl(0);
//       aclp.param_bnds.f_std_bnds[1][0] = dbl(300);
//       aclp.param_bnds.f_std_bnds[1][1] = dbl(900);
//       // aclp.fl_params.lr_mean[0] = dbl(0);
//       aclp.fl_params.lr_mean[1] = dbl(0.1);
//       // aclp.fl_params.lr_std[0] = dbl(0);
//       aclp.fl_params.lr_std[1] = dbl(0.1);
//     default :
//       APP_LOG(APP_LOG_LEVEL_ERROR, "acticlass_learn_alg1: no such acti class");
//   }
//   return aclp;
// }
//
//
// struct ActivityClassProbDist get_activityclass_post(uint16_t ac_i){
//   // get a simple
//   // to do this, we simply get the f_bnds from the priors, and the
//   // hardcoded
//   struct acticlass_learn_alg_state aclas;
//   persist_read_data(ACTICLASS_LEARN_ALG_STATE_PERSIST_KEY, &aclas, sizeof(aclas));
//
//   // struct ActivityClassLearnPrior prior = get_activityclass_prior(ac_i);
//   struct ActivityClassProbDist acpd;
//
//   switch(ac_i){
//     case NO_ACTICLASS :
//     case SLOW_WALK :
//       acpd.class = SLOW_WALK;
//       acpd.pdf = f1_bnd_f2_normal_dist;
//       // acpd.params.f_bnds = prior.P_dist.params.f_bnds;
//       acpd.params.f_bnds[0][0] = dbl(200);
//       acpd.params.f_bnds[0][1] = dbl(500);
//       acpd.params.f_mean[1] = dbl(aclas.f_mean[SLOW_WALK][1]);
//       acpd.params.f_std[1] = dbl(aclas.f_std[SLOW_WALK][1]);
//       acpd.P_max_smp = dbl(0.4);
//       acpd.P_trun_smp = dbl(0.1);
//     case WALK :
//       acpd.class = WALK;
//       acpd.pdf = f1_bnd_f2_normal_dist;
//       // acpd.params.f_bnds = prior.P_dist.params.f_bnds;
//       acpd.params.f_bnds[0][0] = dbl(400);
//       acpd.params.f_bnds[0][1] = dbl(1500);
//       acpd.params.f_mean[1] = dbl(aclas.f_mean[WALK][1]);
//       acpd.params.f_std[1] = dbl(aclas.f_std[WALK][1]);
//       acpd.P_max_smp = dbl(0.4);
//       acpd.P_trun_smp = dbl(0.1);
//     case FAST_WALK :
//       acpd.class = FAST_WALK;
//       acpd.pdf = f1_bnd_f2_normal_dist;
//       // acpd.params.f_bnds = prior.P_dist.params.f_bnds;
//       acpd.params.f_bnds[0][0] = dbl(1300);
//       acpd.params.f_bnds[0][1] = dbl(2500);
//       acpd.params.f_mean[1] = dbl(aclas.f_mean[FAST_WALK][1]);
//       acpd.params.f_std[1] = dbl(aclas.f_std[FAST_WALK][1]);
//       acpd.P_max_smp = dbl(0.4);
//       acpd.P_trun_smp = dbl(0.1);
//     case RUN :
//       acpd.class = RUN;
//       acpd.pdf = f1_bnd_f2_normal_dist;
//       // acpd.params.f_bnds = prior.P_dist.params.f_bnds;
//       acpd.params.f_bnds[0][0] = dbl(2300);
//       acpd.params.f_bnds[0][1] = dbl(5000);
//       acpd.params.f_mean[1] = dbl(aclas.f_mean[RUN][1]);
//       acpd.params.f_std[1] = dbl(aclas.f_std[RUN][1]);
//       acpd.P_max_smp = dbl(0.4);
//       acpd.P_trun_smp = dbl(0.1);
//     case FAST_RUN :
//       acpd.class = FAST_RUN;
//       acpd.pdf = f1_bnd_f2_normal_dist;
//       // acpd.params.f_bnds = prior.P_dist.params.f_bnds;
//       acpd.params.f_bnds[0][0] = dbl(4500);
//       acpd.params.f_bnds[0][1] = dbl(10000);
//       acpd.params.f_mean[1] = dbl(aclas.f_mean[FAST_RUN][1]);
//       acpd.params.f_std[1] = dbl(aclas.f_std[FAST_RUN][1]);
//       acpd.P_max_smp = dbl(0.4);
//       acpd.P_trun_smp = dbl(0.1);
//     default :
//       APP_LOG(APP_LOG_LEVEL_ERROR, "acticlass_learn_alg1: no such acti class");
//   }
//
//   // return the ActivityClassProbDist
//   return acpd;
// }
//
// void set_activityclass_f_params(uint16_t ac_i, struct PdfParams params){
//   struct acticlass_learn_alg_state aclas;
//   persist_read_data(ACTICLASS_LEARN_ALG_STATE_PERSIST_KEY, &aclas, sizeof(aclas));
//
//   for(int16_t i = 0; i < NUM_ACTICLASS; i++){
//     aclas.f_mean[ac_i][i] = (uint16_t) d2i(params.f_mean[i]);
//     aclas.f_std[ac_i][i] = (uint16_t) d2i(params.f_std[i]);
//   }
//   persist_write_data(ACTICLASS_LEARN_ALG_STATE_PERSIST_KEY, &aclas, sizeof(aclas));
// }
//
//
// // hard code initializing the post dists
// void init_learn_alg1_stepc_posts_ps(){
//   // this simply iterates over the priors and sets the initial mean and STD
//   // of each posterior to be the same as the prior
//   struct ActivityClassLearnPrior prior;
//
//   for(int16_t i = 0; i < NUM_ACLF; i++ ){
//     prior = get_activityclass_prior(i);
//     set_activityclass_f_params(i,prior.P_dist.params);
//   }
// }
//
//
// // ---------------------------------------------------------
// // ---------------- END CONVIENCE FUNCTIONS ----------------
// // ---------------------------------------------------------
//
//
//
// void update_fsmp_buf(struct FeatureSample new_fsmp){
//   // push the elements of the array along .
//   for(int16_t i = 0; i < (FSMP_BUF_LEN - 1); i++){
//     fsmp_buf[i] = fsmp_buf[i+1];
//   }
//   // append the new sample to the end of the array
//   fsmp_buf[FSMP_BUF_LEN - 1] = new_fsmp;
// }
//
// void update_trained_dists(){
//   // over each prior, adjust each post
//   double P_buf_prior[NUM_ACTICLASS];
//   double P_buf, scl_alpha, test_f_mean, test_f_std;
//   double f_ary[FSMP_BUF_LEN];
//   struct ActivityClassLearnPrior prior;
//   struct ActivityClassProbDist post;
//
//   // we start at 1 to avoid the NO_ACTIVITY_CLASS == 0 case
//   for(int16_t i = SLOW_WALK; i < NUM_ACTICLASS; i++ ){
//     prior = get_activityclass_prior(i);
//     post = get_activityclass_post(i);
//
//     P_buf = 1;
//     // see the multiplicative probabiity that all elments of the current
//     // FeatureSample buffer are drawn from the given prior, i
//     for(int16_t j = 0; j < FSMP_BUF_LEN; j++){
//       P_buf = P_buf*(prior.P_dist.pdf(fsmp_buf[j],prior.P_dist.params));
//     }
//     // rescale the probability
//     // P_buf = P_buf/pow_fxp(prior.P_dist.P_max_smp, FSMP_BUF_LEN);
//     P_buf = P_buf/pow_dbl_int_fxp(prior.P_dist.P_max_smp, (uint32_t)FSMP_BUF_LEN);
//
//     P_buf_prior[i] = (P_buf > prior.P_dist.P_trun_smp) ? P_buf : 0;
//
//     // now, we get the new estimates for the learned dists' parameters
//     // NOTE: we learn *all* the params, and let the pdf and learning rates
//     // determine which parameters actually matter
//
//     // iterate over all features.
//     for(int16_t k = 0; k < NUM_ACLF; k++){
//       // extract an array with only tha elements from the given feature
//       for(int16_t l = 0; l< FSMP_BUF_LEN; l++){
//         f_ary[l] = fsmp_buf[l].f[k];
//       }
//       // LEARN MEAN
//       // get the new scaled learning parameter for mean
//       scl_alpha = prior.fl_params.lr_mean[k] * P_buf_prior[i];
//         // printf(":sa: %f ", scl_alpha);
//       // get the test learned mean
//       test_f_mean = ((1 - scl_alpha) * post.params.f_mean[k])
//         + (scl_alpha * mean_dbl(f_ary,FSMP_BUF_LEN) );
//       // if within bounds, update the learned mean
//         // printf(":tm: %f ", test_f_mean);
//       if((prior.param_bnds.f_mean_bnds[k][0]<= test_f_mean) &&
//         (test_f_mean <= prior.param_bnds.f_mean_bnds[k][1])){
//         post.params.f_mean[k] = test_f_mean;
//       }
//
//       // LEARN STD
//       // get the new scaled learning parameter for std
//       scl_alpha = prior.fl_params.lr_std[k] * P_buf_prior[i];
//       // get test learned std
//       test_f_std = ((1 - scl_alpha) * post.params.f_std[k])
//         + (scl_alpha *  MAD_dbl(f_ary,FSMP_BUF_LEN, MAD_SCL_FACTOR_NORMAL,
//                                   &(post.params.f_mean[k])) );
//       // if within bounds, update the learned std
//       if((prior.param_bnds.f_std_bnds[k][0]<= test_f_std) &&
//         (test_f_std <= prior.param_bnds.f_std_bnds[k][1])){
//         post.params.f_std[k] = test_f_std;
//       }
//       // set_activityclass_f_mean_std(i,k,post.params.f_mean[k],post.params.f_std[k]);
//       // set_activityclass_f_std(i,k,post.params.f_std[k]);
//       set_activityclass_f_params(i,post.params);
//     }
//     // END feature learn loop
//   }
//   // END learn pairs loop
// }
//
// enum ActivityClass classify_feature_sample(struct FeatureSample new_fsmp){
//
//   struct ActivityClassProbDist post;
//   double P_test = 0;
//   double P_max = 0;
//   int16_t P_max_i = 0;
//
//   for(int16_t i = 0; i < NUM_ACTICLASS; i++ ){
//     post = get_activityclass_post(i);
//
//     P_test = post.pdf(new_fsmp, post.params);
//     // if it is bigger than the truncation, we consider it part of the dists
//     // Then, we want to see which distribution it is most likely to be a part of
//     if((P_test > post.P_trun_smp) && (P_test > P_max)){
//       P_max = P_test;
//       P_max_i = i;
//     }
//   }
//   // if there is a distribution which this makes the non-zero truncation cutoff
//   // then we return the "best" distribution
//   printf("P_max: %f : P_max_i %d : P_max_i.f2mean %f : P_max_i.f2std %f\n ",
//     P_max,P_max_i, get_activityclass_post(P_max_i).params.f_mean[1],
//     get_activityclass_post(P_max_i).params.f_std[1]);
//
//   if(P_max > 0 ){
//     return get_activityclass_post(P_max_i).class;
//   }else{
//     return NO_ACTICLASS;
//   }
// }
//
//
// //
// // // this function initializes all priors and posts to the same distribution
// // void init_priors(){
// //
// //
// //   if(N_LEARN_PAIRS == 1){
// //     // initialize the probability distributions
// //
// //     // WALK
// //     class_learn_posts[0].class = WALK;
// //     class_learn_posts[0].pdf = f1_bnd_f2_normal_dist;
// //     class_learn_posts[0].params.f_bnds[0][0] = 400;
// //     class_learn_posts[0].params.f_bnds[0][1] = 1500;
// //     class_learn_posts[0].params.f_bnds[1][0] = 0;
// //     class_learn_posts[0].params.f_bnds[1][1] = 0;
// //     class_learn_posts[0].params.f_mean[0] = 0;
// //     class_learn_posts[0].params.f_mean[1] = 2500;
// //     class_learn_posts[0].params.f_std[0] = 0;
// //     class_learn_posts[0].params.f_std[1] = 750;
// //     class_learn_posts[0].P_max_smp = 0.4;
// //     class_learn_posts[0].P_trun_smp = 0.1;
// //
// //
// //     // initialize all the learning prior distributions
// //     // WALK
// //     memcpy(&(class_learn_priors[0].P_dist),&(class_learn_posts[0]) ,sizeof(struct ActivityClassProbDist));
// //     class_learn_priors[0].P_dist.P_trun_smp = 0.05;
// //     class_learn_priors[0].param_bnds.f_mean_bnds[0][0] = 0;
// //     class_learn_priors[0].param_bnds.f_mean_bnds[0][1] = 0;
// //     class_learn_priors[0].param_bnds.f_mean_bnds[1][0] = 1000;
// //     class_learn_priors[0].param_bnds.f_mean_bnds[1][1] = 4000;
// //     class_learn_priors[0].param_bnds.f_std_bnds[0][0] = 0;
// //     class_learn_priors[0].param_bnds.f_std_bnds[0][1] = 0;
// //     class_learn_priors[0].param_bnds.f_std_bnds[1][0] = 300;
// //     class_learn_priors[0].param_bnds.f_std_bnds[1][1] = 900;
// //     class_learn_priors[0].fl_params.lr_mean[0] = 0;
// //     class_learn_priors[0].fl_params.lr_mean[1] = 0.1;
// //     class_learn_priors[0].fl_params.lr_std[0] = 0;
// //     class_learn_priors[0].fl_params.lr_std[1] = 0.1;
// //
// //   }else if(N_LEARN_PAIRS == 5){
// //     // SLOW_WALK
// //     class_learn_posts[0].class = SLOW_WALK;
// //     class_learn_posts[0].pdf = f1_bnd_f2_normal_dist;
// //     class_learn_posts[0].params.f_bnds[0][0] = 200;
// //     class_learn_posts[0].params.f_bnds[0][1] = 500;
// //     class_learn_posts[0].params.f_bnds[1][0] = 0;
// //     class_learn_posts[0].params.f_bnds[1][1] = 0;
// //     class_learn_posts[0].params.f_mean[0] = 0;
// //     class_learn_posts[0].params.f_mean[1] = 2500;
// //     class_learn_posts[0].params.f_std[0] = 0;
// //     class_learn_posts[0].params.f_std[1] = 750;
// //     class_learn_posts[0].P_max_smp = 0.4;
// //     class_learn_posts[0].P_trun_smp = 0.1;
// //
// //     // WALK
// //     class_learn_posts[1].class = WALK;
// //     class_learn_posts[1].pdf = f1_bnd_f2_normal_dist;
// //     class_learn_posts[1].params.f_bnds[0][0] = 400;
// //     class_learn_posts[1].params.f_bnds[0][1] = 1500;
// //     class_learn_posts[1].params.f_bnds[1][0] = 0;
// //     class_learn_posts[1].params.f_bnds[1][1] = 0;
// //     class_learn_posts[1].params.f_mean[0] = 0;
// //     class_learn_posts[1].params.f_mean[1] = 2500;
// //     class_learn_posts[1].params.f_std[0] = 0;
// //     class_learn_posts[1].params.f_std[1] = 750;
// //     class_learn_posts[1].P_max_smp = 0.4;
// //     class_learn_posts[1].P_trun_smp = 0.1;
// //
// //     // FAST_WALK
// //     class_learn_posts[2].class = FAST_WALK;
// //     class_learn_posts[2].pdf = f1_bnd_f2_normal_dist;
// //     class_learn_posts[2].params.f_bnds[0][0] = 1300;
// //     class_learn_posts[2].params.f_bnds[0][1] = 2500;
// //     class_learn_posts[2].params.f_bnds[1][0] = 0;
// //     class_learn_posts[2].params.f_bnds[1][1] = 0;
// //     class_learn_posts[2].params.f_mean[0] = 0;
// //     class_learn_posts[2].params.f_mean[1] = 2500;
// //     class_learn_posts[2].params.f_std[0] = 0;
// //     class_learn_posts[2].params.f_std[1] = 750;
// //     class_learn_posts[2].P_max_smp = 0.4;
// //     class_learn_posts[2].P_trun_smp = 0.1;
// //
// //     // RUN
// //     class_learn_posts[3].class = RUN;
// //     class_learn_posts[3].pdf = f1_bnd_f2_normal_dist;
// //     class_learn_posts[3].params.f_bnds[0][0] = 2300;
// //     class_learn_posts[3].params.f_bnds[0][1] = 5000;
// //     class_learn_posts[3].params.f_bnds[1][0] = 0;
// //     class_learn_posts[3].params.f_bnds[1][1] = 0;
// //     class_learn_posts[3].params.f_mean[0] = 0;
// //     class_learn_posts[3].params.f_mean[1] = 2500;
// //     class_learn_posts[3].params.f_std[0] = 0;
// //     class_learn_posts[3].params.f_std[1] = 750;
// //     class_learn_posts[3].P_max_smp = 0.4;
// //     class_learn_posts[3].P_trun_smp = 0.1;
// //
// //     // FAST_RUN
// //     class_learn_posts[4].class = FAST_RUN;
// //     class_learn_posts[4].pdf = f1_bnd_f2_normal_dist;
// //     class_learn_posts[4].params.f_bnds[0][0] = 4500;
// //     class_learn_posts[4].params.f_bnds[0][1] = 10000;
// //     class_learn_posts[4].params.f_bnds[1][0] = 0;
// //     class_learn_posts[4].params.f_bnds[1][1] = 0;
// //     class_learn_posts[4].params.f_mean[0] = 0;
// //     class_learn_posts[4].params.f_mean[1] = 2500;
// //     class_learn_posts[4].params.f_std[0] = 0;
// //     class_learn_posts[4].params.f_std[1] = 750;
// //     class_learn_posts[4].P_max_smp = 0.4;
// //     class_learn_posts[4].P_trun_smp = 0.1;
// //
// //     // --------------------------------------------
// //     // initialize all the learning prior distributions
// //     // SLOW_WALK
// //     memcpy(&(class_learn_priors[0].P_dist),&(class_learn_posts[0]) ,sizeof(struct ActivityClassProbDist));
// //     class_learn_priors[0].P_dist.P_trun_smp = 0.05;
// //     class_learn_priors[0].param_bnds.f_mean_bnds[0][0] = 0;
// //     class_learn_priors[0].param_bnds.f_mean_bnds[0][1] = 0;
// //     class_learn_priors[0].param_bnds.f_mean_bnds[1][0] = 1000;
// //     class_learn_priors[0].param_bnds.f_mean_bnds[1][1] = 4000;
// //     class_learn_priors[0].param_bnds.f_std_bnds[0][0] = 0;
// //     class_learn_priors[0].param_bnds.f_std_bnds[0][1] = 0;
// //     class_learn_priors[0].param_bnds.f_std_bnds[1][0] = 300;
// //     class_learn_priors[0].param_bnds.f_std_bnds[1][1] = 900;
// //     class_learn_priors[0].fl_params.lr_mean[0] = 0;
// //     class_learn_priors[0].fl_params.lr_mean[1] = 0.1;
// //     class_learn_priors[0].fl_params.lr_std[0] = 0;
// //     class_learn_priors[0].fl_params.lr_std[1] = 0.1;
// //
// //     // WALK
// //     memcpy(&(class_learn_priors[1].P_dist),&(class_learn_posts[1]) ,sizeof(struct ActivityClassProbDist));
// //     class_learn_priors[1].P_dist.P_trun_smp = 0.05;
// //     class_learn_priors[1].param_bnds.f_mean_bnds[0][0] = 0;
// //     class_learn_priors[1].param_bnds.f_mean_bnds[0][1] = 0;
// //     class_learn_priors[1].param_bnds.f_mean_bnds[1][0] = 1000;
// //     class_learn_priors[1].param_bnds.f_mean_bnds[1][1] = 4000;
// //     class_learn_priors[1].param_bnds.f_std_bnds[0][0] = 0;
// //     class_learn_priors[1].param_bnds.f_std_bnds[0][1] = 0;
// //     class_learn_priors[1].param_bnds.f_std_bnds[1][0] = 300;
// //     class_learn_priors[1].param_bnds.f_std_bnds[1][1] = 900;
// //     class_learn_priors[1].fl_params.lr_mean[0] = 0;
// //     class_learn_priors[1].fl_params.lr_mean[1] = 0.5;
// //     class_learn_priors[1].fl_params.lr_std[0] = 0;
// //     class_learn_priors[1].fl_params.lr_std[1] = 0.5;
// //
// //     // FAST_WALK
// //     memcpy(&(class_learn_priors[2].P_dist),&(class_learn_posts[2]) ,sizeof(struct ActivityClassProbDist));
// //     class_learn_priors[2].P_dist.P_trun_smp = 0.05;
// //     class_learn_priors[2].param_bnds.f_mean_bnds[0][0] = 0;
// //     class_learn_priors[2].param_bnds.f_mean_bnds[0][1] = 0;
// //     class_learn_priors[2].param_bnds.f_mean_bnds[1][0] = 1000;
// //     class_learn_priors[2].param_bnds.f_mean_bnds[1][1] = 4000;
// //     class_learn_priors[2].param_bnds.f_std_bnds[0][0] = 0;
// //     class_learn_priors[2].param_bnds.f_std_bnds[0][1] = 0;
// //     class_learn_priors[2].param_bnds.f_std_bnds[1][0] = 300;
// //     class_learn_priors[2].param_bnds.f_std_bnds[1][1] = 900;
// //     class_learn_priors[2].fl_params.lr_mean[0] = 0;
// //     class_learn_priors[2].fl_params.lr_mean[1] = 0.1;
// //     class_learn_priors[2].fl_params.lr_std[0] = 0;
// //     class_learn_priors[2].fl_params.lr_std[1] = 0.1;
// //
// //     // RUN
// //     memcpy(&(class_learn_priors[3].P_dist),&(class_learn_posts[3]) ,sizeof(struct ActivityClassProbDist));
// //     class_learn_priors[3].P_dist.P_trun_smp = 0.05;
// //     class_learn_priors[3].param_bnds.f_mean_bnds[0][0] = 0;
// //     class_learn_priors[3].param_bnds.f_mean_bnds[0][1] = 0;
// //     class_learn_priors[3].param_bnds.f_mean_bnds[1][0] = 1000;
// //     class_learn_priors[3].param_bnds.f_mean_bnds[1][1] = 4000;
// //     class_learn_priors[3].param_bnds.f_std_bnds[0][0] = 0;
// //     class_learn_priors[3].param_bnds.f_std_bnds[0][1] = 0;
// //     class_learn_priors[3].param_bnds.f_std_bnds[1][0] = 300;
// //     class_learn_priors[3].param_bnds.f_std_bnds[1][1] = 900;
// //     class_learn_priors[3].fl_params.lr_mean[0] = 0;
// //     class_learn_priors[3].fl_params.lr_mean[1] = 0.1;
// //     class_learn_priors[3].fl_params.lr_std[0] = 0;
// //     class_learn_priors[3].fl_params.lr_std[1] = 0.1;
// //
// //     // FAST_RUN
// //     memcpy(&(class_learn_priors[4].P_dist),&(class_learn_posts[4]) ,sizeof(struct ActivityClassProbDist));
// //     class_learn_priors[4].P_dist.P_trun_smp = 0.05;
// //     class_learn_priors[4].param_bnds.f_mean_bnds[0][0] = 0;
// //     class_learn_priors[4].param_bnds.f_mean_bnds[0][1] = 0;
// //     class_learn_priors[4].param_bnds.f_mean_bnds[1][0] = 1000;
// //     class_learn_priors[4].param_bnds.f_mean_bnds[1][1] = 4000;
// //     class_learn_priors[4].param_bnds.f_std_bnds[0][0] = 0;
// //     class_learn_priors[4].param_bnds.f_std_bnds[0][1] = 0;
// //     class_learn_priors[4].param_bnds.f_std_bnds[1][0] = 300;
// //     class_learn_priors[4].param_bnds.f_std_bnds[1][1] = 900;
// //     class_learn_priors[4].fl_params.lr_mean[0] = 0;
// //     class_learn_priors[4].fl_params.lr_mean[1] = 0.1;
// //     class_learn_priors[4].fl_params.lr_std[0] = 0;
// //     class_learn_priors[4].fl_params.lr_std[1] = 0.1;
// //   }
// // }
//
//
//
//
// // int main( int argc, char *argv[]){
// //
// //   // FeatureSample P_max_smp = {prior->P_dist.params.f1_mean, prior->P_dist.params.f2_mean};
// //   // double P_buf_prior_scale = pow(prior->P_dist.pdf(P_max_smp, prior->P_dist.params),FSMP_BUF_LEN);
// //
// //   printf("P: %f \n", pdf_normal_dist(atof(argv[1]),atof(argv[2]),atof(argv[3])));
// //
// //   return 0;
// // }
//
//
//
// //
// //
// // void test_struct_comm(uint32_t dlen, double f1[dlen], double f2[dlen],
// //     struct PdfParams *pPP,struct PdfParamBounds *pPPB,
// //     struct FeatureLearnParams *pFLP,
// //     double test[sizeof(struct FeatureSample)
// //       + sizeof(struct PdfParams) + sizeof(struct PdfParamBounds)
// //       + sizeof(struct FeatureLearnParams)] ){
// //
// //     struct FeatureSample fsmp;
// //     fsmp.f[0] = f1[0];
// //     fsmp.f[1] = f2[0];
// //
// //     test[0] = fsmp.f[0];
// //     test[1] = fsmp.f[1];
// //     test[2] = pPP->f_bnds[0][0];
// //     test[3] = pPP->f_bnds[0][1];
// //     test[4] = pPP->f_bnds[1][0];
// //     test[5] = pPP->f_bnds[1][1];
// //     test[6] = pPP->f_mean[0];
// //     test[7] = pPP->f_mean[1];
// //     test[8] = pPP->f_std[0];
// //     test[9] = pPP->f_std[1];
// //     test[10] = pPPB->f_mean_bnds[0][0];
// //     test[11] = pPPB->f_mean_bnds[0][1];
// //     test[12] = pPPB->f_mean_bnds[1][0];
// //     test[13] = pPPB->f_mean_bnds[1][1];
// //     test[14] = pPPB->f_std_bnds[0][0];
// //     test[15] = pPPB->f_std_bnds[0][1];
// //     test[16] = pPPB->f_std_bnds[1][0];
// //     test[17] = pPPB->f_std_bnds[1][1];
// //     test[18] = pFLP->lr_mean[0];
// //     test[19] = pFLP->lr_mean[1];
// //     test[20] = pFLP->lr_std[0];
// //     test[21] = pFLP->lr_std[1];
// // }
//
//
//
// //
// // double median_dbl(double d[], uint32_t dlen){
// //   // NOTE: we can't modify the original array because we need to have order
// //   // maintained through time. hence, we must copy the array here
// //   double d_cpy[] = calloc(dlen,sizeof(double));
// //   for(uint32_t i = 0; i < dlen; i++)}{ d_cpy[i] = d[i];}
// //
// //   // sort the array from
// //   qsort(d_cpy, dlen,sizeof(double),cmpfunc_dbl_descend);
// //   // get the resutl
// //   double result;
// //   if(dlen%2  == 1){
// //     // odd length, return the
// //     result = d_cpy[dlen/2];
// //   }else{
// //     // even length, return the mean of the two middle ones
// //     result = (d_cpy[(dlen/2)-1] + d_cpy[dlen/2])/2;
// //   }
// //   free(d_cpy);
// //   return result;
// // }
//
//
//
//
//
//
//
//
//
//
//
// //
