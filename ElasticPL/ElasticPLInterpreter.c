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
	char* cond = 0;
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
				sprintf(result, "m(0)");
			else
				sprintf(result, "m(%d)", exp->value);
			break;
		case NODE_VAR_CONST:
			if (exp->value < 0 || exp->value > VM_MEMORY_SIZE)
				sprintf(result, "mem[m(0)]");
			else
				sprintf(result, "mem[m(%d)]", exp->value);
			break;
		case NODE_VAR_EXP:
			sprintf(result, "mem[m(%s)]", lval);
			break;
		case NODE_ASSIGN:
			sprintf(result, "%s = %s;\n", lval, rval);
			break;
		case NODE_IF:
			if (exp->right->type != NODE_ELSE) {
				rval = convert(exp->right);				// If Body (No Else Condition)
				sprintf(result, "if( %s ) %s\n", lval, rval);
			}
			else {
				cond = lval;
				lval = convert(exp->right->left);		// If Body
				rval = convert(exp->right->right);		// Else Body
				sprintf(result, "if( %s ) %selse %s\n", cond, lval, rval);
			}
			break;
		case NODE_ELSE:
			break;
		case NODE_BLOCK:
			if (!blk_old[0])
				sprintf(blk_new, "%s}\n", lval);
			else
				sprintf(blk_new, "%s%s", lval, blk_old);
			sprintf(result, "{\n%s", blk_new);
			strcpy(blk_old, blk_new);
			break;
		case NODE_ADD:
			sprintf(result, "(%s + %s)", lval, rval);
			break;
		case NODE_SUB:
			sprintf(result, "(%s - %s)", lval, rval);
			break;
		case NODE_MUL:
			sprintf(result, "(%s * %s)", lval, rval);
			break;
		case NODE_DIV:
			sprintf(result, "((%s > 0) ? %s / %s : 0)", rval, lval, rval);
			break;
		case NODE_MOD:
			sprintf(result, "((%s > 0) ? %s %%%% %s : 0)", rval, lval, rval);
			break;
		case NODE_LSHIFT:
			sprintf(result, "(%s << %s)", lval, rval);
			break;
		case NODE_LROT:
			sprintf(result, "(rotl32( %s, %s %%%% 32))", lval, rval);
			break;
		case NODE_RSHIFT:
			sprintf(result, "(%s >> %s)", lval, rval);
			break;
		case NODE_RROT:
			sprintf(result, "(rotr32( %s, %s %%%% 32))", lval, rval);
			break;
		case NODE_NOT:
			sprintf(result, "!%s", rval);
			break;
		case NODE_COMPL:
			sprintf(result, "~%s", rval);
			break;
		case NODE_AND:
			sprintf(result, "(%s && %s)", lval, rval);
			break;
		case NODE_OR:
			sprintf(result, "(%s || %s)", lval, rval);
			break;
		case NODE_BITWISE_AND:
			sprintf(result, "(%s & %s)", lval, rval);
			break;
		case NODE_BITWISE_XOR:
			sprintf(result, "(%s ^ %s)", lval, rval);
			break;
		case NODE_BITWISE_OR:
			sprintf(result, "(%s | %s)", lval, rval);
			break;
		case NODE_EQ:
			sprintf(result, "(%s == %s)", lval, rval);
			break;
		case NODE_NE:
			sprintf(result, "(%s != %s)", lval, rval);
			break;
		case NODE_GT:
			sprintf(result, "(%s > %s)", lval, rval);
			break;
		case NODE_LT:
			sprintf(result, "(%s < %s)", lval, rval);
			break;
		case NODE_GE:
			sprintf(result, "(%s >= %s)", lval, rval);
			break;
		case NODE_LE:
			sprintf(result, "(%s <= %s)", lval, rval);
			break;
		case NODE_NEG:
			sprintf(result, "-%s", lval);
			break;
		case NODE_POS:
			sprintf(result, "+%s", rval);
			break;
		case NODE_VERIFY:
			sprintf(result, "return (%s != 0 ? 1 : 0);\n", lval);
			break;
		case NODE_SHA256:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'sha256' not supported by this miner\\n\");\n");
			break;
		case NODE_SHA512:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'sha512' not supported by this miner\\n\");\n");
			break;
		case NODE_WHIRLPOOL:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'whirlpool' not supported by this miner\\n\");\n");
			break;
		case NODE_MD5:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'md5' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP192K_PTP:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP192K1PrivToPub' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP192K_PA:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP192K1PointAdd' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP192K_PS:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP192K1PointSub' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP192K_PSM:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP192K1PointScalarMult' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP192K_PN:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP192K1PointNegate' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP192R_PTP:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP192R1PrivToPub' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP192R_PA:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP192R1PointAdd' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP192R_PS:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP192R1PointSub' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP192R_PSM:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP192R1PointScalarMult' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP192R_PN:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP192R1PointNegate' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP224K_PTP:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP224K1PrivToPub' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP224K_PA:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP224K1PointAdd' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP224K_PS:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP224K1PointSub' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP224K_PSM:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP224K1PointScalarMult' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP224K_PN:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP224K1PointNegate' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP224R_PTP:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP224R1PrivToPub' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP224R_PA:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP224R1PointAdd' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP224R_PS:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP224R1PointSub' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP224R_PSM:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP224R1PointScalarMult' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP224R_PN:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP224R1PointNegate' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP256K_PTP:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP256K1PrivToPub' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP256K_PA:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP256K1PointAdd' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP256K_PS:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP256K1PointSub' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP256K_PSM:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP256K1PointScalarMult' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP256K_PN:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP256K1PointNegate' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP256R_PTP:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP256R1PrivToPub' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP256R_PA:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP256R1PointAdd' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP256R_PS:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP256R1PointSub' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP256R_PSM:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP256R1PointScalarMult' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP256R_PN:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP256R1PointNegate' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP384R_PTP:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP384R1PrivToPub' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP384R_PA:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP384R1PointAdd' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP384R_PS:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP384R1PointSub' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP384R_PSM:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP384R1PointScalarMult' not supported by this miner\\n\");\n");
			break;
		case NODE_SECP384R_PN:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'SECP384R1PointNegate' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM192V1_PTP:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME192V1PrivToPub' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM192V1_PA:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME192V1PointAdd' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM192V1_PS:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME192V1PointSub' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM192V1_PSM:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME192V1PointScalarMult' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM192V1_PN:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME192V1PointNegate' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM192V2_PTP:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME192V2PrivToPub' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM192V2_PA:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME192V2PointAdd' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM192V2_PS:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME192V2PointSub' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM192V2_PSM:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME192V2PointScalarMult' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM192V2_PN:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME192V2PointNegate' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM192V3_PTP:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME192V3PrivToPub' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM192V3_PA:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME192V3PointAdd' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM192V3_PS:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME192V3PointSub' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM192V3_PSM:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME192V3PointScalarMult' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM192V3_PN:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME192V3PointNegate' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM256V1_PTP:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME256V1PrivToPub' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM256V1_PA:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME256V1PointAdd' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM256V1_PS:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME256V1PointSub' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM256V1_PSM:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME256V1PointScalarMult' not supported by this miner\\n\");\n");
			break;
		case NODE_PRM256V1_PN:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'PRIME256V1PointNegate' not supported by this miner\\n\");\n");
			break;
		case NODE_TIGER:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'Tiger' not supported by this miner\\n\");\n");
			break;
		case NODE_RIPEMD160:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'RIPEMD160' not supported by this miner\\n\");\n");
			break;
		case NODE_RIPEMD128:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - 'RIPEMD128' not supported by this miner\\n\");\n");
			break;
		default:
			sprintf(result, "fprintf(stderr, \n\"ERROR: VM Runtime - Unsupported Operation (%d)\n\");\n", exp->type);
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
