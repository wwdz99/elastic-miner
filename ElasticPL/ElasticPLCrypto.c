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

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[VM_BI_SIZE]) || (a < ptr[0]) || (a > ptr[VM_BI_SIZE]))
		return 0;

	sha256_init(&ctx);

	if (a->_mp_size) {
		buf_len = a->_mp_size * 4;
		buf = malloc(buf_len);

		// Populate Buffer With Big Int Data
		len = big_get_bin(a, buf, buf_len, ptr);
		if (!len) {
			free(buf);
			return 0;
		}
		sha256_update(&ctx, buf, len);
		free(buf);
	}
	else
		sha256_update(&ctx, "", 0);

	sha256_final(&ctx, hash);

//	dump_hex("Hash", hash, 32);

	// Save Result As Big Int
	big_set_bin(out, hash, 32, ptr, bi_size);

	// Return Lowest 32 Bit of Big Int To Be Mangled
	return mpz_get_si(out);
}

extern uint32_t epl_sha512(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size) {
	uint8_t hash[64], *buf = NULL;
	size_t buf_len, len;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[VM_BI_SIZE]) || (a < ptr[0]) || (a > ptr[VM_BI_SIZE]))
		return 0;

	if (a->_mp_size) {
		buf_len = a->_mp_size * 4;
		buf = malloc(buf_len);

		// Populate Buffer With Big Int Data
		len = big_get_bin(a, buf, buf_len, ptr);
		if (!len) {
			free(buf);
			return 0;
		}
		SHA512(buf, len, hash);
		free(buf);
	}
	else
		SHA512("", 0, hash);

//	dump_hex("Hash", hash, 64);

	// Save Result As Big Int
	big_set_bin(out, hash, 64, ptr, bi_size);

	// Return Lowest 32 Bit of Big Int To Be Mangled
	return mpz_get_si(out);;
}

extern uint32_t epl_md5(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size) {
	uint8_t hash[16], *buf = NULL;
	size_t buf_len, len;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[VM_BI_SIZE]) || (a < ptr[0]) || (a > ptr[VM_BI_SIZE]))
		return 0;

	if (a->_mp_size) {
		buf_len = a->_mp_size * 4;
		buf = malloc(buf_len);

		// Populate Buffer With Big Int Data
		len = big_get_bin(a, buf, buf_len, ptr);
		if (!len) {
			free(buf);
			return 0;
		}
		MD5(buf, len, hash);
		free(buf);
	}
	else
		MD5("", 0, hash);

//	dump_hex("Hash", hash, 16);

	// Save Result As Big Int
	big_set_bin(out, hash, 16, ptr, bi_size);

	// Return Lowest 32 Bit of Big Int To Be Mangled
	return mpz_get_si(out);
}

extern uint32_t epl_ripemd160(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size) {
	uint8_t hash[20], *buf = NULL;
	size_t buf_len, len;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[VM_BI_SIZE]) || (a < ptr[0]) || (a > ptr[VM_BI_SIZE]))
		return 0;

	if (a->_mp_size) {
		buf_len = a->_mp_size * 4;
		buf = malloc(buf_len);

		// Populate Buffer With Big Int Data
		len = big_get_bin(a, buf, buf_len, ptr);
		if (!len) {
			free(buf);
			return 0;
		}
		RIPEMD160(buf, len, hash);
		free(buf);
	}
	else
		RIPEMD160("", 0, hash);

//	dump_hex("Hash", hash, 20);

	// Save Result As Big Int
	big_set_bin(out, hash, 20, ptr, bi_size);

	// Return Lowest 32 Bit of Big Int To Be Mangled
	return mpz_get_si(out);
}

extern uint32_t epl_whirlpool(mpz_t out, mpz_t a, mpz_t *ptr, uint32_t *bi_size) {
	uint8_t hash[64], *buf = NULL;
	size_t buf_len, len;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[VM_BI_SIZE]) || (a < ptr[0]) || (a > ptr[VM_BI_SIZE]))
		return 0;

	if (a->_mp_size) {
		buf_len = a->_mp_size * 4;
		buf = malloc(buf_len);

		// Populate Buffer With Big Int Data
		len = big_get_bin(a, buf, buf_len, ptr);
		if (!len) {
			free(buf);
			return 0;
		}
		whirlpool_hash(buf, len, hash);
		free(buf);
	}
	else
		whirlpool_hash("", 0, hash);

//	dump_hex("Hash", hash, 64);

	// Save Result As Big Int
	big_set_bin(out, hash, 64, ptr, bi_size);

	// Return Lowest 32 Bit of Big Int To Be Mangled
	return mpz_get_si(out);
}

extern uint32_t epl_ec_priv_to_pub(mpz_t out, mpz_t a, bool compressed, int nid, size_t comp_sz, size_t uncomp_sz, mpz_t *ptr, uint32_t *bi_size) {

	EC_KEY *key = NULL;
	EC_POINT *pub_key = NULL;
	const EC_GROUP *group = NULL;
	BIGNUM *priv_key = NULL;

	uint8_t buf[EC_BUF_SZ];
	size_t len;

	// Init Empty OpenSSL EC Keypair
	key = EC_KEY_new_by_curve_name(nid);
	if (!key)
		return 0;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[VM_BI_SIZE]))
		return 0;

	// Populate Buffer With Big Int Data
	len = big_get_bin(a, buf, EC_BUF_SZ, ptr);
	if (len != ((compressed) ? comp_sz : uncomp_sz))
		return 0;
	
//	dump_hex("PrivKey", buf, len);

	// Set Private Key Through BIGNUM
	priv_key = BN_new();
	BN_bin2bn(buf, len, priv_key);
	if (!EC_KEY_set_private_key(key, priv_key))
		return 0;

	// Derive Public Key From Private Key And Group
	group = EC_KEY_get0_group(key);
	pub_key = EC_POINT_new(group);
	if (!EC_POINT_mul(group, pub_key, priv_key, NULL, NULL, NULL))
		return 0;

	if (compressed)
		len = EC_POINT_point2oct(group, pub_key, POINT_CONVERSION_COMPRESSED, buf, EC_BUF_SZ, NULL);
	else
		len = EC_POINT_point2oct(group, pub_key, POINT_CONVERSION_UNCOMPRESSED, buf, EC_BUF_SZ, NULL);

	if (!len)
		return 0;

//	dump_hex("PK", buf, len);

	// Save Result As Big Int
	big_set_bin(out, buf, len, ptr, bi_size);

	// Free Resources
	EC_KEY_free(key);
	EC_POINT_free(pub_key);
	BN_clear_free(priv_key);

	// Return Lowest 32 Bit of Big Int To Be Mangled
	return mpz_get_si(out);
}

extern uint32_t epl_ec_add(mpz_t out, bool comp, mpz_t a, bool compa, mpz_t b, bool compb, int nid, size_t comp_sz, size_t uncomp_sz, mpz_t *ptr, uint32_t *bi_size) {
	BN_CTX *ctx = NULL;
	EC_GROUP *group = NULL;
	EC_POINT *P = NULL;
	EC_POINT *Q = NULL;
	EC_POINT *R = NULL;

	uint8_t buf[EC_BUF_SZ];
	size_t len;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[VM_BI_SIZE]) || (a < ptr[0]) || (a > ptr[VM_BI_SIZE]) || (b < ptr[0]) || (b > ptr[VM_BI_SIZE]) || (comp_sz <= 0) || (uncomp_sz <= 0))
		return 0;

	// Initialize EC Data
	ctx = BN_CTX_new();
	group = EC_GROUP_new_by_curve_name(nid);
	P = EC_POINT_new(group);
	Q = EC_POINT_new(group);
	R = EC_POINT_new(group);

	// Populate Data For Point 1
	if (a->_mp_size == 0) {		// Point 1 = Infinity
		buf[0] = 0;
		len = 1;
	}
	else {
		len = big_get_bin(a, buf, EC_BUF_SZ, ptr);
		if (len != (compa ? comp_sz : uncomp_sz))
			return 0;
	}
	if (!EC_POINT_oct2point(group, P, buf, len, ctx))
		return 0;

//	dump_hex("Point1", buf, len);

	// Populate Data For Point 2
	if (b->_mp_size == 0) {		// Point 2 = Infinity
		buf[0] = 0;
		len = 1;
	}
	else {
		len = big_get_bin(b, buf, EC_BUF_SZ, ptr);
		if (len != (compb ? comp_sz : uncomp_sz))
			return 0;
	}
	if (!EC_POINT_oct2point(group, Q, buf, len, ctx))
		return 0;

//	dump_hex("Point2", buf, len);

	// Add Points
	if (!EC_POINT_add(group, R, P, Q, ctx))
		return 0;

	if (comp)
		len = EC_POINT_point2oct(group, R, POINT_CONVERSION_COMPRESSED, buf, EC_BUF_SZ, ctx);
	else
		len = EC_POINT_point2oct(group, R, POINT_CONVERSION_UNCOMPRESSED, buf, EC_BUF_SZ, ctx);

	if (!len)
		return 0;

//	dump_hex("P1 + P2", buf, len);

	// Save Result As Big Int
	big_set_bin(out, buf, len, ptr, bi_size);

	// Free Resources
	EC_POINT_free(P);
	EC_POINT_free(Q);
	EC_POINT_free(R);
	EC_GROUP_free(group);
	BN_CTX_free(ctx);

	// Return Lowest 32 Bit of Big Int To Be Mangled
	return mpz_get_si(out);
}

extern uint32_t epl_ec_sub(mpz_t out, bool comp, mpz_t a, bool compa, mpz_t b, bool compb, int nid, size_t comp_sz, size_t uncomp_sz, mpz_t *ptr, uint32_t *bi_size) {
	BN_CTX *ctx = NULL;
	EC_GROUP *group = NULL;
	EC_POINT *P = NULL;
	EC_POINT *Q = NULL;
	EC_POINT *R = NULL;

	uint8_t buf[EC_BUF_SZ];
	size_t len;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[VM_BI_SIZE]) || (a < ptr[0]) || (a > ptr[VM_BI_SIZE]) || (b < ptr[0]) || (b > ptr[VM_BI_SIZE]) || (comp_sz <= 0) || (uncomp_sz <= 0))
		return 0;

	// Initialize EC Data
	ctx = BN_CTX_new();
	group = EC_GROUP_new_by_curve_name(nid);
	P = EC_POINT_new(group);
	Q = EC_POINT_new(group);
	R = EC_POINT_new(group);

	// Populate Data For Point 1
	if (a->_mp_size == 0) {		// Point 1 = Infinity
		buf[0] = 0;
		len = 1;
	}
	else {
		len = big_get_bin(a, buf, EC_BUF_SZ, ptr);
		if (len != (compa ? comp_sz : uncomp_sz))
			return 0;
	}
	if (!EC_POINT_oct2point(group, P, buf, len, ctx))
		return 0;

//	dump_hex("Point1", buf, len);

	// Populate Data For Point 2
	if (b->_mp_size == 0) {		// Point 1 = Infinity
		buf[0] = 0;
		len = 1;
	}
	else {
		len = big_get_bin(b, buf, EC_BUF_SZ, ptr);
		if (len != (compb ? comp_sz : uncomp_sz))
			return 0;
	}
	if (!EC_POINT_oct2point(group, Q, buf, len, ctx))
		return 0;

//	dump_hex("Point2", buf, len);

	// Invert Q, Then Add Points
	if (!EC_POINT_invert(group, Q, ctx))
		return 0;
	if (!EC_POINT_add(group, R, P, Q, ctx))
		return 0;

	if (comp)
		len = EC_POINT_point2oct(group, R, POINT_CONVERSION_COMPRESSED, buf, EC_BUF_SZ, ctx);
	else
		len = EC_POINT_point2oct(group, R, POINT_CONVERSION_UNCOMPRESSED, buf, EC_BUF_SZ, ctx);

	if (!len) {
		free(buf);
		return 0;
	}

//	dump_hex("P1 - P2", buf, len);

	// Save Result As Big Int
	big_set_bin(out, buf, len, ptr, bi_size);

	// Free Resources
	EC_POINT_free(P);
	EC_POINT_free(Q);
	EC_POINT_free(R);
	EC_GROUP_free(group);
	BN_CTX_free(ctx);

	// Return Lowest 32 Bit of Big Int To Be Mangled
	return mpz_get_si(out);
}

extern uint32_t epl_ec_neg(mpz_t out, bool comp, mpz_t a, bool compa, int nid, size_t comp_sz, size_t uncomp_sz, mpz_t *ptr, uint32_t *bi_size) {
	BN_CTX *ctx = NULL;
	EC_GROUP *group = NULL;
	EC_POINT *P = NULL;

	uint8_t buf[EC_BUF_SZ];
	size_t len;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[VM_BI_SIZE]) || (a < ptr[0]) || (a > ptr[VM_BI_SIZE])|| (comp_sz <= 0) || (uncomp_sz <= 0))
		return 0;

	// Initialize EC Data
	ctx = BN_CTX_new();
	group = EC_GROUP_new_by_curve_name(nid);
	P = EC_POINT_new(group);

	// Populate Data For Point
	if (a->_mp_size == 0) {		// Point 1 = Infinity
		buf[0] = 0;
		len = 1;
	}
	else {
		len = big_get_bin(a, buf, EC_BUF_SZ, ptr);
		if (len != (compa ? comp_sz : uncomp_sz))
			return 0;
	}
	if (!EC_POINT_oct2point(group, P, buf, len, ctx))
		return 0;

//	dump_hex("Point", buf, len);

	// Invert Point
	if (!EC_POINT_invert(group, P, ctx))
		return 0;

	if (comp)
		len = EC_POINT_point2oct(group, P, POINT_CONVERSION_COMPRESSED, buf, EC_BUF_SZ, ctx);
	else
		len = EC_POINT_point2oct(group, P, POINT_CONVERSION_UNCOMPRESSED, buf, EC_BUF_SZ, ctx);

	if (!len)
		return 0;

//	dump_hex("-P", buf, len);

	// Save Result As Big Int
	big_set_bin(out, buf, len, ptr, bi_size);

	// Free Resources
	EC_POINT_free(P);
	EC_GROUP_free(group);
	BN_CTX_free(ctx);

	// Return Lowest 32 Bit of Big Int To Be Mangled
	return mpz_get_si(out);
}

extern uint32_t epl_ec_mul(mpz_t out, bool comp, mpz_t a, bool compa, mpz_t b, int nid, size_t comp_sz, size_t uncomp_sz, mpz_t *ptr, uint32_t *bi_size) {
	BN_CTX *ctx = NULL;
	EC_GROUP *group = NULL;
	EC_POINT *R = NULL;
	EC_POINT *Q = NULL;
	BIGNUM *N = NULL;

	uint8_t buf[EC_BUF_SZ], *buf_b;
	size_t len, len_b;

	// Validate Inputs
	if ((out < ptr[0]) || (out > ptr[VM_BI_SIZE]) || (a < ptr[0]) || (a > ptr[VM_BI_SIZE]) || (b < ptr[0]) || (b > ptr[VM_BI_SIZE]) || (comp_sz <= 0) || (uncomp_sz <= 0))
		return 0;

	// Initialize EC Data
	ctx = BN_CTX_new();
	group = EC_GROUP_new_by_curve_name(nid);
	R = EC_POINT_new(group);
	Q = EC_POINT_new(group);
	N = BN_new();

	// Populate Data For Point 1
	if (a->_mp_size == 0) {		// Point 1 = Infinity
		buf[0] = 0;
		len = 1;
	}
	else {
		len = big_get_bin(a, buf, EC_BUF_SZ, ptr);
		if (len != (compa ? comp_sz : uncomp_sz))
			return 0;
	}
	if (!EC_POINT_oct2point(group, Q, buf, len, ctx))
		return 0;

	dump_hex("Point1", buf, len);

	// Populate Data For Scalar
	if (b->_mp_size == 0) {		// Scalar = Infinity
		buf_b = malloc(1);
		buf_b[0] = 0;
		len_b = 1;
	}
	else {
		len_b = 4 * b->_mp_size;
		buf_b = malloc(len_b);
		len = big_get_bin(b, buf_b, len_b, ptr);
		if (!len) {
			free(buf_b);
			return 0;
		}
	}
	if (!BN_bin2bn(buf_b, len_b, N)) {
		free(buf_b);
		return 0;
	}

	dump_hex("Scalar", buf_b, len_b);
	free(buf_b);


	// Multiply Points
	if (!EC_POINT_mul(group, R, N, Q, NULL, ctx))
		return 0;

	if (comp)
		len = EC_POINT_point2oct(group, R, POINT_CONVERSION_COMPRESSED, buf, EC_BUF_SZ, ctx);
	else
		len = EC_POINT_point2oct(group, R, POINT_CONVERSION_UNCOMPRESSED, buf, EC_BUF_SZ, ctx);

	if (!len)
		return 0;

//	dump_hex("P1 * P2", buf, len);

	// Save Result As Big Int
	big_set_bin(out, buf, len, ptr, bi_size);

	// Free Resources
	EC_POINT_free(R);
	EC_POINT_free(Q);
	BN_free(N);
	EC_GROUP_free(group);
	BN_CTX_free(ctx);

	// Return Lowest 32 Bit of Big Int To Be Mangled
	return mpz_get_si(out);
}
