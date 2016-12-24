/*
* Copyright 2016 sprocket
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free
* Software Foundation; either version 2 of the License, or (at your option)
* any later version.
*/

#ifndef ELASTICPLFUNCTIONS_H_
#define ELASTICPLFUNCTIONS_H_

#include <stdint.h>

#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>

#define VM_TMP_MEMORY_SZ 4000

#define MAX( a, b ) ( (a)>(b) ? (a) : (b) )
#define MIN( a, b ) ( (a)<(b) ? (a) : (b) )

extern void whirlpool_hash(const uint8_t *message, uint32_t len, uint8_t *hash);

extern uint32_t epl_sha256(int idx, int len, int32_t *mem);
extern uint32_t epl_sha512(int idx, int len, int32_t *mem);
extern uint32_t epl_md5(int idx, int len, int32_t *mem);
extern uint32_t epl_ripemd160(int idx, int len, int32_t *mem);
extern uint32_t epl_whirlpool(int idx, int len, int32_t *mem);
extern uint32_t epl_ec_priv_to_pub(size_t idx, bool compressed, int32_t *mem, int nid, size_t len);
extern uint32_t epl_ec_add(size_t idx1, bool comp1, size_t idx2, bool comp2, bool comp, int32_t *mem, int nid, size_t comp_sz, size_t uncomp_sz);
extern uint32_t epl_ec_sub(size_t idx1, bool comp1, size_t idx2, bool comp2, bool comp, int32_t *mem, int nid, size_t comp_sz, size_t uncomp_sz);
extern uint32_t epl_ec_neg(size_t idx1, bool comp1, bool comp, int32_t *mem, int nid, size_t comp_sz, size_t uncomp_sz);
extern uint32_t epl_ec_mult(size_t idx1, bool comp1, size_t idx2, size_t n2, bool comp, int32_t *mem, int nid, size_t comp_sz, size_t uncomp_sz);

extern int32_t gcd(int32_t	a, int32_t b);

extern void big_init_const(int32_t *m, int len, unsigned char* str);
extern void big_init_expr(int32_t *m, int len, int32_t a);
extern void big_add(int32_t *m1, int32_t len1, int32_t *m2, int32_t len2, int32_t *m3, int32_t len3, uint32_t * tmp);
//extern void big_sub(mpz_t out, mpz_t a, mpz_t b);
//extern void big_mul(mpz_t out, mpz_t a, mpz_t b);
//extern void big_div(mpz_t out, mpz_t a, mpz_t b);
//extern void big_ceil_div(mpz_t out, mpz_t a, mpz_t b);
//extern void big_floor_div(mpz_t out, mpz_t a, mpz_t b);
//extern void big_truncate_div(mpz_t out, mpz_t a, mpz_t b);
//extern void big_div_exact(mpz_t out, mpz_t a, mpz_t b);
//extern void big_mod(mpz_t out, mpz_t a, mpz_t b);
//extern void big_neg(mpz_t out, mpz_t a, mpz_t b);
//extern void big_lshift(mpz_t out, mpz_t a, mpz_t b);
//extern void big_rshift(mpz_t out, mpz_t a, mpz_t b);
//extern void big_gcd(mpz_t out, mpz_t a, mpz_t b);
//extern int32_t big_divisible(mpz_t a, mpz_t b);
//extern void big_congruent_mod_p(mpz_t a, mpz_t b, mpz_t p);
//extern void big_pow(mpz_t out, mpz_t a, uint32_t b);
//extern void big_pow2(mpz_t out, uint32_t b);
//extern void big_pow_mod_p(mpz_t out, mpz_t a, mpz_t b, mpz_t c);
//extern void big_pow2_mod_p(mpz_t out, mpz_t a, mpz_t b);
//extern int32_t big_compare(mpz_t a, mpz_t b);
//extern int32_t big_compare_abs(mpz_t a, mpz_t b);
//extern int32_t big_sign(mpz_t a);
//extern void big_or(mpz_t out, mpz_t a, mpz_t b);
//extern void big_and(mpz_t out, mpz_t a, mpz_t b);
//extern void big_xor(mpz_t out, mpz_t a, mpz_t b);
//extern void big_or_integer(mpz_t out, mpz_t a, mpz_t b);
//extern void big_and_integer(mpz_t out, mpz_t a, mpz_t b);
//extern void big_xor_integer(mpz_t out, mpz_t a, mpz_t b);
//extern int big_least_32bit(mpz_t a);

#endif // ELASTICPLFUNCTIONS_H_