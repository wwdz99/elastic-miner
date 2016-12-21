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
		case NODE_SHA256:
			wcet = (wcet < (0xFFFFFFFF - (100 + rval)) ? (100 + rval) : 0);
			break;
		case NODE_SHA512:
			wcet = (wcet < (0xFFFFFFFF - (120 + rval)) ? (120 + rval) : 0);
			break;
		case NODE_WHIRLPOOL:
			wcet = (wcet < (0xFFFFFFFF - (150 + rval)) ? (150 + rval) : 0);
			break;
		case NODE_MD5:
			wcet = (wcet < (0xFFFFFFFF - (80 + rval)) ? (80 + rval) : 0);
			break;
		case NODE_SECP192K_PTP:
		case NODE_SECP192R_PTP:
		case NODE_SECP224K_PTP:
		case NODE_SECP224R_PTP:
		case NODE_SECP256K_PTP:
		case NODE_SECP256R_PTP:
		case NODE_SECP384R_PTP:
		case NODE_PRM192V2_PTP:
		case NODE_PRM192V3_PTP:
		case NODE_PRM256V1_PTP:
			wcet = (wcet < (0xFFFFFFFF - (5000 + rval)) ? (5000 + rval) : 0);
			break;
		case NODE_SECP192K_PA:
		case NODE_SECP192K_PS:
		case NODE_SECP192K_PSM:
		case NODE_SECP192K_PN:
		case NODE_SECP192R_PA:
		case NODE_SECP192R_PS:
		case NODE_SECP192R_PSM:
		case NODE_SECP192R_PN:
		case NODE_SECP224K_PA:
		case NODE_SECP224K_PS:
		case NODE_SECP224K_PSM:
		case NODE_SECP224K_PN:
		case NODE_SECP224R_PA:
		case NODE_SECP224R_PS:
		case NODE_SECP224R_PSM:
		case NODE_SECP224R_PN:
		case NODE_SECP256K_PA:
		case NODE_SECP256K_PS:
		case NODE_SECP256K_PSM:
		case NODE_SECP256K_PN:
		case NODE_SECP256R_PA:
		case NODE_SECP256R_PS:
		case NODE_SECP256R_PSM:
		case NODE_SECP256R_PN:
		case NODE_SECP384R_PA:
		case NODE_SECP384R_PS:
		case NODE_SECP384R_PSM:
		case NODE_SECP384R_PN:
		case NODE_PRM192V1_PTP:
		case NODE_PRM192V1_PA:
		case NODE_PRM192V1_PS:
		case NODE_PRM192V1_PSM:
		case NODE_PRM192V1_PN:
		case NODE_PRM192V2_PA:
		case NODE_PRM192V2_PS:
		case NODE_PRM192V2_PSM:
		case NODE_PRM192V2_PN:
		case NODE_PRM192V3_PA:
		case NODE_PRM192V3_PS:
		case NODE_PRM192V3_PSM:
		case NODE_PRM192V3_PN:
		case NODE_PRM256V1_PA:
		case NODE_PRM256V1_PS:
		case NODE_PRM256V1_PSM:
		case NODE_PRM256V1_PN:
			wcet = (wcet < (0xFFFFFFFF - (1000 + rval)) ? (1000 + rval) : 0);
			break;
		case NODE_TIGER:
			wcet = (wcet < (0xFFFFFFFF - (100 + rval)) ? (100 + rval) : 0);
			break;
		case NODE_RIPEMD160:
			wcet = (wcet < (0xFFFFFFFF - (120 + rval)) ? (120 + rval) : 0);
			break;
		case NODE_RIPEMD128:
			wcet = (wcet < (0xFFFFFFFF - (100 + rval)) ? (100 + rval) : 0);
			break;
		default:
			break;
		}
	}

	return wcet;
}

static void push(long l, bool memory) {
	vm_stack[++vm_stack_idx].value = l;
	vm_stack[vm_stack_idx].memory = memory;
}

static vm_stack_item pop_item() {
	return vm_stack[vm_stack_idx--];
}

static long pop() {
	if (vm_stack[vm_stack_idx].memory)
		return vm_m[vm_stack[vm_stack_idx--].value];
	else
		return vm_stack[vm_stack_idx--].value;
}

extern int interpret_ast() {
	int i;

	vm_bounty = false;

	for (i = 0; i < vm_ast_cnt; i++) {
		if (!interpret(vm_ast[i], false))
			return 0;
	}

	if (vm_stack_idx != -1)
		applog(LOG_WARNING, "WARNING: Possible VM Memory Leak");

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

static bool get_param(ast *exp, int *param, size_t num) {
	if (num > 0) {
		if (!exp->right->left)
			return false;
		else
			param[0] = exp->right->left->value;
	}
	if (num > 1) {
		if (!exp->right->right->left)
			return false;
		else
			param[1] = exp->right->right->left->value;
	}
	if (num > 2) {
		if (!exp->right->right->right->left)
			return false;
		else
			param[2] = exp->right->right->right->left->value;
	}
	if (num > 3) {
		if (!exp->right->right->right->right->left)
			return false;
		else
			param[3] = exp->right->right->right->right->left->value;
	}
	if (num > 4) {
		if (!exp->right->right->right->right->right->left)
			return false;
		else
			param[4] = exp->right->right->right->right->right->left->value;
	}
	if (num > 5 || num <= 0)
		return false;

	return true;
}

static int32_t interpret(ast* exp, bool mangle) {
	long lval, rval, val;
	int param[5];

	if (exp == NULL)
		return 0;

	switch (exp->type) {
		case NODE_CONSTANT:
			return exp->value;
		case NODE_VAR_CONST:
			if (exp->value < 0 || exp->value > VM_MEMORY_SIZE)
				val = 0;
			else
				val = exp->value;
			if (mangle)					// This Allows Us To Mangle All Memory Locations On Left Side Of Assign Operator
				//mangle_state(val);
			return vm_m[val];
		case NODE_VAR_EXP:
			lval = interpret(exp->left, mangle);
			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;
			if (mangle)					// This Allows Us To Mangle All Memory Locations On Left Side Of Assign Operator
				mangle_state(lval);
			return vm_m[lval];
		case NODE_ASSIGN:
			rval = interpret(exp->right, false);
			mangle_state(rval);
			if (exp->left->type == NODE_VAR_CONST) {
				lval = exp->left->value;
				mangle_state(lval);
			}
			else
				lval = interpret(exp->left, true);
			if (lval < 0 || lval > VM_MEMORY_SIZE)
				return 0;
			vm_m[lval] = rval;
//			mangle_state(lval);
			return 1;
		case NODE_IF:
			if (exp->right->type != NODE_ELSE) {
				if (interpret(exp->left, false))
					return interpret(exp->right, false);			// If Body (No Else Condition)
			}
			else {
				if (interpret(exp->left, false))
					return interpret(exp->right->left, false);		// If Body
				else
					return interpret(exp->right->right, false);		// Else Body
			}
			break;
		case NODE_REPEAT:
			lval = interpret(exp->left, false);
			if (lval > 0) {
				int i;
				for (i = 0; i < lval; i++)
					interpret(exp->right, false);					// Repeat Body
			}
			return 1;
		case NODE_BLOCK:
			interpret(exp->left, false);
			if (exp->right)
				interpret(exp->right, false);
			return 1;
		case NODE_ADD:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = (lval + rval);
			//mangle_state(val);
			return val;

//
// Fix
		case NODE_INCREMENT_R:
			return 0;
// Fix
//
		case NODE_SUB:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = (lval - rval);
			//mangle_state(val);
			return val;
		case NODE_MUL:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = (lval * rval);
			//mangle_state(val);
			return val;
		case NODE_DIV:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			if (rval != 0)
				val = lval / rval;
			else
				val = 0;
			//mangle_state(val);
			return val;
		case NODE_MOD:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			if (rval > 0)
				val = lval % rval;
			else
				val = 0;
			//mangle_state(val);
			return val;
		case NODE_LSHIFT:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = (lval << rval);
			//mangle_state(val);
			return val;
		case NODE_LROT:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = rotl32(lval, rval % 32);
			//mangle_state(val);
			return val;
		case NODE_RSHIFT:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = (lval >> rval);
			//mangle_state(val);
			return val;
		case NODE_RROT:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = rotr32(lval, rval % 32);
			//mangle_state(val);
			return val;
		case NODE_NOT:
			lval = interpret(exp->left, mangle);
			mangle_state(!lval);
			return !lval;
		case NODE_COMPL:
			lval = interpret(exp->left, mangle);
			mangle_state(~lval);
			return ~lval;
		case NODE_AND:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
//			val = (lval && rval);
			val = (lval >! rval);
			//mangle_state(val);
			return val;
		case NODE_OR:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = (lval || rval);
			//mangle_state(val);
			return val;
		case NODE_BITWISE_AND:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = (lval & rval);
			//mangle_state(val);
			return val;
		case NODE_BITWISE_XOR:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = (lval ^ rval);
			//mangle_state(val);
			return val;
		case NODE_BITWISE_OR:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = (lval | rval);
			//mangle_state(val);
			return val;
		case NODE_EQ:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = (lval == rval);
			//mangle_state(val);
			return val;
		case NODE_NE:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = (lval != rval);
			//mangle_state(val);
			return val;
		case NODE_GT:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = (lval > rval);
			//mangle_state(val);
			return val;
		case NODE_LT:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = (lval < rval);
			//mangle_state(val);
			return val;
		case NODE_GE:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = (lval >= rval);
			//mangle_state(val);
			return val;
		case NODE_LE:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			val = (lval <= rval);
			//mangle_state(val);
			return val;
		case NODE_NEG:
			lval = interpret(exp->left, mangle);
			mangle_state(-lval);
			return -lval;
		case NODE_VERIFY:
			lval = interpret(exp->left, mangle);
			rval = interpret(exp->right, mangle);
			vm_bounty = (lval != rval);
			mangle_state(vm_bounty);
			return vm_bounty;
		case NODE_PARAM:
			break;
		case NODE_SHA256:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_sha256(param[0], param[1], vm_m);
			//mangle_state(val);
			return 1;
		case NODE_SHA512:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_sha512(param[0], param[1], vm_m);
			//mangle_state(val);
			return 1;
		case NODE_WHIRLPOOL:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_whirlpool(param[0], param[1], vm_m);
			//mangle_state(val);
			return 1;
		case NODE_MD5:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_md5(param[0], param[1], vm_m);
			//mangle_state(val);
			return 1;
		case NODE_SECP192K_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_m, NID_secp192k1, 24);
			//mangle_state(val);
			return 1;
		case NODE_SECP192K_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_m, NID_secp192k1, 25, 49);
			//mangle_state(val);
			return 1;
		case NODE_SECP192K_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_m, NID_secp192k1, 25, 49);
			//mangle_state(val);
			return 1;
		case NODE_SECP192K_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_m, NID_secp192k1, 25, 49);
			//mangle_state(val);
			return 1;
		case NODE_SECP192K_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_m, NID_secp192k1, 25, 49);
			//mangle_state(val);
			return 1;
		case NODE_SECP224K_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_m, NID_secp224k1, 28);
			//mangle_state(val);
			return 1;
		case NODE_SECP224K_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_m, NID_secp224k1, 29, 57);
			//mangle_state(val);
			return 1;
		case NODE_SECP224K_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_m, NID_secp224k1, 29, 57);
			//mangle_state(val);
			return 1;
		case NODE_SECP224K_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_m, NID_secp224k1, 29, 57);
			//mangle_state(val);
			return 1;
		case NODE_SECP224K_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_m, NID_secp224k1, 29, 57);
			//mangle_state(val);
			return 1;
		case NODE_SECP224R_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_m, NID_secp224r1, 28);
			//mangle_state(val);
			return 1;
		case NODE_SECP224R_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_m, NID_secp224r1, 29, 57);
			//mangle_state(val);
			return 1;
		case NODE_SECP224R_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_m, NID_secp224r1, 29, 57);
			//mangle_state(val);
			return 1;
		case NODE_SECP224R_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_m, NID_secp224r1, 29, 57);
			//mangle_state(val);
			return 1;
		case NODE_SECP224R_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_m, NID_secp224r1, 29, 57);
			//mangle_state(val);
			return 1;
		case NODE_SECP256K_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_m, NID_secp256k1, 32);
			//mangle_state(val);
			return 1;
		case NODE_SECP256K_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_m, NID_secp256k1, 33, 65);
			//mangle_state(val);
			return 1;
		case NODE_SECP256K_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_m, NID_secp256k1, 33, 65);
			//mangle_state(val);
			return 1;
		case NODE_SECP256K_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_m, NID_secp256k1, 33, 65);
			//mangle_state(val);
			return 1;
		case NODE_SECP256K_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_m, NID_secp256k1, 33, 65);
			//mangle_state(val);
			return 1;
		case NODE_SECP384R_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_m, NID_secp384r1, 48);
			//mangle_state(val);
			return 1;
		case NODE_SECP384R_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_m, NID_secp384r1, 48, 97);
			//mangle_state(val);
			return 1;
		case NODE_SECP384R_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_m, NID_secp384r1, 48, 97);
			//mangle_state(val);
			return 1;
		case NODE_SECP384R_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_m, NID_secp384r1, 48, 97);
			//mangle_state(val);
			return 1;
		case NODE_SECP384R_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_m, NID_secp384r1, 48, 97);
			//mangle_state(val);
			return 1;
		case NODE_PRM192V1_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_m, NID_X9_62_prime192v1, 24);
			//mangle_state(val);
			return 1;
		case NODE_PRM192V1_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_m, NID_X9_62_prime192v1, 25, 49);
			//mangle_state(val);
			return 1;
		case NODE_PRM192V1_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_m, NID_X9_62_prime192v1, 25, 49);
			//mangle_state(val);
			return 1;
		case NODE_PRM192V1_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_m, NID_X9_62_prime192v1, 25, 49);
			//mangle_state(val);
			return 1;
		case NODE_PRM192V1_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_m, NID_X9_62_prime192v1, 25, 49);
			//mangle_state(val);
			return 1;
		case NODE_PRM192V2_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_m, NID_X9_62_prime192v2, 24);
			//mangle_state(val);
			return 1;
		case NODE_PRM192V2_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_m, NID_X9_62_prime192v2, 25, 49);
			//mangle_state(val);
			return 1;
		case NODE_PRM192V2_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_m, NID_X9_62_prime192v2, 25, 49);
			//mangle_state(val);
			return 1;
		case NODE_PRM192V2_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_m, NID_X9_62_prime192v2, 25, 49);
			//mangle_state(val);
			return 1;
		case NODE_PRM192V2_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_m, NID_X9_62_prime192v2, 25, 49);
			//mangle_state(val);
			return 1;
		case NODE_PRM192V3_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_m, NID_X9_62_prime192v3, 24);
			//mangle_state(val);
			return 1;
		case NODE_PRM192V3_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_m, NID_X9_62_prime192v3, 25, 49);
			//mangle_state(val);
			return 1;
		case NODE_PRM192V3_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_m, NID_X9_62_prime192v3, 25, 49);
			//mangle_state(val);
			return 1;
		case NODE_PRM192V3_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_m, NID_X9_62_prime192v3, 25, 49);
			//mangle_state(val);
			return 1;
		case NODE_PRM192V3_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_m, NID_X9_62_prime192v3, 25, 49);
			//mangle_state(val);
			return 1;
		case NODE_PRM256V1_PTP:
			if (!get_param(exp, param, 2))
				val = 0;
			else
				val = epl_ec_priv_to_pub(param[0], param[1], vm_m, NID_X9_62_prime256v1, 32);
			//mangle_state(val);
			return 1;
		case NODE_PRM256V1_PA:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_add(param[0], param[1], param[2], param[3], param[4], vm_m, NID_X9_62_prime256v1, 33, 65);
			//mangle_state(val);
			return 1;
		case NODE_PRM256V1_PS:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_sub(param[0], param[1], param[2], param[3], param[4], vm_m, NID_X9_62_prime256v1, 33, 65);
			//mangle_state(val);
			return 1;
		case NODE_PRM256V1_PSM:
			if (!get_param(exp, param, 5))
				val = 0;
			else
				val = epl_ec_mult(param[0], param[1], param[2], param[3], param[4], vm_m, NID_X9_62_prime256v1, 33, 65);
			//mangle_state(val);
			return 1;
		case NODE_PRM256V1_PN:
			if (!get_param(exp, param, 3))
				val = 0;
			else
				val = epl_ec_neg(param[0], param[1], param[2], vm_m, NID_X9_62_prime256v1, 33, 65);
			//mangle_state(val);
			return 1;

		default:
			applog(LOG_ERR, "ERROR: VM Runtime - Unsupported Operation (%d)", exp->type);
	}

	if (vm_stack_idx >= VM_STACK_SIZE) {
		applog(LOG_ERR, "ERROR: VM Runtime - Stack Overflow!");
		return 0;
	}

	return 1;
}
