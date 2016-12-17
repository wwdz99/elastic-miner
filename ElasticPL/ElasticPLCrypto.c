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

extern uint32_t epl_ec_priv_to_pub(size_t idx, bool compressed, int32_t *mem, int nid, size_t len) {

	EC_KEY *key = NULL;
	EC_POINT *pub_key = NULL;
	const EC_GROUP *group = NULL;
	BIGNUM priv_key;

	uint8_t *input, buf[256];
	uint32_t *input32, value;
	uint32_t *buf32 = (uint32_t *)buf;
	size_t buf_len;
	int i, n;

	n = (int)(len / 4) + ((len % 4) ? 1 : 0);

	// Check Boundary Conditions Of Inputs
	if ((idx < 0) || (len <= 0) || ((idx + n) >= VM_MEMORY_SIZE))
		return 0;

	input = (uint8_t *)malloc(n * sizeof(int));
	if (!input)
		return 0;

	input32 = (uint32_t *)input;

	// Change Endianess Of Private Key
	for (i = 0; i < n; i++) {
		//		printf("m[%d] = %lu, %08X\n", i + idx, mem[idx + i], mem[idx + i]);
		input32[i] = swap32(mem[idx + i]);
	}

	//	dump_hex("PrivKey", (uint8_t*)(input), len);

	// Init Empty OpenSSL EC Keypair
	key = EC_KEY_new_by_curve_name(nid);
	if (!key)
		return 0;

	// Set Private Key Through BIGNUM
	BN_init(&priv_key);
	BN_bin2bn(input, len, &priv_key);
	if (!EC_KEY_set_private_key(key, &priv_key))
		return 0;

	// Derive Public Key From Private Key And Group
	group = EC_KEY_get0_group(key);
	pub_key = EC_POINT_new(group);
	if (!EC_POINT_mul(group, pub_key, &priv_key, NULL, NULL, NULL))
		return 0;

	memset(buf, 0, 256);
	if (compressed)
		buf_len = EC_POINT_point2oct(group, pub_key, POINT_CONVERSION_COMPRESSED, buf, 256, NULL);
	else
		buf_len = EC_POINT_point2oct(group, pub_key, POINT_CONVERSION_UNCOMPRESSED, buf, 256, NULL);

	if (!buf_len)
		return 0;

	//	dump_hex("PK", buf, buf_len);

	n = (int)(buf_len / 4) + ((buf_len % 4) ? 1 : 0);
	for (i = 0; i < n; i++) {
		mem[idx + i] = swap32(buf32[i]);
	}

	// Get Value For Mangle State
	value = swap32(buf32[0]);

	//	printf("Val: %d, %08X\n", value, value);

	// Free Resources
	free(input);
	EC_KEY_free(key);
	EC_POINT_free(pub_key);
	BN_clear_free(&priv_key);

	return value;
}

extern uint32_t epl_ec_add(size_t idx1, bool comp1, size_t idx2, bool comp2, bool comp, int32_t *mem, int nid, size_t comp_sz, size_t uncomp_sz) {

	BN_CTX *ctx = NULL;
	EC_GROUP *group = NULL;
	EC_POINT *P = NULL;
	EC_POINT *Q = NULL;
	EC_POINT *R = NULL;

	uint8_t *input1, *input2, buf[256];
	uint32_t *input1_32, *input2_32, value;
	uint32_t *buf32 = (uint32_t *)buf;
	size_t buf_len;
	size_t len1, len2;
	int i, n, n1, n2;

	if ((comp_sz <= 0) || (uncomp_sz <= 0))
		return 0;

	// Initialize Point Data
	if (!mem[idx1]) // Infinity
		len1 = 1;
	else
		len1 = ((comp1) ? comp_sz : uncomp_sz);

	if (!mem[idx2]) // Infinity
		len2 = 1;
	else
		len2 = ((comp2) ? comp_sz : uncomp_sz);

	n1 = (int)(len1 / 4) + ((len1 % 4) ? 1 : 0);
	n2 = (int)(len2 / 4) + ((len2 % 4) ? 1 : 0);

	if ((idx1 < 0) || (idx2 < 0) || ((idx1 + uncomp_sz) >= VM_MEMORY_SIZE) || ((idx2 + uncomp_sz) >= VM_MEMORY_SIZE))
		return 0;

	input1 = (uint8_t *)malloc(n1 * sizeof(int));
	input2 = (uint8_t *)malloc(n2 * sizeof(int));
	if (!input1 || !input2)
		return 0;

	input1_32 = (uint32_t *)input1;
	input2_32 = (uint32_t *)input2;

	// Change Endianess Of Input Data
	for (i = 0; i < n1; i++)
		input1_32[i] = swap32(mem[idx1 + i]);
	for (i = 0; i < n2; i++)
		input2_32[i] = swap32(mem[idx2 + i]);

	//	dump_hex("Point1", (uint8_t*)(input1), len1);
	//	dump_hex("Point2", (uint8_t*)(input2), len2);

	// Initialize EC Data
	ctx = BN_CTX_new();
	group = EC_GROUP_new_by_curve_name(nid);
	P = EC_POINT_new(group);
	Q = EC_POINT_new(group);
	R = EC_POINT_new(group);

	// Create EC Points & Add Them Together
	if (!EC_POINT_oct2point(group, P, input1, len1, ctx))
		return 0;
	if (!EC_POINT_oct2point(group, Q, input2, len2, ctx))
		return 0;
	if (!EC_POINT_add(group, R, P, Q, ctx))
		return 0;

	memset(buf, 0, 256);
	if (comp)
		buf_len = EC_POINT_point2oct(group, R, POINT_CONVERSION_COMPRESSED, buf, 256, ctx);
	else
		buf_len = EC_POINT_point2oct(group, R, POINT_CONVERSION_UNCOMPRESSED, buf, 256, ctx);

	if (!buf_len)
		return 0;

	//	dump_hex("Result", (uint8_t *)buf, buf_len);

	// Copy Result Back To VM
	n = (int)(buf_len / 4) + ((buf_len % 4) ? 1 : 0);
	for (i = 0; i < n; i++) {
		mem[idx1 + i] = swap32(buf32[i]);
	}

	// Get Value For Mangle State
	value = swap32(buf32[0]);

	//	printf("Val: %d, %08X\n", value, value);

	// Free Resources
	free(input1);
	free(input2);
	EC_POINT_free(P);
	EC_POINT_free(Q);
	EC_POINT_free(R);
	EC_GROUP_free(group);
	BN_CTX_free(ctx);

	return value;
}

extern uint32_t epl_ec_sub(size_t idx1, bool comp1, size_t idx2, bool comp2, bool comp, int32_t *mem, int nid, size_t comp_sz, size_t uncomp_sz) {

	BN_CTX *ctx = NULL;
	EC_GROUP *group = NULL;
	EC_POINT *P = NULL;
	EC_POINT *Q = NULL;
	EC_POINT *R = NULL;

	uint8_t *input1, *input2, buf[256];
	uint32_t *input1_32, *input2_32, value;
	uint32_t *buf32 = (uint32_t *)buf;
	size_t buf_len;
	size_t len1, len2;
	size_t i, n, n1, n2;

	if ((comp_sz <= 0) || (uncomp_sz <= 0))
		return 0;

	// Initialize Point Data
	if (!mem[idx1]) // Infinity
		len1 = 1;
	else
		len1 = ((comp1) ? comp_sz : uncomp_sz);

	if (!mem[idx2]) // Infinity
		len2 = 1;
	else
		len2 = ((comp2) ? comp_sz : uncomp_sz);

	n1 = (int)(len1 / 4) + ((len1 % 4) ? 1 : 0);
	n2 = (int)(len2 / 4) + ((len2 % 4) ? 1 : 0);

	if ((idx1 < 0) || (idx2 < 0) || ((idx1 + uncomp_sz) >= VM_MEMORY_SIZE) || ((idx2 + uncomp_sz) >= VM_MEMORY_SIZE))
		return 0;

	input1 = (uint8_t *)malloc(n1 * sizeof(int));
	input2 = (uint8_t *)malloc(n2 * sizeof(int));
	if (!input1 || !input2)
		return 0;

	input1_32 = (uint32_t *)input1;
	input2_32 = (uint32_t *)input2;

	// Change Endianess Of Input Data
	for (i = 0; i < n1; i++)
		input1_32[i] = swap32(mem[idx1 + i]);
	for (i = 0; i < n2; i++)
		input2_32[i] = swap32(mem[idx2 + i]);

	//	dump_hex("Point1", (uint8_t*)(input1), len1);
	//	dump_hex("Point2", (uint8_t*)(input2), len2);

	// Initialize EC Data
	ctx = BN_CTX_new();
	group = EC_GROUP_new_by_curve_name(nid);
	P = EC_POINT_new(group);
	Q = EC_POINT_new(group);
	R = EC_POINT_new(group);

	// Create EC Points, Invert Q, Then Add Them Together
	if (!EC_POINT_oct2point(group, P, input1, len1, ctx))
		return 0;
	if (!EC_POINT_oct2point(group, Q, input2, len2, ctx))
		return 0;
	if (!EC_POINT_invert(group, Q, ctx))
		return 0;
	if (!EC_POINT_add(group, R, P, Q, ctx))
		return 0;

	memset(buf, 0, 256);
	if (comp)
		buf_len = EC_POINT_point2oct(group, R, POINT_CONVERSION_COMPRESSED, buf, 256, ctx);
	else
		buf_len = EC_POINT_point2oct(group, R, POINT_CONVERSION_UNCOMPRESSED, buf, 256, ctx);

	if (!buf_len)
		return 0;

	//	dump_hex("Result", (uint8_t *)buf, buf_len);

	// Copy Result Back To VM
	n = (int)(buf_len / 4) + ((buf_len % 4) ? 1 : 0);
	for (i = 0; i < n; i++) {
		mem[idx1 + i] = swap32(buf32[i]);
	}

	// Get Value For Mangle State
	value = swap32(buf32[0]);

	//	printf("Val: %d, %08X\n", value, value);

	// Free Resources
	free(input1);
	free(input2);
	EC_POINT_free(P);
	EC_POINT_free(Q);
	EC_POINT_free(R);
	EC_GROUP_free(group);
	BN_CTX_free(ctx);

	return value;
}

extern uint32_t epl_ec_neg(size_t idx1, bool comp1, bool comp, int32_t *mem, int nid, size_t comp_sz, size_t uncomp_sz) {

	BN_CTX *ctx = NULL;
	EC_GROUP *group = NULL;
	EC_POINT *P = NULL;

	uint8_t *input1, buf[256];
	uint32_t *input1_32, value;
	uint32_t *buf32 = (uint32_t *)buf;
	size_t buf_len;
	size_t len1;
	size_t i, n, n1;

	if ((comp_sz <= 0) || (uncomp_sz <= 0))
		return 0;

	// Initialize Point Data
	if (!mem[idx1]) // Infinity
		len1 = 1;
	else
		len1 = ((comp1) ? comp_sz : uncomp_sz);

	n1 = (int)(len1 / 4) + ((len1 % 4) ? 1 : 0);

	if ((idx1 < 0) || ((idx1 + uncomp_sz) >= VM_MEMORY_SIZE))
		return 0;

	input1 = (uint8_t *)malloc(n1 * sizeof(int));
	if (!input1)
		return 0;

	input1_32 = (uint32_t *)input1;

	// Change Endianess Of Input Data
	for (i = 0; i < n1; i++)
		input1_32[i] = swap32(mem[idx1 + i]);

	//	dump_hex("Point1", (uint8_t*)(input1), len1);

	// Initialize EC Data
	ctx = BN_CTX_new();
	group = EC_GROUP_new_by_curve_name(nid);
	P = EC_POINT_new(group);

	// Create EC Points, Invert Q, Then Add Them Together
	if (!EC_POINT_oct2point(group, P, input1, len1, ctx))
		return 0;
	if (!EC_POINT_invert(group, P, ctx))
		return 0;

	memset(buf, 0, 256);
	if (comp)
		buf_len = EC_POINT_point2oct(group, P, POINT_CONVERSION_COMPRESSED, buf, 256, ctx);
	else
		buf_len = EC_POINT_point2oct(group, P, POINT_CONVERSION_UNCOMPRESSED, buf, 256, ctx);

	if (!buf_len)
		return 0;

	//	dump_hex("Result", (uint8_t *)buf, buf_len);

	// Copy Result Back To VM
	n = (int)(buf_len / 4) + ((buf_len % 4) ? 1 : 0);
	for (i = 0; i < n; i++) {
		mem[idx1 + i] = swap32(buf32[i]);
	}

	// Get Value For Mangle State
	value = swap32(buf32[0]);

	//	printf("Val: %d, %08X\n", value, value);

	// Free Resources
	free(input1);
	EC_POINT_free(P);
	EC_GROUP_free(group);
	BN_CTX_free(ctx);

	return value;
}

extern uint32_t epl_ec_mult(size_t idx1, bool comp1, size_t idx2, size_t n2, bool comp, int32_t *mem, int nid, size_t comp_sz, size_t uncomp_sz) {

	BN_CTX *ctx = NULL;
	EC_GROUP *group = NULL;
	EC_POINT *P = NULL;
	BIGNUM *s = NULL;

	uint8_t *input1, *input2, buf[256];
	uint32_t *input1_32, *input2_32, value;
	uint32_t *buf32 = (uint32_t *)buf;
	size_t buf_len;
	size_t len1, len2;
	size_t i, n, n1;

	if ((comp_sz <= 0) || (uncomp_sz <= 0) || (n2 <= 0))
		return 0;

	// Initialize Point Data
	if (!mem[idx1]) // Infinity
		len1 = 1;
	else
		len1 = ((comp1) ? comp_sz : uncomp_sz);

	len2 = n2 * 4;
	n1 = (int)(len1 / 4) + ((len1 % 4) ? 1 : 0);

	if ((idx1 < 0) || (idx2 < 0) || ((idx1 + uncomp_sz) >= VM_MEMORY_SIZE) || ((idx2 + n2) >= VM_MEMORY_SIZE))
		return 0;

	input1 = (uint8_t *)malloc(n1 * sizeof(int));
	input2 = (uint8_t *)malloc(n2 * sizeof(int));
	if (!input1 || !input2)
		return 0;

	input1_32 = (uint32_t *)input1;
	input2_32 = (uint32_t *)input2;

	// Change Endianess Of Input Data
	for (i = 0; i < n1; i++)
		input1_32[i] = swap32(mem[idx1 + i]);
	for (i = 0; i < n2; i++)
		input2_32[i] = swap32(mem[idx2 + i]);

	//	dump_hex("Point1", (uint8_t*)(input1), len1);
	//	dump_hex("Scalar", (uint8_t*)(input2), len2);

	// Initialize EC Data
	ctx = BN_CTX_new();
	group = EC_GROUP_new_by_curve_name(nid);
	P = EC_POINT_new(group);
	s = BN_new();

	// Create EC Point & Scalar, Then Multiply Them Together
	if (!EC_POINT_oct2point(group, P, input1, len1, ctx))
		return 0;
	if (!BN_bin2bn(input2, len2, s))
		return 0;
	if (!EC_POINT_mul(group, P, s, NULL, NULL, ctx))
		return 0;

	memset(buf, 0, 256);
	if (comp)
		buf_len = EC_POINT_point2oct(group, P, POINT_CONVERSION_COMPRESSED, buf, 256, ctx);
	else
		buf_len = EC_POINT_point2oct(group, P, POINT_CONVERSION_UNCOMPRESSED, buf, 256, ctx);

	if (!buf_len)
		return 0;

	//	dump_hex("Result", (uint8_t *)buf, buf_len);

	// Copy Result Back To VM
	n = (int)(buf_len / 4) + ((buf_len % 4) ? 1 : 0);
	for (i = 0; i < n; i++) {
		mem[idx1 + i] = swap32(buf32[i]);
	}

	// Get Value For Mangle State
	value = swap32(buf32[0]);

	//	printf("Val: %d, %08X\n", value, value);

	// Free Resources
	free(input1);
	free(input2);
	EC_POINT_free(P);
	EC_GROUP_free(group);
	BN_free(s);
	BN_CTX_free(ctx);

	return value;
}