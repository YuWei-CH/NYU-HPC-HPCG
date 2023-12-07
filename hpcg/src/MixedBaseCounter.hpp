/*******************************************************************************
* Copyright 2014-2022 Intel Corporation.
*
* This software and the related documents are Intel copyrighted  materials,  and
* your use of  them is  governed by the  express license  under which  they were
* provided to you (License).  Unless the License provides otherwise, you may not
* use, modify, copy, publish, distribute,  disclose or transmit this software or
* the related documents without Intel's prior written permission.
*
* This software and the related documents  are provided as  is,  with no express
* or implied  warranties,  other  than those  that are  expressly stated  in the
* License.
*******************************************************************************/

class MixedBaseCounter {
  private:
    int length; //!< number of prime factor counts (cannot exceed 32 for a 32-bit integer)
    int max_counts[32+1]; //!< maximum value for prime factor counts
    int cur_counts[32+1]; //!< current prime factor counts

  public:
    MixedBaseCounter(int *counts, int length);
    MixedBaseCounter(MixedBaseCounter & left, MixedBaseCounter & right); 
    void next();
    int is_zero();
    int product(int * multipliers);
};
