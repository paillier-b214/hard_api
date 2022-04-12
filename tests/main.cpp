//
// Created by 16586 on 2022/3/30.
//

#include "paillier/paillier.h"
#include <iostream>

int paillier_test(Paillier &h) {

  double m1 = 3.1415926, m2 = 4;
  std::cout << "Message: " << m1 << std::endl;

  h.key_gen(1);

  mpz_class message1, message2;
  h.encode(message1, m1);
  h.encode(message2, m2);

  std::cout << "Encoded message1: " << message1.get_str(16) << std::endl;
  std::cout << "Encoded message2: " << message2.get_str(16) << std::endl;


  mpz_class cipher1 = h.encrypt(message1);
  mpz_class cipher2 = h.encrypt(message2);

  std::cout << "Cipher1: " << cipher1.get_str(16) << std::endl;
  std::cout << "Cipher2: " << cipher2.get_str(16) << std::endl;

  auto add_cipher = h.add(cipher1, cipher2);
  auto mul_cipher = h.mul(cipher1, message2);

  std::cout << "Add cipher: " << add_cipher.get_str(16) << std::endl;

  auto de_message1 = h.decrypt(cipher1);
  auto de_message2 = h.decrypt(cipher2);
  auto de_add = h.decrypt(add_cipher);
  auto de_mul = h.decrypt(mul_cipher);

  std::cout << "Decrypted message1: " << de_message1.get_str(16) << std::endl;
  std::cout << "Decrypted message2: " << de_message2.get_str(16) << std::endl;
  std::cout << "Decrypted add: " << de_add.get_str(16) << std::endl;
  std::cout << "Decrypted mul: " << de_mul.get_str(16) << std::endl;

  double res1, res2, add_res, mul_res;
  h.decode(res1, de_message1, false);
  h.decode(res2, de_message2, false);
  h.decode(add_res, de_add, false);
  h.decode(mul_res, de_mul, true);
  std::cout << "Decoded message1: " << res1 << std::endl;
  std::cout << "Decoded message2: " << res2 << std::endl;
  std::cout << "Decoded add res: " << add_res << std::endl;
  std::cout << "Decoded mul res: " << mul_res << std::endl;

  // regen key test
  h.key_gen(clock());

  cipher1 = h.encrypt(message1);
  std::cout << "Cipher is: " << cipher1.get_str(16) << std::endl;
  de_message1 = h.decrypt(cipher1);
  std::cout << "De_message is: " << de_message1.get_str(16) << std::endl;

  return 0;
}

int main() {
  auto pynq = hard::PynqPaillier(1024);
  paillier_test(pynq);
  auto base = Paillier(1024);
  paillier_test(base);
}