
#ifndef HARD_API_INCLUDE_PAILLIER_PLATFORM_PYNQ_H
#define HARD_API_INCLUDE_PAILLIER_PLATFORM_PYNQ_H

#include "base_paillier.h"
extern "C" {
#include "pynq/pynq_api.h"
}
#include <string>

namespace hard {
class PynqPaillier : public Paillier {
  PYNQ_SHARED_MEMORY *memory_1, *memory_2, *memory_param;
  PYNQ_HLS *encrypt_core, *decrypt_core, *param_core;

  /**
   * return how much param loaded
   * @param dst
   * @return
   */
  size_t load_param(uint64_t *dst) const;

public:
  explicit PynqPaillier(unsigned key_size, const std::string &bit_file = "/home/xilinx/bits/full_06/design_1.bit");

  ~PynqPaillier() override;

  void encrypt(mpz_class &c, const mpz_class &m) override;

  void decrypt(mpz_class &m, const mpz_class &c) override;

  void key_gen(unsigned long seed) override;
};
}// namespace hard

#endif//HARD_API_INCLUDE_PAILLIER_PLATFORM_PYNQ_H
