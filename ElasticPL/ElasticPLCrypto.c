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

#ifndef VM_MEMORY_SIZE
#define VM_MEMORY_SIZE 64000
#endif

static uint32_t swap32(int a) {
	return ((a << 24) | ((a << 8) & 0x00FF0000) | ((a >> 8) & 0x0000FF00) | ((a >> 24) & 0x000000FF));
}

static void dump_hex(unsigned char *label, unsigned char *bytes, int len) {
	int i;

	printf("%s: ", label);
	for (i = 0; i < len; i++)
		printf("%02X", bytes[i]);
	printf("\n");
}

extern uint32_t epl_sha256(int idx, int len, int32_t *mem) {
	sha256_ctx ctx;
	unsigned char *msg;
	unsigned char hash[32];
	uint32_t *hash32 = (uint32_t *)hash;
	uint32_t *msg32;
	uint32_t value;
	int i, n;

	n = (int)(len / 4) + ((len % 4) ? 1 : 0);

	// Check Boundary Conditions Of Inputs
	if ((idx < 0) || (len <= 0) || ((idx + n) >= VM_MEMORY_SIZE) || ((idx + 8) >= VM_MEMORY_SIZE))
		return 0;

	msg = (unsigned char *)malloc(n * sizeof(int));
	if (!msg)
		return 0;

	msg32 = (uint32_t *)msg;

	// Change Endianess Of Message
	for (i = 0; i < n; i++) {
		msg32[i] = swap32(mem[idx + i]);
	}

	//	dump_hex("Msg", (unsigned char*)(msg), len);

	sha256_init(&ctx);
	sha256_update(&ctx, msg, len);
	sha256_final(&ctx, hash);

	//	dump_hex("Hash", (unsigned char*)(hash), 32);

	for (i = 0; i < 8; i++) {
		mem[idx + i] = swap32(hash32[i]);
	}

	// Get Value For Mangle State
	value = swap32(hash32[0]);

	//	printf("Val: %d, %08X\n", value, value);

	free(msg);
	return value;
}

extern uint32_t epl_sha512(int idx, int len, int32_t *mem) {
	unsigned char *msg;
	unsigned char hash[64];
	uint32_t *hash32 = (uint32_t *)hash;
	uint32_t *msg32;
	uint32_t value;
	int i, n;

	n = (int)(len / 4) + ((len % 4) ? 1 : 0);

	// Check Boundary Conditions Of Inputs
	if ((idx < 0) || (len <= 0) || ((idx + n) >= VM_MEMORY_SIZE) || ((idx + 16) >= VM_MEMORY_SIZE))
		return 0;

	msg = (unsigned char *)malloc(n * sizeof(int));
	if (!msg)
		return 0;

	msg32 = (uint32_t *)msg;

	// Change Endianess Of Message
	for (i = 0; i < n; i++) {
		msg32[i] = swap32(mem[idx + i]);
	}

	//	dump_hex("Msg", (unsigned char*)(msg), len);

	SHA512(msg, len, hash);

	//	dump_hex("Hash", (unsigned char*)(hash), 64);

	for (i = 0; i < 16; i++) {
		mem[idx + i] = swap32(hash32[i]);
	}

	// Get Value For Mangle State
	value = swap32(hash32[0]);

	//	printf("Val: %d, %08X\n", value, value);

	free(msg);
	return value;
}

extern uint32_t epl_md5(int idx, int len, int32_t *mem) {
	unsigned char *msg;
	unsigned char hash[16];
	uint32_t *hash32 = (uint32_t *)hash;
	uint32_t *msg32;
	uint32_t value;
	int i, n;

	n = (int)(len / 4) + ((len % 4) ? 1 : 0);

	// Check Boundary Conditions Of Inputs
	if ((idx < 0) || (len <= 0) || ((idx + n) >= VM_MEMORY_SIZE) || ((idx + 4) >= VM_MEMORY_SIZE))
		return 0;

	msg = (unsigned char *)malloc(n * sizeof(int));
	if (!msg)
		return 0;

	msg32 = (uint32_t *)msg;

	// Change Endianess Of Message
	for (i = 0; i < n; i++) {
		msg32[i] = swap32(mem[idx + i]);
	}

	//	dump_hex("Msg", (unsigned char*)(msg), len);

	MD5(msg, len, hash);

	//	dump_hex("Hash", (unsigned char*)(hash), 16);

	for (i = 0; i < 4; i++) {
		mem[idx + i] = swap32(hash32[i]);
	}

	// Get Value For Mangle State
	value = swap32(hash32[0]);

	//	printf("Val: %d, %08X\n", value, value);

	free(msg);
	return value;
}

extern uint32_t epl_ripemd160(int idx, int len, int32_t *mem) {
	unsigned char *msg;
	unsigned char hash[20];
	uint32_t *hash32 = (uint32_t *)hash;
	uint32_t *msg32;
	uint32_t value;
	int i, n;

	n = (int)(len / 4) + ((len % 4) ? 1 : 0);

	// Check Boundary Conditions Of Inputs
	if ((idx < 0) || (len <= 0) || ((idx + n) >= VM_MEMORY_SIZE) || ((idx + 5) >= VM_MEMORY_SIZE))
		return 0;

	msg = (unsigned char *)malloc(n * sizeof(int));
	if (!msg)
		return 0;

	msg32 = (uint32_t *)msg;

	// Change Endianess Of Message
	for (i = 0; i < n; i++) {
		msg32[i] = swap32(mem[idx + i]);
	}

	//	dump_hex("Msg", (unsigned char*)(msg), len);

	RIPEMD160(msg, len, hash);

	//	dump_hex("Hash", (unsigned char*)(hash), 16);

	for (i = 0; i < 5; i++) {
		mem[idx + i] = swap32(hash32[i]);
	}

	// Get Value For Mangle State
	value = swap32(hash32[0]);

	//	printf("Val: %d, %08X\n", value, value);

	free(msg);
	return value;
}


extern uint32_t epl_whirlpool(int idx, int len, int32_t *mem) {
	unsigned char *msg;
	unsigned char hash[64];
	uint32_t *hash32 = (uint32_t *)hash;
	uint32_t *msg32;
	uint32_t value;
	int i, n;

	n = (int)(len / 4) + ((len % 4) ? 1 : 0);

	// Check Boundary Conditions Of Inputs
	if ((idx < 0) || (len <= 0) || ((idx + n) >= VM_MEMORY_SIZE) || ((idx + 16) >= VM_MEMORY_SIZE))
		return 0;

	msg = (unsigned char *)malloc(n * sizeof(int));
	if (!msg)
		return 0;

	msg32 = (uint32_t *)msg;

	// Change Endianess Of Message
	for (i = 0; i < n; i++) {
		msg32[i] = swap32(mem[idx + i]);
	}

	//	dump_hex("Msg", (unsigned char*)(msg), len);

	whirlpool_hash(msg, len, hash);

	//	dump_hex("Hash", (unsigned char*)(hash), 64);

	for (i = 0; i < 16; i++) {
		mem[idx + i] = swap32(hash32[i]);
	}

	// Get Value For Mangle State
	value = swap32(hash32[0]);

	//	printf("Val: %d, %08X\n", value, value);

	free(msg);
	return value;
}

extern uint32_t epl_ec_priv_to_pub(mpz_t out, mpz_t a, bool compressed, int nid, size_t len, mpz_t *ptr, uint32_t *bi_size) {

	EC_KEY *key = NULL;
	EC_POINT *pub_key = NULL;
	const EC_GROUP *group = NULL;
	BIGNUM *priv_key = NULL;

	uint8_t *buf;
	size_t buf_len;
	uint32_t value = 0;

	// Init Empty OpenSSL EC Keypair
	key = EC_KEY_new_by_curve_name(nid);
	if (!key)
		return 0;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[99]) || (len <= 0))
		return 0;

	if (compressed)
		buf_len = len + 1;
	else
		buf_len = ((len * 2) + 1);

	buf = malloc(buf_len);

	// Populate Buffer With Big Int Data
	big_get_bin(a, (uint32_t *)buf, len, ptr);
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

	// Write Public Key To Big Int
	big_set_bin(out, buf, len, ptr, bi_size);

	// Get Value For Mangle State
//	value = swap32(buf32[0]);

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

	buf_len = uncomp_sz + 3;
	buf = malloc(buf_len);

	len_a = (compa ? comp_sz : uncomp_sz);
	len_b = (compb ? comp_sz : uncomp_sz);

	// Populate Data For Point 1
	if (a->_mp_size == 0) {		// Point 1 = Infinity
		buf[3] = 0;
		len_a = 1;
	}
	else {
		big_get_bin(a, (uint32_t *)buf, buf_len, ptr);
		if (!buf)
			return 0;
	}

//	dump_hex("Point1", buf + 3, len_a);

	if (!EC_POINT_oct2point(group, P, buf + 3, len_a, ctx)) {
		free(buf);
		return 0;
	}

	// Populate Data For Point 2
	if (b->_mp_size == 0) {		// Point 2 = Infinity
		buf[3] = 0;
		len_b = 1;
	}
	else {
		big_get_bin(b, (uint32_t *)buf, buf_len, ptr);
		if (!buf)
			return 0;
	}

//	dump_hex("Point2", buf + 3, len_b);

	if (!EC_POINT_oct2point(group, Q, buf + 3, len_b, ctx)) {
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

	big_set_bin(out, buf, len, ptr, bi_size);

	// Get Value For Mangle State
//	value = swap32(buf32[0]);

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

	buf_len = uncomp_sz + 3;
	buf = malloc(buf_len);

	len_a = (compa ? comp_sz : uncomp_sz);
	len_b = (compb ? comp_sz : uncomp_sz);

	// Populate Data For Point 1
	if (a->_mp_size == 0) {		// Point 1 = Infinity
		buf[3] = 0;
		len_a = 1;
	}
	else {
		big_get_bin(a, (uint32_t *)buf, buf_len, ptr);
		if (!buf)
			return 0;
	}

//	dump_hex("Point1", buf + 3, len_a);

	if (!EC_POINT_oct2point(group, P, buf + 3, len_a, ctx)) {
		free(buf);
		return 0;
	}

	// Populate Data For Point 2
	if (b->_mp_size == 0) {		// Point 2 = Infinity
		buf[3] = 0;
		len_b = 1;
	}
	else {
		big_get_bin(b, (uint32_t *)buf, buf_len, ptr);
		if (!buf)
			return 0;
	}

//	dump_hex("Point2", buf + 3, len_b);

	if (!EC_POINT_oct2point(group, Q, buf + 3, len_b, ctx)) {
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

	big_set_bin(out, buf, len, ptr, bi_size);

	// Get Value For Mangle State
	//	value = swap32(buf32[0]);

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

	buf_len = uncomp_sz + 3;
	buf = malloc(buf_len);

	len = (compa ? comp_sz : uncomp_sz);

	// Populate Data For Point
	if (a->_mp_size == 0) {		// Point 1 = Infinity
		buf[3] = 0;
		len = 1;
	}
	else {
		big_get_bin(a, (uint32_t *)buf, buf_len, ptr);
		if (!buf)
			return 0;
	}

//	dump_hex("Point", buf + 3, len);

	if (!EC_POINT_oct2point(group, P, buf + 3, len, ctx)) {
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

	big_set_bin(out, buf, len, ptr, bi_size);

	// Get Value For Mangle State
	//	value = swap32(buf32[0]);

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

	if (len_b > (len_a + 2))
		buf_len = len_b;
	else
		buf_len = uncomp_sz + 3;

	buf = malloc(buf_len);

	// Populate Data For Point 1
	if (a->_mp_size == 0) {		// Point 1 = Infinity
		buf[3] = 0;
		len_a = 1;
	}
	else {
		big_get_bin(a, (uint32_t *)buf, buf_len, ptr);
		if (!buf)
			return 0;
	}

//	dump_hex("Point1", buf + 3, len_a);

	if (!EC_POINT_oct2point(group, Q, buf + 3, len_a, ctx)) {
		free(buf);
		return 0;
	}

	// Populate Data For Scalar
	if (b->_mp_size == 0) {		// Scalar = Infinity
		buf[0] = 0;
		len_b = 1;
	}
	else {
		big_get_bin(b, (uint32_t *)buf, buf_len, ptr);
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

	big_set_bin(out, buf, len, ptr, bi_size);

	// Get Value For Mangle State
	//	value = swap32(buf32[0]);

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
