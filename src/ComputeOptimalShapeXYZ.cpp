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

#include <cmath>
#include <cstdlib>

#ifdef HPCG_CUBIC_RADICAL_SEARCH
#include <algorithm>
#endif
#include <map>

#include "ComputeOptimalShapeXYZ.hpp"
#include "MixedBaseCounter.hpp"

#ifdef HPCG_CUBIC_RADICAL_SEARCH
static int min3(int a, int b, int c) { return std::min(a, std::min(b, c)); }

static int max3(int a, int b, int c) { return std::max(a, std::max(b, c)); }

static void cubic_radical_search(int n, int& x, int& y, int& z) {
  double best = 0.0;

  for (int f1 = (int)(pow(n, 1.0 / 3.0) + 0.5); f1 > 0; --f1)
    if (n % f1 == 0) {
      int n1 = n / f1;
      for (int f2 = (int)(pow(n1, 0.5) + 0.5); f2 > 0; --f2)
        if (n1 % f2 == 0) {
          int f3         = n1 / f2;
          double current = (double)min3(f1, f2, f3) / max3(f1, f2, f3);
          if (current > best) {
            best = current;
            x    = f1;
            y    = f2;
            z    = f3;
          }
        }
    }
}

#else

static void ComputePrimeFactors(int n, std::map<int, int>& factors) {
  int d, sq = int((sqrt(double(n))) + 1L);
  div_t r;

  // remove 2 as a factor with shifts instead "/" and "%"
  for (; n > 1 && (n & 1) == 0; n >>= 1) {
    factors[2]++;
  }

  // keep removing subsequent odd numbers
  for (d = 3; d <= sq; d += 2) {
    while (1) {
      r = div(n, d);
      if (r.rem == 0) {
        factors[d]++;
        n = r.quot;
        continue;
      }
      break;
    }
  }
  if (n > 1 || factors.size() == 0)  // left with a prime or x==1
    factors[n]++;
}

static int pow_i(int x, int p) {
  int v;

  if (0 == x || 1 == x) return x;

  if (p < 0) return 0;

  for (v = 1; p; p >>= 1) {
    if (1 & p) v *= x;
    x *= x;
  }

  return v;
}

#endif

void ComputeOptimalShapeXYZ(int xyz, int& x, int& y, int& z) {
#ifdef HPCG_CUBIC_RADICAL_SEARCH
  cubic_radical_search(xyz, x, y, z);
#else
  std::map<int, int> factors;

  ComputePrimeFactors(xyz, factors);  // factors are sorted: ascending order

  std::map<int, int>::iterator iter = factors.begin();

  // there is at least one prime factor
  x = (iter++)->first;  // cache the first factor, move to the next one

  y = iter != factors.end() ? (iter++)->first
                            : y;  // try to cache the second factor in "y"

  if (factors.size() == 1) {  // only a single factor
    z = pow_i(x, factors[x] / 3);
    y = pow_i(x, factors[x] / 3 + ((factors[x] % 3) >= 2 ? 1 : 0));
    x = pow_i(x, factors[x] / 3 + ((factors[x] % 3) >= 1 ? 1 : 0));

  } else if (factors.size() == 2 && factors[x] == 1 &&
             factors[y] == 1) {  // two distinct prime factors
    z = 1;

  } else if (factors.size() == 2 &&
             factors[x] + factors[y] ==
                 3) {             // three prime factors, one repeated
    z = factors[x] == 2 ? x : y;  // test which factor is repeated

  } else if (factors.size() == 3 && factors[x] == 1 && factors[y] == 1 &&
             iter->second == 1) {  // three distinct and single prime factors
    z = iter->first;

  } else {  // 3 or more prime factors so try all possible 3-subsets

    int i, distinct_factors[32 + 1], count_factors[32 + 1];

    i = 0;
    for (std::map<int, int>::iterator it = factors.begin(); it != factors.end();
         ++it, ++i) {
      distinct_factors[i] = it->first;
      count_factors[i]    = it->second;
    }

    // count total number of prime factors in "c_main" and distribute some
    // factors into "c1"
    MixedBaseCounter c_main(count_factors, factors.size()),
        c1(count_factors, factors.size());

    // at the beginning, minimum area is the maximum area
    double area, min_area = 2.0 * xyz + 1.0;

    for (c1.next(); !c1.is_zero(); c1.next()) {
      MixedBaseCounter c2(c_main, c1);  // "c2" gets the factors remaining in
                                        // "c_main" that "c1" doesn't have
      for (c2.next(); !c2.is_zero(); c2.next()) {
        int tf1 = c1.product(distinct_factors);
        int tf2 = c2.product(distinct_factors);
        int tf3 = xyz / tf1 / tf2;  // we derive the third dimension, we don't
                                    // keep track of the factors it has

        area = tf1 * double(tf2) + tf2 * double(tf3) + tf1 * double(tf3);
        if (area < min_area) {
          min_area = area;
          x        = tf1;
          y        = tf2;
          z        = tf3;
        }
      }
    }
  }
#endif
}
