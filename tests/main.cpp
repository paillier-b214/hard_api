//
// Created by 16586 on 2022/3/30.
//

#include <iostream>
#include "../include/hard_api.h"

int main() {
  mpz_class a("0x9205993da57");

  auto h = hard::Hard();

  mpz_class res = h.encrypt(a);

  std::cout << "cipher is: " << res.get_str(16) << std::endl;

  return 0;
}