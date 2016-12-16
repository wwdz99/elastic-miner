/*
* Copyright 2016 sprocket
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free
* Software Foundation; either version 2 of the License, or (at your option)
* any later version.
*/

#include <stdio.h>

#include "ElasticPL.h"
#include "../miner.h"

extern bool create_epl_vm(char *source) {
	int i;
	SOURCE_TOKEN_LIST token_list;

	if (!source) {
		applog(LOG_ERR, "ERROR: Missing ElasticPL Source Code!");
		return false;
	}

	stack_op_idx = -1;
	stack_exp_idx = -1;
	top_op = -1;
	stack_op = calloc(PARSE_STACK_SIZE, sizeof(int));
	stack_exp = calloc(PARSE_STACK_SIZE, sizeof(ast) / sizeof(stack_exp[0]));

	if (!stack_op || !stack_exp) {
		applog(LOG_ERR, "ERROR: Unable To Allocate VM Parser Stack!");
		return false;
	}

	if (!init_token_list(&token_list, TOKEN_LIST_SIZE)) {
		applog(LOG_ERR, "ERROR: Unable To Allocate Token List For Parser!");
		return false;
	}

	// Parse EPL Source Code Into Tokens
	if (!get_token_list(source, &token_list)) {
		return false;
	}

	// Parse Tokens Into AST
	if (!parse_token_list(&token_list)) {
		applog(LOG_ERR, "ERROR: Unable To Parse ElasticPL Tokens!");
		return false;
	}

	// Copy Parsed Statements Into VM Array
	vm_ast_cnt = stack_exp_idx + 1;

	vm_ast = calloc(vm_ast_cnt, sizeof(ast*));
	memcpy(vm_ast, stack_exp, vm_ast_cnt * sizeof(ast*));

	free(stack_op);
	free(stack_exp);

	if (!vm_ast) {
		applog(LOG_ERR, "ERROR: ElasticPL Parser Failed!");
		return false;
	}

	if (opt_debug_epl) {
		fprintf(stdout, "--------------------------------\n");
		for (i = 0; i<vm_ast_cnt; i++) {
			dump_vm_ast(vm_ast[i]);
			fprintf(stdout, "--------------------------------\n");
		}
	}

	return true;
}

// Temporary - For Debugging Only
extern void dump_vm_ast(ast* root) {
	char val[12];

	if (root != NULL) {
		dump_vm_ast(root->left);
		dump_vm_ast(root->right);

		if (root->type == NODE_CONSTANT || root->type == NODE_VAR_CONST)
			sprintf(val, "%ld", root->value);
		else
			strcpy(val, "");

		fprintf(stdout, "Type: %d,\t%s\t%s\n", root->type, get_node_str(root->type), val);
	}
}

extern char* get_node_str(NODE_TYPE node_type) {
	switch (node_type) {
	case NODE_CONSTANT:		return "";
	case NODE_VAR_CONST:	return "m/f[]";
	case NODE_VAR_EXP:		return "m/f[x]";
	case NODE_VERIFY:		return "verify";
	case NODE_ASSIGN:		return "=";
	case NODE_OR:			return "||";
	case NODE_AND:			return "&&";
	case NODE_BITWISE_OR:	return "|";
	case NODE_BITWISE_XOR:	return "^";
	case NODE_BITWISE_AND:	return "&";
	case NODE_EQ:			return "==";
	case NODE_NE:			return "!=";
	case NODE_LT:			return "<";
	case NODE_GT:			return ">";
	case NODE_LE:			return "<=";
	case NODE_GE:			return ">=";
	case NODE_INCREMENT_R:	return "++";
	case NODE_INCREMENT_L:	return "++";
	case NODE_ADD_ASSIGN:	return "+=";
	case NODE_SUB_ASSIGN:	return "-=";
	case NODE_MUL_ASSIGN:	return "*=";
	case NODE_DIV_ASSIGN:	return "/=";
	case NODE_MOD_ASSIGN:	return "%=";
	case NODE_LSHFT_ASSIGN:	return "<<=";
	case NODE_RSHFT_ASSIGN:	return ">>=";
	case NODE_AND_ASSIGN:	return "&=";
	case NODE_XOR_ASSIGN:	return "^=";
	case NODE_OR_ASSIGN:	return "|=";
	case NODE_ADD:			return "+";
	case NODE_DECREMENT_R:	return "--";
	case NODE_DECREMENT_L:	return "--";
	case NODE_SUB:			return "-";
	case NODE_NEG:			return "'-'";
	case NODE_MUL:			return "*";
	case NODE_DIV:			return "/";
	case NODE_MOD:			return "%";
	case NODE_RSHIFT:		return ">>";
	case NODE_LSHIFT:		return "<<";
	case NODE_RROT:			return "<<<";
	case NODE_LROT:			return ">>>";
	case NODE_COMPL:		return "~";
	case NODE_NOT:			return "!";
	case NODE_TRUE:			return "true";
	case NODE_FALSE:		return "false";
	case NODE_BLOCK:		return "{}";
	case NODE_IF:			return "if";
	case NODE_ELSE:			return "else";
	case NODE_REPEAT:		return "repeat";
	case NODE_BREAK:		return "break";
	case NODE_CONTINUE:		return "continue";
	case NODE_PARAM:		return "param";
	case NODE_SIN:			return "sin";
	case NODE_COS:			return "cos";
	case NODE_TAN:			return "tan";
	case NODE_SINH:			return "sinh";
	case NODE_COSH:			return "cosh";
	case NODE_TANH:			return "tanh";
	case NODE_ASIN:			return "asin";
	case NODE_ACOS:			return "acos";
	case NODE_ATAN:			return "atan";
	case NODE_ATAN2:		return "atan2";
	case NODE_EXPNT:		return "exp";
	case NODE_LOG:			return "log";
	case NODE_LOG10:		return "log10";
	case NODE_POW:			return "pow";
	case NODE_SQRT:			return "sqrt";
	case NODE_CEIL:			return "ceil";
	case NODE_FLOOR:		return "floor";
	case NODE_ABS:			return "abs";
	case NODE_FABS:			return "fabs";
	case NODE_FMOD:			return "fmod";
	case NODE_SHA256:		return "sha256";
	case NODE_SHA512:		return "sha512";
	case NODE_WHIRLPOOL:    return "whirlpool";
	case NODE_MD5:          return "md5";
	case NODE_SECP192K_PTP: return "SECP192K1PrivToPub";
	case NODE_SECP192K_PA:  return "SECP192K1PointAdd";
	case NODE_SECP192K_PS:  return "SECP192K1PointSub";
	case NODE_SECP192K_PSM: return "SECP192K1PointScalarMult";
	case NODE_SECP192K_PN:  return "SECP192K1PointNegate";
	case NODE_SECP192R_PTP: return "SECP192R1PrivToPub";
	case NODE_SECP192R_PA:  return "SECP192R1PointAdd";
	case NODE_SECP192R_PS:  return "SECP192R1PointSub";
	case NODE_SECP192R_PSM: return "SECP192R1PointScalarMult";
	case NODE_SECP192R_PN:  return "SECP192R1PointNegate";
	case NODE_SECP224K_PTP: return "SECP224K1PrivToPub";
	case NODE_SECP224K_PA:  return "SECP224K1PointAdd";
	case NODE_SECP224K_PS:  return "SECP224K1PointSub";
	case NODE_SECP224K_PSM: return "SECP224K1PointScalarMult";
	case NODE_SECP224K_PN:  return "SECP224K1PointNegate";
	case NODE_SECP224R_PTP: return "SECP224R1PrivToPub";
	case NODE_SECP224R_PA:  return "SECP224R1PointAdd";
	case NODE_SECP224R_PS:  return "SECP224R1PointSub";
	case NODE_SECP224R_PSM: return "SECP224R1PointScalarMult";
	case NODE_SECP224R_PN:  return "SECP224R1PointNegate";
	case NODE_SECP256K_PTP: return "SECP256K1PrivToPub";
	case NODE_SECP256K_PA:  return "SECP256K1PointAdd";
	case NODE_SECP256K_PS:  return "SECP256K1PointSub";
	case NODE_SECP256K_PSM: return "SECP256K1PointScalarMult";
	case NODE_SECP256K_PN:  return "SECP256K1PointNegate";
	case NODE_SECP256R_PTP: return "SECP256R1PrivToPub";
	case NODE_SECP256R_PA:  return "SECP256R1PointAdd";
	case NODE_SECP256R_PS:  return "SECP256R1PointSub";
	case NODE_SECP256R_PSM: return "SECP256R1PointScalarMult";
	case NODE_SECP256R_PN:  return "SECP256R1PointNegate";
	case NODE_SECP384R_PTP: return "SECP384R1PrivToPub";
	case NODE_SECP384R_PA:  return "SECP384R1PointAdd";
	case NODE_SECP384R_PS:  return "SECP384R1PointSub";
	case NODE_SECP384R_PSM: return "SECP384R1PointScalarMult";
	case NODE_SECP384R_PN:  return "SECP384R1PointNegate";
	case NODE_PRM192V1_PTP: return "PRIME192V1PrivToPub";
	case NODE_PRM192V1_PA:  return "PRIME192V1PointAdd";
	case NODE_PRM192V1_PS:  return "PRIME192V1PointSub";
	case NODE_PRM192V1_PSM: return "PRIME192V1PointScalarMult";
	case NODE_PRM192V1_PN:  return "PRIME192V1PointNegate";
	case NODE_PRM192V2_PTP: return "PRIME192V2PrivToPub";
	case NODE_PRM192V2_PA:  return "PRIME192V2PointAdd";
	case NODE_PRM192V2_PS:  return "PRIME192V2PointSub";
	case NODE_PRM192V2_PSM: return "PRIME192V2PointScalarMult";
	case NODE_PRM192V2_PN:  return "PRIME192V2PointNegate";
	case NODE_PRM192V3_PTP: return "PRIME192V3PrivToPub";
	case NODE_PRM192V3_PA:  return "PRIME192V3PointAdd";
	case NODE_PRM192V3_PS:  return "PRIME192V3PointSub";
	case NODE_PRM192V3_PSM: return "PRIME192V3PointScalarMult";
	case NODE_PRM192V3_PN:  return "PRIME192V3PointNegate";
	case NODE_PRM256V1_PTP: return "PRIME256V1PrivToPub";
	case NODE_PRM256V1_PA:  return "PRIME256V1PointAdd";
	case NODE_PRM256V1_PS:  return "PRIME256V1PointSub";
	case NODE_PRM256V1_PSM: return "PRIME256V1PointScalarMult";
	case NODE_PRM256V1_PN:  return "PRIME256V1PointNegate";
	case NODE_TIGER:        return "Tiger";
	case NODE_RIPEMD160:    return "RIPEMD160";
	case NODE_RIPEMD128:    return "RIPEMD128";
	default: return "Unknown";
	}
}