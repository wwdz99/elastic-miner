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

static uint32_t swap32(int a) {
	return ((a << 24) | ((a << 8) & 0x00FF0000) | ((a >> 8) & 0x0000FF00) | ((a >> 24) & 0x000000FF));
}

extern uint32_t epl_sha256(int idx, int len, int32_t *mem) {
	sha256_ctx ctx;
	unsigned char *msg;
	unsigned char hash[32];
	uint32_t *hash32 = (uint32_t *)hash;
	uint32_t *msg32;
	uint32_t value;
	int i, n;

	// Check Boundary Conditions Of Inputs
	if ((idx <= 0) || (len <= 0) || ((idx + len) > 64000))
		return 0;

	msg = (unsigned char *)malloc(len);
	if (!msg)
		return 0;

	msg32 = (uint32_t *)msg;

	// Change Endianess Of Message
	n = (int)(len / 4);
	for (i = 0; i < n; i++) {
		msg32[i] = swap32(mem[idx + i]);
	}

//	unsigned char* b = (unsigned char*)(msg);
//	printf("Msg: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15], b[16], b[17], b[18], b[19], b[20], b[21], b[22], b[23], b[24], b[25], b[26], b[27], b[28], b[29], b[30], b[31]);

	sha256_init(&ctx);
	sha256_update(&ctx, msg, len);
	sha256_final(&ctx, hash);

//	b = (unsigned char*)(hash);
//	printf("Hsh: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15], b[16], b[17], b[18], b[19], b[20], b[21], b[22], b[23], b[24], b[25], b[26], b[27], b[28], b[29], b[30], b[31]);


	for (i = 0; i < 8; i++) {
		mem[idx + i] = swap32(hash32[i]);
	}

	// Get Value For Mangle State
	value = swap32(hash32[0]);

//	printf("Val: %d, %08X\n", value, value);

	free(msg);
	return value;
}