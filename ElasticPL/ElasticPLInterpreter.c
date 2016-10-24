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
#include "../miner.h"

char blk_new[4096];
char blk_old[4096];

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
			sprintf(result, "((%s > 0) ? m(%s / %s) : m(0))", rval, lval, rval);
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
			sprintf(result, "m(%s && %s)", lval, rval);
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
			sprintf(result, "sha256( %s );\n", rval);
			break;
		case NODE_SHA512:
			sprintf(result, "sha512( %s );\n", rval);
			break;
		case NODE_WHIRLPOOL:
			sprintf(result, "whirlpool( %s );\n", rval);
			break;
		case NODE_MD5:
			sprintf(result, "md5( %s );\n", rval);
			break;
		case NODE_SECP192K_PTP:
			sprintf(result, "SECP192K1PrivToPub( %s );\n", rval);
			break;
		case NODE_SECP192K_PA:
			sprintf(result, "SECP192K1PointAdd( %s );\n", rval);
			break;
		case NODE_SECP192K_PS:
			sprintf(result, "SECP192K1PointSub( %s );\n", rval);
			break;
		case NODE_SECP192K_PSM:
			sprintf(result, "SECP192K1PointScalarMult( %s );\n", rval);
			break;
		case NODE_SECP192K_PN:
			sprintf(result, "SECP192K1PointNegate( %s );\n", rval);
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
			sprintf(result, "SECP224K1PrivToPub( %s );\n", rval);
			break;
		case NODE_SECP224K_PA:
			sprintf(result, "SECP224K1PointAdd( %s );\n", rval);
			break;
		case NODE_SECP224K_PS:
			sprintf(result, "SECP224K1PointSub( %s );\n", rval);
			break;
		case NODE_SECP224K_PSM:
			sprintf(result, "SECP224K1PointScalarMult( %s );\n", rval);
			break;
		case NODE_SECP224K_PN:
			sprintf(result, "SECP224K1PointNegate( %s );\n", rval);
			break;
		case NODE_SECP224R_PTP:
			sprintf(result, "SECP224R1PrivToPub( %s );\n", rval);
			break;
		case NODE_SECP224R_PA:
			sprintf(result, "SECP224R1PointAdd( %s );\n", rval);
			break;
		case NODE_SECP224R_PS:
			sprintf(result, "SECP224R1PointSub( %s );\n", rval);
			break;
		case NODE_SECP224R_PSM:
			sprintf(result, "SECP224R1PointScalarMult( %s );\n", rval);
			break;
		case NODE_SECP224R_PN:
			sprintf(result, "SECP224R1PointNegate( %s );\n", rval);
			break;
		case NODE_SECP256K_PTP:
			sprintf(result, "SECP256K1PrivToPub( %s );\n", rval);
			break;
		case NODE_SECP256K_PA:
			sprintf(result, "SECP256K1PointAdd( %s );\n", rval);
			break;
		case NODE_SECP256K_PS:
			sprintf(result, "SECP256K1PointSub( %s );\n", rval);
			break;
		case NODE_SECP256K_PSM:
			sprintf(result, "SECP256K1PointScalarMult( %s );\n", rval);
			break;
		case NODE_SECP256K_PN:
			sprintf(result, "SECP256K1PointNegate( %s );\n", rval);
			break;
		case NODE_SECP256R_PTP:
			sprintf(result, "SECP256R1PrivToPub( %s );\n", rval);
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
			sprintf(result, "SECP384R1PrivToPub( %s );\n", rval);
			break;
		case NODE_SECP384R_PA:
			sprintf(result, "SECP384R1PointAdd( %s );\n", rval);
			break;
		case NODE_SECP384R_PS:
			sprintf(result, "SECP384R1PointSub( %s );\n", rval);
			break;
		case NODE_SECP384R_PSM:
			sprintf(result, "SECP384R1PointScalarMult( %s );\n", rval);
			break;
		case NODE_SECP384R_PN:
			sprintf(result, "SECP384R1PointNegate( %s );\n", rval);
			break;
		case NODE_PRM192V1_PTP:
			sprintf(result, "PRIME192V1PrivToPub( %s );\n", rval);
			break;
		case NODE_PRM192V1_PA:
			sprintf(result, "PRIME192V1PointAdd( %s );\n", rval);
			break;
		case NODE_PRM192V1_PS:
			sprintf(result, "PRIME192V1PointSub( %s );\n", rval);
			break;
		case NODE_PRM192V1_PSM:
			sprintf(result, "PRIME192V1PointScalarMult( %s );\n", rval);
			break;
		case NODE_PRM192V1_PN:
			sprintf(result, "PRIME192V1PointNegate( %s );\n", rval);
			break;
		case NODE_PRM192V2_PTP:
			sprintf(result, "PRIME192V2PrivToPub( %s );\n", rval);
			break;
		case NODE_PRM192V2_PA:
			sprintf(result, "PRIME192V2PointAdd( %s );\n", rval);
			break;
		case NODE_PRM192V2_PS:
			sprintf(result, "PRIME192V2PointSub( %s );\n", rval);
			break;
		case NODE_PRM192V2_PSM:
			sprintf(result, "PRIME192V2PointScalarMult( %s );\n", rval);
			break;
		case NODE_PRM192V2_PN:
			sprintf(result, "PRIME192V2PointNegate( %s );\n", rval);
			break;
		case NODE_PRM192V3_PTP:
			sprintf(result, "PRIME192V3PrivToPub( %s );\n", rval);
			break;
		case NODE_PRM192V3_PA:
			sprintf(result, "PRIME192V3PointAdd( %s );\n", rval);
			break;
		case NODE_PRM192V3_PS:
			sprintf(result, "PRIME192V3PointSub( %s );\n", rval);
			break;
		case NODE_PRM192V3_PSM:
			sprintf(result, "PRIME192V3PointScalarMult( %s );\n", rval);
			break;
		case NODE_PRM192V3_PN:
			sprintf(result, "PRIME192V3PointNegate( %s );\n", rval);
			break;
		case NODE_PRM256V1_PTP:
			sprintf(result, "PRIME256V1PrivToPub( %s );\n", rval);
			break;
		case NODE_PRM256V1_PA:
			sprintf(result, "PRIME256V1PointAdd( %s );\n", rval);
			break;
		case NODE_PRM256V1_PS:
			sprintf(result, "PRIME256V1PointSub( %s );\n", rval);
			break;
		case NODE_PRM256V1_PSM:
			sprintf(result, "PRIME256V1PointScalarMult( %s );\n", rval);
			break;
		case NODE_PRM256V1_PN:
			sprintf(result, "PRIME256V1PointNegate( %s );\n", rval);
			break;
		case NODE_TIGER:
			sprintf(result, "Tiger( %s );\n", rval);
			break;
		case NODE_RIPEMD160:
			sprintf(result, "RIPEMD160( %s );\n", rval);
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
