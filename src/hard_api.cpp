#include <iostream>
extern "C" {
#include "../platform/pynq/pynq_api.h"
}
#include "../include/hard_api.h"
#include "gmpxx.h"
#include "xbnz_hwh.h"

void pynq_test() {

  PYNQ_loadBitstream("/home/xilinx/bits/enc_1024_04/design_1.bit");

  PYNQ_SHARED_MEMORY shared_memory_1, shared_memory_2;

  PYNQ_allocatedSharedMemory(&shared_memory_1, sizeof(uint64_t) * 257, 0);
  PYNQ_allocatedSharedMemory(&shared_memory_2, sizeof(uint64_t) * 257, 0);

  auto *d1 = (uint64_t *) shared_memory_1.pointer;
  auto *d2 = (uint64_t *) shared_memory_2.pointer;

  for (int i = 0; i < 257; i++) {
    d1[i] = 0;
    d2[i] = 0;
  }
  d1[0] = 0x8a5d;

  PYNQ_HLS overlay;
  PYNQ_openHLS(&overlay, 0x80000000, 64 * 1024 / 8);
  uint32_t un = 1, op = 1, len = 0;
  PYNQ_writeToHLS(&overlay, &shared_memory_1.physical_address, XBNZ_CONTROL_ADDR_UP_DATA, XBNZ_CONTROL_BITS_UP_DATA / 8);
  PYNQ_writeToHLS(&overlay, &shared_memory_2.physical_address, XBNZ_CONTROL_ADDR_RP_DATA, XBNZ_CONTROL_BITS_RP_DATA / 8);

  PYNQ_writeToHLS(&overlay, &un, XBNZ_CONTROL_ADDR_UN_DATA, XBNZ_CONTROL_BITS_UN_DATA / 8);
  PYNQ_writeToHLS(&overlay, &op, XBNZ_CONTROL_ADDR_OP_DATA, XBNZ_CONTROL_BITS_OP_DATA / 8);

  clock_t start = clock();
  PYNQ_startHLS(&overlay);
  PYNQ_waitForHLS(&overlay);
  clock_t end = clock();
  std::cout << "hls finished: in " << ((double) (end - start) / CLOCKS_PER_SEC) << " secs." << std::endl;
  PYNQ_readFromHLS(&overlay, &len, XBNZ_CONTROL_ADDR_AP_RETURN, XBNZ_CONTROL_BITS_AP_RETURN / 8);

  PYNQ_closeHLS(&overlay);

  for (int i = 0; i < len; i++) {
    std::cout << std::hex << d2[i] << std::endl;
  }
}

int main() {
  mpz_class a("0x9205993da57");

  auto h = hard::Hard();

  clock_t start = clock();
  mpz_class res = h.encrypt(a);
  clock_t end = clock();

  std::cout << "encrypt finished in: " << ((double) (end - start) / CLOCKS_PER_SEC) << " secs." << std::endl;
  std::cout << "cipher is: " << res.get_str(16) << std::endl;

  return 0;
}
hard::Hard::Hard() {
  PYNQ_loadBitstream("/home/xilinx/bits/enc_1024_04/design_1.bit");

  PYNQ_allocatedSharedMemory(&this->memory_1, sizeof(uint64_t) * 257, 1);
  PYNQ_allocatedSharedMemory(&this->memory_2, sizeof(uint64_t) * 257, 1);

  PYNQ_openHLS(&this->overlay, 0x80000000, 64 * 1024 / 8);
}
mpz_class hard::Hard::encrypt(const mpz_class &m) {
  uint32_t op = 1;
  size_t un = 0;
  uint32_t zero = 0;
  auto *up = (uint64_t *) this->memory_1.pointer;
  auto *rp = (uint64_t *) this->memory_2.pointer;

  // Write m to up (shared memory).
  // -1 for least significant word first.
  // First 0 for native endianness.
  // Second 0 for no nails.
  mpz_export(up, &un, -1, sizeof(uint64_t), 0, 0, m.get_mpz_t());

  // rp
  PYNQ_writeToHLS(&overlay, &memory_2.physical_address, XBNZ_CONTROL_ADDR_RP_DATA, XBNZ_CONTROL_BITS_RP_DATA / 8);
  // up
  PYNQ_writeToHLS(&overlay, &memory_1.physical_address, XBNZ_CONTROL_ADDR_UP_DATA, XBNZ_CONTROL_BITS_UP_DATA / 8);
  PYNQ_writeToHLS(&overlay, &un, XBNZ_CONTROL_ADDR_UN_DATA, XBNZ_CONTROL_BITS_UN_DATA / 8);
  // vp
  PYNQ_writeToHLS(&overlay, &zero, XBNZ_CONTROL_ADDR_VP_DATA, XBNZ_CONTROL_BITS_VP_DATA / 8);
  PYNQ_writeToHLS(&overlay, &zero, XBNZ_CONTROL_ADDR_VN_DATA, XBNZ_CONTROL_BITS_VN_DATA / 8);
  // op
  PYNQ_writeToHLS(&overlay, &op, XBNZ_CONTROL_ADDR_OP_DATA, XBNZ_CONTROL_BITS_OP_DATA / 8);

  uint32_t rn = 0;
  PYNQ_startHLS(&overlay);
  PYNQ_waitForHLS(&overlay);
  PYNQ_readFromHLS(&overlay, &rn, XBNZ_CONTROL_ADDR_AP_RETURN, XBNZ_CONTROL_BITS_AP_RETURN / 8);

  mpz_t res;
  mpz_init(res);
  mpz_import(res, rn, -1, sizeof(uint64_t), 0, 0, rp);
  return mpz_class(res);
}
hard::Hard::~Hard() {
  PYNQ_freeSharedMemory(&this->memory_1);
  PYNQ_freeSharedMemory(&this->memory_2);
  PYNQ_closeHLS(&this->overlay);
}
