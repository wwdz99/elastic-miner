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
#include <malloc.h>

#include "sha2.h"
#include "elasticpl_crypto.h"

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

	n = (int)(len / 4);

	// Check Boundary Conditions Of Inputs
	if ((idx < 0) || (len <= 0) || ((idx + n) > VM_MEMORY_SIZE))
		return 0;

	msg = (unsigned char *)malloc(len);
	if (!msg)
		return 0;

	msg32 = (uint32_t *)msg;

	// Change Endianess Of Message
	for (i = 0; i < n; i++) {
		msg32[i] = swap32(mem[idx + i]);
	}

	dump_hex("Msg", (unsigned char*)(msg), 80);

	sha256_init(&ctx);
	sha256_update(&ctx, msg, len);
	sha256_final(&ctx, hash);

	dump_hex("Hash", (unsigned char*)(hash), 32);

	for (i = 0; i < 8; i++) {
		mem[idx + i] = swap32(hash32[i]);
	}

	// Get Value For Mangle State
	value = swap32(hash32[0]);

	printf("Val: %d, %08X\n", value, value);

	free(msg);
	return value;
}


extern uint32_t epl_ec_priv_to_pub(size_t idx, bool compressed, int32_t *mem, int nid, size_t len ) {

	EC_KEY *key = NULL;
	EC_POINT *pub_key = NULL;
	const EC_GROUP *group = NULL;
	BIGNUM priv_key;
	BN_CTX *ctx;

	uint8_t *input, *pk;
	uint32_t *input32, *pk32, value;
	size_t len_pk;
	int i, n;


	n = (int)(len / 4);

	// Check Boundary Conditions Of Inputs
	if ((idx < 0) || (len <= 0) || ((idx + n) > VM_MEMORY_SIZE))
		return 0;

	input = (unsigned char *)malloc(len);
	if (!input)
		return 0;

	input32 = (uint32_t *)input;

	// Change Endianess Of Message
	for (i = 0; i < n; i++) {
		input32[i] = swap32(mem[idx + i]);
	}

	dump_hex("PrivKey", (uint8_t*)(input), len);

	// Init Empty OpenSSL EC Keypair
	key = EC_KEY_new_by_curve_name(nid);

	// Set Private Key Through BIGNUM
	BN_init(&priv_key);
	BN_bin2bn(input, len, &priv_key);
	EC_KEY_set_private_key(key, &priv_key);

	// Derive Public Key From Private Key And Group
	ctx = BN_CTX_new();
//	BN_CTX_start(ctx);
	group = EC_KEY_get0_group(key);
	pub_key = EC_POINT_new(group);
	EC_POINT_mul(group, pub_key, &priv_key, NULL, NULL, ctx);
	EC_KEY_set_public_key(key, pub_key);

	if (compressed)
		EC_KEY_set_conv_form(key, POINT_CONVERSION_COMPRESSED);
	else
		EC_KEY_set_conv_form(key, POINT_CONVERSION_UNCOMPRESSED);

	len_pk = i2o_ECPublicKey(key, NULL);
	pk = calloc(len_pk, sizeof(uint8_t));

	pk32 = (uint32_t *)pk;
	if (!i2o_ECPublicKey(key, &pk))
			return 0;

	dump_hex("PK:", (uint8_t *)pk32, len_pk);

	n = (int)((len_pk + 3) / 4);
	for (i = 0; i < n; i++) {
		mem[idx + i] = swap32(pk32[i]);
	}

	// Get Value For Mangle State
	value = swap32(pk32[0]);

	printf("Val: %d, %08X\n", value, value);

	// Free Resources
	free(pk32);
	free(input);
	EC_POINT_free(pub_key);
	//BN_CTX_end(ctx);
	BN_CTX_free(ctx);
	BN_clear_free(&priv_key);

	return value;
}
