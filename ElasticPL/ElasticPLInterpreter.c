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

uint32_t wcet_block;

static uint32_t calc_weight(ast* root) {
	uint32_t weight, total_weight = 0;
	uint32_t block_weight[100];
	int block_level = -1;
	bool downward = true;
	ast *new_ptr = NULL;
	ast *old_ptr = NULL;

	if (!root)
		return 0;

	new_ptr = root;

	while (new_ptr) {
		old_ptr = new_ptr;

		// Navigate Down The Tree
		if (downward) {
			// Navigate To Lowest Left Parent Node
			while (new_ptr->left) {
				if (!new_ptr->left->left)
					break;
				new_ptr = new_ptr->left;
			}

			// Get weight Of Left Node
			if (new_ptr->left)
				weight = get_weight(new_ptr->left);

			// Check For "Repeat" Blocks
			if (new_ptr->type == NODE_REPEAT) {
				total_weight += weight;
				block_level++;
				if (block_level >= 100)
					return 0;
				block_weight[block_level] = 0;
			}

			// Switch To Right Node
			if (new_ptr->right) {
				new_ptr = new_ptr->right;
			}
			else {
				// Print Right Node & Navigate Back Up The Tree
				if (old_ptr != root) {
					weight = get_weight(new_ptr);
					new_ptr = old_ptr->parent;
				}
				downward = false;
			}
		}

		// Navigate Back Up The Tree
		else {
			if (new_ptr == root)
				break;

			// Get weight Of Parent Node
			weight = get_weight(new_ptr);

			// Check If We Need To Navigate Back Down A Right Branch
			if ((new_ptr == new_ptr->parent->left) && (new_ptr->parent->right)) {
				new_ptr = new_ptr->parent->right;
				downward = true;
			}
			else {
				new_ptr = old_ptr->parent;
			}
		}

		if (block_level >= 0)
			block_weight[block_level] += weight;
		else
			total_weight += (total_weight < (0xFFFFFFFF - weight) ? weight : 0);

		// Get Total weight For The "Repeat" Block
		if ((block_level >= 0) && (new_ptr->type == NODE_REPEAT)) {
			if (block_level == 0)
				total_weight += (((new_ptr->left->value > 0) ? new_ptr->left->value : 0) * block_weight[block_level]);
			else
				block_weight[block_level - 1] += (((new_ptr->left->value > 0) ? new_ptr->left->value : 0) * block_weight[block_level]);
			block_level--;
		}
	}

	// Get weight Of Root Node
	weight = get_weight(new_ptr);
	total_weight += (total_weight < (0xFFFFFFFF - weight) ? weight : 0);

	return total_weight;
}

static uint32_t get_weight(ast* exp) {
	uint32_t weight = 1;

	bool l_is_float = false;
	bool r_is_float = false;

	if (!exp)
		return 0;

	// Check If Leafs Are Float Or Int To Determine Weight
	if (exp->left != NULL)
		l_is_float = (exp->left->is_float);
	if (exp->right != NULL)
		r_is_float = (exp->right->is_float);

	exp->is_float = exp->is_float | l_is_float | r_is_float;

	// Increase Weight For Double Operations
	if (exp->is_float)
		weight = 2;

	switch (exp->type) {
		case NODE_IF:
		case NODE_ELSE:
		case NODE_REPEAT:
		case NODE_COND_ELSE:
			return weight * 4;

		case NODE_BREAK:
		case NODE_CONTINUE:
			return weight;

		// Variable / Constants (Weight x 1)
		case NODE_CONSTANT:
		case NODE_VAR_CONST:
		case NODE_VAR_EXP:
			return weight;

		// Assignments (Weight x 1)
		case NODE_ASSIGN:
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
			return weight;

		// Simple Operations (Weight x 1)
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
			return weight;

		case NODE_NOT:
		case NODE_COMPL:
		case NODE_NEG:
		case NODE_INCREMENT_R:
		case NODE_INCREMENT_L:
		case NODE_DECREMENT_R:
		case NODE_DECREMENT_L:
			return weight;

		// Medium Operations (Weight x 2)
		case NODE_ADD:
		case NODE_SUB:
		case NODE_LSHIFT:
		case NODE_RSHIFT:
		case NODE_VERIFY:
		case NODE_CONDITIONAL:
			return weight * 2;

		// Complex Operations (Weight x 3)
		case NODE_MUL:
		case NODE_DIV:
		case NODE_MOD:
		case NODE_LROT:
		case NODE_RROT:
			return weight * 3;

		// Complex Operations (Weight x 2)
		case NODE_ABS:
		case NODE_CEIL:
		case NODE_FLOOR:
		case NODE_FABS:
			return weight * 2;

		// Medium Functions (Weight x 4)
		case NODE_SIN:
		case NODE_COS:
		case NODE_TAN:
		case NODE_SINH:
		case NODE_COSH:
		case NODE_TANH:
		case NODE_ASIN:
		case NODE_ACOS:
		case NODE_ATAN:
		case NODE_FMOD:
			return weight * 4;

		// Complex Functions (Weight x 6)
		case NODE_EXPNT:
		case NODE_LOG:
		case NODE_LOG10:
		case NODE_SQRT:
		case NODE_ATAN2:
		case NODE_POW:
		case NODE_GCD:
			return weight * 6;

		case NODE_BLOCK:
		case NODE_PARAM:
			break;

		default:
			break;
	}

	return 0;
}

extern uint32_t calc_wcet() {
	int i;
	uint32_t wcet, total = 0;

	wcet_block = 0;

	for (i = 0; i < vm_ast_cnt; i++) {
		//wcet = get_wcet(vm_ast[i]);
		wcet = calc_weight(vm_ast[i]);
		applog(LOG_DEBUG, "DEBUG: Statement WCET = %lu", wcet);
		if (wcet >(0xFFFFFFFF - total)) {
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
	uint32_t weight = 1;
	uint32_t lval = 0;
	uint32_t rval = 0;
	uint32_t tmp = 0;
	uint32_t wcet = 0;

	bool l_is_float = false;
	bool r_is_float = false;

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

		// Check If Leafs Are Float Or Int To Determine Weight
		if (exp->left != NULL)
			l_is_float = (exp->left->is_float);
		if (exp->right != NULL)
			r_is_float = (exp->right->is_float);

		exp->is_float = exp->is_float | l_is_float | r_is_float;

		// Increase Weight For Double Operations
		if (exp->is_float)
			weight = 2;

		switch (exp->type) {
		case NODE_IF:
			if (exp->right->type != NODE_ELSE) {
				rval = get_wcet(exp->right);				// If Body (No Else Condition)
				wcet = (wcet < (0xFFFFFFFF - (lval + rval + (4 * weight))) ? (lval + rval + (4 * weight)) : 0);
			}
			else {
				tmp = lval;									// Condition
				lval = get_wcet(exp->right->left);			// If Body
				rval = get_wcet(exp->right->right);			// Else Body
				if (lval >= rval)
					wcet = (wcet < (0xFFFFFFFF - (tmp + lval + (4 * weight))) ? (tmp + lval + (4 * weight)) : 0);
				else
					wcet = (wcet < (0xFFFFFFFF - (tmp + rval + (4 * weight))) ? (tmp + rval + (4 * weight)) : 0);
			}
			break;
		case NODE_REPEAT:
			tmp = exp->left->value;
			wcet = (wcet < (0xFFFFFFFF - ((tmp * rval) + (4 * weight))) ? ((tmp * rval) + (4 * weight)) : 0);
			break;
		case NODE_BLOCK:
			wcet_block += lval;
			wcet = wcet_block;
			break;
		case NODE_PARAM:
			wcet_block += lval;
			wcet = wcet_block;
			break;
		case NODE_BREAK:
		case NODE_CONTINUE:
			wcet = (wcet < (0xFFFFFFFF - (0 + weight)) ? (0 + weight) : 0);
			break;

		// Variable / Constants (Weight x 1)
		case NODE_CONSTANT:
		case NODE_VAR_CONST:
		case NODE_VAR_EXP:
			wcet = (wcet < 0xFFFFFFFF ? weight : 0);
			break;

		// Assignments (Weight x 1)
		case NODE_ASSIGN:
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
			wcet = (wcet < (0xFFFFFFFF - (lval + rval + weight)) ? (lval + rval + weight) : 0);
			break;

		// Simple Operations (Weight x 1)
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
			wcet = (wcet < (0xFFFFFFFF - (lval + rval + weight)) ? (lval + rval + weight) : 0);
			break;

		case NODE_NOT:
		case NODE_COMPL:
		case NODE_NEG:
		case NODE_INCREMENT_R:
		case NODE_INCREMENT_L:
		case NODE_DECREMENT_R:
		case NODE_DECREMENT_L:
			wcet = (wcet < (0xFFFFFFFF - (lval + weight)) ? (lval + weight) : 0);
			break;

		// Medium Operations (Weight x 2)
		case NODE_ADD:
		case NODE_SUB:
		case NODE_LSHIFT:
		case NODE_RSHIFT:
		case NODE_VERIFY:
		case NODE_CONDITIONAL:
			wcet = (wcet < (0xFFFFFFFF - (lval + rval + (2 * weight))) ? (lval + rval + (2 * weight)) : 0);
			break;

		// Complex Operations (Weight x 3)
		case NODE_MUL:
		case NODE_DIV:
		case NODE_MOD:
		case NODE_LROT:
		case NODE_RROT:
			wcet = (wcet < (0xFFFFFFFF - (lval + rval + (3 * weight))) ? (lval + rval + (3 * weight)) : 0);
			break;

		// Complex Operations (Weight x 2)
		case NODE_ABS:
		case NODE_CEIL:
		case NODE_FLOOR:
		case NODE_FABS:
			wcet = (wcet < (0xFFFFFFFF - (lval + rval + (2 * weight))) ? (lval + rval + (2 * weight)) : 0);
			break;

		// Medium Functions (Weight x 4)
		case NODE_SIN:
		case NODE_COS:
		case NODE_TAN:
		case NODE_SINH:
		case NODE_COSH:
		case NODE_TANH:
		case NODE_ASIN:
		case NODE_ACOS:
		case NODE_ATAN:
		case NODE_FMOD:
			wcet = (wcet < (0xFFFFFFFF - (lval + rval + (4 * weight))) ? (lval + rval + (4 * weight)) : 0);
			break;

		// Complex Functions (Weight x 6)
		case NODE_EXPNT:
		case NODE_LOG:
		case NODE_LOG10:
		case NODE_SQRT:
		case NODE_ATAN2:
		case NODE_POW:
		case NODE_GCD:
			wcet = (wcet < (0xFFFFFFFF - (lval + rval + (6 * weight))) ? (lval + rval + (6 * weight)) : 0);
			break;

		case NODE_COND_ELSE:
		case NODE_ELSE:
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

extern int interpret_ast(bool first_run) {
	int i, idx = 0;

	vm_bounty = false;
	vm_break = false;
	vm_continue = false;

	if (vm_ast[0]->type == NODE_INIT_ONCE) {
		idx = 1;
		if (first_run) {
			if (!interpret(vm_ast[0]->left))
				return 0;
		}
	}

	for (i = idx; i < vm_ast_cnt; i++) {
		if (!interpret(vm_ast[i]) && (vm_ast[i]->type != NODE_VAR_CONST) && (vm_ast[i]->type != NODE_VAR_EXP) && (vm_ast[i]->type != NODE_CONSTANT))
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

static void mangle_state(int x) {
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
			if (exp->data_type == DT_FLOAT)
				return vm_f[lval];
			return vm_m[lval];
		case NODE_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left->left);
			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rfval = interpret(exp->right);

			if (exp->left->is_float) {
				vm_f[lval] = rfval;
				mangle_state((int32_t)vm_f[lval]);
			}
			else {
				vm_m[lval] = (int32_t)rfval;
				mangle_state(vm_m[lval]);
			}
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
				lval = (int32_t)interpret(exp->left->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rfval = interpret(exp->right);

			if (exp->left->is_float) {
				vm_f[lval] += rfval;
				mangle_state((int32_t)vm_f[lval]);
			}
			else {
				vm_m[lval] += (int32_t)rfval;
				mangle_state(vm_m[lval]);
			}
			return 1;
		case NODE_SUB_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rfval = interpret(exp->right);

			if (exp->left->is_float) {
				vm_f[lval] -= rfval;
				mangle_state((int32_t)vm_f[lval]);
			}
			else {
				vm_m[lval] -= (int32_t)rfval;
				mangle_state(vm_m[lval]);
			}
			return 1;
		case NODE_MUL_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rfval = interpret(exp->right);

			if (exp->left->is_float) {
				vm_f[lval] *= rfval;
				mangle_state((int32_t)vm_f[lval]);
			}
			else {
				vm_m[lval] *= (int32_t)rfval;
				mangle_state(vm_m[lval]);
			}
			return 1;
		case NODE_DIV_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rfval = interpret(exp->right);
			if (rfval == 0.0)
				return 0;

			if (exp->left->is_float) {
				vm_f[lval] /= rfval;
				mangle_state((int32_t)vm_f[lval]);
			}
			else {
				vm_m[lval] = (int32_t)(vm_m[lval] / rfval);
				mangle_state(vm_m[lval]);
			}
			return 1;
		case NODE_MOD_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rval = (int32_t)interpret(exp->right);
			if (rval == 0)
				return 0;

			if (exp->left->is_float) {
				vm_f[lval] = (double)((int32_t)vm_f[lval] % rval);
				mangle_state((int32_t)vm_f[lval]);
			}
			else {
				vm_m[lval] %= rval;
				mangle_state(vm_m[lval]);
			}
			return 1;
		case NODE_LSHFT_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rval = (int32_t)interpret(exp->right);

			if (exp->left->is_float) {
				vm_f[lval] = (double)((int32_t)vm_f[lval] << rval);
				mangle_state((int32_t)vm_f[lval]);
			}
			else {
				vm_m[lval] <<= rval;
				mangle_state(vm_m[lval]);
			}
			return 1;
		case NODE_RSHFT_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rval = (int32_t)interpret(exp->right);

			if (exp->left->is_float) {
				vm_f[lval] = (double)((int32_t)vm_f[lval] >> rval);
				mangle_state((int32_t)vm_f[lval]);
			}
			else {
				vm_m[lval] >>= rval;
				mangle_state(vm_m[lval]);
			}
			return 1;
		case NODE_AND_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rval = (int32_t)interpret(exp->right);

			if (exp->left->is_float) {
				vm_f[lval] = (double)((int32_t)vm_f[lval] & rval);
				mangle_state((int32_t)vm_f[lval]);
			}
			else {
				vm_m[lval] &= rval;
				mangle_state(vm_m[lval]);
			}
			return 1;
		case NODE_XOR_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rval = (int32_t)interpret(exp->right);

			if (exp->left->is_float) {
				vm_f[lval] = (double)((int32_t)vm_f[lval] ^ rval);
				mangle_state((int32_t)vm_f[lval]);
			}
			else {
				vm_m[lval] ^= rval;
				mangle_state(vm_m[lval]);
			}
			return 1;
		case NODE_OR_ASSIGN:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			rval = (int32_t)interpret(exp->right);

			if (exp->left->is_float) {
				vm_f[lval] = (double)((int32_t)vm_f[lval] | rval);
				mangle_state((int32_t)vm_f[lval]);
			}
			else {
				vm_m[lval] |= rval;
				mangle_state(vm_m[lval]);
			}
			return 1;
		case NODE_INCREMENT_R:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			if (exp->end_stmnt) {
				mangle_state((exp->left->is_float) ? ++vm_f[lval] : ++vm_m[lval]);
				return 1;
			}
			else {
				return (exp->left->is_float) ? ++vm_f[lval] : ++vm_m[lval];
			}
		case NODE_INCREMENT_L:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			if (exp->end_stmnt) {
				mangle_state((exp->left->is_float) ? ++vm_f[lval] : ++vm_m[lval]);
				return 1;
			}
			else {
				if (exp->left->is_float) {
					rfval = vm_f[lval]++;
					return rfval;
				}
				else {
					rval = vm_m[lval]++;
					return rval;
				}
			}
		case NODE_DECREMENT_R:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			if (exp->end_stmnt) {
				mangle_state((exp->left->is_float) ? --vm_f[lval] : --vm_m[lval]);
				return 1;
			}
			else {
				return (exp->left->is_float) ? --vm_f[lval] : --vm_m[lval];
			}
		case NODE_DECREMENT_L:
			if (exp->left->type == NODE_VAR_CONST)
				lval = exp->left->value;
			else
				lval = (int32_t)interpret(exp->left->left);

			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;

			if (exp->end_stmnt) {
				mangle_state((exp->left->is_float) ? --vm_f[lval] : --vm_m[lval]);
				return 1;
			}
			else {
				if (exp->left->is_float) {
					rfval = vm_f[lval]--;
					return rfval;
				}
				else {
					rval = vm_m[lval]--;
					return rval;
				}
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
			vm_param_num = 0;
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
			return pow(vm_param_val[0], vm_param_val[1]);
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
