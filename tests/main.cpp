//
// Created by 16586 on 2022/3/30.
//

#include "paillier/paillier.h"
#include <iostream>

int paillier_test(Paillier &h) {
  mpz_class message("0x9205993da57");

  std::cout << "Message: " << message.get_str(16) << std::endl;
  h.key_gen(1);


  mpz_class cipher = h.encrypt(message);

  std::cout << "Cipher is: " << cipher.get_str(16) << std::endl;

  auto de_message = h.decrypt(cipher);

  std::cout << "De_message is: " << de_message.get_str(16) << std::endl;

  h.key_gen(clock());

  cipher = h.encrypt(message);
  std::cout << "Cipher is: " << cipher.get_str(16) << std::endl;
  de_message = h.decrypt(cipher);
  std::cout << "De_message is: " << de_message.get_str(16) << std::endl;

  return 0;
}

int main() {
  auto pynq = hard::PynqPaillier(1024);
  paillier_test(pynq);
  auto base = Paillier(1024);
  paillier_test(base);
}