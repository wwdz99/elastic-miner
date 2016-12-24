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
#include "ElasticPLFunctions.h"

static bool hex2bin(uint32_t *p, int array_sz, const char *hex, int len) {
	int i, j, idx;
	unsigned char val;
	unsigned char *c = (unsigned char*)p;
	char hex_char[3];
	char *ep;

	if (array_sz <= 0 || len <= 0 || len > (8 * array_sz)) {
		return false;
	}

	hex_char[2] = '\0';
	idx = len - 1;

	for (i = array_sz - 1; i >= 0; i--) {
		for (j = 0; j < 4; j++) {
			val = 0;
			if (idx == 0) {
				hex_char[1] = hex[idx--];
				hex_char[0] = '0';
				val = (unsigned char)strtol(hex_char, &ep, 16);
			}
			else if (idx > 0) {
				hex_char[1] = hex[idx--];
				hex_char[0] = hex[idx--];
				val = (unsigned char)strtol(hex_char, &ep, 16);
			}
			c[(i * 4) + j] = val;
		}
	}
	return true;
}

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

extern void big_init_const(int32_t *m, int len, unsigned char* str) {
	int i;
	uint32_t *val = malloc(len * sizeof(uint32_t));

	// Reset Memory
	for (i = 0; i < len; i++)
		m[i] = 0;

	if (!str)
		return;

	int x = strlen(str);

	// Check For Hex - If Found, Convert To Decimal String
	if (str[0] == '0' && str[1] == 'x' && strlen(str) > 2 && strlen(str) <= 66) {
		hex2bin(val, len, str + 2, strlen(str) - 2);
	}
	else if (strlen(str) <= 10) {
		for (i = 0; i < len; i++)
			val[i] = (str[0] == '-') ? 0xFFFFFFFF : 0;
		val[len-1] = (uint32_t)strtod(&str[0], NULL);
	}

	for (i = 0; i < len; i++)
		m[i] = val[i];

	free(val);
}

extern void big_init_expr(int32_t *m, int len, int32_t a) {
	int i;

	// Reset Memory
	for (i = 0; i < len; i++)
		m[i] = 0;

	m[len - 1] = a;
}

extern void big_add(int32_t *m1, int32_t len1, int32_t *m2, int32_t len2, int32_t *m3, int32_t len3, uint32_t *tmp) {
	size_t i;
	int32_t len;
	uint64_t t1, t2, sum = 0;
	
	// Reset Memory
	for (i = 0; i < len1; i++)
		m1[i] = 0;

	// Check For Invalid Lengths
	if ((len1 < 0) || (len2 < 0) || (len3 < 0))
		return;

	// Check For Overflow
	len = MAX(len2, len3);
	if (len > (VM_TMP_MEMORY_SZ - 1))
		return;

	// Add m2[] + m3[]
	for (i = 0; i < len; i++) {
		t1 = (uint64_t)((i < len2) ? m2[len2 - i - 1] & 0xFFFFFFFF : 0);
		t2 = (uint64_t)((i < len3) ? m3[len3 - i - 1] & 0xFFFFFFFF : 0);
		sum += (t1 + t2);
		tmp[i] = (uint32_t)sum;
		sum >>= 32;	// VM Mem Uses 32 Bits
	}

	// Store Remainder
	if (sum > 0)
		tmp[len++] = (uint32_t)sum;

	// Copy Result To m1[]
	len = MIN(len1, len);
	for (i = 0; i < len; i++)
		m1[len1 - i - 1] = tmp[i];
}

//extern void big_sub(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern void big_mul(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern void big_div(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern void big_ceil_div(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern void big_floor_div(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern void big_truncate_div(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern void big_div_exact(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern void big_mod(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern void big_neg(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern void big_lshift(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern void big_rshift(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern void big_gcd(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern int32_t big_divisible(mpz_t a, mpz_t b) {
//	return 0;
//}
//extern void big_congruent_mod_p(mpz_t a, mpz_t b, mpz_t p) {
//	;
//}
//extern void big_pow(mpz_t out, mpz_t a, uint32_t b) {
//	;
//}
//extern void big_pow2(mpz_t out, uint32_t b) {
//	;
//}
//extern void big_pow_mod_p(mpz_t out, mpz_t a, mpz_t b, mpz_t c) {
//	;
//}
//extern void big_pow2_mod_p(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern int32_t big_compare(mpz_t a, mpz_t b) {
//	return 0;
//}
//extern int32_t big_compare_abs(mpz_t a, mpz_t b) {
//	return 0;
//}
//extern int32_t big_sign(mpz_t a) {
//	return 0;
//}
//extern void big_or(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern void big_and(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern void big_xor(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern void big_or_integer(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern void big_and_integer(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern void big_xor_integer(mpz_t out, mpz_t a, mpz_t b) {
//	;
//}
//extern int32_t big_least_32bit(mpz_t a) {
//	return 0;
//}