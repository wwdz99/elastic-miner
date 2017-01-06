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

#include <gmp.h>


#ifndef VM_MEMORY_SIZE
#define VM_MEMORY_SIZE 64000
#endif
#ifndef VM_BI_SIZE
#define VM_BI_SIZE 100
#endif

#define EC_BUF_SZ		100		// Max Bytes For Elliptic Curve Key Buffer
#define BIG_INT_MAX_SZ 1000		// Total Size (in Ints) For All Big Integers Combined

#define MAX( a, b ) ( (a)>(b) ? (a) : (b) )
#define MIN( a, b ) ( (a)<(b) ? (a) : (b) )

extern void whirlpool_hash(const uint8_t *message, uint32_t len, uint8_t *hash);

extern uint32_t epl_sha256(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size);
extern uint32_t epl_sha512(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size);
extern uint32_t epl_md5(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size);
extern uint32_t epl_ripemd160(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size);
extern uint32_t epl_whirlpool(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size);
extern uint32_t epl_ec_priv_to_pub(mpz_t out, mpz_t a, bool compressed, int nid, size_t comp_sz, size_t uncomp_sz, mpz_t *ptr, uint32_t *bi_size);
extern uint32_t epl_ec_add(mpz_t out, bool comp, mpz_t a, bool compa, mpz_t b, bool compb, int nid, size_t comp_sz, size_t uncomp_sz, mpz_t *ptr, uint32_t *bi_size);
extern uint32_t epl_ec_sub(mpz_t out, bool comp, mpz_t a, bool compa, mpz_t b, bool compb, int nid, size_t comp_sz, size_t uncomp_sz, mpz_t *ptr, uint32_t *bi_size);
extern uint32_t epl_ec_neg(mpz_t out, bool comp, mpz_t a, bool compa, int nid, size_t comp_sz, size_t uncomp_sz, mpz_t *ptr, uint32_t *bi_size);
extern uint32_t epl_ec_mul(mpz_t out, bool comp, mpz_t a, bool compa, mpz_t b, int nid, size_t comp_sz, size_t uncomp_sz, mpz_t *ptr, uint32_t *bi_size);

extern int32_t gcd(int32_t	a, int32_t b);

extern void big_init_const(mpz_t out, unsigned char* str, mpz_t *ptr, uint32_t *bi_size);
extern void big_init_expr(mpz_t out, int32_t a, mpz_t *ptr, uint32_t *bi_size);
extern void big_copy(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size);
extern void big_add(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_sub(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_mul(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_div(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_ceil_div(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_floor_div(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_truncate_div(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_div_exact(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_mod(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_neg(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size);
extern void big_lshift(mpz_t out, mpz_t a, int32_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_rshift(mpz_t out, mpz_t a, int32_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_gcd(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size);
extern int32_t big_divisible(mpz_t a, mpz_t b, mpz_t *ptr);
extern int32_t big_congruent_mod_p(mpz_t a, mpz_t b, mpz_t p, mpz_t *ptr);
extern void big_pow(mpz_t out, mpz_t a, uint32_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_pow2(mpz_t out, uint32_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_pow_mod_p(mpz_t out, mpz_t a, mpz_t b, mpz_t c, mpz_t *ptr, uint32_t *bi_size);
extern void big_pow2_mod_p(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size);
extern int32_t big_compare(mpz_t a, mpz_t b, mpz_t *ptr);
extern int32_t big_compare_abs(mpz_t a, mpz_t b, mpz_t *ptr);
extern int32_t big_sign(mpz_t a, mpz_t *ptr);
extern void big_or(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_and(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_xor(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_or_integer(mpz_t out, mpz_t a, int32_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_and_integer(mpz_t out, mpz_t a, int32_t b, mpz_t *ptr, uint32_t *bi_size);
extern void big_xor_integer(mpz_t out, mpz_t a, int32_t b, mpz_t *ptr, uint32_t *bi_size);
extern int32_t big_least_32bit(mpz_t a, mpz_t *ptr);
extern size_t big_get_bin(mpz_t a, uint8_t *buf, size_t len, mpz_t *ptr);
extern void big_set_bin(mpz_t a, uint8_t *buf, size_t len, mpz_t *ptr, uint32_t *bi_size);

#endif // ELASTICPLFUNCTIONS_H_