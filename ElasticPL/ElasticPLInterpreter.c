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

//int vm_stack_idx = -1;
//vm_stack_item vm_stack[VM_STACK_SIZE];
//bool vm_bounty;
//long *vm_mem;

void push(long l, bool memory) {
	vm_stack[++vm_stack_idx].value = l;
	vm_stack[vm_stack_idx].memory = memory;
}

static vm_stack_item pop_item() {
	return vm_stack[vm_stack_idx--];
}

static long pop() {
	if (vm_stack[vm_stack_idx].memory)
		return vm_mem[vm_stack[vm_stack_idx--].value];
	else
		return vm_stack[vm_stack_idx--].value;
}

extern int interpret_ast() {
	int i;

	vm_state1 = 0L;
	vm_state2 = 0L;

//	vm_stack_idx = -1;

	for (i = 0; i < vm_ast_cnt; i++) {
		//if (opt_debug_vm)
		//	dump_statement(i, vm_ast[i]->type);
		if(!interpret(vm_ast[i]))
			return 0;
	}

	//if (opt_debug_vm) {
	//	for (i = 0; i < 12; i++) {
	//		fprintf(stdout, "\tvm_mem[%d] = %d\n", i, vm_mem[i]);
	//	}
	//}

	if (vm_stack_idx != -1)
		applog(LOG_WARNING, "WARNING: Possible VM Memory Leak");

	if (!vm_bounty)
		return 1;
	else
		return 2;
}

// Use Post Order Traversal To Process The Expressions In The AST
extern bool interpret(ast* exp) {
	long lval, rval;

	if (exp != NULL) {

		if(!interpret(exp->left))
			return false;
		
		// Check For If Statement As Right Side Is Conditional
		if (exp->type != NODE_IF)
			if (!interpret(exp->right))
				return false;

		switch (exp->type) {
		case NODE_CONSTANT:
			push(exp->value, false);
			break;
		case NODE_VAR_CONST:
			if (exp->value < 0 || exp->value > VM_MEMORY_SIZE) {
				exp->value = 0;
			}
			push(exp->value, true);
			break;
		case NODE_VAR_EXP:
			lval = pop();
			if (lval < 0 || lval > VM_MEMORY_SIZE) {
				lval = 0;
			}
			push(lval, true);
			break;
		case NODE_ASSIGN:
			rval = pop();
			lval = pop_item().value;	// Get The Memory Location, Not The Value
			if (lval < 0 || lval > VM_MEMORY_SIZE) {
				lval = 0;
			}
			vm_mem[lval] = rval;
			mangle_state(lval);
			mangle_state(rval);
			break;
		case NODE_IF:
			if (exp->right->type != NODE_ELSE) {
				if (pop())
					if (!interpret(exp->right))			// If Body (No Else Condition)
						return false;
			}
			else {
				if (pop()) {
					if (!interpret(exp->right->left))	// If Body
						return false;
				}
				else {
					if (!interpret(exp->right->right))	// Else Body
						return false;
				}
			}
			break;
		case NODE_BLOCK:
			break;
		case NODE_ADD:
			rval = pop();
			lval = pop();
			push(lval + rval, false);
			mangle_state(lval + rval);
			break;
		case NODE_SUB:
			rval = pop();
			lval = pop();
			push(lval - rval, false);
			mangle_state(lval - rval);
			break;
		case NODE_MUL:
			rval = pop();
			lval = pop();
			push(lval * rval, false);
			mangle_state(lval * rval);
			break;
		case NODE_DIV:
			rval = pop();
			lval = pop();
			if (rval != 0){
				push(lval / rval, false);
				mangle_state(lval / rval);
			}
			else {
				push(0, false);
				mangle_state(0);
			}
			
			break;
		case NODE_MOD:
			rval = pop();
			lval = pop();
			if (rval != 0){
				push(lval % rval, false);
				mangle_state(lval % rval);
			}
			else {
				push(0, false);
				mangle_state(0);
			}
			break;
		case NODE_LSHIFT:
			rval = pop();
			lval = pop();
			push(lval << rval, false);
			mangle_state(lval << rval);
			break;
		case NODE_LROT:
			rval = pop();
			lval = pop();
			push((((lval) >> (32 - (rval%32))) | ((lval) << (rval%32))), false);
			mangle_state((((lval) >> (32 - (rval%32))) | ((lval) << (rval%32))));
			break;
		case NODE_RSHIFT:
			rval = pop();
			lval = pop();
			push(lval >> rval, false);
			mangle_state(lval >> rval);
			break;
		case NODE_RROT:
			rval = pop();
			lval = pop();
			push((((lval) << (32 - (rval%32))) | ((lval) >> (rval%32))), false);
			mangle_state((((lval) << (32 - (rval%32))) | ((lval) >> (rval%32))));
			break;
		case NODE_NOT:
			rval = pop();
			push(!rval, false);
			break;
			mangle_state(!rval);
		case NODE_COMPL:
			rval = pop();
			push(~rval, false);
			mangle_state(~rval);
			break;
		case NODE_AND:
			rval = pop();
			lval = pop();
			push(lval && rval, false);
			mangle_state(lval && rval);
			break;
		case NODE_OR:
			rval = pop();
			lval = pop();
			push(lval || rval, false);
			mangle_state(lval || rval);
			break;
		case NODE_BITWISE_AND:
			rval = pop();
			lval = pop();
			push(lval & rval, false);
//			mangle_state(lval & rval);
			break;
		case NODE_BITWISE_XOR:
			rval = pop();
			lval = pop();
			push(lval ^ rval, false);
			mangle_state(lval ^ rval);
			break;
		case NODE_BITWISE_OR:
			rval = pop();
			lval = pop();
			push(lval | rval, false);
			mangle_state(lval | rval);
			break;
		case NODE_EQ:
			rval = pop();
			lval = pop();
			push(lval == rval, false);
			mangle_state(lval ==rval);
			break;
		case NODE_NE:
			rval = pop();
			lval = pop();
			push(lval != rval, false);
			mangle_state(lval != rval);
			break;
		case NODE_GT:
			rval = pop();
			lval = pop();
			push(lval > rval, false);
//			mangle_state(lval > rval);
			break;
		case NODE_LT:
			rval = pop();
			lval = pop();
			push(lval < rval, false);
//			mangle_state(lval < rval);
			break;
		case NODE_GE:
			rval = pop();
			lval = pop();
			push(lval >= rval, false);
//			mangle_state(lval >= rval);
			break;
		case NODE_LE:
			rval = pop();
			lval = pop();
			push(lval <= rval, false);
//			mangle_state(lval <= rval);
			break;
		case NODE_NEG:
			rval = pop();
			push(-rval, false);
			mangle_state(-rval);
			break;
		case NODE_POS:
			rval = pop();
			push(+rval, false);
			mangle_state(+rval);
			break;
		case NODE_SHA256:
			rval = pop();
			lval = pop();

//
// Never crash!
//

			if (((rval % 4) != 0) || ((lval + rval) > VM_MEMORY_SIZE)) {
				applog(LOG_ERR, "ERROR: VM Runtime - Invalid SHA256 byte length\n");
				return false;
			}
			sha256_epl((unsigned char*)&vm_mem[lval], rval, (unsigned char*)&vm_mem[lval]);
//
// Add mangle state logic
//
			break;
		case NODE_MD5:
			applog(LOG_ERR, "ERROR: VM Runtime - MD5 Not Implemented");
			return false;
		case NODE_VERIFY:
			rval = pop();
			vm_bounty = (rval != 0);
			mangle_state(vm_bounty);
			break;
		default:
			applog(LOG_ERR, "ERROR: VM Runtime - Unsupported Operation (%d)", exp->type);
			return false;
		}
	}

	if (vm_stack_idx >= VM_STACK_SIZE) {
		applog(LOG_ERR, "ERROR: VM Runtime - Stack Overflow!");
		return false;
	}

	return true;
}

static void mangle_state(int x) {
	int mod = (x < 0) ? 64 + (x % 64) : (x % 64);
	if (x % 2 == 0) {
		//		internal_state = internal_state << (x % 64);
		vm_state1 = (vm_state1 << mod) | (vm_state1 >> (64 - mod));
		vm_state1 = vm_state1 ^ x;
	}
	else {
		//		internal_state2 = internal_state2 >> (x % 64);
		vm_state2 = ((vm_state2) << (64 - mod)) | ((vm_state2) >> mod);
		vm_state2 = vm_state2 ^ x;
	}
}

extern void dump_statement(int stmnt_num, NODE_TYPE stmnt_type) {
	switch (stmnt_type) {
		case NODE_ASSIGN:
			applog(LOG_DEBUG, "DEBUG: Process VM Statement: %d (Assignment)", stmnt_num);
			break;
		case NODE_IF:
			applog(LOG_DEBUG, "DEBUG: Process VM Statement: %d (If)", stmnt_num);
			break;
		case NODE_REPEAT:
			applog(LOG_DEBUG, "DEBUG: Process VM Statement: %d (Repeat)", stmnt_num);
			break;
		case NODE_VERIFY:
			applog(LOG_DEBUG, "DEBUG: Process VM Statement: %d (Verify)", stmnt_num);
			break;
		case NODE_SHA256:
			applog(LOG_DEBUG, "DEBUG: Process VM Statement: %d (SHA256)", stmnt_num);
			break;
		case NODE_MD5:
			applog(LOG_DEBUG, "DEBUG: Process VM Statement: %d (MD5)", stmnt_num);
			break;
		default:
			applog(LOG_DEBUG, "DEBUG: Process VM Statement: %d (Unknown - %d)", stmnt_num, stmnt_type);
			break;
	}
}
