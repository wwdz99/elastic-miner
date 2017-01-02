/*
* Copyright 2016 sprocket
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free
* Software Foundation; either version 2 of the License, or (at your option)
* any later version.
*/

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <gmp.h>
#include "ElasticPLFunctions.h"

extern int32_t gcd(int32_t a, int32_t b) {
	if (a < 0) a = -a;
	if (b < 0) b = -b;
	while (b != 0) {
		a %= b;
		if (a == 0) return b;
		b %= a;
	}
	return a;
}

extern void big_init_const(mpz_t out, unsigned char* str, uint32_t *bi_size) {
	uint32_t old_sz = (uint32_t)out->_mp_size;

	if (!str)
		return;

	if (str[0] == '0' && str[1] == 'x' && strlen(str) > 2 && strlen(str) <= 66) {
		mpz_init_set_str(out, str + 2, 16);
	}
	else if (str[0] == '0' && str[1] == 'b' && strlen(str) > 2 && strlen(str) <= 66) {
		mpz_init_set_str(out, str + 2, 2);
	}
	else {
		mpz_init_set_str(out, str, 10);
	}

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_init_expr(mpz_t out, int32_t a, uint32_t *bi_size) {
	char str[15];
	uint32_t old_sz = (uint32_t)out->_mp_size;

	sprintf(str, "%d", a);
	mpz_init_set_str(out, str, 10);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_add(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	uint32_t res_sz;
	uint32_t old_sz = (uint32_t)out->_mp_size;

	res_sz = *bi_size + 1;  // At most addition will cause size to increase by 1
	if (res_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else
		mpz_add(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;

	printf("out - alloc: %d, size: %d, val: %08X\n", out->_mp_alloc, out->_mp_size, *out->_mp_d);
	printf("a   - alloc: %d, size: %d, val: %08X\n", a->_mp_alloc, a->_mp_size, *a->_mp_d);
	printf("b   - alloc: %d, size: %d, val: %08X\n", b->_mp_alloc, b->_mp_size, *b->_mp_d);
}

extern void big_sub(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	uint32_t res_sz;
	uint32_t old_sz = (uint32_t)out->_mp_size;

	res_sz = *bi_size + 1;  // At most subtraction will cause size to increase by 1
	if (res_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else
		mpz_sub(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;

	printf("out - alloc: %d, size: %d, val: %08X\n", out->_mp_alloc, out->_mp_size, *out->_mp_d);
	printf("a   - alloc: %d, size: %d, val: %08X\n", a->_mp_alloc, a->_mp_size, *a->_mp_d);
	printf("b   - alloc: %d, size: %d, val: %08X\n", b->_mp_alloc, b->_mp_size, *b->_mp_d);
}
extern void big_mul(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	uint32_t res_sz;
	uint32_t old_sz = (uint32_t)out->_mp_size;

	res_sz = *bi_size + a->_mp_size + b->_mp_size;  // At most multiplication will cause size to increase by a + b
	if (res_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else
		mpz_mul(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;

	printf("out - alloc: %d, size: %d, val: %08X\n", out->_mp_alloc, out->_mp_size, *out->_mp_d);
	printf("a   - alloc: %d, size: %d, val: %08X\n", a->_mp_alloc, a->_mp_size, *a->_mp_d);
	printf("b   - alloc: %d, size: %d, val: %08X\n", b->_mp_alloc, b->_mp_size, *b->_mp_d);
}

extern void big_div(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	uint32_t res_sz;
	uint32_t old_sz = (uint32_t)out->_mp_size;

	// Division should reduce the size
	mpz_div(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;

	printf("out - alloc: %d, size: %d, val: %08X\n", out->_mp_alloc, out->_mp_size, *out->_mp_d);
	printf("a   - alloc: %d, size: %d, val: %08X\n", a->_mp_alloc, a->_mp_size, *a->_mp_d);
	printf("b   - alloc: %d, size: %d, val: %08X\n", b->_mp_alloc, b->_mp_size, *b->_mp_d);
}

extern void big_ceil_div(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	;
}
extern void big_floor_div(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	;
}
extern void big_truncate_div(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	;
}
extern void big_div_exact(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	;
}
extern void big_mod(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	;
}
extern void big_neg(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	;
}
extern void big_lshift(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	;
}
extern void big_rshift(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	;
}
extern void big_gcd(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	;
}
extern int32_t big_divisible(mpz_t a, mpz_t b, uint32_t *bi_size) {
	return 0;
}
extern void big_congruent_mod_p(mpz_t a, mpz_t b, mpz_t p, uint32_t *bi_size) {
	;
}

extern void big_pow(mpz_t out, mpz_t a, uint32_t b, uint32_t *bi_size) {
	uint32_t res_sz;
	uint32_t old_sz = (uint32_t)out->_mp_size;

	res_sz = *bi_size + (a->_mp_size * b);  // At most multiplication will cause size to increase by a * b
	if (res_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else
		mpz_pow_ui(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;

	printf("out - alloc: %d, size: %d, val: %08X\n", out->_mp_alloc, out->_mp_size, *out->_mp_d);
	printf("a   - alloc: %d, size: %d, val: %08X\n", a->_mp_alloc, a->_mp_size, *a->_mp_d);
}

extern void big_pow2(mpz_t out, uint32_t b, uint32_t *bi_size) {
	;
}
extern void big_pow_mod_p(mpz_t out, mpz_t a, mpz_t b, mpz_t c, uint32_t *bi_size) {
	;
}
extern void big_pow2_mod_p(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	;
}
extern int32_t big_compare(mpz_t a, mpz_t b, uint32_t *bi_size) {
	return 0;
}
extern int32_t big_compare_abs(mpz_t a, mpz_t b, uint32_t *bi_size) {
	return 0;
}
extern int32_t big_sign(mpz_t a, uint32_t *bi_size) {
	return 0;
}
extern void big_or(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	;
}
extern void big_and(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	;
}
extern void big_xor(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	;
}
extern void big_or_integer(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	;
}
extern void big_and_integer(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	;
}
extern void big_xor_integer(mpz_t out, mpz_t a, mpz_t b, uint32_t *bi_size) {
	;
}
extern int32_t big_least_32bit(mpz_t a, uint32_t *bi_size) {
	return 0;
}