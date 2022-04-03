//
// Created by 16586 on 2022/3/30.
//

#include <iostream>
#include "../include/hard_api.h"

int main() {
  mpz_class message("0x9205993da57");

  std::cout << "message: " << message.get_str(16) << std::endl;

  auto h = hard::Hard();

  mpz_class cipher = h.encrypt(message);

  std::cout << "cipher is: " << cipher.get_str(16) << std::endl;

  auto de_message = h.decrypt(cipher);

  std::cout << "de_message is: " << de_message.get_str(16) << std::endl;

  return 0;
}