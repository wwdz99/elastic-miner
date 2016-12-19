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
	if ( a < 0 ) a = -a;
	if ( b < 0 ) b = -b;
	while (b != 0) {
		a %= b;
		if (a == 0) return b;
		b %= a;
	}
	return a;
}

extern void big_init_const(unsigned char *out, unsigned char *str) {
	int i;
	uint32_t val[8];
	uint32_t *out32 = (uint32_t *)out;

	memset(out, 0, 32);

	if (!str)
		return;

	int x = strlen(str);

	// Check For Hex - If Found, Convert To Decimal String
	if (str[0] == '0' && str[1] == 'x' && strlen(str) > 2 && strlen(str) <= 66) {
		hex2bin(val, 8, str + 2, strlen(str) - 2);
	}
	else if (strlen(str) <= 10) {
		for (i = 0; i < 7; i++)
			val[i] = (str[0] == '-') ? 0xFFFFFFFF : 0;
		val[7] = (uint32_t)strtod(&str[0], NULL);
	}

	for (i = 0; i < 8; i++)
		out32[i] = ((val[i] << 24) | ((val[i] << 8) & 0x00FF0000) | ((val[i] >> 8) & 0x0000FF00) | ((val[i] >> 24) & 0x000000FF));

}

extern void big_init_expr(unsigned char *out, int32_t a) {
	uint32_t *out32 = (uint32_t *)out;

	memset(out, 0, 32);
	out32[7] = ((a << 24) | ((a << 8) & 0x00FF0000) | ((a >> 8) & 0x0000FF00) | ((a >> 24) & 0x000000FF));
}

extern void big_add(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_sub(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_mul(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_div(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_ceil_div(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_floor_div(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_truncate_div(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_div_exact(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_mod(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_neg(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_lshift(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_rshift(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_gcd(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern int32_t big_divisible(unsigned char *a, unsigned char *b) {
	return 0;
}
extern void big_congruent_mod_p(unsigned char *a, unsigned char *b, unsigned char *p) {
	;
}
extern void big_pow(unsigned char *out, unsigned char *a, uint32_t b) {
	;
}
extern void big_pow2(unsigned char *out, uint32_t b) {
	;
}
extern void big_pow_mod_p(unsigned char *out, unsigned char *a, unsigned char *b, unsigned char *c) {
	;
}
extern void big_pow2_mod_p(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern int32_t big_compare(unsigned char *a, unsigned char *b) {
	return 0;
}
extern int32_t big_compare_abs(unsigned char *a, unsigned char *b) {
	return 0;
}
extern int32_t big_sign(unsigned char *a) {
	return 0;
}
extern void big_or(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_and(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_xor(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_or_integer(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_and_integer(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern void big_xor_integer(unsigned char *out, unsigned char *a, unsigned char *b) {
	;
}
extern int32_t big_least_32bit(unsigned char *a) {
	return 0;
}
