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

extern void big_init_const(unsigned char *str) {
	;
}
extern void big_init_expr(unsigned char *str) {
	;
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
