extern "C" {
#include "../platform/pynq/pynq_api.h"
}
#include "../include/hard_api.h"
#include "../platform/pynq/drivers/drivers.h"
#include "gmpxx.h"

hard::Hard::Hard() {
  PYNQ_loadBitstream("/home/xilinx/bits/full_05/design_1.bit");
  this->memory_1 = new PYNQ_SHARED_MEMORY();
  this->memory_2 = new PYNQ_SHARED_MEMORY();
  this->encrypt_core = new PYNQ_HLS();
  this->decrypt_core = new PYNQ_HLS();

  PYNQ_allocatedSharedMemory(this->memory_1, sizeof(uint64_t) * 257, 1);
  PYNQ_allocatedSharedMemory(this->memory_2, sizeof(uint64_t) * 257, 1);

  PYNQ_openHLS(this->decrypt_core, 0x80000000, 64 * 1024 / 8);
  PYNQ_openHLS(this->encrypt_core, 0x80010000, 64 * 1024 / 8);
}

mpz_class hard::Hard::encrypt(const mpz_class &m) {
  mpz_t res;
  mpz_init(res);
  this->encrypt(res, m.get_mpz_t());
  return mpz_class(res);
}

void hard::Hard::encrypt(mpz_t c, const mpz_t m) {
  size_t un = 0;
  auto *up = (uint64_t *) this->memory_1->pointer;
  auto *rp = (uint64_t *) this->memory_2->pointer;

  // Write m to up (shared memory).
  // -1 for least significant word first.
  // First 0 for native endianness.
  // Second 0 for no nails.
  mpz_export(up, &un, -1, sizeof(uint64_t), 0, 0, m);

  PYNQ_writeToHLS(encrypt_core, &memory_1->physical_address,  XENCRYPT_TOP_CONTROL_ADDR_UP_DATA, XENCRYPT_TOP_CONTROL_BITS_UP_DATA / 8);
  PYNQ_writeToHLS(encrypt_core, &memory_2->physical_address,  XENCRYPT_TOP_CONTROL_ADDR_RP_DATA, XENCRYPT_TOP_CONTROL_BITS_RP_DATA / 8);

  uint32_t rn = 0;
  PYNQ_startHLS(encrypt_core);
  PYNQ_waitForHLS(encrypt_core);
  PYNQ_readFromHLS(encrypt_core, &rn, XENCRYPT_TOP_CONTROL_ADDR_AP_RETURN, XENCRYPT_TOP_CONTROL_BITS_AP_RETURN / 8);

  mpz_import(c, rn, -1, sizeof(uint64_t), 0, 0, rp);
}

void hard::Hard::decrypt(mpz_t m, const mpz_t c) {
  constexpr size_t base_offset = 0;
  size_t un = 0;
  auto *up = (uint64_t *) this->memory_1->pointer;
  auto *rp = (uint64_t *) this->memory_2->pointer;

  mpz_export(up, &un, -1, sizeof(uint64_t), 0, 0, c);

  PYNQ_writeToHLS(decrypt_core, &memory_1->physical_address,  XDECRYPT_TOP_CONTROL_ADDR_UP_DATA, XDECRYPT_TOP_CONTROL_BITS_UP_DATA / 8);
  PYNQ_writeToHLS(decrypt_core, &memory_2->physical_address,  XDECRYPT_TOP_CONTROL_ADDR_RP_DATA, XDECRYPT_TOP_CONTROL_BITS_RP_DATA / 8);

  uint32_t rn = 0;
  PYNQ_startHLS(decrypt_core);
  PYNQ_waitForHLS(decrypt_core);
  PYNQ_readFromHLS(decrypt_core, &rn, XDECRYPT_TOP_CONTROL_ADDR_AP_RETURN, XDECRYPT_TOP_CONTROL_BITS_AP_RETURN / 8);

  mpz_import(m, rn, -1, sizeof(uint64_t), 0, 0, rp);
}

mpz_class hard::Hard::decrypt(const mpz_class &c) {
  mpz_t res;
  mpz_init(res);
  this->decrypt(res, c.get_mpz_t());
  return mpz_class(res);
}

hard::Hard::~Hard() {
  PYNQ_freeSharedMemory(this->memory_1);
  PYNQ_freeSharedMemory(this->memory_2);
  PYNQ_closeHLS(this->encrypt_core);

  delete this->memory_1;
  delete this->memory_2;
  delete this->encrypt_core;
}