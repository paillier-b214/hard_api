#include "paillier/base_paillier.h"
#include <iostream>

/**
 * res = (x - 1) / p
 */
mpz_class l_func(const mpz_class &x, const mpz_class &p) {
  mpz_class xm1 = x - 1;
  mpz_class res;
  mpz_divexact(res.get_mpz_t(), xm1.get_mpz_t(), p.get_mpz_t());
  return res;
}

/**
 * res = invert(((g^(x-1) % xsquare) - 1)) / x)
 */
mpz_class h_func(const mpz_class &g, const mpz_class &x, const mpz_class &xsquare) {
  const mpz_class xm1 = x - 1;
  mpz_class res;
  mpz_powm(res.get_mpz_t(), g.get_mpz_t(), xm1.get_mpz_t(), xsquare.get_mpz_t());
  mpz_class l_res = l_func(res, x);
  mpz_invert(res.get_mpz_t(), l_res.get_mpz_t(), x.get_mpz_t());
  return res;
}

mpz_class crt(const mpz_class &mp, const mpz_class &mq, const mpz_class &p, const mpz_class &q, const mpz_class &p_inverse) {
  mpz_class u = (mq - mp) * p_inverse % q;
  return mp + u * p;
}

void Paillier::key_gen(unsigned long seed) {
  gmp_randclass rr(gmp_randinit_default);
  rr.seed(seed);

  size_t n_len;
  do {
    p = rr.get_z_bits(key_size / 2);
    mpz_nextprime(p.get_mpz_t(), p.get_mpz_t());
    do {
      q = rr.get_z_bits(key_size / 2);
      mpz_nextprime(q.get_mpz_t(), q.get_mpz_t());
    } while (q == p);
    n = p * q;
    n_len = mpz_sizeinbase(n.get_mpz_t(), 2);
  } while (n_len != key_size);

  g = n + 1;
  nsquare = n * n;

  // ensure that p < q
  if (q < p) {
    swap(p, q);
  }
  pm1 = p - 1;
  qm1 = q - 1;
  psquare = p * p;
  qsquare = q * q;

  mpz_invert(p_inverse.get_mpz_t(), p.get_mpz_t(), q.get_mpz_t());
  hp = h_func(g, p, psquare);
  hq = h_func(g, q, qsquare);
}

void Paillier::encrypt(mpz_class &c, const mpz_class &m) {
  mpz_class r(65537);

  // (n * plaintext + 1) % nsquare.
  // plaintext < n, so that: n * plaintext + 1 < nsquare.
  mpz_class nude_ciphertext = n * m + 1;

  mpz_class obfuscator;
  mpz_powm(obfuscator.get_mpz_t(), r.get_mpz_t(), n.get_mpz_t(), nsquare.get_mpz_t());

  // c = nude_ciphertext * obfuscator % nsquare
  mpz_class temp = nude_ciphertext * obfuscator;
  mpz_mod(c.get_mpz_t(), temp.get_mpz_t(), nsquare.get_mpz_t());
}

void Paillier::decrypt(mpz_class &m, const mpz_class &c) {
  mpz_class pmp, pmq;
  mpz_powm(pmp.get_mpz_t(), c.get_mpz_t(), pm1.get_mpz_t(), psquare.get_mpz_t());
  mpz_powm(pmq.get_mpz_t(), c.get_mpz_t(), qm1.get_mpz_t(), qsquare.get_mpz_t());

  mpz_class lp = l_func(pmp, p);
  mpz_class lq = l_func(pmq, q);
  mpz_class dep = lp * hp % p;
  mpz_class deq = lq * hq % q;

  m = crt(dep, deq, p, q, p_inverse);
}

void Paillier::add(mpz_class &res, const mpz_class &c1, const mpz_class &c2) {
  res = c1 * c2 % nsquare;
}
void Paillier::mul(mpz_class &res, const mpz_class &c, const mpz_class &m) {
  mpz_class max_int = n / 3;          // n/3
  mpz_class forNegative = max_int * 2;// 2n/3

  if (cmp(m, forNegative) == 1) {
    mpz_class neg_c, neg_scalar;
    mpz_invert(neg_c.get_mpz_t(), c.get_mpz_t(), nsquare.get_mpz_t());
    neg_scalar = n - m;
    mpz_powm(res.get_mpz_t(), neg_c.get_mpz_t(), neg_scalar.get_mpz_t(), nsquare.get_mpz_t());
    return;
  }

  mpz_powm(res.get_mpz_t(), c.get_mpz_t(), m.get_mpz_t(), nsquare.get_mpz_t());
}

void Paillier::vector_mul(std::vector<mpz_class> &res, const mpz_class &c, const std::vector<mpz_class> &m, int startIndex, int endIndex) {
  for (int i = startIndex; i < endIndex; ++i) {
    mul(res[i], c, m[i - startIndex]);
  }
}
void Paillier::vector_decrypt(std::vector<mpz_class> &res, const std::vector<mpz_class> &c, int startIndex, int endIndex) {
  for (int i = startIndex; i < endIndex; ++i) {
    res[i] = decrypt(c[i]);
  }
}
