/**
 * Morpheus_Timer.hpp
 *
 * EPCC, The University of Edinburgh
 *
 * (c) 2022 The University of Edinburgh
 *
 * Contributing Authors:
 * Christodoulos Stylianou (c.stylianou@ed.ac.uk)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HPCG_MORPHEUS_TIMER_HPP
#define HPCG_MORPHEUS_TIMER_HPP

#ifdef HPCG_WITH_MORPHEUS
double morpheus_timer(void);

#ifdef HPCG_WITH_MULTI_FORMATS
#include <vector>

typedef struct morpheus_timers {
  double SPMV;
  double SYMGS;
  double MG;
  double HALO_SWAP;
  double CG;
  double SPMV_LOCAL, SPMV_GHOST;

  morpheus_timers()
      : SPMV(0),
        SYMGS(0),
        MG(0),
        HALO_SWAP(0),
        CG(0),
        SPMV_LOCAL(0),
        SPMV_GHOST(0) {}

} morpheus_timers;

extern std::vector<morpheus_timers> mtimers, sub_mtimers;

#endif  // HPCG_WITH_MULTI_FORMATS

#define MTICK() t0 = morpheus_timer()  //!< record current time in 't0'
#define MTOCK(t) t += morpheus_timer() - t0

#endif  // HPCG_WITH_MORPEHUS
#endif  // HPCG_MORPHEUS_TIMER_HPP