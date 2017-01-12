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
#include "ElasticPLFunctions.h"
#include "../miner.h"

char *tab[] = { "\t", "\t\t", "\t\t\t", "\t\t\t\t", "\t\t\t\t\t", "\t\t\t\t\t\t", "\t\t\t\t\t\t\t", "\t\t\t\t\t\t\t" };
int tabs;

extern char* convert_ast_to_c() {
	char* ret = NULL;
	int i;

	tabs = 0;

	use_elasticpl_crypto = false;
	use_elasticpl_math = false;

	for (i = 0; i < vm_ast_cnt; i++) {
		ret = append_strings(ret, convert(vm_ast[i]));
	}

	// The Current OpenCL Code Can't Run The Crypto Functions
	if (opt_opencl && use_elasticpl_crypto)
		return NULL;

	return ret;
}

static char* get_index(char *lval) {
	char *str, *index;

	if (!lval)
		return NULL;

	index = malloc(strlen(lval));
	str = strstr(lval, "[");
	strcpy(index, &str[1]);
	index[strlen(index) - 1] = 0;

	return index;
}

// Use Post Order Traversal To Translate The Expressions In The AST to C
static char* convert(ast* exp) {
	char *lval = NULL;
	char *rval = NULL;
	char *tmp = NULL;
	char *result = NULL;

	bool l_is_float = false;
	bool r_is_float = false;

	if (exp != NULL) {

		// Determine Tab Indentations
		if (exp->type == NODE_REPEAT) {
			if (tabs < 6) tabs += 2;
		}
		else if (exp->type == NODE_IF) {
			if (tabs < 7) tabs++;
		}

		// Process Left Side Statements
		if (exp->left != NULL) {
			lval = convert(exp->left);
		}

		// Check For If Statement As Right Side Is Conditional
		if ((exp->type != NODE_IF) && (exp->type != NODE_CONDITIONAL))
			rval = convert(exp->right);

		// Check If Leafs Are Float Or Int To Determine If Casting Is Needed
		if (exp->left != NULL)
			l_is_float = (exp->left->is_float);
		if (exp->right != NULL)
			r_is_float = (exp->right->is_float);

		// Allocate Memory For Results
		result = malloc((lval ? strlen(lval) : 0) + (rval ? strlen(rval) : 0) + 256);
		result[0] = 0;

		switch (exp->type) {
		case NODE_CONSTANT:
			if (exp->data_type == DT_INT)
				sprintf(result, "%d", exp->value);
			else if (exp->data_type == DT_FLOAT)
				sprintf(result, "%f", exp->fvalue);
			else if (exp->data_type == DT_STRING)
				sprintf(result, "%s", exp->svalue);
			break;
		case NODE_VAR_CONST:
			if (exp->value < 0 || exp->value > VM_MEMORY_SIZE) {
				if (exp->data_type == DT_INT)
					sprintf(result, "m[0]");
				else if (exp->data_type == DT_FLOAT)
					sprintf(result, "f[0]");
				else if (exp->data_type == DT_BIGINT)
					sprintf(result, "b[0]");
			}
			else {
				if (exp->data_type == DT_INT)
					sprintf(result, "m[%lu]", exp->value);
				else if (exp->data_type == DT_FLOAT)
					sprintf(result, "f[%lu]", exp->value);
				else if (exp->data_type == DT_BIGINT)
					sprintf(result, "b[%lu]", exp->value);
			}
			break;
		case NODE_VAR_EXP:
			if (exp->data_type == DT_INT)
				sprintf(result, "m[%s]", lval);
			else if (exp->data_type == DT_FLOAT)
				sprintf(result, "f[%s]", lval);
			else if (exp->data_type == DT_BIGINT)
				sprintf(result, "b[%s]", lval);
			break;
		case NODE_ASSIGN:
			tmp = get_index(lval);
			if (l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = (double)(%s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (!l_is_float && r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = (int)(%s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else
				sprintf(result, "index = %s;\n%s%s[index] = %s;\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			break;
		case NODE_IF:
			if (tabs < 1) tabs = 1;
			if (exp->right->type != NODE_ELSE) {
				rval = convert(exp->right);				// If Body (No Else Condition)
				result = realloc(result, (lval ? strlen(lval) : 0) + (rval ? strlen(rval) : 0) + 256);
				sprintf(result, "%sif( %s ) {\n%s%s%s}\n", tab[tabs - 1], lval, (rval[0] == '\t' ? "" : tab[tabs]), rval, tab[tabs - 1]);
			}
			else {
				tmp = lval;
				lval = convert(exp->right->left);		// If Body
				rval = convert(exp->right->right);		// Else Body
				result = realloc(result, (lval ? strlen(lval) : 0) + (rval ? strlen(rval) : 0) + 256);
				sprintf(result, "%sif( %s ) {\n%s%s%s}\n%selse {\n%s%s%s}\n", tab[tabs - 1], tmp, (lval[0] == '\t' ? "" : tab[tabs]), lval, tab[tabs - 1], tab[tabs - 1], (rval[0] == '\t' ? "" : tab[tabs]), rval, tab[tabs - 1]);
			}
			if (tabs) tabs--;
			break;
		case NODE_CONDITIONAL:
			tmp = lval;
			lval = convert(exp->right->left);		// If Body
			rval = convert(exp->right->right);		// Else Body
			result = realloc(result, (lval ? strlen(lval) : 0) + (rval ? strlen(rval) : 0) + 256);
			sprintf(result, "(( %s ) ? %s : %s)", tmp, lval, rval);
			break;
		case NODE_COND_ELSE:
		case NODE_ELSE:
			break;
		case NODE_REPEAT:
			if (tabs < 2) tabs = 2;
			sprintf(result, "%sif ( %s > 0 ) {\n%sint loop%d;\n%sfor (loop%d = 0; loop%d < ( %s ); loop%d++) {\n%s%s%s}\n%s}\n", tab[tabs - 2], lval, tab[tabs - 1], exp->token_num, tab[tabs - 1], exp->token_num, exp->token_num, lval, exp->token_num, "", rval, tab[tabs - 1], tab[tabs - 2]);
			if (tabs > 1) tabs -= 2;
			break;
		case NODE_BREAK:
			sprintf(result, "break");
			break;
		case NODE_CONTINUE:
			sprintf(result, "continue");
			break;
		case NODE_BLOCK:
			if (lval[0] != '\t')
				sprintf(result, "%s%s%s", tab[tabs], lval, (rval ? rval : ""));
			else
				sprintf(result, "%s%s", lval, (rval ? rval : ""));
			break;
		case NODE_INCREMENT_R:
			sprintf(result, "++%s", lval);
			exp->is_float = l_is_float;
			break;
		case NODE_INCREMENT_L:
			sprintf(result, "%s++", lval);
			exp->is_float = l_is_float;
			break;
		case NODE_ADD:
			if (l_is_float && !r_is_float)
				sprintf(result, "(%s + (double)(%s))", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "((double)(%s) + %s)", lval, rval);
			else
				sprintf(result, "(%s + %s)", lval, rval);
			exp->is_float = l_is_float | r_is_float;
			break;
		case NODE_ADD_ASSIGN:
			tmp = get_index(lval);
			if (l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] += (double)(%s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (!l_is_float && r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] += (int)(%s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else
				sprintf(result, "index = %s;\n%s%s[index] += %s;\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			break;
		case NODE_SUB_ASSIGN:
			tmp = get_index(lval);
			if (l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] -= (double)(%s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (!l_is_float && r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] -= (int)(%s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else
				sprintf(result, "index = %s;\n%s%s[index] -= %s;\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			break;
		case NODE_MUL_ASSIGN:
			tmp = get_index(lval);
			if (l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] *= (double)(%s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (!l_is_float && r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] *= (int)(%s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else
				sprintf(result, "index = %s;\n%s%s[index] *= %s;\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			break;
		case NODE_DIV_ASSIGN:
			tmp = get_index(lval);
			if (!l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = (((double)(%s) != 0.0) ? (int)((double)(%s[index]) / (double)(%s)) : 0);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = (((double)(%s) != 0.0) ? %s[index] / (double)(%s) : 0.0);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (!l_is_float && r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = ((%s != 0.0) ? (int)((double)(%s[index]) / %s) : 0);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else
				sprintf(result, "index = %s;\n%s%s[index] = ((%s != 0.0) ? %s[index] / %s : 0.0);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			exp->is_float = true;
			break;
		case NODE_MOD_ASSIGN:
			tmp = get_index(lval);
			if (!l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = ((%s != 0) ? %s[index] %% %s : 0);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = ((%s != 0) ? (double)((int)(%s[index]) %% %s) : 0.0);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (!l_is_float && r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = (((int)(%s) != 0) ? %s[index] %% (int)(%s) : 0);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else
				sprintf(result, "index = %s;\n%s%s[index] = (((int)(%s) != 0) ? (int)(%s[index]) %% (int)(%s) : 0.0);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), rval, (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			exp->is_float = true;
			break;
		case NODE_LSHFT_ASSIGN:
			tmp = get_index(lval);
			if (!l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = %s[index] << %s;\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = (double)((int)(%s[index]) << %s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (!l_is_float && r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = %s[index] << (int)(%s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else
				sprintf(result, "index = %s;\n%s%s[index] = (double)((int)(%s[index]) << (int)(%s));\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			exp->is_float = true;
			break;
		case NODE_RSHFT_ASSIGN:
			tmp = get_index(lval);
			if (!l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = %s[index] >> %s;\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = (double)((int)(%s[index]) >> %s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (!l_is_float && r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = %s[index] >> (int)(%s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else
				sprintf(result, "index = %s;\n%s%s[index] = (double)((int)(%s[index]) >> (int)(%s));\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			exp->is_float = true;
			break;
		case NODE_AND_ASSIGN:
			tmp = get_index(lval);
			if (!l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = %s[index] & %s;\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = (double)((int)(%s[index]) & %s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (!l_is_float && r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = %s[index] & (int)(%s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else
				sprintf(result, "index = %s;\n%s%s[index] = (double)((int)(%s[index]) & (int)(%s));\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			exp->is_float = true;
			break;
		case NODE_XOR_ASSIGN:
			tmp = get_index(lval);
			if (!l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = %s[index] ^ %s;\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = (double)((int)(%s[index]) ^ %s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (!l_is_float && r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = %s[index] ^ (int)(%s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else
				sprintf(result, "index = %s;\n%s%s[index] = (double)((int)(%s[index]) ^ (int)(%s));\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			exp->is_float = true;
			break;
		case NODE_OR_ASSIGN:
			tmp = get_index(lval);
			if (!l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = %s[index] | %s;\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (l_is_float && !r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = (double)((int)(%s[index]) | %s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else if (!l_is_float && r_is_float)
				sprintf(result, "index = %s;\n%s%s[index] = %s[index] | (int)(%s);\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			else
				sprintf(result, "index = %s;\n%s%s[index] = (double)((int)(%s[index]) | (int)(%s));\n%smangle(index, %s)", tmp, tab[tabs], (l_is_float ? "f" : "m"), (l_is_float ? "f" : "m"), rval, tab[tabs], (l_is_float ? "true" : "false"));
			exp->is_float = true;
			break;
		case NODE_DECREMENT_R:
			sprintf(result, "--%s", lval);
			exp->is_float = l_is_float;
			break;
		case NODE_DECREMENT_L:
			sprintf(result, "%s--", lval);
			exp->is_float = l_is_float;
			break;
		case NODE_SUB:
			if (l_is_float && !r_is_float)
				sprintf(result, "(%s - (double)(%s))", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "((double)(%s) - %s)", lval, rval);
			else
				sprintf(result, "(%s - %s)", lval, rval);
			exp->is_float = l_is_float | r_is_float;
			break;
		case NODE_MUL:
			if (l_is_float && !r_is_float)
				sprintf(result, "(%s * (double)(%s))", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "((double)(%s) * %s)", lval, rval);
			else
				sprintf(result, "(%s * %s)", lval, rval);
			exp->is_float = l_is_float | r_is_float;
			break;
		case NODE_DIV:
			if (!l_is_float && !r_is_float)
				sprintf(result, "(((double)(%s) != 0.0) ? (double)(%s) / (double)(%s) : 0.0)", rval, lval, rval);
			else if (l_is_float && !r_is_float)
				sprintf(result, "((%s != 0.0) ? %s / (double)(%s) : 0.0)", rval, lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "(((double)(%s) != 0.0) ? (double)(%s) / %s : 0.0)", rval, lval, rval);
			else
				sprintf(result, "((%s != 0.0) ? %s / %s : 0.0)", rval, lval, rval);
			exp->is_float = true;
			break;
		case NODE_MOD:
			if (l_is_float && r_is_float)
				sprintf(result, "(((int)(%s) > 0) ? (int)(%s) %% (int)(%s) : 0)", rval, lval, rval);
			else if (l_is_float && !r_is_float)
				sprintf(result, "((%s > 0) ? (int)(%s) %% %s : 0)", rval, lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "(((int)(%s) > 0) ? %s %% (int)(%s) : 0)", rval, lval, rval);
			else
				sprintf(result, "((%s > 0) ? %s %% %s : 0)", rval, lval, rval);
			exp->is_float = false;
			break;
		case NODE_LSHIFT:
			if (l_is_float && r_is_float)
				sprintf(result, "((int)(%s) << (int)(%s))", lval, rval);
			else if (l_is_float && !r_is_float)
				sprintf(result, "((int)(%s) << %s)", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "(%s << (int)(%s))", lval, rval);
			else
				sprintf(result, "(%s << %s)", lval, rval);
			exp->is_float = false;
			break;
		case NODE_LROT:
			if (l_is_float && r_is_float)
				sprintf(result, "rotl32( (int)(%s), (int)(%s) %% 32)", lval, rval);
			else if (l_is_float && !r_is_float)
				sprintf(result, "rotl32( (int)(%s), %s %% 32)", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "rotl32( %s, (int)(%s) %% 32)", lval, rval);
			else
				sprintf(result, "rotl32( %s, %s %% 32)", lval, rval);
			exp->is_float = false;
			break;
		case NODE_RSHIFT:
			if (l_is_float && r_is_float)
				sprintf(result, "((int)(%s) >> (int)(%s))", lval, rval);
			else if (l_is_float && !r_is_float)
				sprintf(result, "((int)(%s) >> %s)", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "(%s >> (int)(%s))", lval, rval);
			else
				sprintf(result, "(%s >> %s)", lval, rval);
			exp->is_float = false;
			break;
		case NODE_RROT:
			if (l_is_float && r_is_float)
				sprintf(result, "rotr32( (int)(%s), (int)(%s) %% 32)", lval, rval);
			else if (l_is_float && !r_is_float)
				sprintf(result, "rotr32( (int)(%s), %s %% 32)", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "rotr32( %s, (int)(%s) %% 32)", lval, rval);
			else
				sprintf(result, "rotr32( %s, %s %% 32)", lval, rval);
			exp->is_float = false;
			break;
		case NODE_NOT:
			sprintf(result, "!(%s)", lval);
			exp->is_float = l_is_float;
			break;
		case NODE_COMPL:
			sprintf(result, "~(%s)", lval);
			exp->is_float = l_is_float;
			break;
		case NODE_AND:
			if (l_is_float && !r_is_float)
				sprintf(result, "(%s && (double)(%s))", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "(%s && (int)(%s))", lval, rval);
			else
				sprintf(result, "(%s && %s)", lval, rval);
			exp->is_float = false;
			break;
		case NODE_OR:
			if (l_is_float && !r_is_float)
				sprintf(result, "(%s || (double)(%s))", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "(%s || (int)(%s))", lval, rval);
			else
				sprintf(result, "(%s || %s)", lval, rval);
			exp->is_float = false;
			break;
		case NODE_BITWISE_AND:
			if (l_is_float && r_is_float)
				sprintf(result, "((int)(%s) & (int)(%s))", lval, rval);
			else if (l_is_float && !r_is_float)
				sprintf(result, "((int)(%s) & %s)", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "(%s & (int)(%s))", lval, rval);
			else
				sprintf(result, "(%s & %s)", lval, rval);
			exp->is_float = false;
			break;
		case NODE_BITWISE_XOR:
			if (l_is_float && r_is_float)
				sprintf(result, "((int)(%s) ^ (int)(%s))", lval, rval);
			else if (l_is_float && !r_is_float)
				sprintf(result, "((int)(%s) ^ %s)", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "(%s ^ (int)(%s))", lval, rval);
			else
				sprintf(result, "(%s ^ %s)", lval, rval);
			exp->is_float = false;
			break;
		case NODE_BITWISE_OR:
			if (l_is_float && r_is_float)
				sprintf(result, "((int)(%s) | (int)(%s))", lval, rval);
			else if (l_is_float && !r_is_float)
				sprintf(result, "((int)(%s) | %s)", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "(%s | (int)(%s))", lval, rval);
			else
				sprintf(result, "(%s | %s)", lval, rval);
			exp->is_float = false;
			break;
		case NODE_EQ:
			if (l_is_float && !r_is_float)
				sprintf(result, "(%s == (double)(%s))", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "(%s == (int)(%s))", lval, rval);
			else
				sprintf(result, "(%s == %s)", lval, rval);
			exp->is_float = false;
			break;
		case NODE_NE:
			if (l_is_float && !r_is_float)
				sprintf(result, "(%s != (double)(%s))", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "(%s != (int)(%s))", lval, rval);
			else
				sprintf(result, "(%s != %s)", lval, rval);
			exp->is_float = false;
			break;
		case NODE_GT:
			if (l_is_float && !r_is_float)
				sprintf(result, "(%s > (double)(%s))", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "(%s > (int)(%s))", lval, rval);
			else
				sprintf(result, "(%s > %s)", lval, rval);
			exp->is_float = false;
			break;
		case NODE_LT:
			if (l_is_float && !r_is_float)
				sprintf(result, "(%s < (double)(%s))", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "(%s < (int)(%s))", lval, rval);
			else
				sprintf(result, "(%s < %s)", lval, rval);
			exp->is_float = false;
			break;
		case NODE_GE:
			if (l_is_float && !r_is_float)
				sprintf(result, "(%s >= (double)(%s))", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "(%s >= (int)(%s))", lval, rval);
			else
				sprintf(result, "(%s >= %s)", lval, rval);
			exp->is_float = false;
			break;
		case NODE_LE:
			if (l_is_float && !r_is_float)
				sprintf(result, "(%s <= (double)(%s))", lval, rval);
			else if (!l_is_float && r_is_float)
				sprintf(result, "(%s <= (int)(%s))", lval, rval);
			else
				sprintf(result, "(%s <= %s)", lval, rval);
			exp->is_float = false;
			break;
		case NODE_NEG:
			sprintf(result, "-(%s)", lval);
			exp->is_float = l_is_float;
			break;
		case NODE_VERIFY:
			sprintf(result, "\n\tbounty_found = (%s != 0 ? 1 : 0)", lval);
			break;
		case NODE_PARAM:
			if (rval)
				sprintf(result, "%s, %s", lval, rval);
			else
				sprintf(result, "%s", lval);
			break;
		case NODE_SIN:
			sprintf(result, "sin( %s )", rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_COS:
			sprintf(result, "cos( %s )", rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_TAN:
			sprintf(result, "tan( %s )", rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_SINH:
			sprintf(result, "(((%s >= -1.0) && (%s <= 1.0)) ? sinh( %s ) : 0.0)", rval, rval, rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_COSH:
			sprintf(result, "(((%s >= -1.0) && (%s <= 1.0)) ? cosh( %s ) : 0.0)", rval, rval, rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_TANH:
			sprintf(result, "tanh( %s )", rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_ASIN:
			sprintf(result, "(((%s >= -1.0) && (%s <= 1.0)) ? asin( %s ) : 0.0)", rval, rval, rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_ACOS:
			sprintf(result, "(((%s >= -1.0) && (%s <= 1.0)) ? acos( %s ) : 0.0)", rval, rval, rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_ATAN:
			sprintf(result, "atan( %s )", rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_ATAN2:
			tmp = strstr(rval, ",");	// Point To Second Argurment
			sprintf(result, "((%s != 0) ? atan2( %s ) : 0.0)", tmp + 1, rval);
			tmp = NULL;
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_EXPNT:
			sprintf(result, "(((%s >= -708.0) && (%s <= 709.0)) ? exp( %s ) : 0.0)", rval, rval, rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_LOG:
			sprintf(result, "((%s > 0) ? log( %s ) : 0.0)", rval, rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_LOG10:
			sprintf(result, "((%s > 0) ? log10( %s ) : 0.0)", rval, rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_POW:
			sprintf(result, "pow( %s )", rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_SQRT:
			sprintf(result, "((%s > 0) ? sqrt( %s ) : 0.0)", rval, rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_CEIL:
			sprintf(result, "ceil( %s )", rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_FLOOR:
			sprintf(result, "floor( %s )", rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_ABS:
			sprintf(result, "abs( %s )", rval);
			exp->is_float = false;
			use_elasticpl_math = true;
			break;
		case NODE_FABS:
			sprintf(result, "fabs( %s )", rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_FMOD:
			tmp = strstr(rval, ",");	// Point To Second Argurment
			sprintf(result, "((%s != 0) ? fmod( %s ) : 0.0)", tmp + 1, rval);
			tmp = NULL;
			exp->is_float = true;
			use_elasticpl_math = true;
			break;
		case NODE_GCD:
			sprintf(result, "gcd( %s )", rval);
			exp->is_float = true;
			use_elasticpl_math = true;
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_CONST:
			tmp = strstr(rval, ",");
			tmp[1] = '\"';
			tmp = NULL;
			sprintf(result, "mangle_state( big_init_const( %s\", b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_EXPR:
			sprintf(result, "mangle_state( big_init_expr( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_COPY:
			sprintf(result, "mangle_state( big_copy( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_ADD:
			sprintf(result, "mangle_state( big_add( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_SUB:
			sprintf(result, "mangle_state( big_sub( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_MUL:
			sprintf(result, "mangle_state( big_mul( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_DIV:
			sprintf(result, "mangle_state( big_div( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_CEIL_DIV:
			sprintf(result, "mangle_state( big_ceil_div( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_FLOOR_DIV:
			sprintf(result, "mangle_state( big_floor_div( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_TRUNC_DIV:
			sprintf(result, "mangle_state( big_truncate_div( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_DIV_EXACT:
			sprintf(result, "mangle_state( big_div_exact( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_MOD:
			sprintf(result, "mangle_state( big_mod( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_NEG:
			sprintf(result, "mangle_state( big_neg( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_LSHIFT:
			sprintf(result, "mangle_state( big_lshift( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_RSHIFT:
			sprintf(result, "mangle_state( big_rshift( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_GCD:
			sprintf(result, "mangle_state( big_gcd( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_DIVISIBLE:
			sprintf(result, "big_divisible( %s, b )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_CNGR_MOD_P:
			sprintf(result, "big_congruent_mod_p( %s, b )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_POW:
			sprintf(result, "mangle_state( big_pow( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_POW2:
			sprintf(result, "mangle_state( big_pow2( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_POW_MOD_P:
			sprintf(result, "mangle_state( big_pow_mod_p( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_POW2_MOD_P:
			sprintf(result, "mangle_state( big_pow2_mod_p( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_COMP:
			sprintf(result, "big_compare( %s, b )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_COMP_ABS:
			sprintf(result, "big_compare_abs( %s, b )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_SIGN:
			sprintf(result, "big_sign( %s, b )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_OR:
			sprintf(result, "mangle_state( big_or( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_AND:
			sprintf(result, "mangle_state( big_and( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_XOR:
			sprintf(result, "mangle_state( big_xor( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_OR_INT:
			sprintf(result, "mangle_state( big_or_integer( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_AND_INT:
			sprintf(result, "mangle_state( big_and_integer( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_XOR_INT:
			sprintf(result, "mangle_state( big_xor_integer( %s, b, &bi_size ) )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_BI_LEAST_32:
			sprintf(result, "big_least_32bit( %s, b )", rval);
			use_elasticpl_bigint = true;
			break;
		case NODE_SHA256:
			sprintf(result, "mangle_state( epl_sha256( %s, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SHA512:
			sprintf(result, "mangle_state( epl_sha512( %s, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_WHIRLPOOL:
			sprintf(result, "mangle_state( epl_whirlpool( %s, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_MD5:
			sprintf(result, "mangle_state( epl_md5( %s, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP192K_PTP:
			sprintf(result, "mangle_state( epl_ec_priv_to_pub( %s, NID_secp192k1, 24, 48, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP192K_PA:
			sprintf(result, "mangle_state( epl_ec_add( %s, NID_secp192k1, 25, 49, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP192K_PS:
			sprintf(result, "mangle_state( epl_ec_sub( %s, NID_secp192k1, 25, 49, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP192K_PSM:
			sprintf(result, "mangle_state( epl_ec_mul( %s, NID_secp192k1, 25, 49, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP192K_PN:
			sprintf(result, "mangle_state( epl_ec_neg( %s, NID_secp192k1, 25, 49, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP224K_PTP:
			sprintf(result, "mangle_state( epl_ec_priv_to_pub( %s, NID_secp224k1, 28, 56, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP224K_PA:
			sprintf(result, "mangle_state( epl_ec_add( %s, NID_secp224k1, 29, 57, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP224K_PS:
			sprintf(result, "mangle_state( epl_ec_sub( %s, NID_secp224k1, 29, 57, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP224K_PSM:
			sprintf(result, "mangle_state( epl_ec_mul( %s, NID_secp224k1, 29, 57, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP224K_PN:
			sprintf(result, "mangle_state( epl_ec_neg( %s, NID_secp224k1, 29, 57, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP224R_PTP:
			sprintf(result, "mangle_state( epl_ec_priv_to_pub( %s, NID_secp224r1, 28, 56, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP224R_PA:
			sprintf(result, "mangle_state( epl_ec_add( %s, NID_secp224r1, 29, 57, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP224R_PS:
			sprintf(result, "mangle_state( epl_ec_sub( %s, NID_secp224r1, 29, 57, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP224R_PSM:
			sprintf(result, "mangle_state( epl_ec_mul( %s, NID_secp224r1, 29, 57, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP224R_PN:
			sprintf(result, "mangle_state( epl_ec_neg( %s, NID_secp224r1, 29, 57, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP256K_PTP:
			sprintf(result, "mangle_state( epl_ec_priv_to_pub( %s, NID_secp256k1, 32, 64, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP256K_PA:
			sprintf(result, "mangle_state( epl_ec_add( %s, NID_secp256k1, 33, 65, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP256K_PS:
			sprintf(result, "mangle_state( epl_ec_sub( %s, NID_secp256k1, 33, 65, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP256K_PSM:
			sprintf(result, "mangle_state( epl_ec_mul( %s, NID_secp256k1, 33, 65, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP256K_PN:
			sprintf(result, "mangle_state( epl_ec_neg( %s, NID_secp256k1, 33, 65, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP384R_PTP:
			sprintf(result, "mangle_state( epl_ec_priv_to_pub( %s, NID_secp384r1, 48, 96, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP384R_PA:
			sprintf(result, "mangle_state( epl_ec_add( %s, NID_secp384r1, 49, 97, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP384R_PS:
			sprintf(result, "mangle_state( epl_ec_sub( %s, NID_secp384r1, 49, 97, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP384R_PSM:
			sprintf(result, "mangle_state( epl_ec_mul( %s, NID_secp384r1, 49, 97, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_SECP384R_PN:
			sprintf(result, "mangle_state( epl_ec_neg( %s, NID_secp384r1, 49, 97, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM192V1_PTP:
			sprintf(result, "mangle_state( epl_ec_priv_to_pub( %s, NID_X9_62_prime192v1, 24, 48, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM192V1_PA:
			sprintf(result, "mangle_state( epl_ec_add( %s, NID_X9_62_prime192v1, 25, 49, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM192V1_PS:
			sprintf(result, "mangle_state( epl_ec_sub( %s, NID_X9_62_prime192v1, 25, 49, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM192V1_PSM:
			sprintf(result, "mangle_state( epl_ec_mul( %s, NID_X9_62_prime192v1, 25, 49, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM192V1_PN:
			sprintf(result, "mangle_state( epl_ec_neg( %s, NID_X9_62_prime192v1, 25, 49, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM192V2_PTP:
			sprintf(result, "mangle_state( epl_ec_priv_to_pub( %s, NID_X9_62_prime192v2, 24, 48, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM192V2_PA:
			sprintf(result, "mangle_state( epl_ec_add( %s, NID_X9_62_prime192v2, 25, 49, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM192V2_PS:
			sprintf(result, "mangle_state( epl_ec_sub( %s, NID_X9_62_prime192v2, 25, 49, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM192V2_PSM:
			sprintf(result, "mangle_state( epl_ec_mul( %s, NID_X9_62_prime192v2, 25, 49, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM192V2_PN:
			sprintf(result, "mangle_state( epl_ec_neg( %s, NID_X9_62_prime192v2, 25, 49, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM192V3_PTP:
			sprintf(result, "mangle_state( epl_ec_priv_to_pub( %s, NID_X9_62_prime192v3, 24, 48, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM192V3_PA:
			sprintf(result, "mangle_state( epl_ec_add( %s, NID_X9_62_prime192v3, 25, 49, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM192V3_PS:
			sprintf(result, "mangle_state( epl_ec_sub( %s, NID_X9_62_prime192v3, 25, 49, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM192V3_PSM:
			sprintf(result, "mangle_state( epl_ec_mul( %s, NID_X9_62_prime192v3, 25, 49, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM192V3_PN:
			sprintf(result, "mangle_state( epl_ec_neg( %s, NID_X9_62_prime192v3, 25, 49, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM256V1_PTP:
			sprintf(result, "mangle_state( epl_ec_priv_to_pub( %s, NID_X9_62_prime256v1, 32, 64, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM256V1_PA:
			sprintf(result, "mangle_state( epl_ec_add( %s, NID_X9_62_prime256v1, 33, 65, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM256V1_PS:
			sprintf(result, "mangle_state( epl_ec_sub( %s, NID_X9_62_prime256v1, 33, 65, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM256V1_PSM:
			sprintf(result, "mangle_state( epl_ec_mul( %s, NID_X9_62_prime256v1, 33, 65, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_PRM256V1_PN:
			sprintf(result, "mangle_state( epl_ec_neg( %s, NID_X9_62_prime256v1, 33, 65, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		case NODE_RIPEMD160:
			sprintf(result, "mangle_state( epl_ripemd160( %s, b, &bi_size ) )", rval);
			use_elasticpl_crypto = true;
			break;
		default:
			sprintf(result, "fprintf(stderr, \"ERROR: VM Runtime - Unsupported Operation (%d)\");\n", exp->type);
		}

		if (lval) free(lval);
		if (rval) free(rval);
		if (tmp) free(tmp);

		// Terminate Statements
		if (exp->end_stmnt && (exp->type != NODE_BLOCK)) {
			tmp = malloc(strlen(result) + 20);
			sprintf(tmp, "%s%s;\n", tab[tabs], result);
			free(result);
			result = tmp;
		}
	}

	return result;
}

static char* append_strings(char * old, char * new) {

	char* out = NULL;

	if (new == NULL && old != NULL) {
		out = calloc(strlen(old) + 1, sizeof(char));
		strcpy(out, old);
	}
	else if (old == NULL && new != NULL) {
		out = calloc(strlen(new) + 1, sizeof(char));
		strcpy(out, new);
	}
	else if (new == NULL && old == NULL) {
		// pass
	}
	else {
		// find the size of the string to allocate
		const size_t old_len = strlen(old), new_len = strlen(new);
		const size_t out_len = old_len + new_len + 1;
		// allocate a pointer to the new string
		out = malloc(out_len);
		// concat both strings and return
		memcpy(out, old, old_len);
		strcpy(out + old_len, new);
	}

	// Free here
	if (old != NULL) {
		free(old);
		old = NULL;
	}
	if (new != NULL) {
		free(new);
		new = NULL;
	}

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
