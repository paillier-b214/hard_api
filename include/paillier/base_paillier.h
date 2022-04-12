
#ifndef HARD_API_INCLUDE_PAILLIER_H
#define HARD_API_INCLUDE_PAILLIER_H
#include "gmpxx.h"

class Paillier {
protected:
  mpz_class n, g, nsquare;
  mpz_class p, q, pm1, qm1, psquare, qsquare, p_inverse, hp, hq;
  const unsigned key_size;

public:
  explicit Paillier(unsigned key_size): key_size(key_size) {}
  virtual ~Paillier() = default;

  mpz_class encrypt(const mpz_class &m) {
    mpz_class res;
    this->encrypt(res, m);
    return std::move(res);
  }
  virtual void encrypt(mpz_class &c, const mpz_class &m);

  mpz_class decrypt(const mpz_class &c) {
    mpz_class res;
    this->decrypt(res, c);
    return std::move(res);
  }
  virtual void decrypt(mpz_class &m, const mpz_class &c);

  virtual void key_gen(unsigned long seed);

  mpz_class add(const mpz_class &c1, const mpz_class &c2) {
    mpz_class res;
    this->add(res, c1, c2);
    return std::move(res);
  }

  virtual void add(mpz_class &res ,const mpz_class &c1, const mpz_class &c2);

  mpz_class mul(const mpz_class &c, const mpz_class &m) {
    mpz_class res;
    this->mul(res, c, m);
    return std::move(res);
  }

  virtual void mul(mpz_class &res, const mpz_class &c, const mpz_class &m);
};

#endif//HARD_API_INCLUDE_PAILLIER_H
