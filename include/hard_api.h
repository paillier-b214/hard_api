//
// Created by 16586 on 2022/3/30.
//

#ifndef HARD_API_HARD_API_H
#define HARD_API_HARD_API_H

#include "gmpxx.h"
#include "memory"

/** All platform transparent interface */
namespace hard {
static_assert(GMP_LIMB_BITS == 64, "gmp limb bits should be 64");

class Hard {
 PYNQ_SHARED_MEMORY memory_1{}, memory_2{};
 PYNQ_HLS overlay{};

public:
  Hard();

  mpz_class encrypt(const mpz_class &m);
  ~Hard();
};
}// namespace hard

#endif//HARD_API_HARD_API_H
