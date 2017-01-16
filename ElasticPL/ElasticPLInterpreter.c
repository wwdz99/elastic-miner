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
#include <math.h>

#include "ElasticPL.h"
#include "ElasticPLFunctions.h"
#include "../miner.h"

char blk_new[4096];
char blk_old[4096];

uint32_t wcet_block;

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
		case NODE_INCREMENT_R:
		case NODE_INCREMENT_L:
		case NODE_ADD:
		case NODE_DECREMENT_R:
		case NODE_DECREMENT_L:
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
		default:
			break;
		}
	}

	return wcet;
}

static double expnt(double X) {
	return exp(X);
}

extern int interpret_ast() {
	int i;

	vm_bounty = false;
	vm_break = false;
	vm_continue = false;

	for (i = 0; i < vm_ast_cnt; i++) {
		if (!interpret(vm_ast[i]))
			return 0;
	}

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

static double interpret(ast* exp) {
	double lfval, rfval;
	int32_t lval, rval;

	if (exp == NULL)
		return 0;

	if (vm_break || vm_continue)
		return 1;

	switch (exp->type) {
		case NODE_CONSTANT:
			if (exp->data_type == DT_FLOAT)
				return exp->fvalue;
			else
				return exp->value;
		case NODE_VAR_CONST:
			if (exp->value < 0 || exp->value > VM_MEMORY_SIZE)
				return 0;
			if (exp->data_type == DT_FLOAT)
				return vm_f[exp->value];
			return vm_m[exp->value];
		case NODE_VAR_EXP:
			lval = (int32_t)interpret(exp->left);
			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;
			return lval;
		case NODE_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left);
			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rfval = interpret(exp->right);
			if (exp->left->is_float)
				vm_f[lval] = rfval;
			else
				vm_m[lval] = (int32_t)rfval;

			mangle_state(lval);
			mangle_state((int32_t)rfval);
			return 1;
		case NODE_IF:
			if (exp->right->type != NODE_ELSE) {
				if (interpret(exp->left))
					interpret(exp->right);					// If Body (No Else Condition)
			}
			else {
				if (interpret(exp->left))
					interpret(exp->right->left);			// If Body
				else
					interpret(exp->right->right);			// Else Body
			}
			return 1;
		case NODE_CONDITIONAL:
			if (interpret(exp->left))
				return interpret(exp->right->left);
			else
				return interpret(exp->right->right);
		case NODE_REPEAT:
			vm_break = false;
			lval = (int32_t)interpret(exp->left);
			if (lval > 0) {
				int i;
				for (i = 0; i < lval; i++) {
					vm_continue = false;
					if (vm_break)
						break;
					else if (vm_continue)
						continue;
					else
						interpret(exp->right);					// Repeat Body
				}
			}
			vm_break = false;
			vm_continue = false;
			return 1;
		case NODE_BLOCK:
			interpret(exp->left);
			if (exp->right)
				interpret(exp->right);
			return 1;
		case NODE_BREAK:
			vm_break = true;
			break;
		case NODE_CONTINUE:
			vm_continue = true;
			break;
		case NODE_ADD_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rfval = interpret(exp->right);

			if (exp->left->is_float)
				vm_f[lval] += rfval;
			else 
				vm_m[lval] += (int32_t)rfval;
			mangle_state(lval);
			mangle_state((int32_t)rfval);
			return 1;
		case NODE_SUB_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rfval = interpret(exp->right);

			if (exp->left->is_float)
				vm_f[lval] -= rfval;
			else
				vm_m[lval] -= (int32_t)rfval;
			mangle_state(lval);
			mangle_state((int32_t)rfval);
			return 1;
		case NODE_MUL_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rfval = interpret(exp->right);

			if (exp->left->is_float)
				vm_f[lval] *= rfval;
			else
				vm_m[lval] *= (int32_t)rfval;
			mangle_state(lval);
			mangle_state((int32_t)rfval);
			return 1;
		case NODE_DIV_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rfval = interpret(exp->right);
			if (rfval == 0.0)
				return 0;

			if (exp->left->is_float)
				vm_f[lval] /= rfval;
			else
				vm_m[lval] = (int32_t)(vm_m[lval] / rfval);
			mangle_state(lval);
			mangle_state((int32_t)rfval);
			return 1;
		case NODE_MOD_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rval = (int32_t)interpret(exp->right);
			if (rval == 0)
				return 0;

			if (exp->left->is_float)
				vm_f[lval] = (double)((int32_t)vm_f[lval] % rval);
			else
				vm_m[lval] %= rval;
			mangle_state(lval);
			mangle_state((int32_t)rval);
			return 1;
		case NODE_LSHFT_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rval = (int32_t)interpret(exp->right);
			if (exp->left->is_float)
				vm_f[lval] = (double)((int32_t)vm_f[lval] << rval);
			else
				vm_m[lval] <<= rval;
			mangle_state(lval);
			mangle_state((int32_t)rval);
			return 1;
		case NODE_RSHFT_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rval = (int32_t)interpret(exp->right);
			if (exp->left->is_float)
				vm_f[lval] = (double)((int32_t)vm_f[lval] >> rval);
			else
				vm_m[lval] >>= rval;
			mangle_state(lval);
			mangle_state((int32_t)rval);
			return 1;
		case NODE_AND_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rval = (int32_t)interpret(exp->right);
			if (exp->left->is_float)
				vm_f[lval] = (double)((int32_t)vm_f[lval] & rval);
			else
				vm_m[lval] &= rval;
			mangle_state(lval);
			mangle_state((int32_t)rval);
			return 1;
		case NODE_XOR_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rval = (int32_t)interpret(exp->right);
			if (exp->left->is_float)
				vm_f[lval] = (double)((int32_t)vm_f[lval] ^ rval);
			else
				vm_m[lval] ^= rval;
			mangle_state(lval);
			mangle_state((int32_t)rval);
			return 1;
		case NODE_OR_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rval = (int32_t)interpret(exp->right);
			if (exp->left->is_float)
				vm_f[lval] = (double)((int32_t)vm_f[lval] | rval);
			else
				vm_m[lval] |= rval;
			mangle_state(lval);
			mangle_state((int32_t)rval);
			return 1;
		case NODE_INCREMENT_R:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			if (exp->left->is_float) {
				++vm_f[lval];
				return ((exp->end_stmnt) ? 1 : vm_f[lval]);
			}
			else {
				++vm_m[lval];
				return ((exp->end_stmnt) ? 1 : vm_m[lval]);
			}
		case NODE_INCREMENT_L:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			if (exp->left->is_float) {
				rfval = vm_f[lval]++;
				return ((exp->end_stmnt) ? 1 : rfval);
			}
			else {
				rval = vm_m[lval]++;
				return ((exp->end_stmnt) ? 1 : rval);
			}
		case NODE_DECREMENT_R:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			if (exp->left->is_float) {
				--vm_f[lval];
				return ((exp->end_stmnt) ? 1 : vm_f[lval]);
			}
			else {
				--vm_m[lval];
				return ((exp->end_stmnt) ? 1 : vm_m[lval]);
			}
		case NODE_DECREMENT_L:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			if (exp->left->is_float) {
				rfval = vm_f[lval]--;
				return ((exp->end_stmnt) ? 1 : rfval);
			}
			else {
				rval = vm_m[lval]--;
				return ((exp->end_stmnt) ? 1 : rval);
			}
		case NODE_ADD:
			lfval = interpret(exp->left);
			rfval = interpret(exp->right);
			return (lfval + rfval);
		case NODE_SUB:
			lfval = interpret(exp->left);
			rfval = interpret(exp->right);
			return (lfval - rfval);
		case NODE_MUL:
			lfval = interpret(exp->left);
			rfval = interpret(exp->right);
			return (lfval * rfval);
		case NODE_DIV:
			lfval = interpret(exp->left);
			rfval = interpret(exp->right);
			if (rfval != 0.0)
				return (lfval / rfval);
			else
				return 0;
		case NODE_MOD:
			lval = (int32_t)interpret(exp->left);
			rval = (int32_t)interpret(exp->right);
			if (rval > 0)
				return (lval % rval);
			else
				return 0;
		case NODE_LSHIFT:
			lval = (int32_t)interpret(exp->left);
			rval = (int32_t)interpret(exp->right);
			return (lval << rval);
		case NODE_LROT:
			lval = (int32_t)interpret(exp->left);
			rval = (int32_t)interpret(exp->right);
			return rotl32(lval, rval % 32);
		case NODE_RSHIFT:
			lval = (int32_t)interpret(exp->left);
			rval = (int32_t)interpret(exp->right);
			return (lval >> rval);
		case NODE_RROT:
			lval = (int32_t)interpret(exp->left);
			rval = (int32_t)interpret(exp->right);
			return rotr32(lval, rval % 32);
		case NODE_NOT:
			lfval = interpret(exp->left);
			return !lfval;
		case NODE_COMPL:
			lval = (int32_t)interpret(exp->left);
			return ~lval;
		case NODE_AND:
			lfval = interpret(exp->left);
			rfval = interpret(exp->right);
			return (lfval && rfval);
		case NODE_OR:
			lfval = interpret(exp->left);
			rfval = interpret(exp->right);
			return (lfval || rfval);
		case NODE_BITWISE_AND:
			lval = (int32_t)interpret(exp->left);
			rval = (int32_t)interpret(exp->right);
			return (lval & rval);
		case NODE_BITWISE_XOR:
			lval = (int32_t)interpret(exp->left);
			rval = (int32_t)interpret(exp->right);
			return (lval ^ rval);
		case NODE_BITWISE_OR:
			lval = (int32_t)interpret(exp->left);
			rval = (int32_t)interpret(exp->right);
			return (lval | rval);
		case NODE_EQ:
			lfval = interpret(exp->left);
			rfval = interpret(exp->right);
			return (lfval == rfval);
		case NODE_NE:
			lfval = interpret(exp->left);
			rfval = interpret(exp->right);
			return (lfval != rfval);
		case NODE_GT:
			lfval = interpret(exp->left);
			rfval = interpret(exp->right);
			return (lfval > rfval);
		case NODE_LT:
			lfval = interpret(exp->left);
			rfval = interpret(exp->right);
			return (lfval < rfval);
		case NODE_GE:
			lfval = interpret(exp->left);
			rfval = interpret(exp->right);
			return (lfval >= rfval);
		case NODE_LE:
			lfval = interpret(exp->left);
			rfval = interpret(exp->right);
			return (lfval <= rfval);
		case NODE_NEG:
			lfval = interpret(exp->left);
			return -lfval;
		case NODE_PARAM:
			vm_param_val[vm_param_num] = interpret(exp->left);
			vm_param_idx[vm_param_num++] = exp->left->value;
			rval = (int32_t)interpret(exp->right);
			return 1;
		case NODE_SIN:
			vm_param_num = 0;
			interpret(exp->right);
			return sin(vm_param_val[0]);
		case NODE_COS:
			vm_param_num = 0;
			interpret(exp->right);
			return cos(vm_param_val[0]);
		case NODE_TAN:
			vm_param_num = 0;
			interpret(exp->right);
			return tan(vm_param_val[0]);
		case NODE_SINH:
			vm_param_num = 0;
			interpret(exp->right);
			return sinh(vm_param_val[0]);
		case NODE_COSH:
			vm_param_num = 0;
			interpret(exp->right);
			return cosh(vm_param_val[0]);
		case NODE_TANH:
			vm_param_num = 0;
			interpret(exp->right);
			return tanh(vm_param_val[0]);
		case NODE_ASIN:
			vm_param_num = 0;
			interpret(exp->right);
			return asin(vm_param_val[0]);
		case NODE_ACOS:
			vm_param_num = 0;
			interpret(exp->right);
			return acos(vm_param_val[0]);
		case NODE_ATAN:
			vm_param_num = 0;
			interpret(exp->right);
			return atan(vm_param_val[0]);
		case NODE_ATAN2:
			vm_param_num = 0;
			interpret(exp->right);
			if (vm_param_val[1] == 0.0)
				return 0;
			return atan2(vm_param_val[0], vm_param_val[1]);
		case NODE_EXPNT:
			vm_param_num = 0;
			interpret(exp->right);
			if ((vm_param_val[0] < -708.0) || (vm_param_val[0] > 709.0))
				return 0;
			return expnt(vm_param_val[0]);
		case NODE_LOG:
			vm_param_num = 0;
			interpret(exp->right);
			if (vm_param_val[0] <= 0.0)
				return 0;
			return log(vm_param_val[0]);
		case NODE_LOG10:
			vm_param_num = 0;
			interpret(exp->right);
			if (vm_param_val[0] <= 0.0)
				return 0;
			return log10(vm_param_val[0]);
		case NODE_POW:
			vm_param_num = 0;
			interpret(exp->right);
			return pow(vm_param_val[0], vm_param_val[0]);
		case NODE_SQRT:
			vm_param_num = 0;
			interpret(exp->right);
			if (vm_param_val[0] <= 0.0)
				return 0;
			return sqrt(vm_param_val[0]);
		case NODE_CEIL:
			vm_param_num = 0;
			interpret(exp->right);
			return ceil(vm_param_val[0]);
		case NODE_FLOOR:
			vm_param_num = 0;
			interpret(exp->right);
			return floor(vm_param_val[0]);
		case NODE_ABS:
			vm_param_num = 0;
			interpret(exp->right);
			return abs((int32_t)vm_param_val[0]);
		case NODE_FABS:
			vm_param_num = 0;
			interpret(exp->right);
			return fabs(vm_param_val[0]);
		case NODE_FMOD:
			vm_param_num = 0;
			interpret(exp->right);
			if (vm_param_val[1] == 0.0)
				return 0;
			return fmod(vm_param_val[0], vm_param_val[1]);
		case NODE_GCD:
			vm_param_num = 0;
			interpret(exp->right);
			return gcd((int32_t)vm_param_val[0], (int32_t)vm_param_val[1]);
		case NODE_BI_CONST:
			vm_param_num = 0;
			interpret(exp->right);

			if (!exp->right->right->left->svalue)
				return 0;

			mangle_state(big_init_const(vm_b[vm_param_idx[0]], exp->right->right->left->svalue, vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_EXPR:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_init_expr(vm_b[vm_param_idx[0]], (int32_t)vm_param_val[1], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_COPY:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_copy(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_ADD:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_add(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b[vm_param_idx[2]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_SUB:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_sub(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b[vm_param_idx[2]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_MUL:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_mul(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b[vm_param_idx[2]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_DIV:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_div(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b[vm_param_idx[2]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_CEIL_DIV:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_ceil_div(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b[vm_param_idx[2]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_FLOOR_DIV:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_floor_div(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b[vm_param_idx[2]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_TRUNC_DIV:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_truncate_div(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b[vm_param_idx[2]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_DIV_EXACT:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_div_exact(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b[vm_param_idx[2]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_MOD:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_mod(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b[vm_param_idx[2]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_NEG:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_neg(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_LSHIFT:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_lshift(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], (uint32_t)vm_param_val[2], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_RSHIFT:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_rshift(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], (uint32_t)vm_param_val[2], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_GCD:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_gcd(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b[vm_param_idx[2]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_DIVISIBLE:
			vm_param_num = 0;
			interpret(exp->right);
			return big_divisible(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b);
		case NODE_BI_CNGR_MOD_P:
			vm_param_num = 0;
			interpret(exp->right);
			return big_congruent_mod_p(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b[vm_param_idx[2]], vm_b);
		case NODE_BI_POW:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_pow(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], (uint32_t)vm_param_val[2], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_POW2:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_pow2(vm_b[vm_param_idx[0]], (uint32_t)vm_param_val[1], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_POW_MOD_P:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_pow_mod_p(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b[vm_param_idx[2]], vm_b[vm_param_idx[3]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_POW2_MOD_P:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_pow2_mod_p(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b[vm_param_idx[2]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_COMP:
			vm_param_num = 0;
			interpret(exp->right);
			return big_compare(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b);
		case NODE_BI_COMP_ABS:
			vm_param_num = 0;
			interpret(exp->right);
			return big_compare_abs(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b);
		case NODE_BI_SIGN:
			vm_param_num = 0;
			interpret(exp->right);
			return big_sign(vm_b[vm_param_idx[0]], vm_b);
		case NODE_BI_OR:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_or(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b[vm_param_idx[2]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_AND:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_and(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b[vm_param_idx[2]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_XOR:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_xor(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], vm_b[vm_param_idx[2]], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_OR_INT:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_or_integer(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], (uint32_t)vm_param_val[2], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_AND_INT:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_and_integer(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], (uint32_t)vm_param_val[2], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_XOR_INT:
			vm_param_num = 0;
			interpret(exp->right);
			mangle_state(big_xor_integer(vm_b[vm_param_idx[0]], vm_b[vm_param_idx[1]], (uint32_t)vm_param_val[2], vm_b, &vm_bi_size));
			return 1;
		case NODE_BI_LEAST_32:
			vm_param_num = 0;
			interpret(exp->right);
			return big_least_32bit(vm_b[vm_param_idx[0]], vm_b);
		case NODE_VERIFY:
			lval = (int32_t)interpret(exp->left);
			vm_bounty = (lval != 0);
			return vm_bounty;
		default:
			applog(LOG_ERR, "ERROR: VM Runtime - Unsupported Operation (%d)", exp->type);
			return 0;
	}

	return 1;
}
