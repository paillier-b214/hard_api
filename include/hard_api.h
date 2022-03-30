//
// Created by 16586 on 2022/3/30.
//

#ifndef HARD_API_HARD_API_H
#define HARD_API_HARD_API_H

#include "gmpxx.h"

/** All platform transparent interface */
namespace hard {
  struct PYNQ_SHARED_MEMORY;

class Hard {
  PYNQ_SHARED_MEMORY* memory_1, *memory_2;

public:
  Hard();
};

void init();

void encrypt(mpz_class &c, const mpz_class &m);
}// namespace hard

#endif//HARD_API_HARD_API_H
