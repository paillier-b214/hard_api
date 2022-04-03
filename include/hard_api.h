//
// Created by 16586 on 2022/3/30.
//

#ifndef HARD_API_HARD_API_H
#define HARD_API_HARD_API_H

#include <gmpxx.h>
#include <memory>
extern "C" {
#include "../platform/pynq/pynq_api.h"
}

/** All platform transparent interface */
namespace hard {
static_assert(GMP_LIMB_BITS == 64, "gmp limb bits should be 64");

class Hard {
 PYNQ_SHARED_MEMORY *memory_1, *memory_2;
 PYNQ_HLS *encrypt_core, *decrypt_core;

public:
  Hard();

  /**
   * c++ style interface
   * @param m plain message to be encrypted
   * @return cypher
   */
  mpz_class encrypt(const mpz_class &m);

  /**
   * c style interface
   * @param c return cypher
   * @param m plain message to be encrypted
   */
  void encrypt(mpz_t c, const mpz_t m);

  void decrypt(mpz_t m, const mpz_t c);

  mpz_class decrypt(const mpz_class &c);

  ~Hard();
};
}// namespace hard

#endif//HARD_API_HARD_API_H
