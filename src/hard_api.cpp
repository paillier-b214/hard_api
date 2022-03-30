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
      pynq_test();
//  mpz_class a("0x8a5d");
//  mpz_class b("-5678");
//  mpz_class c = a + b;
//  std::cout << "sum is " << c << std::endl;
//  std::cout << "absolute value is " << abs(c) << std::endl;

  return 0;
}
