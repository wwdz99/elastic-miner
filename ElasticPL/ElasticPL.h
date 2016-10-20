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
	NODE_SHA512,
	NODE_WHIRLPOOL,
	NODE_MD5,
	NODE_SECP192K_PTP,
	NODE_SECP192K_PA,
	NODE_SECP192K_PS,
	NODE_SECP192K_PSM,
	NODE_SECP192K_PN,
	NODE_SECP192R_PTP,
	NODE_SECP192R_PA,
	NODE_SECP192R_PS,
	NODE_SECP192R_PSM,
	NODE_SECP192R_PN,
	NODE_SECP224K_PTP,
	NODE_SECP224K_PA,
	NODE_SECP224K_PS,
	NODE_SECP224K_PSM,
	NODE_SECP224K_PN,
	NODE_SECP224R_PTP,
	NODE_SECP224R_PA,
	NODE_SECP224R_PS,
	NODE_SECP224R_PSM,
	NODE_SECP224R_PN,
	NODE_SECP256K_PTP,
	NODE_SECP256K_PA,
	NODE_SECP256K_PS,
	NODE_SECP256K_PSM,
	NODE_SECP256K_PN,
	NODE_SECP256R_PTP,
	NODE_SECP256R_PA,
	NODE_SECP256R_PS,
	NODE_SECP256R_PSM,
	NODE_SECP256R_PN,
	NODE_SECP384R_PTP,
	NODE_SECP384R_PA,
	NODE_SECP384R_PS,
	NODE_SECP384R_PSM,
	NODE_SECP384R_PN,
	NODE_PRM192V1_PTP,
	NODE_PRM192V1_PA,
	NODE_PRM192V1_PS,
	NODE_PRM192V1_PSM,
	NODE_PRM192V1_PN,
	NODE_PRM192V2_PTP,
	NODE_PRM192V2_PA,
	NODE_PRM192V2_PS,
	NODE_PRM192V2_PSM,
	NODE_PRM192V2_PN,
	NODE_PRM192V3_PTP,
	NODE_PRM192V3_PA,
	NODE_PRM192V3_PS,
	NODE_PRM192V3_PSM,
	NODE_PRM192V3_PN,
	NODE_PRM256V1_PTP,
	NODE_PRM256V1_PA,
	NODE_PRM256V1_PS,
	NODE_PRM256V1_PSM,
	NODE_PRM256V1_PN,
	NODE_TIGER,
	NODE_RIPEMD160,
	NODE_RIPEMD128
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
	TOKEN_SHA512,
	TOKEN_WHIRLPOOL,
	TOKEN_MD5,
	TOKEN_SECP192K_PTP,
	TOKEN_SECP192K_PA,
	TOKEN_SECP192K_PS,
	TOKEN_SECP192K_PSM,
	TOKEN_SECP192K_PN,
	TOKEN_SECP192R_PTP,
	TOKEN_SECP192R_PA,
	TOKEN_SECP192R_PS,
	TOKEN_SECP192R_PSM,
	TOKEN_SECP192R_PN,
	TOKEN_SECP224K_PTP,
	TOKEN_SECP224K_PA,
	TOKEN_SECP224K_PS,
	TOKEN_SECP224K_PSM,
	TOKEN_SECP224K_PN,
	TOKEN_SECP224R_PTP,
	TOKEN_SECP224R_PA,
	TOKEN_SECP224R_PS,
	TOKEN_SECP224R_PSM,
	TOKEN_SECP224R_PN,
	TOKEN_SECP256K_PTP,
	TOKEN_SECP256K_PA,
	TOKEN_SECP256K_PS,
	TOKEN_SECP256K_PSM,
	TOKEN_SECP256K_PN,
	TOKEN_SECP256R_PTP,
	TOKEN_SECP256R_PA,
	TOKEN_SECP256R_PS,
	TOKEN_SECP256R_PSM,
	TOKEN_SECP256R_PN,
	TOKEN_SECP384R_PTP,
	TOKEN_SECP384R_PA,
	TOKEN_SECP384R_PS,
	TOKEN_SECP384R_PSM,
	TOKEN_SECP384R_PN,
	TOKEN_PRM192V1_PTP,
	TOKEN_PRM192V1_PA,
	TOKEN_PRM192V1_PS,
	TOKEN_PRM192V1_PSM,
	TOKEN_PRM192V1_PN,
	TOKEN_PRM192V2_PTP,
	TOKEN_PRM192V2_PA,
	TOKEN_PRM192V2_PS,
	TOKEN_PRM192V2_PSM,
	TOKEN_PRM192V2_PN,
	TOKEN_PRM192V3_PTP,
	TOKEN_PRM192V3_PA,
	TOKEN_PRM192V3_PS,
	TOKEN_PRM192V3_PSM,
	TOKEN_PRM192V3_PN,
	TOKEN_PRM256V1_PTP,
	TOKEN_PRM256V1_PA,
	TOKEN_PRM256V1_PS,
	TOKEN_PRM256V1_PSM,
	TOKEN_PRM256V1_PN,
	TOKEN_TIGER,
	TOKEN_RIPEMD160,
	TOKEN_RIPEMD128
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

extern bool init_token_list(SOURCE_TOKEN_LIST *token_list, size_t size);
static bool add_token(SOURCE_TOKEN_LIST *token_list, int token_id, char *literal, int line_num);
extern void delete_token_list(SOURCE_TOKEN_LIST *token_list);
static int validate_token_list(SOURCE_TOKEN_LIST *token_list);
extern bool get_token_list(char *str, SOURCE_TOKEN_LIST *token_list);
static void dump_token_list(SOURCE_TOKEN_LIST *token_list);

extern bool parse_token_list(SOURCE_TOKEN_LIST *token_list);
static bool create_exp(SOURCE_TOKEN *token, int token_num);
static NODE_TYPE get_node_type(SOURCE_TOKEN *token);
static bool validate_binary_exp(SOURCE_TOKEN *token, NODE_TYPE node_type);
static bool validate_unary_exp(SOURCE_TOKEN *token, int token_num, NODE_TYPE node_type);
static bool validate_binary_stmnt(SOURCE_TOKEN *token, NODE_TYPE node_type);
static bool validate_unary_stmnt(SOURCE_TOKEN *token, NODE_TYPE node_type);
static ast* pop_exp();
static void push_exp(ast* exp);
static int pop_op();
static void push_op(int token_id);
static ast* add_exp(NODE_TYPE node_type, TOKEN_EXP exp_type, long value, int token_num, int line_num, ast* left, ast* right);
extern char* get_node_str(NODE_TYPE node_type);
extern void dump_vm_ast(ast* root);

extern char* convert_ast_to_c();
static char* convert(ast* exp);
static char* append_strings(char * old, char * new);
static char *replace(char* old, char* a, char* b);

#endif // ELASTICPL_H_