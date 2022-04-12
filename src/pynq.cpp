#include "pynq.h"
#include "pynq/drivers.h"
#include <cassert>
#include <iostream>

#define KEYSIZE 1024
#include "hls/include/config.h"

hard::PynqPaillier::PynqPaillier(unsigned int key_size, const std::string &bit_file) : Paillier(key_size) {
  PYNQ_loadBitstream(bit_file.c_str());
  this->memory_1 = new PYNQ_SHARED_MEMORY();
  this->memory_2 = new PYNQ_SHARED_MEMORY();
  this->memory_param = new PYNQ_SHARED_MEMORY();
  this->encrypt_core = new PYNQ_HLS();
  this->decrypt_core = new PYNQ_HLS();
  this->param_core = new PYNQ_HLS();

  PYNQ_allocatedSharedMemory(this->memory_1, sizeof(uint64_t) * 257, 1);
  PYNQ_allocatedSharedMemory(this->memory_2, sizeof(uint64_t) * 257, 1);
  PYNQ_allocatedSharedMemory(this->memory_param, sizeof(uint64_t) * 1024, 1);

  PYNQ_openHLS(this->decrypt_core, 0x80000000, 64 * 1024 / 8);
  PYNQ_openHLS(this->encrypt_core, 0x80010000, 64 * 1024 / 8);
  PYNQ_openHLS(this->param_core, 0x80020000, 64 * 1024 / 8);

  PYNQ_writeToHLS(encrypt_core, &memory_1->physical_address, XENCRYPT_TOP_CONTROL_ADDR_UP_DATA, XENCRYPT_TOP_CONTROL_BITS_UP_DATA / 8);
  PYNQ_writeToHLS(encrypt_core, &memory_2->physical_address, XENCRYPT_TOP_CONTROL_ADDR_RP_DATA, XENCRYPT_TOP_CONTROL_BITS_RP_DATA / 8);

  PYNQ_writeToHLS(decrypt_core, &memory_2->physical_address, XDECRYPT_TOP_CONTROL_ADDR_UP_DATA, XDECRYPT_TOP_CONTROL_BITS_UP_DATA / 8);
  PYNQ_writeToHLS(decrypt_core, &memory_1->physical_address, XDECRYPT_TOP_CONTROL_ADDR_RP_DATA, XDECRYPT_TOP_CONTROL_BITS_RP_DATA / 8);

  PYNQ_writeToHLS(param_core, &memory_param->physical_address, XLOAD_PARAM_TOP_CONTROL_ADDR_DATA_DATA, XLOAD_PARAM_TOP_CONTROL_BITS_DATA_DATA / 8);
}

hard::PynqPaillier::~PynqPaillier() {
  PYNQ_freeSharedMemory(this->memory_1);
  PYNQ_freeSharedMemory(this->memory_2);
  PYNQ_freeSharedMemory(this->memory_param);
  PYNQ_closeHLS(this->encrypt_core);
  PYNQ_closeHLS(this->decrypt_core);
  PYNQ_closeHLS(this->param_core);

  delete this->memory_1;
  delete this->memory_2;
  delete this->memory_param;
  delete this->encrypt_core;
  delete this->decrypt_core;
  delete this->param_core;
}

void hard::PynqPaillier::encrypt(mpz_class &c, const mpz_class &m) {
  size_t un = 0;
  auto *up = (uint64_t *) this->memory_1->pointer;
  auto *rp = (uint64_t *) this->memory_2->pointer;

  // Write m to up (shared memory).
  // -1 for least significant word first.
  // First 0 for native endianness.
  // Second 0 for no nails.
  mpz_export(up, &un, -1, sizeof(uint64_t), 0, 0, m.get_mpz_t());

  uint32_t rn = 0;
  PYNQ_startHLS(encrypt_core);
  PYNQ_waitForHLS(encrypt_core);
  PYNQ_readFromHLS(encrypt_core, &rn, XENCRYPT_TOP_CONTROL_ADDR_AP_RETURN, XENCRYPT_TOP_CONTROL_BITS_AP_RETURN / 8);

  mpz_import(c.get_mpz_t(), rn, -1, sizeof(uint64_t), 0, 0, rp);
}

void hard::PynqPaillier::decrypt(mpz_class &m, const mpz_class &c) {
  size_t un = 0;
  auto *rp = (uint64_t *) this->memory_1->pointer;
  auto *up = (uint64_t *) this->memory_2->pointer;

  mpz_export(up, &un, -1, sizeof(uint64_t), 0, 0, c.get_mpz_t());

  uint32_t rn = 0;
  PYNQ_startHLS(decrypt_core);
  PYNQ_waitForHLS(decrypt_core);
  PYNQ_readFromHLS(decrypt_core, &rn, XDECRYPT_TOP_CONTROL_ADDR_AP_RETURN, XDECRYPT_TOP_CONTROL_BITS_AP_RETURN / 8);

  mpz_import(m.get_mpz_t(), rn, -1, sizeof(uint64_t), 0, 0, rp);
}

void hard::PynqPaillier::key_gen(unsigned long seed) {
  Paillier::key_gen(seed);

  auto *up = (uint64_t *) this->memory_param->pointer;
  for(int i = 0;i < 1024; i++) {
    up[i] = 0;
  }
  load_param(up);

  PYNQ_startHLS(param_core);
  PYNQ_waitForHLS(param_core);
}

#define BIT_SIZE(x) mpz_sizeinbase((x).get_mpz_t(), 2)

uint64_t generate_t(uint64_t rdx, const mpz_class &square) {
  mpz_class temp;
  auto mr = (1 << rdx) - 1;

  mpz_mod_ui(temp.get_mpz_t(), square.get_mpz_t(), 1 << rdx);
  auto nn = temp.get_ui();
  uint64_t t = 1;
  for (int i = 0; i < rdx - 1; i++) {
    t = (t * t * nn) & mr;
  }
  return mr + 1 - t;
}

size_t hard::PynqPaillier::load_param(uint64_t *dst) const {
#define SIZE_TEMP size_t __size_temp
#define PARAM_EXPORT(v, name)                                                                     \
  do {                                                                                            \
    mpz_export(dst + (p::addr::name), &__size_temp, -1, sizeof(uint64_t), 0, 0, (v).get_mpz_t()); \
    assert(__size_temp == (p::size::name));                                                       \
  } while (0)
#define GMP_POWM(res, base, exp, mod) mpz_powm((res).get_mpz_t(), mpz_class(base).get_mpz_t(), mpz_class(exp).get_mpz_t(), mod.get_mpz_t())

  SIZE_TEMP;

  PARAM_EXPORT(n, N);
  PARAM_EXPORT(nsquare, NSQUARE);

  mpz_class ns_r2m, ns_r1m;
  GMP_POWM(ns_r2m, 2, 2 * 2 * BIT_SIZE(n), nsquare);
  GMP_POWM(ns_r1m, 2, 2 * BIT_SIZE(n), nsquare);
  PARAM_EXPORT(ns_r2m, NS_R2M);
  PARAM_EXPORT(ns_r1m, NS_R1M);

  PARAM_EXPORT(q, Q);
  PARAM_EXPORT(p, P);
  PARAM_EXPORT(qsquare, QSQUARE);
  PARAM_EXPORT(psquare, PSQUARE);

  mpz_class q_r2m, p_r2m;
  GMP_POWM(q_r2m, 2, 2 * BIT_SIZE(q), q);
  GMP_POWM(p_r2m, 2, 2 * BIT_SIZE(p), p);
  PARAM_EXPORT(q_r2m, Q_R2M);
  PARAM_EXPORT(p_r2m, P_R2M);

  mpz_class qs_r3m, ps_r3m;
  GMP_POWM(qs_r3m, 2, 2 * 3 * BIT_SIZE(q), qsquare);
  GMP_POWM(ps_r3m, 2, 2 * 3 * BIT_SIZE(p), psquare);
  PARAM_EXPORT(qs_r3m, QS_R3M);
  PARAM_EXPORT(ps_r3m, PS_R3M);

  mpz_class qs_r1m, ps_r1m;
  GMP_POWM(qs_r1m, 2, 2 * BIT_SIZE(q), qsquare);
  GMP_POWM(ps_r1m, 2, 2 * BIT_SIZE(p), psquare);
  PARAM_EXPORT(qs_r1m, QS_R1M);
  PARAM_EXPORT(ps_r1m, PS_R1M);

#define MTGY(dst, src, mod)               \
  do {                                    \
    mpz_ui_pow_ui(dst.get_mpz_t(), 2, BIT_SIZE(mod)); \
    dst *= src;                           \
    dst %= mod;                           \
  } while (0)

  mpz_class p_inverse_mont;
  MTGY(p_inverse_mont, p_inverse, q);
  PARAM_EXPORT(p_inverse_mont, P_INVERSE_MONT);

  mpz_class hp_mont, hq_mont;
  MTGY(hq_mont, hq, q);
  MTGY(hp_mont, hp, p);
  PARAM_EXPORT(hq_mont, HQ_MONT);
  PARAM_EXPORT(hp_mont, HP_MONT);

  dst[p::addr::T] = generate_t(RADIX, nsquare);
  dst[p::addr::T_QS] = generate_t(RADIX, qsquare);
  dst[p::addr::T_PS] = generate_t(RADIX, psquare);
  dst[p::addr::T_Q] = generate_t(RADIX, q);
  dst[p::addr::T_P] = generate_t(RADIX, p);

  return p::addr::T_P+1;

#undef MTGY
#undef GMP_POWM
#undef PARAM_EXPORT
#undef SIZE_TEMP
}
