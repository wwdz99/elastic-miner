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

#include "ElasticPL.h"

int debug_stack_op[100];
ast* debug_stack_exp[100];

ast* add_exp(NODE_TYPE node_type, TOKEN_EXP exp_type, long value, int token_num, int line_num, ast* left, ast* right) {
	ast* e = calloc(1, sizeof(ast));
	if (e) {
		e->type = node_type;
		e->exp = exp_type;
		e->value = value;
		e->token_num = token_num;
		e->line_num = line_num;
		e->left = left;
		e->right = right;
	}
	return e;
};

static void push_op(int token_id) {
	stack_op[++stack_op_idx] = token_id;
	top_op = token_id;

//
//
memcpy(debug_stack_op, stack_op, 100 * sizeof(int));
//
//

}

static int pop_op() {
	int op = -1;
	if (stack_op_idx >= 0) {
		op = stack_op[stack_op_idx];
		stack_op[stack_op_idx--] = -1;
	}

	if (stack_op_idx >= 0)
		top_op = stack_op[stack_op_idx];
	else
		top_op = -1;


//
//
memcpy(debug_stack_op, stack_op, 100 * sizeof(int));
//
//
	return op;
}

static void push_exp(ast* exp) {
	stack_exp[++stack_exp_idx] = exp;


//
//
	//int i;
	//for (i = 0; i < 10; i++)
	//	debug_stack_exp[i] = stack_exp[i];
	memcpy(&debug_stack_exp[0], &stack_exp[0], 100 * sizeof(struct ast*));
//
//


}

static ast* pop_exp() {
	ast *exp = NULL;

	if (stack_exp_idx >= 0) {
		exp = stack_exp[stack_exp_idx];
		stack_exp[stack_exp_idx--] = NULL;
	}


//
//
	//int i;
	//for (i = 0; i < 10; i++)
	//	debug_stack_exp[i] = stack_exp[i];
	memcpy(&debug_stack_exp[0], &stack_exp[0], 100 * sizeof(struct ast*));
//
//

	return exp;
}

static NODE_TYPE validate_unary_stmnt(SOURCE_TOKEN *token) {
	NODE_TYPE node_type;
	TOKEN_EXP l_exp;

	switch (token->type) {
	case TOKEN_VERIFY:		node_type = NODE_VERIFY;	break;
	default: return NODE_ERROR;
	}

	// Validate That There Is At Least 1 Expressions On The Stack
	if (stack_exp_idx < 0) {
		printf("Syntax Error - Line: %d  Missing Operand: \"%s\"\n", token->line_num, get_node_str(node_type));
		return NODE_ERROR;
	}

	l_exp = stack_exp[stack_exp_idx]->exp;

	// Validate Left Item Is Not A Statement
	if (l_exp == UNARY_STATEMENT || l_exp == BINARY_STATEMENT) {
		printf("Syntax Error - Line: %d  Invalid Operand: \"%s\"\n", token->line_num, get_node_str(node_type));
		return NODE_ERROR;
	}

	return node_type;
}

static NODE_TYPE validate_binary_stmnt(SOURCE_TOKEN *token) {
	NODE_TYPE node_type;
	TOKEN_EXP l_exp, r_exp;

	switch (token->type) {
	case TOKEN_BLOCK_END:		node_type = NODE_BLOCK;			break;
	case TOKEN_IF:				node_type = NODE_IF;			break;
	case TOKEN_ELSE:			node_type = NODE_ELSE;			break;
	case TOKEN_REPEAT:			node_type = NODE_REPEAT;		break;
	case TOKEN_ASSIGN:			node_type = NODE_ASSIGN;		break;
	case TOKEN_SHA256:			node_type = NODE_SHA256;		break;
	case TOKEN_SHA512:			node_type = NODE_SHA512;		break;
	case TOKEN_WHIRLPOOL:	   	node_type = NODE_WHIRLPOOL;		break;
	case TOKEN_MD5:	           	node_type = NODE_MD5;			break;
	case TOKEN_SECP192K_PTP:	node_type = NODE_SECP192K_PTP;	break;
	case TOKEN_SECP192K_PA:	   	node_type = NODE_SECP192K_PA;	break;
	case TOKEN_SECP192K_PS:	   	node_type = NODE_SECP192K_PS;	break;
	case TOKEN_SECP192K_PSM:	node_type = NODE_SECP192K_PSM;	break;
	case TOKEN_SECP192K_PN:	    node_type = NODE_SECP192K_PN;	break;
	case TOKEN_SECP192R_PTP:	node_type = NODE_SECP192R_PTP;	break;
	case TOKEN_SECP192R_PA:	    node_type = NODE_SECP192R_PA;	break;
	case TOKEN_SECP192R_PS:	    node_type = NODE_SECP192R_PS;	break;
	case TOKEN_SECP192R_PSM:	node_type = NODE_SECP192R_PSM;	break;
	case TOKEN_SECP192R_PN:	    node_type = NODE_SECP192R_PN;	break;
	case TOKEN_SECP224K_PTP:	node_type = NODE_SECP224K_PTP;	break;
	case TOKEN_SECP224K_PA:	    node_type = NODE_SECP224K_PA;	break;
	case TOKEN_SECP224K_PS:	    node_type = NODE_SECP224K_PS;	break;
	case TOKEN_SECP224K_PSM:	node_type = NODE_SECP224K_PSM;	break;
	case TOKEN_SECP224K_PN:	    node_type = NODE_SECP224K_PN;	break;
	case TOKEN_SECP224R_PTP:	node_type = NODE_SECP224R_PTP;	break;
	case TOKEN_SECP224R_PA:	    node_type = NODE_SECP224R_PA;	break;
	case TOKEN_SECP224R_PS:	    node_type = NODE_SECP224R_PS;	break;
	case TOKEN_SECP224R_PSM:	node_type = NODE_SECP224R_PSM;	break;
	case TOKEN_SECP224R_PN:	    node_type = NODE_SECP224R_PN;	break;
	case TOKEN_SECP256K_PTP:	node_type = NODE_SECP256K_PTP;	break;
	case TOKEN_SECP256K_PA:	    node_type = NODE_SECP256K_PA;	break;
	case TOKEN_SECP256K_PS:	    node_type = NODE_SECP256K_PS;	break;
	case TOKEN_SECP256K_PSM:	node_type = NODE_SECP256K_PSM;	break;
	case TOKEN_SECP256K_PN:	    node_type = NODE_SECP256K_PN;	break;
	case TOKEN_SECP256R_PTP:	node_type = NODE_SECP256R_PTP;	break;
	case TOKEN_SECP256R_PA:	    node_type = NODE_SECP256R_PA;	break;
	case TOKEN_SECP256R_PS:	    node_type = NODE_SECP256R_PS;	break;
	case TOKEN_SECP256R_PSM:	node_type = NODE_SECP256R_PSM;	break;
	case TOKEN_SECP256R_PN:	    node_type = NODE_SECP256R_PN;	break;
	case TOKEN_SECP384R_PTP:	node_type = NODE_SECP384R_PTP;	break;
	case TOKEN_SECP384R_PA:	    node_type = NODE_SECP384R_PA;	break;
	case TOKEN_SECP384R_PS:	    node_type = NODE_SECP384R_PS;	break;
	case TOKEN_SECP384R_PSM:	node_type = NODE_SECP384R_PSM;	break;
	case TOKEN_SECP384R_PN:	    node_type = NODE_SECP384R_PN;	break;
	case TOKEN_PRM192V1_PTP:	node_type = NODE_PRM192V1_PTP;	break;
	case TOKEN_PRM192V1_PA:	    node_type = NODE_PRM192V1_PA;	break;
	case TOKEN_PRM192V1_PS:	    node_type = NODE_PRM192V1_PS;	break;
	case TOKEN_PRM192V1_PSM:	node_type = NODE_PRM192V1_PSM;	break;
	case TOKEN_PRM192V1_PN:	    node_type = NODE_PRM192V1_PN;	break;
	case TOKEN_PRM192V2_PTP:	node_type = NODE_PRM192V2_PTP;	break;
	case TOKEN_PRM192V2_PA:	    node_type = NODE_PRM192V2_PA;	break;
	case TOKEN_PRM192V2_PS:	    node_type = NODE_PRM192V2_PS;	break;
	case TOKEN_PRM192V2_PSM:	node_type = NODE_PRM192V2_PSM;	break;
	case TOKEN_PRM192V2_PN:	    node_type = NODE_PRM192V2_PN;	break;
	case TOKEN_PRM192V3_PTP:	node_type = NODE_PRM192V3_PTP;	break;
	case TOKEN_PRM192V3_PA:	    node_type = NODE_PRM192V3_PA;	break;
	case TOKEN_PRM192V3_PS:	    node_type = NODE_PRM192V3_PS;	break;
	case TOKEN_PRM192V3_PSM:	node_type = NODE_PRM192V3_PSM;	break;
	case TOKEN_PRM192V3_PN:	    node_type = NODE_PRM192V3_PN;	break;
	case TOKEN_PRM256V1_PTP:	node_type = NODE_PRM256V1_PTP;	break;
	case TOKEN_PRM256V1_PA:	    node_type = NODE_PRM256V1_PA;	break;
	case TOKEN_PRM256V1_PS:	    node_type = NODE_PRM256V1_PS;	break;
	case TOKEN_PRM256V1_PSM:	node_type = NODE_PRM256V1_PSM;	break;
	case TOKEN_PRM256V1_PN:	    node_type = NODE_PRM256V1_PN;	break;
	case TOKEN_TIGER:	        node_type = NODE_TIGER;			break;
	case TOKEN_RIPEMD160:	    node_type = NODE_RIPEMD160;		break;
	case TOKEN_RIPEMD128:      	node_type = NODE_RIPEMD128;		break;
	default: return NODE_ERROR;
	}

	// Validate That There Are At Least 2 Expressions On The Stack
	if (stack_exp_idx < 1) {
		printf("Syntax Error - Line: %d  Missing Operand: \"%s\"\n", token->line_num, get_node_str(node_type));
		return NODE_ERROR;
	}

	l_exp = stack_exp[stack_exp_idx - 1]->exp;
	r_exp = stack_exp[stack_exp_idx]->exp;


	//
	// Fix For Blocks
	//

	// Validate Left Item Is Not A Statement
	//if (l_exp == UNARY_STATEMENT || l_exp == BINARY_STATEMENT) {
	//	printf("Syntax Error - Line: %d  Invalid Left Operand: \"%s\"\n", token->line_num, get_node_str(node_type));
	//	return NODE_ERROR;
	//}

	// Validate Right Item Is Not A Statement
	//if (r_exp == UNARY_STATEMENT || r_exp == BINARY_STATEMENT) {
	//	printf("Syntax Error - Line: %d  Invalid Right Operand: \"%s\"\n", token->line_num, node_str2[node_type]);
	//	return NODE_ERROR;
	//}

	return node_type;
}

// Validate Unary Operations Have 1 Valid Expression
static NODE_TYPE validate_unary_exp(SOURCE_TOKEN *token, int token_num) {
	NODE_TYPE node_type;

	// Convert Token Type To Node Type
	switch (token->type) {
	case TOKEN_VAR_END:
		if (stack_exp_idx >= 0 && stack_exp[stack_exp_idx]->type == NODE_CONSTANT)
			node_type = NODE_VAR_CONST;
		else
			node_type = NODE_VAR_EXP;
		break;
	case TOKEN_COMPL:	node_type = NODE_COMPL;	break;
	case TOKEN_NOT:		node_type = NODE_NOT;	break;
	case TOKEN_POS:		node_type = NODE_POS;	break;
	case TOKEN_NEG:		node_type = NODE_NEG;	break;
	case TOKEN_LITERAL:	return NODE_CONSTANT;
	default: return NODE_ERROR;
	}

	// Validate That There Is At Least 1 Expression On The Stack
	if (stack_exp_idx < 0) {
		printf("Syntax Error - Line: %d  Invalid Use Of: \"%s\"\n", token->line_num, get_node_str(node_type));
		return NODE_ERROR;
	}

	//
	// Add Check That Unary Operator Can't Be First Token On New Statement
	//

	// Validate Expression Is Not A Statement (Left For Variables, Right For Other Unary Expressions)
	if (stack_exp[stack_exp_idx]->exp != UNARY_STATEMENT && stack_exp[stack_exp_idx]->exp != BINARY_STATEMENT) {

		// Check Left Expression For Variables
		if (node_type == NODE_VAR_CONST || node_type == NODE_VAR_EXP) {
			if (stack_exp[stack_exp_idx]->token_num >= token_num) {
				printf("Syntax Error - Line: %d  Invalid Operand For: \"%s\"\n", token->line_num, get_node_str(node_type));
				return NODE_ERROR;
			}
		}
		// Check Right Expression For Other Unary Operators
		else {
			if (stack_exp[stack_exp_idx]->token_num <= token_num) {
				printf("Syntax Error - Line: %d  Invalid Operand For: \"%s\"\n", token->line_num, get_node_str(node_type));
				return NODE_ERROR;
			}
		}
	}

	return node_type;
}

static NODE_TYPE validate_binary_exp(SOURCE_TOKEN *token) {
	NODE_TYPE node_type;
	TOKEN_EXP l_exp, r_exp;

	switch (token->type) {
	case TOKEN_MUL:		node_type = NODE_MUL;	break;
	case TOKEN_DIV:		node_type = NODE_DIV;	break;
	case TOKEN_MOD:		node_type = NODE_MOD;	break;
	case TOKEN_ADD:		node_type = NODE_ADD;	break;
	case TOKEN_SUB:		node_type = NODE_SUB;	break;
	case TOKEN_LROT:	node_type = NODE_LROT;	break;
	case TOKEN_LSHIFT:	node_type = NODE_LSHIFT; break;
	case TOKEN_RROT:	node_type = NODE_RROT;	break;
	case TOKEN_RSHIFT:	node_type = NODE_RSHIFT; break;
	case TOKEN_LE:		node_type = NODE_LE;	break;
	case TOKEN_GE:		node_type = NODE_GE;	break;
	case TOKEN_LT:		node_type = NODE_LT;	break;
	case TOKEN_GT:		node_type = NODE_GT;	break;
	case TOKEN_EQ:		node_type = NODE_EQ;	break;
	case TOKEN_NE:		node_type = NODE_NE;	break;
	case TOKEN_BITWISE_AND: node_type = NODE_BITWISE_AND; break;
	case TOKEN_BITWISE_XOR: node_type = NODE_BITWISE_XOR; break;
	case TOKEN_BITWISE_OR: node_type = NODE_BITWISE_OR; break;
	case TOKEN_AND:		node_type = NODE_AND;	break;
	case TOKEN_OR:		node_type = NODE_OR;	break;
	default: return NODE_ERROR;
	}

	// Validate That There Are At Least 2 Expressions On The Stack
	if (stack_exp_idx < 1) {
		printf("Syntax Error - Line: %d  Missing Operand: \"%s\"\n", token->line_num, get_node_str(node_type));
		return NODE_ERROR;
	}

	l_exp = stack_exp[stack_exp_idx - 1]->exp;
	r_exp = stack_exp[stack_exp_idx]->exp;

	// Validate Left Item Is Not A Statement
	if (l_exp == UNARY_STATEMENT || l_exp == BINARY_STATEMENT) {
		printf("Syntax Error - Line: %d  Invalid Left Operand: \"%s\"\n", token->line_num, get_node_str(node_type));
		return NODE_ERROR;
	}

	// Validate Right Item Is Not A Statement
	if (r_exp == UNARY_STATEMENT || r_exp == BINARY_STATEMENT) {
		printf("Syntax Error - Line: %d  Invalid Right Operand: \"%s\"\n", token->line_num, get_node_str(node_type));
		return NODE_ERROR;
	}

	return node_type;
}

static bool create_exp(SOURCE_TOKEN *token, int token_num) {
	long long value = 0;
	NODE_TYPE node_type = NODE_ERROR;
	ast *exp, *left = NULL, *right = NULL;

	switch (token->exp) {

	case BINARY_EXPRESSION:
		node_type = validate_binary_exp(token);
		if (node_type == NODE_ERROR) {
			return false;
		}
		else {
			right = pop_exp();
			left = pop_exp();
		}
		break;

	case UNARY_EXPRESSION:
		node_type = validate_unary_exp(token, token_num);
		if (node_type == NODE_ERROR) {
			return false;
		}
		else {
			if (node_type == NODE_CONSTANT)	// Constants Have Values Not Leafs
				value = (long long)strtod(token->literal, NULL);
			else {
				left = pop_exp();
				if (node_type == NODE_VAR_CONST) { // Remove Expression For Variables w/ Constant ID
					value = left->value;
					left = NULL;
				}
			}
		}
		break;

	case BINARY_STATEMENT:
	case BINARY_STMNT_EXP:
		node_type = validate_binary_stmnt(token);
		if (node_type == NODE_ERROR) {
			return false;
		}
		else {
			if (node_type == NODE_BLOCK && stack_exp[stack_exp_idx]->type != NODE_BLOCK)
				right = NULL;
			else
				right = pop_exp();
			left = pop_exp();
		}
		break;

	case UNARY_STATEMENT:
	case UNARY_STMNT_EXP:
		node_type = validate_unary_stmnt(token);
		if (node_type == NODE_ERROR)
			return false;
		else
			left = pop_exp();
		break;
	}

	exp = add_exp(node_type, token->exp, (long)value, token_num, token->line_num, left, right);

	if (exp)
		push_exp(exp);
	else
		return false;

	return true;
}

static bool validate_exp_list() {
	int i;

	if (stack_exp_idx < 0) {
		printf("Syntax Error - Invalid Source File\n");
		return false;
	}

	for (i = 0; i < stack_exp_idx; i++) {
		if (stack_exp[i]->type == NODE_VERIFY) {
			printf("Syntax Error - Line: %d Invalid Verify Statement\n", stack_exp[i]->line_num);
			return false;
		}
	}

	if (stack_exp[stack_exp_idx]->type != NODE_VERIFY) {
		printf("Syntax Error - Missing Verify Statement\n");
		return false;
	}

	for (i = 0; i < stack_exp_idx; i++) {
		if (stack_exp[i]->exp != BINARY_STATEMENT && stack_exp[i]->exp != UNARY_STATEMENT && stack_exp[i]->exp != BINARY_STMNT_EXP) {
			printf("Syntax Error - Line: %d  Invalid Statement\n", stack_exp[i]->line_num);
			//			return false;
		}
	}

	if (stack_op_idx >= 0) {
		printf("Syntax Error - Unable To Clear Operator Stack\n");
		return false;
	}

	return true;
}

extern bool parse_token_list(SOURCE_TOKEN_LIST *token_list) {

	int i, token_id;
	ast *left, *right;

	for (i = 0; i < token_list->num; i++) {

		switch (token_list->token[i].type) {

		case TOKEN_LITERAL:
			if (!create_exp(&token_list->token[i], i)) return false;
			break;

		case TOKEN_VAR_BEGIN:
		case TOKEN_OPEN_PAREN:
		case TOKEN_BLOCK_BEGIN:
			push_op(i);
			break;

		case TOKEN_VAR_END:
			// Process Expressions Within The Variable Brackets
			while ((top_op >= 0) && (token_list->token[top_op].type != TOKEN_VAR_BEGIN)) {
				token_id = pop_op();
				if (!create_exp(&token_list->token[token_id], token_id)) return false;
			}
			pop_op();
			if (!create_exp(&token_list->token[i], i)) return false;

			// Check For Unary Operators On The Variable
			while ((top_op >= 0) && (token_list->token[top_op].type != TOKEN_VAR_BEGIN) && (token_list->token[top_op].exp == UNARY_EXPRESSION)) {
				token_id = pop_op();
				if (!create_exp(&token_list->token[token_id], token_id)) return false;
			}

			break;

		case TOKEN_CLOSE_PAREN:
			// Process Expressions Within The Paranthesis
			while ((top_op >= 0) && (token_list->token[top_op].type != TOKEN_OPEN_PAREN)) {
				token_id = pop_op();
				if (!create_exp(&token_list->token[token_id], token_id)) return false;
			}
			pop_op();
			break;

		case TOKEN_BLOCK_END:
			// Create A Linked List Of All Statements In The Block
			while (stack_exp_idx > 0 && stack_exp[stack_exp_idx - 1]->token_num >= top_op && stack_exp[stack_exp_idx]->token_num < i) {
				if (!create_exp(&token_list->token[i], top_op)) return false;
			}
			pop_op();

			// Process "If" And "Repeat" Expressions Opperator Than Owns The Block
			if ((top_op >= 0) && (token_list->token[top_op].type == TOKEN_IF || token_list->token[top_op].type == TOKEN_REPEAT)) {
				token_id = pop_op();
				if (!create_exp(&token_list->token[token_id], token_id))
					return false;
			}
			break;

		case TOKEN_ELSE:
			// Swap The Else Expression For The If Statement Right Expression
			if (stack_exp_idx >= 0 && stack_exp[stack_exp_idx]->type == NODE_IF) {
				// Put If Back On Stack & Add Else
				push_op(stack_exp[stack_exp_idx]->token_num);

				left = stack_exp[stack_exp_idx]->left;
				right = stack_exp[stack_exp_idx]->right;

				// Remove If Expression
				pop_exp();

				// Return Left & Right Expressions Back To Stack
				push_exp(left);
				push_exp(right);
			}
			else if (stack_op_idx < 0 || token_list->token[stack_op[stack_op_idx]].type != TOKEN_IF) {
				printf("Syntax Error - Line: %d  Invalid 'Else' Statement\n", token_list->token[token_id].line_num);
				return false;
			}
			push_op(i);
			break;

		default:
			// Process Expressions Already In Stack Based On Precedence
			while ((top_op >= 0) && (token_list->token[top_op].prec >= token_list->token[i].prec)) {
				token_id = pop_op();
				if (!create_exp(&token_list->token[token_id], token_id))
					return false;
			}

			// Don't Push ; Or <EOF> Onto Stack
			if (token_list->token[i].type != TOKEN_END_STATEMENT && token_list->token[i].type != TOKEN_EOF)
				push_op(i);

			break;
		}
	}

	if (stack_exp_idx < 0 || !validate_exp_list())
		return false;

	return true;
}