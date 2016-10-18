/* Project Kraepelin, Main file
The MIT License (MIT)

Copyright (c) 2015, Nathaniel T. Stockham

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

This license is taken to apply to any other files in the Project Kraepelin
Pebble App roject.
*/


#include "wavelet.h"


/* NOTE: the decomposition and reconstruction wavelet transforms are
* inverses of each other, each operation being undone by a mirror operation
* in exactly opposide order.
*/



const int32_t SQRT_TWO_4P = 14142; // we work to four points of precision


void ordering_switch(int16_t *d, int16_t lb, int16_t ub, int16_t ofs){
  // ub = upper bound
  // lb = lower bound
  // ofs = offset
  int16_t tmp = 0;
  for(int16_t i = 0; i < ofs ; i++){
    tmp = d[lb + ofs + i];
    d[lb + ofs + i] = d[lb + 2*ofs + i];
    d[lb + 2*ofs + i] = tmp;
  }
}


void decomp_haar_order(int16_t *d, int16_t lb, int16_t ub){
  // ub = upper bound
  // lb = lower bound

  // end condition
  if((ub-1) == lb){
    return;
  }
  int16_t ofs = (ub - lb + 1)/4; // offset
  // recursive calls
  decomp_haar_order(d, lb, lb + 2*ofs - 1);
  decomp_haar_order(d, lb + 2*ofs, ub);
  // swtich the order of the + - elements
  ordering_switch(d, lb, ub, ofs);
}

void recon_haar_order(int16_t *d, int16_t lb, int16_t ub){
  // ub = upper bound
  // lb = lower bound

  // end condition
  if((ub-1) == lb){
    return;
  }

  int16_t ofs = (ub - lb + 1)/4; // offset
  // swtich the order of the + - elements
  ordering_switch(d, lb, ub, ofs);
  // recursive calls
  recon_haar_order(d, lb, lb + 2*ofs - 1);
  recon_haar_order(d, lb + 2*ofs, ub);
}

void haar_add_sub_adjacent(int16_t *d, int16_t i_ubnd, int32_t mul, int32_t div ){
  // i_ubnd = upper bound of index
  // mul = multiplication factor
  // div = division factor

  // this code adds and subtracts adjacent elements
  for( int16_t j = 0; j < i_ubnd; j++){
    int32_t j0 = d[2*j];      // even elements
    int32_t j1 = d[2*j + 1];  // odd
    d[2*j] =(int16_t) (( ( j0 + j1 )*mul) / div);
    d[2*j + 1] = (int16_t) (( ( j0 - j1 )*mul) / div);
  }
}


// the haar wavelet decomposition
void dwt_haar(int16_t *d, int16_t dlenpwr, int16_t depth){
  // dlenpwr -> 2^dlenpwr is the length of the data array
  // depth -> the depth of the analysis to be performed

  for(int i = (dlenpwr-1); i > (dlenpwr - 1 - depth); i-- ){
    // the i_ubnd of the current level of data we are looking at
    // also, it is the appropriate level because we are searching 2*j and 2*j+1
    int16_t i_ubnd =  pow_int(2,i);

    // decomp
    //   a1' = ((a1+a2)*10000)/sqrt(2)
    //   a2' = ((a1-a2)*10000)/sqrt(2)
    haar_add_sub_adjacent(d, i_ubnd,10000,SQRT_TWO_4P);
    // reorder the elements toward the wavelet transform
    decomp_haar_order(d, 0, 2*i_ubnd-1);
  }
}

// the haar wavelet reconstruction
void idwt_haar(int16_t *d, int16_t dlenpwr, int16_t depth){
  // dlenpwr -> 2^dlenpwr is the length of the data array
  // depth -> the depth of the analysis to be performed

  for(int16_t i = (dlenpwr - depth) ; i < (dlenpwr); i++ ){
    // the i_ubnd of the current level of data we are looking at
    // also, it is the appropriate level because we are searching 2*j and 2*j+1
    int16_t i_ubnd =  pow_int(2,i);
    // reorder the elements away from the wavelet transform
    recon_haar_order(d, 0, 2*i_ubnd-1);

    // we have 2*10000 instead of 10000 because the addition and subtraction operation
    // in the reconstruction phase has the form
    // recon
    //   a1'' = ((((a1+a2)*10000)/sqrt(2))*sqrt(2))/(2*10000) = (2*a1 + a2 - a2)/2 = a1
    //   a2'' = ((((a1+a2)*10000)/sqrt(2))*sqrt(2))/(2*10000) = (a1 - a1 + 2*a2)/2 = a2
    haar_add_sub_adjacent(d, i_ubnd, SQRT_TWO_4P,2*10000);
  }
}
