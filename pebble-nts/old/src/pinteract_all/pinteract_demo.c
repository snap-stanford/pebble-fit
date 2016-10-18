// #include "pinteract_func.h"



// static char* opt0(bool exec){
//   if(exec){
//     pinteract_driver(11);

//   }
//   return "Current State?";
// }

// static char* opt1(bool exec){
//   if(exec){
//     pinteract_driver(12);

//   }
//   return "Sleep Quality?";
// }

// static char* opt2(bool exec){
//   if(exec){
//     pinteract_driver(13);

//   }
//   return "You feel?";
// }




// // create the struct that will define the question and answers
// // of the survery we are about to call

// static struct pif_survey_params survey_params = {
//   .pinteract_code = 0,
//   .nitems = 3,
//   .init_selection = 1,
//   .survey_type = 0,
//   .title = "P Interacts",
//   .opt0fptr = opt0,
//   .opt1fptr = opt1,
//   .opt2fptr = opt2,
// };


// void pinteract_demo(){
//   // THIS is where we determine which survey function we are going into
//   pinteract_Nitem_survey(&survey_params);


// }
