/*
* Copyright 2016 sprocket
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free
* Software Foundation; either version 2 of the License, or (at your option)
* any later version.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "ElasticPL.h"
#include "../crypto/elasticpl_crypto.h"
#include "../miner.h"

char blk_new[4096];
char blk_old[4096];

uint32_t wcet_block;

extern char* convert_ast_to_c() {
	blk_new[0] = 0;
	blk_old[0] = 0;

	char* ret = NULL;
	int i;
	for (i = 0; i < vm_ast_cnt; i++) {
		ret = append_strings(ret, convert(vm_ast[i]));
	}
	return ret;
}

// Use Post Order Traversal To Translate The Expressions In The AST to C
static char* convert(ast* exp) {
	char* lval = 0;
	char* rval = 0;
	char* tmp = 0;
	char *result = malloc(sizeof(char) * 256);
	result[0] = 0;

	if (exp != NULL) {

		// Reset Temp Block Strings
		if (exp->type != NODE_BLOCK) {
			blk_new[0] = 0;
			blk_old[0] = 0;
		}

		if (exp->left != NULL) {
			lval = convert(exp->left);
		}

		// Check For If Statement As Right Side Is Conditional
		if (exp->type != NODE_IF)
			rval = convert(exp->right);

		switch (exp->type) {
		case NODE_CONSTANT:
			if (exp->value < 0 || exp->value > VM_MEMORY_SIZE)
				sprintf(result, "0");
			else
				sprintf(result, "%d", exp->value);
			break;
		case NODE_VAR_CONST:
			if (exp->value < 0 || exp->value > VM_MEMORY_SIZE)
				sprintf(result, "mem[0]");
			else
				sprintf(result, "mem[%d]", exp->value);
			break;
		case NODE_VAR_EXP:
			sprintf(result, "mem[%s]", lval);
			break;
		case NODE_ASSIGN:
			tmp = replace(lval, "mem[", "mem[m(");
			free(lval);
			lval = replace(tmp, "]", ")]");
			free(tmp);
			sprintf(result, "%s = m(%s);\n", lval, rval);
			break;
		case NODE_IF:
			if (exp->right->type != NODE_ELSE) {
				rval = convert(exp->right);				// If Body (No Else Condition)
				result = realloc(result, strlen(lval) + strlen(rval) + 256);
				sprintf(result, "if( %s ) {\n\t%s}\n", lval, rval);
			}
			else {
				tmp = lval;
				lval = convert(exp->right->left);		// If Body
				rval = convert(exp->right->right);		// Else Body
				result = realloc(result, strlen(lval) + strlen(rval) + 256);
				sprintf(result, "if( %s ) {\n\t%s}\nelse {\n\t%s}\n", tmp, lval, rval);
			}
			break;
		case NODE_ELSE:
			break;
		case NODE_REPEAT:
			result = realloc(result, (2 * strlen(lval)) + strlen(rval) + 256);
			sprintf(result, "if ( %s > 0 ) {\n\tint loop%d;\n\tfor (loop%d = 0; loop%d < ( %s ); loop%d++) {\n\t%s\t}\n}\n", lval, exp->token_num, exp->token_num, exp->token_num, lval, exp->token_num, rval);
			break;
		case NODE_BLOCK:
			if (!blk_old[0])
				sprintf(blk_new, "%s", lval);
			else
				sprintf(blk_new, "%s\t%s", lval, blk_old);
			result = realloc(result, strlen(blk_new) + 1);
			sprintf(result, "%s", blk_new);
			strcpy(blk_old, blk_new);
			break;
		case NODE_ADD:
			sprintf(result, "m(%s + %s)", lval, rval);
			break;
		case NODE_SUB:
			sprintf(result, "m(%s - %s)", lval, rval);
			break;
		case NODE_MUL:
			sprintf(result, "m(%s * %s)", lval, rval);
			break;
		case NODE_DIV:
			sprintf(result, "((%s != 0) ? m(%s / %s) : m(0))", rval, lval, rval);
			break;
		case NODE_MOD:
			sprintf(result, "((%s > 0) ? m(%s %%%% %s) : m(0))", rval, lval, rval);
			break;
		case NODE_LSHIFT:
			sprintf(result, "m(%s << %s)", lval, rval);
			break;
		case NODE_LROT:
			sprintf(result, "m(rotl32( %s, %s %%%% 32))", lval, rval);
			break;
		case NODE_RSHIFT:
			sprintf(result, "m(%s >> %s)", lval, rval);
			break;
		case NODE_RROT:
			sprintf(result, "m(rotr32( %s, %s %%%% 32))", lval, rval);
			break;
		case NODE_NOT:
			sprintf(result, "m(!%s)", lval);
			break;
		case NODE_COMPL:
			sprintf(result, "m(~%s)", lval);
			break;
		case NODE_AND:
			sprintf(result, "m(%s >! %s)", lval, rval);		// Required To Match Java Results
//			sprintf(result, "m(%s && %s)", lval, rval);
			break;
		case NODE_OR:
			sprintf(result, "m(%s || %s)", lval, rval);
			break;
		case NODE_BITWISE_AND:
			sprintf(result, "m(%s & %s)", lval, rval);
			break;
		case NODE_BITWISE_XOR:
			sprintf(result, "m(%s ^ %s)", lval, rval);
			break;
		case NODE_BITWISE_OR:
			sprintf(result, "m(%s | %s)", lval, rval);
			break;
		case NODE_EQ:
			sprintf(result, "m(%s == %s)", lval, rval);
			break;
		case NODE_NE:
			sprintf(result, "m(%s != %s)", lval, rval);
			break;
		case NODE_GT:
			sprintf(result, "m(%s > %s)", lval, rval);
			break;
		case NODE_LT:
			sprintf(result, "m(%s < %s)", lval, rval);
			break;
		case NODE_GE:
			sprintf(result, "m(%s >= %s)", lval, rval);
			break;
		case NODE_LE:
			sprintf(result, "m(%s <= %s)", lval, rval);
			break;
		case NODE_NEG:
			sprintf(result, "m(-%s)", lval);
			break;
		case NODE_VERIFY:
			sprintf(result, "\n\trc = m(%s != 0 ? 1 : 0);\n\n", lval);
			break;
		case NODE_PARAM:
			if (!blk_old[0])
				sprintf(blk_new, "%s", lval);
			else
				sprintf(blk_new, "%s, %s", lval, blk_old);
			result = realloc(result, strlen(blk_new) + 1);
			sprintf(result, "%s", blk_new);
			strcpy(blk_old, blk_new);
			break;
		case NODE_SHA256:
			sprintf(result, "\tm(epl_sha256( %s, mem ));\n", rval);
			break;
		case NODE_SHA512:
			sprintf(result, "\tm(epl_sha512( %s, mem ));\n", rval);
			break;
		case NODE_WHIRLPOOL:
			sprintf(result, "\tm(epl_whirlpool( %s, mem ));\n", rval);
			break;
		case NODE_MD5:
			sprintf(result, "\tm(epl_md5( %s, mem ));\n", rval);
			break;
		case NODE_SECP192K_PTP:
			sprintf(result, "\tm(epl_ec_priv_to_pub( %s, mem, NID_secp192k1, 24 ));\n", rval);
			break;
		case NODE_SECP192K_PA:
			sprintf(result, "\tm(epl_ec_add( %s, mem, NID_secp192k1, 25, 49 ));\n", rval);
			break;
		case NODE_SECP192K_PS:
			sprintf(result, "\tm(epl_ec_sub( %s, mem, NID_secp192k1, 25, 49 ));\n", rval);
			break;
		case NODE_SECP192K_PSM:
			sprintf(result, "\tm(epl_ec_mult( %s, mem, NID_secp192k1, 25, 49 ));\n", rval);
			break;
		case NODE_SECP192K_PN:
			sprintf(result, "\tm(epl_ec_neg( %s, mem, NID_secp192k1, 25, 49 ));\n", rval);
			break;
		case NODE_SECP192R_PTP:
			sprintf(result, "SECP192R1PrivToPub( %s );\n", rval);
			break;
		case NODE_SECP192R_PA:
			sprintf(result, "SECP192R1PointAdd( %s );\n", rval);
			break;
		case NODE_SECP192R_PS:
			sprintf(result, "SECP192R1PointSub( %s );\n", rval);
			break;
		case NODE_SECP192R_PSM:
			sprintf(result, "SECP192R1PointScalarMult( %s );\n", rval);
			break;
		case NODE_SECP192R_PN:
			sprintf(result, "SECP192R1PointNegate( %s );\n", rval);
			break;
		case NODE_SECP224K_PTP:
			sprintf(result, "\tm(epl_ec_priv_to_pub( %s, mem, NID_secp224k1, 28 ));\n", rval);
			break;
		case NODE_SECP224K_PA:
			sprintf(result, "\tm(epl_ec_add( %s, mem, NID_secp224k1, 29, 57 ));\n", rval);
			break;
		case NODE_SECP224K_PS:
			sprintf(result, "\tm(epl_ec_sub( %s, mem, NID_secp224k1, 29, 57 ));\n", rval);
			break;
		case NODE_SECP224K_PSM:
			sprintf(result, "\tm(epl_ec_mult( %s, mem, NID_secp224k1, 29, 57 ));\n", rval);
			break;
		case NODE_SECP224K_PN:
			sprintf(result, "\tm(epl_ec_neg( %s, mem, NID_secp224k1, 29, 57 ));\n", rval);
			break;
		case NODE_SECP224R_PTP:
			sprintf(result, "\tm(epl_ec_priv_to_pub( %s, mem, NID_secp224r1, 28 ));\n", rval);
			break;
		case NODE_SECP224R_PA:
			sprintf(result, "\tm(epl_ec_add( %s, mem, NID_secp224r1, 29, 57 ));\n", rval);
			break;
		case NODE_SECP224R_PS:
			sprintf(result, "\tm(epl_ec_sub( %s, mem, NID_secp224r1, 29, 57 ));\n", rval);
			break;
		case NODE_SECP224R_PSM:
			sprintf(result, "\tm(epl_ec_mult( %s, mem, NID_secp224r1, 29, 57 ));\n", rval);
			break;
		case NODE_SECP224R_PN:
			sprintf(result, "\tm(epl_ec_neg( %s, mem, NID_secp224r1, 29, 57 ));\n", rval);
			break;
		case NODE_SECP256K_PTP:
			sprintf(result, "\tm(epl_ec_priv_to_pub( %s, mem, NID_secp256k1, 32 ));\n", rval);
			break;
		case NODE_SECP256K_PA:
			sprintf(result, "\tm(epl_ec_add( %s, mem, NID_secp256k1, 33, 65 ));\n", rval);
			break;
		case NODE_SECP256K_PS:
			sprintf(result, "\tm(epl_ec_sub( %s, mem, NID_secp256k1, 33, 65 ));\n", rval);
			break;
		case NODE_SECP256K_PSM:
			sprintf(result, "\tm(epl_ec_mult( %s, mem, NID_secp256k1, 33, 65 ));\n", rval);
			break;
		case NODE_SECP256K_PN:
			sprintf(result, "\tm(epl_ec_neg( %s, mem, NID_secp256k1, 33, 65 ));\n", rval);
			break;
		case NODE_SECP256R_PTP:
//Missing
			sprintf(result, "SECP256R1PrivToPub( %s );\n", rval);
//Missing
			break;
		case NODE_SECP256R_PA:
			sprintf(result, "SECP256R1PointAdd( %s );\n", rval);
			break;
		case NODE_SECP256R_PS:
			sprintf(result, "SECP256R1PointSub( %s );\n", rval);
			break;
		case NODE_SECP256R_PSM:
			sprintf(result, "SECP256R1PointScalarMult( %s );\n", rval);
			break;
		case NODE_SECP256R_PN:
			sprintf(result, "SECP256R1PointNegate( %s );\n", rval);
			break;
		case NODE_SECP384R_PTP:
			sprintf(result, "\tm(epl_ec_priv_to_pub( %s, mem, NID_secp384r1, 48 ));\n", rval);
			break;
		case NODE_SECP384R_PA:
			sprintf(result, "\tm(epl_ec_add( %s, mem, NID_secp384r1, 49, 97 ));\n", rval);
			break;
		case NODE_SECP384R_PS:
			sprintf(result, "\tm(epl_ec_sub( %s, mem, NID_secp384r1, 49, 97 ));\n", rval);
			break;
		case NODE_SECP384R_PSM:
			sprintf(result, "\tm(epl_ec_mult( %s, mem, NID_secp384r1, 49, 97 ));\n", rval);
			break;
		case NODE_SECP384R_PN:
			sprintf(result, "\tm(epl_ec_neg( %s, mem, NID_secp384r1, 49, 97 ));\n", rval);
			break;
		case NODE_PRM192V1_PTP:
			sprintf(result, "\tm(epl_ec_priv_to_pub( %s, mem, NID_X9_62_prime192v1, 24 ));\n", rval);
			break;
		case NODE_PRM192V1_PA:
			sprintf(result, "\tm(epl_ec_add( %s, mem, NID_X9_62_prime192v1, 25, 49 ));\n", rval);
			break;
		case NODE_PRM192V1_PS:
			sprintf(result, "\tm(epl_ec_sub( %s, mem, NID_X9_62_prime192v1, 25, 49 ));\n", rval);
			break;
		case NODE_PRM192V1_PSM:
			sprintf(result, "\tm(epl_ec_mult( %s, mem, NID_X9_62_prime192v1, 25, 49 ));\n", rval);
			break;
		case NODE_PRM192V1_PN:
			sprintf(result, "\tm(epl_ec_neg( %s, mem, NID_X9_62_prime192v1, 25, 49 ));\n", rval);
			break;
		case NODE_PRM192V2_PTP:
			sprintf(result, "\tm(epl_ec_priv_to_pub( %s, mem, NID_X9_62_prime192v2, 24 ));\n", rval);
			break;
		case NODE_PRM192V2_PA:
			sprintf(result, "\tm(epl_ec_add( %s, mem, NID_X9_62_prime192v2, 25, 49 ));\n", rval);
			break;
		case NODE_PRM192V2_PS:
			sprintf(result, "\tm(epl_ec_sub( %s, mem, NID_X9_62_prime192v2, 25, 49 ));\n", rval);
			break;
		case NODE_PRM192V2_PSM:
			sprintf(result, "\tm(epl_ec_mult( %s, mem, NID_X9_62_prime192v2, 25, 49 ));\n", rval);
			break;
		case NODE_PRM192V2_PN:
			sprintf(result, "\tm(epl_ec_neg( %s, mem, NID_X9_62_prime192v2, 25, 49 ));\n", rval);
			break;
		case NODE_PRM192V3_PTP:
			sprintf(result, "\tm(epl_ec_priv_to_pub( %s, mem, NID_X9_62_prime192v3, 24 ));\n", rval);
			break;
		case NODE_PRM192V3_PA:
			sprintf(result, "\tm(epl_ec_add( %s, mem, NID_X9_62_prime192v3, 25, 49 ));\n", rval);
			break;
		case NODE_PRM192V3_PS:
			sprintf(result, "\tm(epl_ec_sub( %s, mem, NID_X9_62_prime192v3, 25, 49 ));\n", rval);
			break;
		case NODE_PRM192V3_PSM:
			sprintf(result, "\tm(epl_ec_mult( %s, mem, NID_X9_62_prime192v3, 25, 49 ));\n", rval);
			break;
		case NODE_PRM192V3_PN:
			sprintf(result, "\tm(epl_ec_neg( %s, mem, NID_X9_62_prime192v3, 25, 49 ));\n", rval);
			break;
		case NODE_PRM256V1_PTP:
			sprintf(result, "\tm(epl_ec_priv_to_pub( %s, mem, NID_X9_62_prime256v1, 32 ));\n", rval);
			break;
		case NODE_PRM256V1_PA:
			sprintf(result, "\tm(epl_ec_add( %s, mem, NID_X9_62_prime256v1, 33, 65 ));\n", rval);
			break;
		case NODE_PRM256V1_PS:
			sprintf(result, "\tm(epl_ec_sub( %s, mem, NID_X9_62_prime256v1, 33, 65 ));\n", rval);
			break;
		case NODE_PRM256V1_PSM:
			sprintf(result, "\tm(epl_ec_mult( %s, mem, NID_X9_62_prime256v1, 33, 65 ));\n", rval);
			break;
		case NODE_PRM256V1_PN:
			sprintf(result, "\tm(epl_ec_neg( %s, mem, NID_X9_62_prime256v1, 33, 65 ));\n", rval);
			break;
		case NODE_TIGER:
			sprintf(result, "Tiger( %s );\n", rval);
			break;
		case NODE_RIPEMD160:
			sprintf(result, "\tm(epl_ripemd160( %s, mem ));\n", rval);
			break;
		case NODE_RIPEMD128:
			sprintf(result, "RIPEMD128( %s );\n", rval);
			break;
		default:
			sprintf(result, "fprintf(stderr, \"ERROR: VM Runtime - Unsupported Operation (%d)\n\");\n", exp->type);
		}
	}

	return result;
}

static char* append_strings(char * old, char * new) {
	if (new == NULL && old != NULL) return old;
	if (old == NULL && new != NULL) return new;
	if (new == NULL && old == NULL) return NULL;

	// find the size of the string to allocate
	const size_t old_len = strlen(old), new_len = strlen(new);
	const size_t out_len = old_len + new_len + 1;

	// allocate a pointer to the new string
	char *out = malloc(out_len);

	// concat both strings and return
	memcpy(out, old, old_len);
	memcpy(out + old_len, new, new_len + 1);

	// Free here
	free(old);
	free(new);

	return out;
}

static char *replace(char* old, char* a, char* b) {
	int idx = 0;
	char *str = calloc(2, strlen(old));
	char *ptr1, *ptr2;

	ptr1 = old;
	ptr2 = old;

	while (ptr2) {
		ptr2 = strstr(ptr1, a);
		if (ptr2) {
			strncpy(str + idx, ptr1, ptr2 - ptr1);
			strcat(str, b);
			ptr1 = ptr2 + strlen(a);
			idx += strlen(str + idx);
		}
	}

	strcat(str, ptr1);

	return str;
}

extern uint32_t calc_wcet() {
	int i;
	uint32_t wcet, total = 0;

	wcet_block = 0;

	for (i = 0; i < vm_ast_cnt; i++) {
		wcet = get_wcet(vm_ast[i]);
		applog(LOG_DEBUG, "DEBUG: Statement WCET = %lu", wcet);
		if (wcet > (0xFFFFFFFF - total)) {
			total = 0xFFFFFFFF;
			break;
		}
		else
			total += wcet;
	}

	applog(LOG_DEBUG, "DEBUG: Total WCET = %lu", total);

	return total;
}

// Use Post Order Traversal To Calculate WCET For Each Statement
static uint32_t get_wcet(ast* exp) {
	uint32_t lval = 0;
	uint32_t rval = 0;
	uint32_t tmp = 0;
	uint32_t wcet = 0;

	if (exp != NULL) {

		// Reset Temp Block WCET Value
		if (exp->type != NODE_BLOCK)
			wcet_block = 0;

		if (exp->left != NULL) {
			lval = get_wcet(exp->left);
		}

		// Check For If Statement As Right Side Is Conditional
		if (exp->type != NODE_IF)
			rval = get_wcet(exp->right);

		switch (exp->type) {
		case NODE_CONSTANT:
		case NODE_VAR_CONST:
		case NODE_VAR_EXP:
			wcet = (wcet < 0xFFFFFFFF ? 1 : 0);
			break;
		case NODE_ASSIGN:
			wcet = (wcet < (0xFFFFFFFF - (lval + rval + 1)) ? (lval + rval + 1) : 0);
			break;
		case NODE_IF:
			if (exp->right->type != NODE_ELSE) {
				rval = get_wcet(exp->right);				// If Body (No Else Condition)
				wcet = (wcet < (0xFFFFFFFF - (lval + rval + 1)) ? (lval + rval + 1) : 0);
			}
			else {
				tmp = lval;									// Condition
				lval = get_wcet(exp->right->left);			// If Body
				rval = get_wcet(exp->right->right);			// Else Body
				if (lval >= rval)
					wcet = (wcet < (0xFFFFFFFF - (tmp + lval + 1)) ? (tmp + lval + 1) : 0);
				else
					wcet = (wcet < (0xFFFFFFFF - (tmp + rval + 1)) ? (tmp + rval + 1) : 0);
			}
			break;
		case NODE_ELSE:
			break;
		case NODE_REPEAT:
			tmp = exp->left->value;
			wcet = (wcet < (0xFFFFFFFF - ((tmp * rval) + 1)) ? ((tmp * rval) + 1) : 0);
			break;
		case NODE_BLOCK:
			wcet_block += lval;
			wcet = wcet_block;
			break;
		case NODE_ADD:
		case NODE_SUB:
		case NODE_MUL:
		case NODE_DIV:
		case NODE_MOD:
		case NODE_LSHIFT:
		case NODE_LROT:
		case NODE_RSHIFT:
		case NODE_RROT:
		case NODE_AND:
		case NODE_OR:
		case NODE_BITWISE_AND:
		case NODE_BITWISE_XOR:
		case NODE_BITWISE_OR:
		case NODE_EQ:
		case NODE_NE:
		case NODE_GT:
		case NODE_LT:
		case NODE_GE:
		case NODE_LE:
			wcet = (wcet < (0xFFFFFFFF - (lval + rval + 1)) ? (lval + rval + 1) : 0);
			break;
		case NODE_NOT:
		case NODE_COMPL:
		case NODE_NEG:
			wcet = (wcet < (0xFFFFFFFF - (lval + 1)) ? (lval + 1) : 0);
			break;
		case NODE_VERIFY:
			wcet = (wcet < (0xFFFFFFFF - (lval + rval + 1)) ? (lval + rval + 1) : 0);
			break;
		case NODE_PARAM:
			wcet_block += lval;
			wcet = wcet_block;
			break;
		case NODE_SHA256:
			wcet = (wcet < (0xFFFFFFFF - (100 + rval)) ? (100 + rval) : 0);
			break;
		case NODE_SHA512:
			wcet = (wcet < (0xFFFFFFFF - (120 + rval)) ? (120 + rval) : 0);
			break;
		case NODE_WHIRLPOOL:
			wcet = (wcet < (0xFFFFFFFF - (150 + rval)) ? (150 + rval) : 0);
			break;
		case NODE_MD5:
			wcet = (wcet < (0xFFFFFFFF - (80 + rval)) ? (80 + rval) : 0);
			break;
		case NODE_SECP192K_PTP:
		case NODE_SECP192R_PTP:
		case NODE_SECP224K_PTP:
		case NODE_SECP224R_PTP:
		case NODE_SECP256K_PTP:
		case NODE_SECP256R_PTP:
		case NODE_SECP384R_PTP:
		case NODE_PRM192V2_PTP:
		case NODE_PRM192V3_PTP:
		case NODE_PRM256V1_PTP:
			wcet = (wcet < (0xFFFFFFFF - (5000 + rval)) ? (5000 + rval) : 0);
			break;
		case NODE_SECP192K_PA:
		case NODE_SECP192K_PS:
		case NODE_SECP192K_PSM:
		case NODE_SECP192K_PN:
		case NODE_SECP192R_PA:
		case NODE_SECP192R_PS:
		case NODE_SECP192R_PSM:
		case NODE_SECP192R_PN:
		case NODE_SECP224K_PA:
		case NODE_SECP224K_PS:
		case NODE_SECP224K_PSM:
		case NODE_SECP224K_PN:
		case NODE_SECP224R_PA:
		case NODE_SECP224R_PS:
		case NODE_SECP224R_PSM:
		case NODE_SECP224R_PN:
		case NODE_SECP256K_PA:
		case NODE_SECP256K_PS:
		case NODE_SECP256K_PSM:
		case NODE_SECP256K_PN:
		case NODE_SECP256R_PA:
		case NODE_SECP256R_PS:
		case NODE_SECP256R_PSM:
		case NODE_SECP256R_PN:
		case NODE_SECP384R_PA:
		case NODE_SECP384R_PS:
		case NODE_SECP384R_PSM:
		case NODE_SECP384R_PN:
		case NODE_PRM192V1_PTP:
		case NODE_PRM192V1_PA:
		case NODE_PRM192V1_PS:
		case NODE_PRM192V1_PSM:
		case NODE_PRM192V1_PN:
		case NODE_PRM192V2_PA:
		case NODE_PRM192V2_PS:
		case NODE_PRM192V2_PSM:
		case NODE_PRM192V2_PN:
		case NODE_PRM192V3_PA:
		case NODE_PRM192V3_PS:
		case NODE_PRM192V3_PSM:
		case NODE_PRM192V3_PN:
		case NODE_PRM256V1_PA:
		case NODE_PRM256V1_PS:
		case NODE_PRM256V1_PSM:
		case NODE_PRM256V1_PN:
			wcet = (wcet < (0xFFFFFFFF - (1000 + rval)) ? (1000 + rval) : 0);
			break;
		case NODE_TIGER:
			wcet = (wcet < (0xFFFFFFFF - (100 + rval)) ? (100 + rval) : 0);
			break;
		case NODE_RIPEMD160:
			wcet = (wcet < (0xFFFFFFFF - (120 + rval)) ? (120 + rval) : 0);
			break;
		case NODE_RIPEMD128:
			wcet = (wcet < (0xFFFFFFFF - (100 + rval)) ? (100 + rval) : 0);
			break;
		default:
			break;
		}
	}

	return wcet;
}

static void push(long l, bool memory) {
	vm_stack[++vm_stack_idx].value = l;
	vm_stack[vm_stack_idx].memory = memory;
}

static vm_stack_item pop_item() {
	return vm_stack[vm_stack_idx--];
}

static long pop() {
	if (vm_stack[vm_stack_idx].memory)
		return vm_mem[vm_stack[vm_stack_idx--].value];
	else
		return vm_stack[vm_stack_idx--].value;
}

extern int interpret_ast() {
	int i;

	vm_bounty = false;

	for (i = 0; i < vm_ast_cnt; i++) {
		if (!interpret(vm_ast[i], false))
			return 0;
	}

	if (vm_stack_idx != -1)
		applog(LOG_WARNING, "WARNING: Possible VM Memory Leak");

	return vm_bounty;
}

static const unsigned int mask32 = (CHAR_BIT * sizeof(uint32_t) - 1);

#ifdef _MSC_VER
static uint32_t rotl32(uint32_t x, int n) {
#else
static uint32_t rotl32(uint32_t x, unsigned int n) {
#endif
n &= mask32;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers
	return (x << n) | (x >> ((-n)&mask32));
}

#ifdef _MSC_VER
static uint32_t rotr32(uint32_t x, int n) {
#else
static uint32_t rotr32(uint32_t x, unsigned int n) {
#endif
	n &= mask32;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers
	return (x >> n) | (x << ((-n)&mask32));
}

static int mangle_state(int x) {
	int mod = x % 32;
	int leaf = mod % 4;
	if (leaf == 0) {
		vm_state[0] = rotl32(vm_state[0], mod);
		vm_state[0] = vm_state[0] ^ x;
	}
	else if (leaf == 1) {
		vm_state[1] = rotl32(vm_state[1], mod);
		vm_state[1] = vm_state[1] ^ x;
	}
	else if (leaf == 2) {
		vm_state[2] = rotl32(vm_state[2], mod);
		vm_state[2] = vm_state[2] ^ x;
	}
	else {
		vm_state[3] = rotl32(vm_state[3], mod);
		vm_state[3] = vm_state[3] ^ x;
	}
	return x;
}


//vm_stack_item d_vm_stack[100];

static bool get_param(ast *exp, int *param, size_t num) {
	if (num > 0) {
		if (!exp->right->left)
			return false;
		else
			param[0] = exp->right->left->value;
	}
	if (num > 1) {
		if (!exp->right->right->left)
			return false;
		else
			param[1] = exp->right->right->left->value;
	}
	if (num > 2) {
		if (!exp->right->right->right->left)
			return false;
		else
			param[2] = exp->right->right->right->left->value;
	}
	if (num > 3) {
		if (!exp->right->right->right->right->left)
			return false;
		else
			param[3] = exp->right->right->right->right->left->value;
	}
	if (num > 4) {
		if (!exp->right->right->right->right->right->left)
			return false;
		else
			param[4] = exp->right->right->right->right->right->left->value;
	}
	if (num > 5 || num <= 0)
		return false;

	return true;
}

static int32_t interpret(ast* exp, bool mangle) {
	long lval, rval, val;
	int param[5];

	if (exp == NULL)
		return 0;

	switch (exp->type) {
		case NODE_CONSTANT:
			return exp->value;
		case NODE_VAR_CONST:
			if (exp->value < 0 || exp->value > VM_MEMORY_SIZE)
				val = 0;
			else
				val = exp->value;
			if (mangle) {
				mangle_state(val);
				return val;
			}
			return vm_mem[val];
		case NODE_VAR_EXP:
			lval = interpret(exp->left, false);
			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;
			else
				return vm_mem[lval];
		case NODE_ASSIGN:
			rval = interpret(exp->right, false);
			mangle_state(rval);
			lval = interpret(exp->left, true);
			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;
			vm_mem[lval] = rval;
			return 1;
		case NODE_IF:
			if (exp->right->type != NODE_ELSE) {
				if (interpret(exp->left, false))
					return interpret(exp->right, false);			// If Body (No Else Condition)
			}
			else {
				if (interpret(exp->left, false))
					return interpret(exp->right->left, false);		// If Body
				else
					return interpret(exp->right->right, false);		// Else Body
			}
			break;
		case NODE_REPEAT:
			lval = interpret(exp->left, false);
			if (lval > 0) {
				int i;
				for (i = 0; i < lval; i++)
					interpret(exp->right, false);					// Repeat Body
			}
			return 1;
		case NODE_BLOCK:
			interpret(exp->left, false);
			if (exp->right)
				interpret(exp->right, false);
			return 1;
		case NODE_ADD:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = (lval + rval);
			mangle_state(val);
			return val;
		case NODE_SUB:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = (lval - rval);
			mangle_state(val);
			return val;
		case NODE_MUL:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = (lval * rval);
			mangle_state(val);
			return val;
		case NODE_DIV:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			if (rval != 0)
				val = lval / rval;
			else
				val = 0;
			mangle_state(val);
			return val;
		case NODE_MOD:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			if (rval > 0)
				val = lval % rval;
			else
				val = 0;
			mangle_state(val);
			return val;
		case NODE_LSHIFT:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = (lval << rval);
			mangle_state(val);
			return val;
		case NODE_LROT:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = rotl32(lval, rval % 32);
			mangle_state(val);
			return val;
		case NODE_RSHIFT:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = (lval >> rval);
			mangle_state(val);
			return val;
		case NODE_RROT:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = rotr32(lval, rval % 32);
			mangle_state(val);
			return val;
		case NODE_NOT:
			lval = interpret(exp->left, false);
			mangle_state(!lval);
			return !lval;
		case NODE_COMPL:
			lval = interpret(exp->left, false);
			mangle_state(~lval);
			return ~lval;
		case NODE_AND:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
//			val = (lval && rval);
			val = (lval >! rval);
			mangle_state(val);
			return val;
		case NODE_OR:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = (lval || rval);
			mangle_state(val);
			return val;
		case NODE_BITWISE_AND:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = (lval & rval);
			mangle_state(val);
			return val;
		case NODE_BITWISE_XOR:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = (lval ^ rval);
			mangle_state(val);
			return val;
		case NODE_BITWISE_OR:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = (lval | rval);
			mangle_state(val);
			return val;
		case NODE_EQ:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = (lval == rval);
			mangle_state(val);
			return val;
		case NODE_NE:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = (lval != rval);
			mangle_state(val);
			return val;
		case NODE_GT:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = (lval > rval);
			mangle_state(val);
			return val;
		case NODE_LT:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = (lval < rval);
			mangle_state(val);
			return val;
		case NODE_GE:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = (lval >= rval);
			mangle_state(val);
			return val;
		case NODE_LE:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			val = (lval <= rval);
			mangle_state(val);
			return val;
		case NODE_NEG:
			lval = interpret(exp->left, false);
			mangle_state(-lval);
			return -lval;
		case NODE_VERIFY:
			rval = interpret(exp->right, false);
			lval = interpret(exp->left, false);
			vm_bounty = (lval != rval);
			mangle_state(vm_bounty);
			return vm_bounty;
		case NODE_PARAM:
			break;
		case NODE_SHA256:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_sha256(param[0], param[1], vm_mem);
			mangle_state(val);
			return 1;
		case NODE_SHA512:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_sha512(param[0], param[1], vm_mem);
			mangle_state(val);
			return 1;
		case NODE_WHIRLPOOL:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_whirlpool(param[0], param[1], vm_mem);
			mangle_state(val);
			return 1;
		case NODE_MD5:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_md5(param[0], param[1], vm_mem);
			mangle_state(val);
			return 1;
		case NODE_SECP192K_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_mem, NID_secp192k1, 24);
			mangle_state(val);
			return 1;
		case NODE_SECP192K_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_secp192k1, 25, 49);
			mangle_state(val);
			return 1;
		case NODE_SECP192K_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_secp192k1, 25, 49);
			mangle_state(val);
			return 1;
		case NODE_SECP192K_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_secp192k1, 25, 49);
			mangle_state(val);
			return 1;
		case NODE_SECP192K_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_mem, NID_secp192k1, 25, 49);
			mangle_state(val);
			return 1;
		case NODE_SECP224K_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_mem, NID_secp224k1, 28);
			mangle_state(val);
			return 1;
		case NODE_SECP224K_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_secp224k1, 29, 57);
			mangle_state(val);
			return 1;
		case NODE_SECP224K_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_secp224k1, 29, 57);
			mangle_state(val);
			return 1;
		case NODE_SECP224K_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_secp224k1, 29, 57);
			mangle_state(val);
			return 1;
		case NODE_SECP224K_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_mem, NID_secp224k1, 29, 57);
			mangle_state(val);
			return 1;
		case NODE_SECP224R_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_mem, NID_secp224r1, 28);
			mangle_state(val);
			return 1;
		case NODE_SECP224R_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_secp224r1, 29, 57);
			mangle_state(val);
			return 1;
		case NODE_SECP224R_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_secp224r1, 29, 57);
			mangle_state(val);
			return 1;
		case NODE_SECP224R_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_secp224r1, 29, 57);
			mangle_state(val);
			return 1;
		case NODE_SECP224R_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_mem, NID_secp224r1, 29, 57);
			mangle_state(val);
			return 1;
		case NODE_SECP256K_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_mem, NID_secp256k1, 32);
			mangle_state(val);
			return 1;
		case NODE_SECP256K_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_secp256k1, 33, 65);
			mangle_state(val);
			return 1;
		case NODE_SECP256K_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_secp256k1, 33, 65);
			mangle_state(val);
			return 1;
		case NODE_SECP256K_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_secp256k1, 33, 65);
			mangle_state(val);
		case NODE_SECP256K_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_mem, NID_secp256k1, 33, 65);
			mangle_state(val);
			return 1;
		case NODE_SECP384R_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_mem, NID_secp384r1, 48);
			mangle_state(val);
			return 1;
		case NODE_SECP384R_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_secp384r1, 48, 97);
			mangle_state(val);
			return 1;
		case NODE_SECP384R_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_secp384r1, 48, 97);
			mangle_state(val);
			return 1;
		case NODE_SECP384R_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_secp384r1, 48, 97);
			mangle_state(val);
			return 1;
		case NODE_SECP384R_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_mem, NID_secp384r1, 48, 97);
			mangle_state(val);
			return 1;
		case NODE_PRM192V1_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_mem, NID_X9_62_prime192v1, 24);
			mangle_state(val);
			return 1;
		case NODE_PRM192V1_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_X9_62_prime192v1, 25, 49);
			mangle_state(val);
			return 1;
		case NODE_PRM192V1_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_X9_62_prime192v1, 25, 49);
			mangle_state(val);
			return 1;
		case NODE_PRM192V1_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_X9_62_prime192v1, 25, 49);
			mangle_state(val);
			return 1;
		case NODE_PRM192V1_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_mem, NID_X9_62_prime192v1, 25, 49);
			mangle_state(val);
			return 1;
		case NODE_PRM192V2_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_mem, NID_X9_62_prime192v2, 24);
			mangle_state(val);
			return 1;
		case NODE_PRM192V2_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_X9_62_prime192v2, 25, 49);
			mangle_state(val);
			return 1;
		case NODE_PRM192V2_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_X9_62_prime192v2, 25, 49);
			mangle_state(val);
			return 1;
		case NODE_PRM192V2_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_X9_62_prime192v2, 25, 49);
			mangle_state(val);
			return 1;
		case NODE_PRM192V2_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_mem, NID_X9_62_prime192v2, 25, 49);
			mangle_state(val);
			return 1;
		case NODE_PRM192V3_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_mem, NID_X9_62_prime192v3, 24);
			mangle_state(val);
			return 1;
		case NODE_PRM192V3_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_X9_62_prime192v3, 25, 49);
			mangle_state(val);
			return 1;
		case NODE_PRM192V3_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_X9_62_prime192v3, 25, 49);
			mangle_state(val);
			return 1;
		case NODE_PRM192V3_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_X9_62_prime192v3, 25, 49);
			mangle_state(val);
			return 1;
		case NODE_PRM192V3_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_mem, NID_X9_62_prime192v3, 25, 49);
			mangle_state(val);
			return 1;
		case NODE_PRM256V1_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_mem, NID_X9_62_prime256v1, 32);
			mangle_state(val);
			return 1;
		case NODE_PRM256V1_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_X9_62_prime256v1, 33, 65);
			mangle_state(val);
			return 1;
		case NODE_PRM256V1_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_X9_62_prime256v1, 33, 65);
			mangle_state(val);
			return 1;
		case NODE_PRM256V1_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_mem, NID_X9_62_prime256v1, 33, 65);
			mangle_state(val);
			return 1;
		case NODE_PRM256V1_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_mem, NID_X9_62_prime256v1, 33, 65);
			mangle_state(val);
			return 1;

		default:
			applog(LOG_ERR, "ERROR: VM Runtime - Unsupported Operation (%d)", exp->type);
	}

	if (vm_stack_idx >= VM_STACK_SIZE) {
		applog(LOG_ERR, "ERROR: VM Runtime - Stack Overflow!");
		return false;
	}

	//int i;
	//for (i = 0; i < 100; i++)
	//	d_vm_stack[i] = vm_stack[i];

	return 1;
}
