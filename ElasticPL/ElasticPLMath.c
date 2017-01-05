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

extern void big_init_const(mpz_t out, unsigned char* str, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || !str)
		return;

	old_sz = (uint32_t)out->_mp_size;

	// Reset The Big Int
	mpz_clear(out);

	// TODO: Determine Max Len Elastic Allows For Hex / Binary Strings
	if (str[0] == '0' && str[1] == 'x' && strlen(str) > 2 && strlen(str) <= 256) {
		mpz_init_set_str(out, str + 2, 16);
	}
	else if (str[0] == '0' && str[1] == 'b' && strlen(str) > 2 && strlen(str) <= 256) {
		mpz_init_set_str(out, str + 2, 2);
	}
	else {
		mpz_init_set_str(out, str, 10);
	}

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_init_expr(mpz_t out, int32_t a, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	// Reset The Big Int
	mpz_clear(out);

	mpz_set_si(out, a);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_copy(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;
	
	if (out == a)
		return;

	// Reset The Big Int
	mpz_clear(out);

	mpz_init_set(out, a);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_add(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + 1;  // At most addition will cause size to increase by 1
	if (new_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else
		mpz_add(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_sub(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + 1;  // At most subtraction will cause size to increase by 1
	if (new_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else
		mpz_sub(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_mul(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + (uint32_t)a->_mp_size + (uint32_t)b->_mp_size;  // At most multiplication will cause size to increase by a + b
	if (new_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else
		mpz_mul(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_div(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + (uint32_t)a->_mp_size;  // At most division will cause size to increase by a
	if ((new_sz > BIG_INT_MAX_SZ) || (b->_mp_size == 0))
		mpz_set_ui(out, 0);
	else
		mpz_div(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_ceil_div(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + (uint32_t)a->_mp_size;  // At most division will cause size to increase by a
	if ((new_sz > BIG_INT_MAX_SZ) || (b->_mp_size == 0))
		mpz_set_ui(out, 0);
	else
		mpz_cdiv_q(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_floor_div(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + (uint32_t)a->_mp_size;  // At most division will cause size to increase by a
	if ((new_sz > BIG_INT_MAX_SZ) || (b->_mp_size == 0))
		mpz_set_ui(out, 0);
	else
		mpz_fdiv_q(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_truncate_div(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + (uint32_t)a->_mp_size;  // At most division will cause size to increase by a
	if ((new_sz > BIG_INT_MAX_SZ) || (b->_mp_size == 0))
		mpz_set_ui(out, 0);
	else
		mpz_tdiv_q(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_div_exact(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + (uint32_t)a->_mp_size;  // At most division will cause size to increase by a
	if ((new_sz > BIG_INT_MAX_SZ) || (b->_mp_size == 0))
		mpz_set_ui(out, 0);
	else
		mpz_divexact(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_mod(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + (uint32_t)a->_mp_size;  // At most mod will cause size to increase by a
	if ((new_sz > BIG_INT_MAX_SZ) || (b->_mp_size == 0))
		mpz_set_ui(out, 0);
	else
		mpz_mod(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_neg(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + (uint32_t)a->_mp_size;  // At most negation will cause size to increase by a
	if (new_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else
		mpz_neg(out, a);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_lshift(mpz_t out, mpz_t a, int32_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + (uint32_t)a->_mp_size + (int)(b / 32) + 1;  // At most left shift will cause size to increase by a + b / 32 + 1
	if ((new_sz > BIG_INT_MAX_SZ) || (b < 0))
		mpz_set_ui(out, 0);
	else
		mpz_mul_2exp(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_rshift(mpz_t out, mpz_t a, int32_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + (uint32_t)a->_mp_size;  // At most right shift will cause size to increase by a
	if ((new_sz > BIG_INT_MAX_SZ) || (b < 0))
		mpz_set_ui(out, 0);
	else
		mpz_div_2exp(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_gcd(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + (uint32_t)a->_mp_size;  // At most gcd will cause size to increase by a
	if ((new_sz > BIG_INT_MAX_SZ) || (b->_mp_size == 0))

	// TODO: Not sure how to validate this one
	new_sz = *bi_size + (uint32_t)a->_mp_size;
	if ((new_sz > BIG_INT_MAX_SZ) || (b < 0))
		mpz_set_ui(out, 0);
	else
		mpz_gcd(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern int32_t big_divisible(mpz_t a, mpz_t b, mpz_t *ptr) {

	// Ensure Inputs Are Valid
	if ((a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return 0;

	return mpz_divisible_p(a, b);
}

extern int32_t big_congruent_mod_p(mpz_t a, mpz_t b, mpz_t p, mpz_t *ptr) {

	// Ensure Inputs Are Valid
	if ((a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]) || (p < ptr[0]) || (p > ptr[99]))
		return 0;

	return mpz_congruent_p(a, b, p);
}

extern void big_pow(mpz_t out, mpz_t a, uint32_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + ((uint32_t)a->_mp_size * b);  // At most POW will cause size to increase by a * b
	if (new_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else
		mpz_pow_ui(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_pow2(mpz_t out, uint32_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	// TODO: Confirm Validation Logic
	new_sz = *bi_size + b;  // At most POW2 will cause size to increase by b
	if (new_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else
		mpz_ui_pow_ui(out, 2, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_pow_mod_p(mpz_t out, mpz_t a, mpz_t b, mpz_t c, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (c > ptr[99]) || (c < ptr[0]) || (c > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	// TODO: Confirm Validation Logic
	new_sz = *bi_size + ((uint32_t)a->_mp_size * (uint32_t)b->_mp_size);  // At most POW will cause size to increase by a * b
	if (new_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else
		mpz_powm(out, a, b, c);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_pow2_mod_p(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	// TODO: Confirm Validation Logic
	new_sz = *bi_size + (uint32_t)a->_mp_size;  // At most POW will cause size to increase by a
	if (new_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else {
		mpz_t c;
		mpz_set_ui(c, 2);
		mpz_powm(out, c, a, b);
		mpz_clear(c);
	}

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern int32_t big_compare(mpz_t a, mpz_t b, mpz_t *ptr) {

	// Ensure Inputs Are Valid
	if ((a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return 0;

	return mpz_cmp(a, b);
}

extern int32_t big_compare_abs(mpz_t a, mpz_t b, mpz_t *ptr) {

	// Ensure Inputs Are Valid
	if ((a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return 0;

	return mpz_cmpabs(a, b);
}

extern int32_t big_sign(mpz_t a, mpz_t *ptr) {

	// Ensure Inputs Are Valid
	if ((a < ptr[0]) || (a > ptr[99]))
		return 0;

	return mpz_sgn(a);
}

extern void big_or(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	// TODO: Confirm Validation Logic
	new_sz = *bi_size + MAX((uint32_t)a->_mp_size, (uint32_t)b->_mp_size);  // At most POW will cause size to increase by max a , b
	if (new_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else
		mpz_ior(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_and(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	// TODO: Confirm Validation Logic
	new_sz = *bi_size + MAX((uint32_t)a->_mp_size, (uint32_t)b->_mp_size);  // At most POW will cause size to increase by max a , b
	if (new_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else
		mpz_and(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_xor(mpz_t out, mpz_t a, mpz_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	// TODO: Confirm Validation Logic
	new_sz = *bi_size + MAX((uint32_t)a->_mp_size, (uint32_t)b->_mp_size);  // At most POW will cause size to increase by max a , b
	if (new_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else
		mpz_xor(out, a, b);

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_or_integer(mpz_t out, mpz_t a, int32_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + (uint32_t)a->_mp_size + 1;  // At most OR will cause size to increase by max a + 1
	if (new_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else {
		mpz_t c;
		mpz_set_ui(c, b);
		mpz_ior(out, a, c);
		mpz_clear(c);
	}

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_and_integer(mpz_t out, mpz_t a, int32_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + (uint32_t)a->_mp_size + 1;  // At most OR will cause size to increase by max a + 1
	if (new_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else {
		mpz_t c;
		mpz_set_ui(c, b);
		mpz_and(out, a, c);
		mpz_clear(c);
	}

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern void big_xor_integer(mpz_t out, mpz_t a, int32_t b, mpz_t *ptr, uint32_t *bi_size) {
	uint32_t old_sz, new_sz;

	// Ensure Inputs Are Valid
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]))
		return;

	old_sz = (uint32_t)out->_mp_size;

	new_sz = *bi_size + (uint32_t)a->_mp_size + 1;  // At most OR will cause size to increase by max a + 1
	if (new_sz > BIG_INT_MAX_SZ)
		mpz_set_ui(out, 0);
	else {
		mpz_t c;
		mpz_set_ui(c, b);
		mpz_xor(out, a, c);
		mpz_clear(c);
	}

	*bi_size += ((uint32_t)out->_mp_size - old_sz);
	if (*bi_size < 0) *bi_size = 0;
}

extern int32_t big_least_32bit(mpz_t a, mpz_t *ptr) {

	// Ensure Inputs Are Valid
	if ((a < ptr[0]) || (a > ptr[99]))
		return 0;

	return mpz_get_si(a);
}

extern void big_get_bin(mpz_t a, uint32_t *buf, size_t len, mpz_t *ptr) {

	// Ensure Inputs Are Valid
	if ((a < ptr[0]) || (a > ptr[99])) {
		free(buf);
		buf = NULL;
		return;
	}

	// Validate Buffer Can Hold Big Int
	if ((a->_mp_size < 0) || ((size_t)(a->_mp_size * 4) > len)) {
		free(buf);
		buf = NULL;
		return;
	}

	mpz_export(buf, NULL, 1, 4, 1, 0, a);
}

extern void big_set_bin(mpz_t a, uint8_t *buf, size_t len, mpz_t *ptr, uint32_t *bi_size) {

	// Ensure Inputs Are Valid
	if ((a < ptr[0]) || (a > ptr[99]) || !buf || len <= 0)
		return;

	mpz_import(a, len, 1, 1, 1, 0, buf);
}