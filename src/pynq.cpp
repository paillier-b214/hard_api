#include "pynq.h"
#include "pynq/drivers.h"
PynqPaillier::PynqPaillier(unsigned int key_size, const std::string &bit_file) : Paillier(key_size) {
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

PynqPaillier::~PynqPaillier() {
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

void PynqPaillier::encrypt(mpz_class &c, const mpz_class &m) {
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

void PynqPaillier::decrypt(mpz_class &m, const mpz_class &c) {
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
void PynqPaillier::key_gen(unsigned long seed) {
  Paillier::key_gen(seed);
  
}
