
#ifndef HARD_API_INCLUDE_PAILLIER_H
#define HARD_API_INCLUDE_PAILLIER_H
#include "gmpxx.h"
#include <vector>
#include <iostream>

class Paillier {
protected:
  mpz_class n, g, nsquare;
  mpz_class p, q, pm1, qm1, psquare, qsquare, p_inverse, hp, hq;
  const unsigned key_size;

public:
  explicit Paillier(unsigned key_size) : key_size(key_size) {}
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

  virtual void add(mpz_class &res, const mpz_class &c1, const mpz_class &c2);

  mpz_class mul(const mpz_class &c, const mpz_class &m) {
    mpz_class res;
    this->mul(res, c, m);
    return std::move(res);
  }

  virtual void mul(mpz_class &res, const mpz_class &c, const mpz_class &m);

  /**
   *
   * @tparam T float or double
   */
  template<class T>
  void encode(mpz_class &res, T scalar, double scale = 1e6) {
    bool flag = (scalar < 0);
    if (flag)
      scalar = -scalar;
    auto after_scale = static_cast<uint64_t>(scalar * scale);
    if (flag) {
      res = n - after_scale;
    } else {
      res = after_scale;
    }
  }

  /**
   *
   * @tparam T float or double
   */
  template<class T>
  void decode(T &res, const mpz_class &plain, bool isMul, double scale_factor = 1e6) {
    uint64_t ret;
    mpz_class max_int = n / 3;          // n/3
    mpz_class forNegative = max_int * 2;// 2n/3
    bool isPositive = max_int > plain;
    bool isNegative = plain > forNegative;

    if (!isMul) {
      if (isNegative) {
        mpz_class tmp = plain - n;
        ret = tmp.get_si();
      } else if (isPositive) {
        ret = plain.get_si();
      } else {
        std::cout << "There is a possible OVERFLOW!\n";
      }
    } else {
      mpz_class tmp = ((isNegative ? n - plain : plain) / scale_factor);
      ret = tmp.get_si();
      if (isNegative)
        ret = -ret;
    }

    res = static_cast<T>(ret) / scale_factor;
  }
  virtual void vector_mul(std::vector<mpz_class> &res, const mpz_class &c, const std::vector<mpz_class> &m, int startIndex, int endIndex);
  virtual void vector_decrypt(std::vector<mpz_class> &res, const std::vector<mpz_class> &c, int startIndex, int endIndex);
};

#endif//HARD_API_INCLUDE_PAILLIER_H
