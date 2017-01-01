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

extern void big_init_const(mpz_t out, unsigned char* str) {

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
}

extern void big_init_expr(mpz_t out, int32_t a) {
	char str[15];
	sprintf(str, "%d", a);
	mpz_init_set_str(out, str, 10);
}

extern void big_add(mpz_t out, mpz_t a, mpz_t b) {
	mpz_add(out, a, b);
}
extern void big_sub(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern void big_mul(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern void big_div(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern void big_ceil_div(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern void big_floor_div(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern void big_truncate_div(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern void big_div_exact(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern void big_mod(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern void big_neg(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern void big_lshift(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern void big_rshift(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern void big_gcd(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern int32_t big_divisible(mpz_t a, mpz_t b) {
	return 0;
}
extern void big_congruent_mod_p(mpz_t a, mpz_t b, mpz_t p) {
	;
}
extern void big_pow(mpz_t out, mpz_t a, uint32_t b) {
	;
}
extern void big_pow2(mpz_t out, uint32_t b) {
	;
}
extern void big_pow_mod_p(mpz_t out, mpz_t a, mpz_t b, mpz_t c) {
	;
}
extern void big_pow2_mod_p(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern int32_t big_compare(mpz_t a, mpz_t b) {
	return 0;
}
extern int32_t big_compare_abs(mpz_t a, mpz_t b) {
	return 0;
}
extern int32_t big_sign(mpz_t a) {
	return 0;
}
extern void big_or(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern void big_and(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern void big_xor(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern void big_or_integer(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern void big_and_integer(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern void big_xor_integer(mpz_t out, mpz_t a, mpz_t b) {
	;
}
extern int32_t big_least_32bit(mpz_t a) {
	return 0;
}