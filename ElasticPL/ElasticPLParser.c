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
#include "../miner.h"

static ast* add_exp(NODE_TYPE node_type, EXP_TYPE exp_type, int32_t value, double fvalue, unsigned char *svalue, int token_num, int line_num, DATA_TYPE data_type, ast* left, ast* right) {
	ast* e = calloc(1, sizeof(ast));
	if (e) {
		e->type = node_type;
		e->exp = exp_type;
		e->value = value;
		e->fvalue = fvalue;
		e->svalue = svalue;
		e->token_num = token_num;
		e->line_num = line_num;
		e->end_stmnt = false;
		e->data_type = data_type;
		e->is_float = (data_type == DT_FLOAT);
		e->left = left;
		e->right = right;
	}
	return e;
};

static void push_op(int token_id) {
	stack_op[++stack_op_idx] = token_id;
	top_op = token_id;
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

	return op;
}

static void push_exp(ast* exp) {
	stack_exp[++stack_exp_idx] = exp;
}

static ast* pop_exp() {
	ast *exp = NULL;

	if (stack_exp_idx >= 0) {
		exp = stack_exp[stack_exp_idx];
		stack_exp[stack_exp_idx--] = NULL;
	}

	return exp;
}


//int i, idx, inputs, num_exps = 0;
//
//if (token->inputs == 0)
//return true;
//
//// For If/Repeat Only Check The Left Expression (Right Side Will Be Statements)
//if ((node_type == NODE_IF) || (node_type == NODE_REPEAT)) {
//	idx = stack_exp_idx - 1;
//	inputs = token->inputs - 1;
//}
//else {
//	idx = stack_exp_idx;
//	inputs = token->inputs;
//}
//
//// Count The Number Of Expressions On The Stack
//for (i = idx; i >= 0; i--) {
//	//	for (i = stack_exp_idx; i >= 0; i--) {
//	if (stack_exp[i]->end_stmnt)
//		break;
//	num_exps++;
//}
//
//// Validate That There Are Enough Expressions On The Stack
//if (num_exps < inputs) {
//	applog(LOG_ERR, "Syntax Error (Invalid Inputs) - Line: %d' ", token->line_num);
//	return false;
//}



static bool validate_input_num(SOURCE_TOKEN *token, NODE_TYPE node_type) {
	int i, num_exps = 0;

	if ((token->inputs == 0) || (node_type == NODE_BLOCK))
		return true;

	// Count The Number Of Expressions On The Stack
	for (i = stack_exp_idx; i >= 0; i--) {

		// First Item For IF / ELSE / REPEAT Is A Statement
		if ((i == stack_exp_idx) && ((node_type == NODE_IF) || (node_type == NODE_ELSE) || (node_type == NODE_REPEAT))) {
			num_exps++;
			continue;
		}
			
		// Second Item For ELSE Is Also A Statement
		if ((i == (stack_exp_idx - 1)) && (node_type == NODE_ELSE)) {
			num_exps++;
			continue;
		}

		if (stack_exp[i]->end_stmnt)
			break;
		num_exps++;
	}

	// Validate That There Are Enough Expressions On The Stack
	if (num_exps < token->inputs) {
		applog(LOG_ERR, "Syntax Error (Invalid Inputs) - Line: %d' ", token->line_num);
		return false;
	}

	// Validate The Inputs Are The Correct Type
	switch (node_type) {

	// Expressions w/ 1 Statement & 1 Int / Float (Ignore Right Side)
	case NODE_IF:
	case NODE_REPEAT:
		if ((stack_exp[stack_exp_idx - 1]->data_type == DT_INT) || (stack_exp[stack_exp_idx - 1]->data_type == DT_FLOAT) &&
				(stack_exp[stack_exp_idx]->end_stmnt == true))
				return true;
		break;

	case NODE_ELSE:
		if ((stack_exp[stack_exp_idx - 1]->end_stmnt == true) && (stack_exp[stack_exp_idx]->end_stmnt == true))
			return true;
		break;

	// Expressions w/ 1 Int
	case NODE_ABS:
	case NODE_VERIFY:
		if (stack_exp[stack_exp_idx]->data_type == DT_INT)
			return true;
		break;

	// Expressions w/ 1 Int or Float
	case NODE_CONSTANT:
	case NODE_VAR_CONST:
	case NODE_VAR_EXP:
	case NODE_INCREMENT_R:
	case NODE_INCREMENT_L:
	case NODE_DECREMENT_R:
	case NODE_DECREMENT_L:
	case NODE_COMPL:
	case NODE_NOT:
	case NODE_NEG:
	case NODE_SIN:
	case NODE_COS:
	case NODE_TAN:
	case NODE_SINH:
	case NODE_COSH:
	case NODE_TANH:
	case NODE_ASIN:
	case NODE_ACOS:
	case NODE_ATAN:
	case NODE_EXPNT:
	case NODE_LOG:
	case NODE_LOG10:
	case NODE_SQRT:
	case NODE_CEIL:
	case NODE_FLOOR:
	case NODE_FABS:
		if ((stack_exp[stack_exp_idx]->data_type == DT_INT) || (stack_exp[stack_exp_idx]->data_type == DT_FLOAT))
			return true;
		break;

	// Expressions w/ 2 Ints or Floats
	case NODE_MUL:
	case NODE_DIV:
	case NODE_MOD:
	case NODE_ADD_ASSIGN:
	case NODE_SUB_ASSIGN:
	case NODE_MUL_ASSIGN:
	case NODE_DIV_ASSIGN:
	case NODE_MOD_ASSIGN:
	case NODE_LSHFT_ASSIGN:
	case NODE_RSHFT_ASSIGN:
	case NODE_AND_ASSIGN:
	case NODE_XOR_ASSIGN:
	case NODE_OR_ASSIGN:
	case NODE_ADD:
	case NODE_SUB:
	case NODE_LROT:
	case NODE_LSHIFT:
	case NODE_RROT:
	case NODE_RSHIFT:
	case NODE_LE:
	case NODE_GE:
	case NODE_LT:
	case NODE_GT:
	case NODE_EQ:
	case NODE_NE:
	case NODE_BITWISE_AND:
	case NODE_BITWISE_XOR:
	case NODE_BITWISE_OR:
	case NODE_AND:
	case NODE_OR:
	case NODE_ASSIGN:
	case NODE_CONDITIONAL:
	case NODE_COND_ELSE:
	case NODE_ATAN2:
	case NODE_POW:
	case NODE_FMOD:
	case NODE_GCD:
		if ((stack_exp[stack_exp_idx - 1]->data_type == DT_INT) || (stack_exp[stack_exp_idx - 1]->data_type == DT_FLOAT) &&
			(stack_exp[stack_exp_idx]->data_type == DT_INT) || (stack_exp[stack_exp_idx]->data_type == DT_FLOAT))
			return true;
		break;

	// Expressions w/ 1 Big Int
	case NODE_BI_SIGN:
	case NODE_BI_LEAST_32:
		if (stack_exp[stack_exp_idx]->data_type == DT_BIGINT)
			return true;
		break;

	// Expressions w/ 1 Big Int & String
	case NODE_BI_CONST:
		if (stack_exp[stack_exp_idx - 1]->data_type == DT_BIGINT && stack_exp[stack_exp_idx]->data_type == DT_STRING)
			return true;
		break;

	// Expressions w/ 1 Big Int & 32bit Int
	case NODE_BI_EXPR:
	case NODE_BI_POW2:
		if (stack_exp[stack_exp_idx - 1]->data_type == DT_BIGINT && stack_exp[stack_exp_idx]->data_type == DT_INT)
			return true;
		break;

	// Expressions w/ 1 Big Int & 2 - 32bit Ints
	case NODE_BI_OR_INT:
	case NODE_BI_AND_INT:
	case NODE_BI_XOR_INT:
		if (stack_exp[stack_exp_idx - 2]->data_type == DT_BIGINT && stack_exp[stack_exp_idx - 1]->data_type == DT_INT && stack_exp[stack_exp_idx]->data_type == DT_INT)
			return true;
		break;

	// Expressions w/ 2 Big Ints
	case NODE_BI_COPY:
	case NODE_BI_DIVISIBLE:
	case NODE_BI_COMP:
	case NODE_BI_COMP_ABS:
	case NODE_BI_NEG:
	case NODE_MD5:
	case NODE_SHA256:
	case NODE_SHA512:
	case NODE_RIPEMD160:
	case NODE_WHIRLPOOL:
		if (stack_exp[stack_exp_idx - 1]->data_type == DT_BIGINT && stack_exp[stack_exp_idx]->data_type == DT_BIGINT)
			return true;
		break;

	// Expressions w/ 2 Big Ints & 32 Bit Int
	case NODE_BI_POW:
	case NODE_BI_LSHIFT:
	case NODE_BI_RSHIFT:
		if (stack_exp[stack_exp_idx - 2]->data_type == DT_BIGINT && stack_exp[stack_exp_idx - 1]->data_type == DT_BIGINT && stack_exp[stack_exp_idx]->data_type == DT_INT)
			return true;
		break;

	// Expressions w/ 2 Big Ints & Bool
	case NODE_SECP192K_PTP:
	case NODE_SECP224K_PTP:
	case NODE_SECP224R_PTP:
	case NODE_SECP256K_PTP:
	case NODE_SECP384R_PTP:
	case NODE_PRM192V1_PTP:
	case NODE_PRM192V2_PTP:
	case NODE_PRM192V3_PTP:
	case NODE_PRM256V1_PTP:
		if (stack_exp[stack_exp_idx - 2]->data_type == DT_BIGINT && stack_exp[stack_exp_idx - 1]->data_type == DT_BIGINT && stack_exp[stack_exp_idx]->data_type == DT_INT )
			return true;
		break;

	// Expressions w/ Big Int - Bool - Big Int - Bool
	case NODE_SECP192K_PN:
	case NODE_SECP224K_PN:
	case NODE_SECP224R_PN:
	case NODE_SECP256K_PN:
	case NODE_SECP384R_PN:
	case NODE_PRM192V1_PN:
	case NODE_PRM192V2_PN:
	case NODE_PRM192V3_PN:
	case NODE_PRM256V1_PN:
		if (stack_exp[stack_exp_idx - 3]->data_type == DT_BIGINT && stack_exp[stack_exp_idx - 2]->data_type == DT_INT && stack_exp[stack_exp_idx - 1]->data_type == DT_BIGINT && stack_exp[stack_exp_idx]->data_type == DT_INT)
			return true;
		break;

	// Expressions w/ Big Int - Bool - Big Int - Bool - Big Int - Bool
	case NODE_SECP192K_PA:
	case NODE_SECP192K_PS:
	case NODE_SECP224K_PA:
	case NODE_SECP224K_PS:
	case NODE_SECP224R_PA:
	case NODE_SECP224R_PS:
	case NODE_SECP256K_PA:
	case NODE_SECP256K_PS:
	case NODE_SECP384R_PA:
	case NODE_SECP384R_PS:
	case NODE_PRM192V1_PA:
	case NODE_PRM192V1_PS:
	case NODE_PRM192V2_PA:
	case NODE_PRM192V2_PS:
	case NODE_PRM192V3_PA:
	case NODE_PRM192V3_PS:
	case NODE_PRM256V1_PA:
	case NODE_PRM256V1_PS:
		if (stack_exp[stack_exp_idx - 5]->data_type == DT_BIGINT && stack_exp[stack_exp_idx - 4]->data_type == DT_INT && stack_exp[stack_exp_idx - 3]->data_type == DT_BIGINT && stack_exp[stack_exp_idx - 2]->data_type == DT_INT && stack_exp[stack_exp_idx - 1]->data_type == DT_BIGINT && stack_exp[stack_exp_idx]->data_type == DT_INT)
			return true;
		break;

	// Expressions w/ Big Int - Bool - Big Int - Bool - Big Int
	case NODE_SECP192K_PSM:
	case NODE_SECP224K_PSM:
	case NODE_SECP224R_PSM:
	case NODE_SECP256K_PSM:
	case NODE_SECP384R_PSM:
	case NODE_PRM192V1_PSM:
	case NODE_PRM192V2_PSM:
	case NODE_PRM192V3_PSM:
	case NODE_PRM256V1_PSM:
		if (stack_exp[stack_exp_idx - 4]->data_type == DT_BIGINT && stack_exp[stack_exp_idx - 3]->data_type == DT_INT && stack_exp[stack_exp_idx - 2]->data_type == DT_BIGINT && stack_exp[stack_exp_idx - 1]->data_type == DT_INT && stack_exp[stack_exp_idx]->data_type == DT_BIGINT)
			return true;
		break;

	// Expressions w/ 3 Big Ints
	case NODE_BI_ADD:
	case NODE_BI_SUB:
	case NODE_BI_MUL:
	case NODE_BI_DIV:
	case NODE_BI_CEIL_DIV:
	case NODE_BI_FLOOR_DIV:
	case NODE_BI_TRUNC_DIV:
	case NODE_BI_DIV_EXACT:
	case NODE_BI_MOD:
	case NODE_BI_GCD:
	case NODE_BI_POW2_MOD_P:
	case NODE_BI_CNGR_MOD_P:
	case NODE_BI_OR:
	case NODE_BI_AND:
	case NODE_BI_XOR:
		if (stack_exp[stack_exp_idx - 2]->data_type == DT_BIGINT && stack_exp[stack_exp_idx - 1]->data_type == DT_BIGINT && stack_exp[stack_exp_idx]->data_type == DT_BIGINT)
			return true;
		break;

	// Expressions w/ 4 Big Ints
	case NODE_BI_POW_MOD_P:
		if (stack_exp[stack_exp_idx - 3]->data_type == DT_BIGINT && stack_exp[stack_exp_idx - 2]->data_type == DT_BIGINT && stack_exp[stack_exp_idx - 1]->data_type == DT_BIGINT && stack_exp[stack_exp_idx]->data_type == DT_BIGINT)
			return true;
		break;

	default:
		break;
	}

	applog(LOG_ERR, "Syntax Error - Line: %d  Invalid inputs for '%s'", token->line_num, get_node_str(node_type));
	return false;
}


static bool validate_unary_stmnt(SOURCE_TOKEN *token, NODE_TYPE node_type) {
	EXP_TYPE l_exp;

	if (node_type == NODE_BREAK)
		return true;

	l_exp = stack_exp[stack_exp_idx]->exp;

	// Validate Left Item Is Not A Statement
	if ((l_exp == EXP_STATEMENT) || (l_exp == EXP_FUNCTION)) {
		printf("Syntax Error - Line: %d  Invalid Operand: \"%s\"\n", token->line_num, get_node_str(node_type));
		return false;
	}

	return true;
}

static bool validate_binary_stmnt(SOURCE_TOKEN *token, NODE_TYPE node_type) {
	return true;
}

// Validate Unary Operations Have 1 Valid Expression
static bool validate_unary_exp(SOURCE_TOKEN *token, int token_num, NODE_TYPE node_type) {

	if (node_type == NODE_CONSTANT)
		return true;

	// Validate Increment / Decrement Have Variable For Operand
	if (token->type == TOKEN_INCREMENT || token->type == TOKEN_DECREMENT) {
		if (stack_exp[stack_exp_idx]->type != NODE_VAR_CONST && stack_exp[stack_exp_idx]->type != NODE_VAR_EXP) {
			printf("Syntax Error - Line: %d  Invalid Operand For: \"%s\"\n", token->line_num, get_node_str(node_type));
			return false;
		}
		return true;
	}

	// Validate Expression Is Not A Statement (Left For Variables, Right For Other Unary Expressions)
	if (stack_exp[stack_exp_idx]->exp != EXP_STATEMENT && stack_exp[stack_exp_idx]->exp != EXP_FUNCTION) {

		// Check Left Expression For Variables & Increment / Decrement
		if ((node_type == NODE_VAR_CONST) || (node_type == NODE_VAR_EXP)) {
				if (stack_exp[stack_exp_idx]->token_num >= token_num) {
				printf("Syntax Error - Line: %d  Invalid Operand For: \"%s\"\n", token->line_num, get_node_str(node_type));
				return false;
			}
		}
		// Check Right Expression For Other Unary Operators
		else {
			if (stack_exp[stack_exp_idx]->token_num <= token_num) {
				printf("Syntax Error - Line: %d  Invalid Operand For: \"%s\"\n", token->line_num, get_node_str(node_type));
				return false;
			}
		}
	}

	return true;
}

static bool validate_binary_exp(SOURCE_TOKEN *token, NODE_TYPE node_type) {
	DATA_TYPE l_data_type, r_data_type;

	l_data_type = stack_exp[stack_exp_idx - 1]->data_type;
	r_data_type = stack_exp[stack_exp_idx]->data_type;

	// Validate Left Item Is Not A Statement (Does Not Include Increment / Decrement)
	if (l_data_type == DT_NONE) {
		printf("Syntax Error - Line: %d  Invalid Left Operand: \"%s\"\n", token->line_num, get_node_str(node_type));
		return false;
	}

	// Validate Right Item Is Not A Statement (Does Not Include Increment / Decrement)
	if (r_data_type == DT_NONE) {
		printf("Syntax Error - Line: %d  Invalid Right Operand: \"%s\"\n", token->line_num, get_node_str(node_type));
		return false;
	}

	return true;
}

static NODE_TYPE get_node_type(SOURCE_TOKEN *token, int token_num) {
	NODE_TYPE node_type;

	switch (token->type) {
	case TOKEN_VAR_END:
		if (stack_exp_idx >= 0 && stack_exp[stack_exp_idx]->type == NODE_CONSTANT)
			node_type = NODE_VAR_CONST;
		else
			node_type = NODE_VAR_EXP;
		break;
	case TOKEN_INCREMENT:
		if (stack_exp_idx >= 0 && (stack_exp[stack_exp_idx]->token_num > token_num))
			node_type = NODE_INCREMENT_R;
		else
			node_type = NODE_INCREMENT_L;
		break;
	case TOKEN_DECREMENT:
		if (stack_exp_idx >= 0 && (stack_exp[stack_exp_idx]->token_num > token_num))
			node_type = NODE_DECREMENT_R;
		else
			node_type = NODE_DECREMENT_L;
		break;
	case TOKEN_COMPL:			node_type = NODE_COMPL;			break;
	case TOKEN_NOT:				node_type = NODE_NOT;			break;
	case TOKEN_NEG:				node_type = NODE_NEG;			break;
	case TOKEN_LITERAL:			node_type = NODE_CONSTANT;		break;
	case TOKEN_TRUE:			node_type = NODE_CONSTANT;		break;
	case TOKEN_FALSE:			node_type = NODE_CONSTANT;		break;
	case TOKEN_MUL:				node_type = NODE_MUL;			break;
	case TOKEN_DIV:				node_type = NODE_DIV;			break;
	case TOKEN_MOD:				node_type = NODE_MOD;			break;
	case TOKEN_ADD_ASSIGN:		node_type = NODE_ADD_ASSIGN;	break;
	case TOKEN_SUB_ASSIGN:		node_type = NODE_SUB_ASSIGN;	break;
	case TOKEN_MUL_ASSIGN:		node_type = NODE_MUL_ASSIGN;	break;
	case TOKEN_DIV_ASSIGN:		node_type = NODE_DIV_ASSIGN;	break;
	case TOKEN_MOD_ASSIGN:		node_type = NODE_MOD_ASSIGN;	break;
	case TOKEN_LSHFT_ASSIGN:	node_type = NODE_LSHFT_ASSIGN;	break;
	case TOKEN_RSHFT_ASSIGN:	node_type = NODE_RSHFT_ASSIGN;	break;
	case TOKEN_AND_ASSIGN:		node_type = NODE_AND_ASSIGN;	break;
	case TOKEN_XOR_ASSIGN:		node_type = NODE_XOR_ASSIGN;	break;
	case TOKEN_OR_ASSIGN:		node_type = NODE_OR_ASSIGN;		break;
	case TOKEN_ADD:				node_type = NODE_ADD;			break;
	case TOKEN_SUB:				node_type = NODE_SUB;			break;
	case TOKEN_LROT:			node_type = NODE_LROT;			break;
	case TOKEN_LSHIFT:			node_type = NODE_LSHIFT;		break;
	case TOKEN_RROT:			node_type = NODE_RROT;			break;
	case TOKEN_RSHIFT:			node_type = NODE_RSHIFT;		break;
	case TOKEN_LE:				node_type = NODE_LE;			break;
	case TOKEN_GE:				node_type = NODE_GE;			break;
	case TOKEN_LT:				node_type = NODE_LT;			break;
	case TOKEN_GT:				node_type = NODE_GT;			break;
	case TOKEN_EQ:				node_type = NODE_EQ;			break;
	case TOKEN_NE:				node_type = NODE_NE;			break;
	case TOKEN_BITWISE_AND:		node_type = NODE_BITWISE_AND;	break;
	case TOKEN_BITWISE_XOR:		node_type = NODE_BITWISE_XOR;	break;
	case TOKEN_BITWISE_OR:		node_type = NODE_BITWISE_OR;	break;
	case TOKEN_AND:				node_type = NODE_AND;			break;
	case TOKEN_OR:				node_type = NODE_OR;			break;
	case TOKEN_BLOCK_END:		node_type = NODE_BLOCK;			break;
	case TOKEN_CONDITIONAL:		node_type = NODE_CONDITIONAL;	break;
	case TOKEN_COND_ELSE:		node_type = NODE_COND_ELSE;		break;
	case TOKEN_IF:				node_type = NODE_IF;			break;
	case TOKEN_ELSE:			node_type = NODE_ELSE;			break;
	case TOKEN_REPEAT:			node_type = NODE_REPEAT;		break;
	case TOKEN_BREAK:			node_type = NODE_BREAK;			break;
	case TOKEN_CONTINUE:		node_type = NODE_CONTINUE;		break;
	case TOKEN_ASSIGN:			node_type = NODE_ASSIGN;		break;
	case TOKEN_VERIFY:			node_type = NODE_VERIFY;		break;
	case TOKEN_SIN:				node_type = NODE_SIN;			break;
	case TOKEN_COS:				node_type = NODE_COS; 			break;
	case TOKEN_TAN:				node_type = NODE_TAN; 			break;
	case TOKEN_SINH:			node_type = NODE_SINH;			break;
	case TOKEN_COSH:			node_type = NODE_COSH;			break;
	case TOKEN_TANH:			node_type = NODE_TANH;			break;
	case TOKEN_ASIN:			node_type = NODE_ASIN;			break;
	case TOKEN_ACOS:			node_type = NODE_ACOS;			break;
	case TOKEN_ATAN:			node_type = NODE_ATAN;			break;
	case TOKEN_ATAN2:			node_type = NODE_ATAN2;			break;
	case TOKEN_EXPNT:			node_type = NODE_EXPNT;			break;
	case TOKEN_LOG:				node_type = NODE_LOG;			break;
	case TOKEN_LOG10:			node_type = NODE_LOG10;			break;
	case TOKEN_POW:				node_type = NODE_POW;			break;
	case TOKEN_SQRT:			node_type = NODE_SQRT;			break;
	case TOKEN_CEIL:			node_type = NODE_CEIL;			break;
	case TOKEN_FLOOR:			node_type = NODE_FLOOR;			break;
	case TOKEN_ABS:				node_type = NODE_ABS;			break;
	case TOKEN_FABS:			node_type = NODE_FABS;			break;
	case TOKEN_FMOD:			node_type = NODE_FMOD; 			break;
	case TOKEN_GCD:				node_type = NODE_GCD; 			break;
	case TOKEN_BI_CONST:		node_type = NODE_BI_CONST;		break;
	case TOKEN_BI_EXPR:			node_type = NODE_BI_EXPR;		break;
	case TOKEN_BI_COPY:			node_type = NODE_BI_COPY;		break;
	case TOKEN_BI_ADD:			node_type = NODE_BI_ADD;		break;
	case TOKEN_BI_SUB:			node_type = NODE_BI_SUB;		break;
	case TOKEN_BI_MUL:			node_type = NODE_BI_MUL;		break;
	case TOKEN_BI_DIV:			node_type = NODE_BI_DIV;		break;
	case TOKEN_BI_CEIL_DIV:		node_type = NODE_BI_CEIL_DIV;	break;
	case TOKEN_BI_FLOOR_DIV:	node_type = NODE_BI_FLOOR_DIV;	break;
	case TOKEN_BI_TRUNC_DIV:	node_type = NODE_BI_TRUNC_DIV;	break;
	case TOKEN_BI_DIV_EXACT:	node_type = NODE_BI_DIV_EXACT;	break;
	case TOKEN_BI_MOD:			node_type = NODE_BI_MOD;		break;
	case TOKEN_BI_NEG:			node_type = NODE_BI_NEG;		break;
	case TOKEN_BI_LSHIFT:		node_type = NODE_BI_LSHIFT;		break;
	case TOKEN_BI_RSHIFT:		node_type = NODE_BI_RSHIFT;		break;
	case TOKEN_BI_GCD:			node_type = NODE_BI_GCD;		break;
	case TOKEN_BI_DIVISIBLE:	node_type = NODE_BI_DIVISIBLE;	break;
	case TOKEN_BI_CNGR_MOD_P:	node_type = NODE_BI_CNGR_MOD_P;	break;
	case TOKEN_BI_POW:			node_type = NODE_BI_POW;		break;
	case TOKEN_BI_POW2:			node_type = NODE_BI_POW2;		break;
	case TOKEN_BI_POW_MOD_P:	node_type = NODE_BI_POW_MOD_P;	break;
	case TOKEN_BI_POW2_MOD_P:	node_type = NODE_BI_POW2_MOD_P;	break;
	case TOKEN_BI_COMP:			node_type = NODE_BI_COMP;		break;
	case TOKEN_BI_COMP_ABS:		node_type = NODE_BI_COMP_ABS;	break;
	case TOKEN_BI_SIGN:			node_type = NODE_BI_SIGN;		break;
	case TOKEN_BI_OR:			node_type = NODE_BI_OR;			break;
	case TOKEN_BI_AND:			node_type = NODE_BI_AND;		break;
	case TOKEN_BI_XOR:			node_type = NODE_BI_XOR;		break;
	case TOKEN_BI_OR_INT:		node_type = NODE_BI_OR_INT;		break;
	case TOKEN_BI_AND_INT:		node_type = NODE_BI_AND_INT;	break;
	case TOKEN_BI_XOR_INT:		node_type = NODE_BI_XOR_INT;	break;
	case TOKEN_BI_LEAST_32:		node_type = NODE_BI_LEAST_32;	break;
	case TOKEN_SHA256:			node_type = NODE_SHA256;		break;
	case TOKEN_SHA512:			node_type = NODE_SHA512;		break;
	case TOKEN_WHIRLPOOL:	   	node_type = NODE_WHIRLPOOL;		break;
	case TOKEN_MD5:	           	node_type = NODE_MD5;			break;
	case TOKEN_SECP192K_PTP:	node_type = NODE_SECP192K_PTP;	break;
	case TOKEN_SECP192K_PA:	   	node_type = NODE_SECP192K_PA;	break;
	case TOKEN_SECP192K_PS:	   	node_type = NODE_SECP192K_PS;	break;
	case TOKEN_SECP192K_PSM:	node_type = NODE_SECP192K_PSM;	break;
	case TOKEN_SECP192K_PN:	    node_type = NODE_SECP192K_PN;	break;
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
	case TOKEN_RIPEMD160:	    node_type = NODE_RIPEMD160;		break;
	default: return NODE_ERROR;
	}

	return node_type;
}

static bool create_exp(SOURCE_TOKEN *token, int token_num) {
	int i;
	long long value = 0;
	double fvalue = 0.0;
	unsigned char *svalue = NULL;
	NODE_TYPE node_type = NODE_ERROR;
	ast *exp, *left = NULL, *right = NULL;

	node_type = get_node_type(token, token_num);
	
	// Map Token To Node Type
	if (node_type == NODE_ERROR) {
		applog(LOG_ERR, "Unknown Token in ElasticPL Source.  Line: %d, Token Type: %d", token->line_num, token->type);
		return false;
	}

	// Confirm Required Number Of Expressions Are On Stack
	if (!validate_input_num(token, node_type))
		return false;

	switch (token->exp) {

	case EXP_EXPRESSION:

		// Constant Expressions
		if (token->inputs == 0) {

			if (token->type == TOKEN_TRUE)
				value = 1;
			else if (token->type == TOKEN_FALSE)
				value = 0;
			else if (node_type == NODE_CONSTANT) {

				if (token->data_type == DT_INT) {

					//// Check For Hex - If Found, Convert To Int
					//if (token->literal[0] == '0' && token->literal[1] == 'x' && strlen(token->literal) > 2 && strlen(token->literal) <= 34) {
					//	hex2ints(bvalue, 4, token->literal + 2, strlen(token->literal) - 2);
					//	sprintf(token->literal, "%d", bvalue[3]);
					//}

					//// Check For Binary - If Found, Convert To Decimal String
					//if (token->literal[0] == '0' && token->literal[1] == 'b' && strlen(token->literal) > 2 && strlen(token->literal) <= 34) {
					//	bvalue[0] = bin2int(token->literal + 2);
					//	sprintf(token->literal, "%d", bvalue[0]);
					//}

					//if (strlen(token->literal) <= 10) {
						value = (long long)strtod(token->literal, NULL);
						fvalue = (double)strtod(token->literal, NULL);
//					}
				}
				else if (token->data_type == DT_FLOAT) {
//					if (strlen(token->literal) <= 10) {
						value = (long long)strtod(token->literal, NULL);
						fvalue = (double)strtod(token->literal, NULL);
//					}
				}
				else {
					svalue = calloc(1, strlen(token->literal) + 1);
					if (!svalue)
						return false;
					strcpy(svalue, token->literal);
				}
			}
		}
		// Unary Expressions
		else if (token->inputs == 1) {

			if (!validate_unary_exp(token, token_num, node_type))
				return false;

			left = pop_exp();

			// Remove Expression For Variables w/ Constant ID
			if (node_type == NODE_VAR_CONST) {
				value = left->value;
				fvalue = left->fvalue;
				left = NULL;
			}
		}
		// Binary Expressions
		else if (token->inputs == 2) {

			if (!validate_binary_exp(token, node_type))
				return false;

			right = pop_exp();
			left = pop_exp();
		}
		
		break;

	case EXP_STATEMENT:

		// Unary Statements
		if (token->inputs == 1) {

			if (!validate_unary_stmnt(token, node_type))
				return false;

			left = pop_exp();
		}
		// Binary Statements
		else if (token->inputs == 2) {

			if (!validate_binary_stmnt(token, node_type))
				return false;

			if (node_type == NODE_BLOCK && stack_exp[stack_exp_idx]->type != NODE_BLOCK)
				right = NULL;
			else
				right = pop_exp();
			left = pop_exp();

		}
		break;

	case EXP_FUNCTION:

		if (token->inputs > 0) {
			// First Paramater
			left = pop_exp();
			exp = add_exp(NODE_PARAM, EXP_EXPRESSION, 0, 0.0, NULL, 0, 0, DT_NONE, left, NULL);
			push_exp(exp);

			// Remaining Paramaters
			for (i = 1; i < token->inputs; i++) {

				if ((stack_exp_idx <= 0) || (stack_exp[stack_exp_idx - 1]->end_stmnt == true) || (stack_exp[stack_exp_idx - 1]->type == NODE_IF) || (stack_exp[stack_exp_idx - 1]->type == NODE_REPEAT))
					break;

				right = pop_exp();
				left = pop_exp();
				exp = add_exp(NODE_PARAM, EXP_EXPRESSION, 0, 0.0, NULL, 0, 0, DT_NONE, left, right);
				push_exp(exp);
			}
			left = NULL;
			right = pop_exp();
		}
		else {
			left = NULL;
			right = NULL;
		}
	}

	exp = add_exp(node_type, token->exp, (int32_t)value, fvalue, svalue, token_num, token->line_num, token->data_type, left, right);

	if (exp)
		push_exp(exp);
	else
		return false;

	return true;
}

static bool validate_exp_list() {
	int i;

	if (stack_exp_idx < 0) {
		applog(LOG_ERR, "Syntax Error - Invalid Source File");
		return false;
	}

	for (i = 0; i < stack_exp_idx; i++) {
		if (stack_exp[i]->type == NODE_VERIFY) {
			applog(LOG_ERR, "Syntax Error - Line: %d Invalid Verify Statement", stack_exp[i]->line_num);
			return false;
		}
	}

	if (stack_exp[stack_exp_idx]->type != NODE_VERIFY) {
		applog(LOG_ERR, "Syntax Error - Missing Verify Statement");
		return false;
	}

	for (i = 0; i < stack_exp_idx; i++) {
		if (stack_exp[i]->exp != EXP_STATEMENT && stack_exp[i]->exp != EXP_FUNCTION && stack_exp[i]->end_stmnt == false) {
				applog(LOG_ERR, "Syntax Error - Line: %d  Invalid Statement", stack_exp[i]->line_num);
			return false;
		}
	}

	if (stack_op_idx >= 0) {
		applog(LOG_ERR, "Syntax Error - Unable To Clear Operator Stack");
		return false;
	}

	return true;
}

extern bool parse_token_list(SOURCE_TOKEN_LIST *token_list) {

	int i, token_id;
	ast *left, *right;

	for (i = 0; i < token_list->num; i++) {

		// Validate That No Verify Statements Are Embeded In Blocks
		if ((stack_exp_idx >= 0) && (stack_exp[stack_exp_idx]->type == NODE_VERIFY)) {
			printf("Syntax Error - Line: %d  Invalid Verify Statement\n", stack_exp[stack_exp_idx]->line_num);
			return false;
		}

		switch (token_list->token[i].type) {

		case TOKEN_COMMA:
			continue;
			break;
	
		case TOKEN_LITERAL:
		case TOKEN_TRUE:
		case TOKEN_FALSE:
			if (!create_exp(&token_list->token[i], i)) return false;
			stack_exp[stack_exp_idx]->data_type = token_list->token[i].data_type;
			break;

		case TOKEN_VAR_BEGIN:
		case TOKEN_OPEN_PAREN:
		case TOKEN_BLOCK_BEGIN:
		case TOKEN_CONDITIONAL:
			push_op(i);
			break;

		case TOKEN_END_STATEMENT:
			// Process Expressions
			while ((top_op >= 0) && (token_list->token[top_op].type != TOKEN_BLOCK_BEGIN) && (token_list->token[top_op].type != TOKEN_IF) && (token_list->token[top_op].type != TOKEN_ELSE) && (token_list->token[top_op].type != TOKEN_REPEAT)) {
					token_id = pop_op();
				if (!create_exp(&token_list->token[token_id], token_id))
					return false;
			}
			stack_exp[stack_exp_idx]->end_stmnt = true;
			break;

		case TOKEN_VAR_END:
			// Process Expressions Within The Variable Brackets
			while ((top_op >= 0) && (token_list->token[top_op].type != TOKEN_VAR_BEGIN)) {
				token_id = pop_op();
				if (!create_exp(&token_list->token[token_id], token_id)) return false;
			}

			if ((stack_exp_idx < 0) || stack_exp[stack_exp_idx]->token_num < top_op) {
				printf("Syntax Error - Line: %d  Missing variable index\n", token_list->token[i].line_num);
				return false;
			}

			// Set TOKEN_VAR_END To Match Data Type 
			token_list->token[i].data_type = token_list->token[stack_op[stack_op_idx]].data_type;

			pop_op();
			if (!create_exp(&token_list->token[i], i)) return false;

			// Check For Unary Operators On The Variable
			while ((top_op >= 0) && (token_list->token[top_op].type != TOKEN_VAR_BEGIN) && (token_list->token[top_op].exp == EXP_EXPRESSION) && (token_list->token[top_op].inputs <= 1)) {
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

			// Check If We Need To Link What's In Parentheses To A Function
			if ((top_op >= 0) && (token_list->token[top_op].exp == EXP_FUNCTION)) {
				token_id = pop_op();
				if (!create_exp(&token_list->token[token_id], token_id))
					return false;
			}

			break;

		case TOKEN_BLOCK_END:
			// Validate That The Top Operator Is The Block Begin
			if (token_list->token[top_op].type != TOKEN_BLOCK_BEGIN)
				return false;

			// Create A Linked List Of All Statements In The Block
			while (stack_exp_idx > 0 && stack_exp[stack_exp_idx - 1]->token_num > top_op && stack_exp[stack_exp_idx]->token_num < i) {
					if (!create_exp(&token_list->token[i], top_op)) return false;
					stack_exp[stack_exp_idx]->end_stmnt = true;
			}
			pop_op();

			// Link Block To If/Repeat Operator
			while ((top_op >= 0) && (token_list->token[top_op].type == TOKEN_IF || token_list->token[top_op].type == TOKEN_ELSE || token_list->token[top_op].type == TOKEN_REPEAT)) {
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

		case TOKEN_COND_ELSE:

			// Process Expressions Within The Conditional Statement
			while ((top_op >= 0) && (token_list->token[top_op].type != TOKEN_CONDITIONAL)) {
				token_id = pop_op();
				if (!create_exp(&token_list->token[token_id], token_id)) return false;
			}

			if (stack_op_idx < 0 || token_list->token[stack_op[stack_op_idx]].type != TOKEN_CONDITIONAL) {
				applog(LOG_ERR, "Syntax Error - Line: %d  Invalid 'Conditional' Statement\n", token_list->token[token_id].line_num);
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

			// Don't Push <EOF> Onto Stack
			if (token_list->token[i].type != TOKEN_EOF)
					push_op(i);

			break;
		}
	}

	if (stack_exp_idx < 0 || !validate_exp_list())
		return false;

	return true;
}