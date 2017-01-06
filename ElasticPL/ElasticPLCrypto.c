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

#ifndef __APPLE__
#include <malloc.h>
#else
#import <malloc/malloc.h>
#endif
#include "../crypto/sha2.h"
#include "ElasticPLFunctions.h"

static void dump_hex(unsigned char *label, unsigned char *bytes, int len) {
	int i;

	printf("%s: ", label);
	for (i = 0; i < len; i++)
		printf("%02X", bytes[i]);
	printf("\n");
}

extern uint32_t epl_sha256(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size) {
	sha256_ctx ctx;
	uint8_t hash[32], *buf = NULL;
	size_t buf_len, len;
	uint32_t value = 0;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]))
		return 0;

	sha256_init(&ctx);

	if (a->_mp_size) {
		buf_len = a->_mp_size * 4;
		buf = malloc(buf_len);

		// Populate Buffer With Big Int Data
		len = big_get_bin(a, buf, buf_len, ptr);
		if (!buf)
			return 0;

		sha256_update(&ctx, buf, len);
	}
	else
		sha256_update(&ctx, "", 0);

	sha256_final(&ctx, hash);

//	dump_hex("Hash", hash, 32);

	// Save Result As Big Int
	big_set_bin(out, hash, 32, ptr, bi_size);

	// Get Value For Mangle State
	value = mpz_get_si(out);
//	printf("Val: %d, %08X\n", value, value);

	if (buf) free(buf);
	return value;
}

extern uint32_t epl_sha512(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size) {
	uint8_t hash[64], *buf = NULL;
	size_t buf_len, len;
	uint32_t value = 0;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]))
		return 0;

	if (a->_mp_size) {
		buf_len = a->_mp_size * 4;
		buf = malloc(buf_len);

		// Populate Buffer With Big Int Data
		len = big_get_bin(a, buf, buf_len, ptr);
		if (!buf)
			return 0;

		SHA512(buf, len, hash);
	}
	else
		SHA512("", 0, hash);

//	dump_hex("Hash", hash, 64);

	// Save Result As Big Int
	big_set_bin(out, hash, 64, ptr, bi_size);

	// Get Value For Mangle State
	value = mpz_get_si(out);
//	printf("Val: %d, %08X\n", value, value);

	if (buf) free(buf);
	return value;
}

extern uint32_t epl_md5(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size) {
	uint8_t hash[16], *buf = NULL;
	size_t buf_len, len;
	uint32_t value = 0;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]))
		return 0;

	if (a->_mp_size) {
		buf_len = a->_mp_size * 4;
		buf = malloc(buf_len);

		// Populate Buffer With Big Int Data
		len = big_get_bin(a, buf, buf_len, ptr);
		if (!buf)
			return 0;

		MD5(buf, len, hash);
	}
	else
		MD5("", 0, hash);

//	dump_hex("Hash", hash, 16);

	// Save Result As Big Int
	big_set_bin(out, hash, 16, ptr, bi_size);

	// Get Value For Mangle State
	value = mpz_get_si(out);
//	printf("Val: %d, %08X\n", value, value);

	if (buf) free(buf);
	return value;
}

extern uint32_t epl_ripemd160(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size) {
	uint8_t hash[20], *buf = NULL;
	size_t buf_len, len;
	uint32_t value = 0;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]))
		return 0;

	if (a->_mp_size) {
		buf_len = a->_mp_size * 4;
		buf = malloc(buf_len);

		// Populate Buffer With Big Int Data
		len = big_get_bin(a, buf, buf_len, ptr);
		if (!buf)
			return 0;

		RIPEMD160(buf, len, hash);
	}
	else
		RIPEMD160("", 0, hash);

//	dump_hex("Hash", hash, 20);

	// Save Result As Big Int
	big_set_bin(out, hash, 20, ptr, bi_size);

	// Get Value For Mangle State
	value = mpz_get_si(out);
//	printf("Val: %d, %08X\n", value, value);

	if (buf) free(buf);
	return value;
}

extern uint32_t epl_whirlpool(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size) {
	uint8_t hash[64], *buf = NULL;
	size_t buf_len, len;
	uint32_t value = 0;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]))
		return 0;

	if (a->_mp_size) {
		buf_len = a->_mp_size * 4;
		buf = malloc(buf_len);

		// Populate Buffer With Big Int Data
		len = big_get_bin(a, buf, buf_len, ptr);
		if (!buf)
			return 0;

		whirlpool_hash(buf, len, hash);
	}
	else
		whirlpool_hash("", 0, hash);

//	dump_hex("Hash", hash, 64);

	// Save Result As Big Int
	big_set_bin(out, hash, 64, ptr, bi_size);

	// Get Value For Mangle State
	value = mpz_get_si(out);
	//	printf("Val: %d, %08X\n", value, value);

	if (buf) free(buf);
	return value;
}

extern uint32_t epl_ec_priv_to_pub(mpz_t out, mpz_t a, bool compressed, int nid, size_t comp_sz, size_t uncomp_sz, mpz_t *ptr, uint32_t *bi_size) {

	EC_KEY *key = NULL;
	EC_POINT *pub_key = NULL;
	const EC_GROUP *group = NULL;
	BIGNUM *priv_key = NULL;

	uint8_t *buf;
	size_t buf_len, len;
	uint32_t value = 0;

	// Init Empty OpenSSL EC Keypair
	key = EC_KEY_new_by_curve_name(nid);
	if (!key)
		return 0;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[99]))
		return 0;

	len = (compressed) ? comp_sz : uncomp_sz;
	buf_len = len + 4;
	buf = malloc(buf_len);

	// Populate Buffer With Big Int Data
	big_get_bin(a, buf, buf_len, ptr);
	if (!buf)
		return 0;
	
//	dump_hex("PrivKey", buf, len);

	// Set Private Key Through BIGNUM
	priv_key = BN_new();
	BN_bin2bn(buf, len, priv_key);
	if (!EC_KEY_set_private_key(key, priv_key)) {
		free(buf);
		return 0;
	}

	// Derive Public Key From Private Key And Group
	group = EC_KEY_get0_group(key);
	pub_key = EC_POINT_new(group);
	if (!EC_POINT_mul(group, pub_key, priv_key, NULL, NULL, NULL)) {
		free(buf);
		return 0;
	}

	if (compressed)
		len = EC_POINT_point2oct(group, pub_key, POINT_CONVERSION_COMPRESSED, buf, buf_len, NULL);
	else
		len = EC_POINT_point2oct(group, pub_key, POINT_CONVERSION_UNCOMPRESSED, buf, buf_len, NULL);

	if (!len) {
		free(buf);
		return 0;
	}

//	dump_hex("PK", buf, len);

	// Save Result As Big Int
	big_set_bin(out, buf, len, ptr, bi_size);

	// Get Value For Mangle State
	value = mpz_get_si(out);
//	printf("Val: %d, %08X\n", value, value);

	// Free Resources
	free(buf);
	EC_KEY_free(key);
	EC_POINT_free(pub_key);
	BN_clear_free(priv_key);

	return value;
}

extern uint32_t epl_ec_add(mpz_t out, bool comp, mpz_t a, bool compa, mpz_t b, bool compb, int nid, size_t comp_sz, size_t uncomp_sz, mpz_t *ptr, uint32_t *bi_size) {
	BN_CTX *ctx = NULL;
	EC_GROUP *group = NULL;
	EC_POINT *P = NULL;
	EC_POINT *Q = NULL;
	EC_POINT *R = NULL;

	uint8_t *buf;
	size_t buf_len, len, len_a, len_b;
	uint32_t value = 0;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]) || (comp_sz <= 0) || (uncomp_sz <= 0))
		return 0;

	// Initialize EC Data
	ctx = BN_CTX_new();
	group = EC_GROUP_new_by_curve_name(nid);
	P = EC_POINT_new(group);
	Q = EC_POINT_new(group);
	R = EC_POINT_new(group);

	len_a = (compa ? comp_sz : uncomp_sz);
	len_b = (compb ? comp_sz : uncomp_sz);
	buf_len = uncomp_sz + 3;
	buf = malloc(buf_len);

	// Populate Data For Point 1
	if (a->_mp_size == 0) {		// Point 1 = Infinity
		buf[0] = 0;
		len_a = 1;
	}
	else {
		big_get_bin(a, buf, buf_len, ptr);
		if (!buf)
			return 0;
	}

//	dump_hex("Point1", buf, len_a);

	if (!EC_POINT_oct2point(group, P, buf, len_a, ctx)) {
		free(buf);
		return 0;
	}

	// Populate Data For Point 2
	if (b->_mp_size == 0) {		// Point 2 = Infinity
		buf[0] = 0;
		len_b = 1;
	}
	else {
		big_get_bin(b, buf, buf_len, ptr);
		if (!buf)
			return 0;
	}

//	dump_hex("Point2", buf, len_b);

	if (!EC_POINT_oct2point(group, Q, buf, len_b, ctx)) {
		free(buf);
		return 0;
	}

	// Add Points
	if (!EC_POINT_add(group, R, P, Q, ctx)) {
		free(buf);
		return 0;
	}

	if (comp)
		len = EC_POINT_point2oct(group, R, POINT_CONVERSION_COMPRESSED, buf, buf_len, ctx);
	else
		len = EC_POINT_point2oct(group, R, POINT_CONVERSION_UNCOMPRESSED, buf, buf_len, ctx);

	if (!len) {
		free(buf);
		return 0;
	}

//	dump_hex("P1 + P2", buf, len);

	// Save Result As Big Int
	big_set_bin(out, buf, len, ptr, bi_size);

	// Get Value For Mangle State
	value = mpz_get_si(out);
//	printf("Val: %d, %08X\n", value, value);

	// Free Resources
	free(buf);
	EC_POINT_free(P);
	EC_POINT_free(Q);
	EC_POINT_free(R);
	EC_GROUP_free(group);
	BN_CTX_free(ctx);

	return value;
}

extern uint32_t epl_ec_sub(mpz_t out, bool comp, mpz_t a, bool compa, mpz_t b, bool compb, int nid, size_t comp_sz, size_t uncomp_sz, mpz_t *ptr, uint32_t *bi_size) {
	BN_CTX *ctx = NULL;
	EC_GROUP *group = NULL;
	EC_POINT *P = NULL;
	EC_POINT *Q = NULL;
	EC_POINT *R = NULL;

	uint8_t *buf;
	size_t buf_len, len, len_a, len_b;
	uint32_t value = 0;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]) || (comp_sz <= 0) || (uncomp_sz <= 0))
		return 0;

	// Initialize EC Data
	ctx = BN_CTX_new();
	group = EC_GROUP_new_by_curve_name(nid);
	P = EC_POINT_new(group);
	Q = EC_POINT_new(group);
	R = EC_POINT_new(group);

	len_a = (compa ? comp_sz : uncomp_sz);
	len_b = (compb ? comp_sz : uncomp_sz);
	buf_len = uncomp_sz + 3;
	buf = malloc(buf_len);

	// Populate Data For Point 1
	if (a->_mp_size == 0) {		// Point 1 = Infinity
		buf[0] = 0;
		len_a = 1;
	}
	else {
		big_get_bin(a, buf, buf_len, ptr);
		if (!buf)
			return 0;
	}

//	dump_hex("Point1", buf, len_a);

	if (!EC_POINT_oct2point(group, P, buf, len_a, ctx)) {
		free(buf);
		return 0;
	}

	// Populate Data For Point 2
	if (b->_mp_size == 0) {		// Point 2 = Infinity
		buf[0] = 0;
		len_b = 1;
	}
	else {
		big_get_bin(b, buf, buf_len, ptr);
		if (!buf)
			return 0;
	}

//	dump_hex("Point2", buf, len_b);

	if (!EC_POINT_oct2point(group, Q, buf, len_b, ctx)) {
		free(buf);
		return 0;
	}

	// Invert Q, Then Add Points
	if (!EC_POINT_invert(group, Q, ctx)) {
		free(buf);
		return 0;
	}
	if (!EC_POINT_add(group, R, P, Q, ctx)) {
		free(buf);
		return 0;
	}

	if (comp)
		len = EC_POINT_point2oct(group, R, POINT_CONVERSION_COMPRESSED, buf, buf_len, ctx);
	else
		len = EC_POINT_point2oct(group, R, POINT_CONVERSION_UNCOMPRESSED, buf, buf_len, ctx);

	if (!len) {
		free(buf);
		return 0;
	}

//	dump_hex("P1 - P2", buf, len);

	// Save Result As Big Int
	big_set_bin(out, buf, len, ptr, bi_size);

	// Get Value For Mangle State
	value = mpz_get_si(out);
	//	printf("Val: %d, %08X\n", value, value);

	// Free Resources
	free(buf);
	EC_POINT_free(P);
	EC_POINT_free(Q);
	EC_POINT_free(R);
	EC_GROUP_free(group);
	BN_CTX_free(ctx);

	return value;
}

extern uint32_t epl_ec_neg(mpz_t out, bool comp, mpz_t a, bool compa, int nid, size_t comp_sz, size_t uncomp_sz, mpz_t *ptr, uint32_t *bi_size) {
	BN_CTX *ctx = NULL;
	EC_GROUP *group = NULL;
	EC_POINT *P = NULL;

	uint8_t *buf;
	size_t buf_len, len;
	uint32_t value = 0;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99])|| (comp_sz <= 0) || (uncomp_sz <= 0))
		return 0;

	// Initialize EC Data
	ctx = BN_CTX_new();
	group = EC_GROUP_new_by_curve_name(nid);
	P = EC_POINT_new(group);

	len = (compa ? comp_sz : uncomp_sz);
	buf_len = uncomp_sz + 3;
	buf = malloc(buf_len);

	// Populate Data For Point
	if (a->_mp_size == 0) {		// Point 1 = Infinity
		buf[0] = 0;
		len = 1;
	}
	else {
		big_get_bin(a, buf, buf_len, ptr);
		if (!buf)
			return 0;
	}

//	dump_hex("Point", buf, len);

	if (!EC_POINT_oct2point(group, P, buf, len, ctx)) {
		free(buf);
		return 0;
	}

	// Invert Point
	if (!EC_POINT_invert(group, P, ctx)) {
		free(buf);
		return 0;
	}

	if (comp)
		len = EC_POINT_point2oct(group, P, POINT_CONVERSION_COMPRESSED, buf, buf_len, ctx);
	else
		len = EC_POINT_point2oct(group, P, POINT_CONVERSION_UNCOMPRESSED, buf, buf_len, ctx);

	if (!len) {
		free(buf);
		return 0;
	}

//	dump_hex("-P", buf, len);

	// Save Result As Big Int
	big_set_bin(out, buf, len, ptr, bi_size);

	// Get Value For Mangle State
	value = mpz_get_si(out);
//	printf("Val: %d, %08X\n", value, value);

	// Free Resources
	free(buf);
	EC_POINT_free(P);
	EC_GROUP_free(group);
	BN_CTX_free(ctx);

	return value;
}

extern uint32_t epl_ec_mul(mpz_t out, bool comp, mpz_t a, bool compa, mpz_t b, int nid, size_t comp_sz, size_t uncomp_sz, mpz_t *ptr, uint32_t *bi_size) {
	BN_CTX *ctx = NULL;
	EC_GROUP *group = NULL;
	EC_POINT *R = NULL;
	EC_POINT *Q = NULL;
	BIGNUM *N = NULL;

	uint8_t *buf;
	size_t buf_len, len, len_a, len_b;
	uint32_t value = 0;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[99]) || (a < ptr[0]) || (a > ptr[99]) || (b < ptr[0]) || (b > ptr[99]) || (comp_sz <= 0) || (uncomp_sz <= 0))
		return 0;

	// Initialize EC Data
	ctx = BN_CTX_new();
	group = EC_GROUP_new_by_curve_name(nid);
	R = EC_POINT_new(group);
	Q = EC_POINT_new(group);
	N = BN_new();

	len_a = (compa ? comp_sz : uncomp_sz);
	len_b = 4 * b->_mp_size;
	buf_len = MAX(len_b, uncomp_sz + 3);

	buf = malloc(buf_len);

	// Populate Data For Point 1
	if (a->_mp_size == 0) {		// Point 1 = Infinity
		buf[0] = 0;
		len_a = 1;
	}
	else {
		big_get_bin(a,buf, buf_len, ptr);
		if (!buf)
			return 0;
	}

//	dump_hex("Point1", buf, len_a);

	if (!EC_POINT_oct2point(group, Q, buf, len_a, ctx)) {
		free(buf);
		return 0;
	}

	// Populate Data For Scalar
	if (b->_mp_size == 0) {		// Scalar = Infinity
		buf[0] = 0;
		len_b = 1;
	}
	else {
		big_get_bin(b, buf, buf_len, ptr);
		if (!buf)
			return 0;
	}

//	dump_hex("Scalar", buf, len_b);

	if (!BN_bin2bn(buf, len_b, N)) {
		free(buf);
		return 0;
	}

	// Multiply Points
	if (!EC_POINT_mul(group, R, N, Q, NULL, ctx)) {
		free(buf);
		return 0;
	}

	if (comp)
		len = EC_POINT_point2oct(group, R, POINT_CONVERSION_COMPRESSED, buf, buf_len, ctx);
	else
		len = EC_POINT_point2oct(group, R, POINT_CONVERSION_UNCOMPRESSED, buf, buf_len, ctx);

	if (!len) {
		free(buf);
		return 0;
	}

//	dump_hex("P1 * P2", buf, len);

	// Save Result As Big Int
	big_set_bin(out, buf, len, ptr, bi_size);

	// Get Value For Mangle State
	value = mpz_get_si(out);
//	printf("Val: %d, %08X\n", value, value);

	// Free Resources
	free(buf);
	EC_POINT_free(R);
	EC_POINT_free(Q);
	BN_free(N);
	EC_GROUP_free(group);
	BN_CTX_free(ctx);

	return value;
}
