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

char* append_strings(char * old, char * new)
{
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

extern char* c_compile_ast() {
	blk_new[0] = 0;
	blk_old[0] = 0;

	char* ret = NULL;
	int i;
	for (i = 0; i < vm_ast_cnt; i++) {
		ret = append_strings(ret, compile(vm_ast[i]));
	}
	return ret;
}

// Use Post Order Traversal To Translate The Expressions In The AST to C
extern char* compile(ast* exp) {
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
			lval = compile(exp->left);
		}

		// Check For If Statement As Right Side Is Conditional
		if (exp->type != NODE_IF)
			rval = compile(exp->right);

		//if (exp->right != NULL) {
		//	rval = compile(exp->right);
		//}

		switch (exp->type) {
		case NODE_CONSTANT:
			sprintf(result, "m(%d)", exp->value);
			break;
		case NODE_VAR_CONST:
			sprintf(result, "mem[m(%d)]", exp->value);
			break;
		case NODE_VAR_EXP:

			break;
		case NODE_ASSIGN:
			sprintf(result, "%s = %s;\n", lval, rval);
			break;
		case NODE_IF:
			if (exp->right->type != NODE_ELSE) {
				rval = compile(exp->right->right);		// If Body (No Else Condition)
				sprintf(result, "if( %s ) %s\n", lval, rval);
			}
			else {
				cond = lval;
				lval = compile(exp->right->left);		// If Body
				rval = compile(exp->right->right);		// Else Body
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

			break;
		case NODE_SUB:

			break;
		case NODE_MUL:

			break;
		case NODE_DIV:

			break;
		case NODE_MOD:

			break;
		case NODE_LSHIFT:

			break;
		case NODE_LROT:

			break;
		case NODE_RSHIFT:

			break;
		case NODE_RROT:

			break;
		case NODE_NOT:

			break;
		case NODE_COMPL:

			break;
		case NODE_AND:

			break;
		case NODE_OR:

			break;
		case NODE_BITWISE_AND:

			break;
		case NODE_BITWISE_XOR:

			break;
		case NODE_BITWISE_OR:

			break;
		case NODE_EQ:
			sprintf(result, "(%s == %s)", lval, rval);
			break;
		case NODE_NE:

			break;
		case NODE_GT:

			break;
		case NODE_LT:

			break;
		case NODE_GE:

			break;
		case NODE_LE:

			break;
		case NODE_NEG:

			break;
		case NODE_POS:

			break;
		case NODE_SHA256:

			break;
		case NODE_MD5:
			applog(LOG_ERR, "ERROR: VM Runtime - MD5 Not Implemented");
			return NULL;
		case NODE_VERIFY:
			sprintf(result, "return (%s != 0 ? 1 : 0);\n", lval);
			break;
		default:
			applog(LOG_ERR, "ERROR: VM Runtime - Unsupported Operation (%d)", exp->type);
			return NULL;
		}
	}

	return result;
}
