/* ************************************************************************
 * Modifications (c) 2022 The University of Edinburgh
 *
 * EPCC, The University of Edinburgh
 *
 * Contributing Authors:
 * Christodoulos Stylianou (c.stylianou@ed.ac.uk)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * ************************************************************************ */

#include <map>

#include "MixedBaseCounter.hpp"

MixedBaseCounter::MixedBaseCounter(int *counts, int length) {
  this->length = length;

  int i;

  for (i = 0; i < 32; ++i) {
    this->max_counts[i] = counts[i];
    this->cur_counts[i] = 0;
  }
  // terminate with 0's
  this->max_counts[i] = this->cur_counts[i] = 0;
  this->max_counts[length] = this->cur_counts[length] = 0;
}

MixedBaseCounter::MixedBaseCounter(MixedBaseCounter &left,
                                   MixedBaseCounter &right) {
  this->length = left.length;
  for (int i = 0; i < left.length; ++i) {
    this->max_counts[i] = left.max_counts[i] - right.cur_counts[i];
    this->cur_counts[i] = 0;
  }
}

void MixedBaseCounter::next() {
  for (int i = 0; i < this->length; ++i) {
    this->cur_counts[i]++;
    if (this->cur_counts[i] > this->max_counts[i]) {
      this->cur_counts[i] = 0;
      continue;
    }
    break;
  }
}

int MixedBaseCounter::is_zero() {
  for (int i = 0; i < this->length; ++i)
    if (this->cur_counts[i]) return 0;
  return 1;
}

int MixedBaseCounter::product(int *multipliers) {
  int k = 0, x = 1;

  for (int i = 0; i < this->length; ++i)
    for (int j = 0; j < this->cur_counts[i]; ++j) {
      k = 1;
      x *= multipliers[i];
    }

  return x * k;
}
