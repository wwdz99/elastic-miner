/*
* Copyright 2016 sprocket
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free
* Software Foundation; either version 2 of the License, or (at your option)
* any later version.
*/

#ifndef ELASTICPL_H_
#define ELASTICPL_H_

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LITERAL_SIZE 100
#define TOKEN_LIST_SIZE 1024
#define PARSE_STACK_SIZE 512

//#define VM_MEMORY_SIZE 1024
#define VM_MEMORY_SIZE 64000
#define VM_STACK_SIZE 1024

typedef enum {
	NODE_INPUT,
	NODE_ERROR,
	NODE_CONSTANT,
	NODE_VAR_CONST,
	NODE_VAR_EXP,
	NODE_VERIFY,
	NODE_ASSIGN,
	NODE_OR,
	NODE_AND,
	NODE_BITWISE_OR,
	NODE_BITWISE_XOR,
	NODE_BITWISE_AND,
	NODE_COMPL,
	NODE_EQ,
	NODE_NE,
	NODE_LT,
	NODE_GT,
	NODE_LE,
	NODE_GE,
	NODE_ADD,
	NODE_POS,
	NODE_SUB,
	NODE_NEG,
	NODE_MUL,
	NODE_DIV,
	NODE_MOD,
	NODE_RSHIFT,
	NODE_LSHIFT,
	NODE_RROT,
	NODE_LROT,
	NODE_NOT,
	NODE_TRUE,
	NODE_FALSE,
	NODE_BLOCK,
	NODE_EXPRESSION,
	NODE_IF,
	NODE_ELSE,
	NODE_REPEAT,
	NODE_SHA256,
	NODE_MD5
} NODE_TYPE;


typedef enum {
	TOKEN_ASSIGN,
	TOKEN_OR,
	TOKEN_AND,
	TOKEN_BITWISE_OR,
	TOKEN_BITWISE_XOR,
	TOKEN_BITWISE_AND,
	TOKEN_EQ,
	TOKEN_NE,
	TOKEN_LT,
	TOKEN_GT,
	TOKEN_LE,
	TOKEN_GE,
	TOKEN_ADD,
	TOKEN_POS,
	TOKEN_SUB,
	TOKEN_NEG,
	TOKEN_MUL,
	TOKEN_DIV,
	TOKEN_MOD,
	TOKEN_RSHIFT,
	TOKEN_LSHIFT,
	TOKEN_RROT,
	TOKEN_LROT,
	TOKEN_COMPL,
	TOKEN_NOT,
	TOKEN_CONSTANT,
	TOKEN_TRUE,
	TOKEN_FALSE,
	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_REPEAT,
	TOKEN_VAR_BEGIN,
	TOKEN_VAR_END,
	TOKEN_BLOCK_BEGIN,
	TOKEN_BLOCK_END,
	TOKEN_OPEN_PAREN,
	TOKEN_CLOSE_PAREN,
	TOKEN_LITERAL,
	TOKEN_END_STATEMENT,
	TOKEN_VERIFY,
	TOKEN_EOF,
	TOKEN_SHA256,
	TOKEN_MD5
} EPL_TOKEN_TYPE;


typedef enum {
	EXP_NONE,
	UNARY_STATEMENT,
	UNARY_EXPRESSION,
	UNARY_STMNT_EXP,
	BINARY_STATEMENT,
	BINARY_EXPRESSION,
	BINARY_STMNT_EXP
} TOKEN_EXP;


// Token Type / Literal Value From ElasticPL Source Code
typedef struct {
	int token_id;
	EPL_TOKEN_TYPE type;
	char *literal;
	TOKEN_EXP exp;
	int prec;
	int line_num;
} SOURCE_TOKEN;


// List Of All Tokens In ElasticPL Source Code
typedef struct {
	SOURCE_TOKEN *token;
	int num;
	int size;
} SOURCE_TOKEN_LIST;


struct EPL_TOKEN_LIST {
	char* str;
	int len;
	EPL_TOKEN_TYPE type;
	TOKEN_EXP exp;
	int prec;
};

typedef struct AST {
	NODE_TYPE type;
	TOKEN_EXP exp;
	long value;
	int token_num;
	int line_num;
	struct AST*	left;
	struct AST*	right;

} ast;

typedef struct VM_STACK_ITEM {
	long value;
	bool memory;
} vm_stack_item;

int stack_op_idx;
int stack_exp_idx;
int top_op;

int *stack_op;		// List Of Operators For Parsing
ast **stack_exp;	// List Of Expresions For Parsing / Final Expression List

int vm_ast_cnt;		// Number Of AST Root Nodes In VM
ast **vm_ast;		// Final AST List For VM

					// Function Declarations
extern bool create_epl_vm(char *source);
extern int run_epl_vm();
extern void delete_epl_vm();

extern bool init_token_list(SOURCE_TOKEN_LIST *token_list, size_t size);
static bool add_token(SOURCE_TOKEN_LIST *token_list, int token_id, char *literal, int line_num);
extern void delete_token_list(SOURCE_TOKEN_LIST *token_list);
static int validate_token_list(SOURCE_TOKEN_LIST *token_list);
extern bool get_token_list(char *str, SOURCE_TOKEN_LIST *token_list);
static void dump_token_list(SOURCE_TOKEN_LIST *token_list);

extern bool parse_token_list(SOURCE_TOKEN_LIST *token_list);
extern char* get_node_str(NODE_TYPE node_type);
extern void dump_vm_ast(ast* root);

extern int interpret_ast();
extern char* c_compile_ast();
extern char* compile(ast* exp);

#endif // ELASTICPL_H_