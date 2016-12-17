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

extern void big_init_const(unsigned char *str);
extern void big_init_expr(unsigned char *str);
extern void big_add(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_sub(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_mul(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_div(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_ceil_div(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_floor_div(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_truncate_div(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_div_exact(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_mod(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_neg(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_lshift(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_rshift(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_gcd(unsigned char *out, unsigned char *a, unsigned char *b);
extern int32_t big_divisible(unsigned char *a, unsigned char *b);
extern void big_congruent_mod_p(unsigned char *a, unsigned char *b, unsigned char *p);
extern void big_pow(unsigned char *out, unsigned char *a, uint32_t b);
extern void big_pow2(unsigned char *out, uint32_t b);
extern void big_pow_mod_p(unsigned char *out, unsigned char *a, unsigned char *b, unsigned char *c);
extern void big_pow2_mod_p(unsigned char *out, unsigned char *a, unsigned char *b);
extern int32_t big_compare(unsigned char *a, unsigned char *b);
extern int32_t big_compare_abs(unsigned char *a, unsigned char *b);
extern int32_t big_sign(unsigned char *a);
extern void big_or(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_and(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_xor(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_or_integer(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_and_integer(unsigned char *out, unsigned char *a, unsigned char *b);
extern void big_xor_integer(unsigned char *out, unsigned char *a, unsigned char *b);
extern int big_least_32bit(unsigned char *a);

#endif // ELASTICPLFUNCTIONS_H_
