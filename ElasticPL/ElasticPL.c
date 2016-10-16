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
		applog(LOG_ERR, "ERROR: Unable To Parse ElasticPL Source Code!");
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
		for (i=0; i<vm_ast_cnt; i++) {
			dump_vm_ast(vm_ast[i]);
			fprintf(stdout, "--------------------------------\n");
		}
	}

	return true;
}

extern int run_epl_vm() {
	// Reset Remaing VM Memory To Zero
	memset(&vm_mem[12], 0, (VM_MEMORY_SIZE - 12) * sizeof(long));

	return interpret_ast();
}

extern void delete_epl_vm() {
	vm_ast_cnt = -1;
	free(vm_ast);
}

// Temporary - For Debugging Only
extern void dump_vm_ast(ast* root) {
	char val[12];

	if (root != NULL) {
		dump_vm_ast(root->left);
		dump_vm_ast(root->right);

		if (root->type == NODE_CONSTANT || root->type == NODE_VAR_CONST)
			sprintf(val, "%d", root->value);
		else
			strcpy(val, "");

		fprintf(stdout, "Type: %d,\t%s\t%s\n", root->type, get_node_str(root->type), val);
	}
}

extern char* get_node_str(NODE_TYPE node_type) {
	switch (node_type) {
	case NODE_CONSTANT:		return "";
	case NODE_VAR_CONST:	return "m[]";
	case NODE_VAR_EXP:		return "m[x]";
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
	case NODE_ADD:			return "+";
	case NODE_POS:			return "'+'";
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
	case NODE_SHA256:		return "sha256";
	case NODE_MD5:			return "md5";
	default: return "Unknown";
	}
}