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

/*****************************************************************************
	ElasticPL Token List

	Format:  Str, Len, Type, Exp, Prec

	Str:	Token String
	Len:	String Length Used For "memcmp"
	Type:	Enumerated Token Type
	Exp:	Enumerated Num Of Expressions To Link To Node
	Prec:	(Precedence) Determines Parsing Order
******************************************************************************/
struct EPL_TOKEN_LIST epl_token[] = {
	{ "<eof>",	5,	TOKEN_EOF,			EXP_NONE,				0 },
	{ "{",		1,	TOKEN_BLOCK_BEGIN,	BINARY_STATEMENT,		1 },
	{ "}",		1,	TOKEN_BLOCK_END,	BINARY_STATEMENT,		1 },
	{ "(",		1,	TOKEN_OPEN_PAREN,	EXP_NONE,				1 },
	{ ")",		1,	TOKEN_CLOSE_PAREN,	EXP_NONE,				1 },
	{ ";",		1,	TOKEN_END_STATEMENT,EXP_NONE,				2 },
	{ "verify",	6,	TOKEN_VERIFY,		UNARY_STATEMENT,		2 },
	{ "repeat",	6,	TOKEN_REPEAT,		BINARY_STATEMENT,		2 },
	{ "if",		2,	TOKEN_IF,			BINARY_STATEMENT,		2 },
	{ "else",	4,	TOKEN_ELSE,			BINARY_STATEMENT,		2 },

	{ "m[",		2,	TOKEN_VAR_BEGIN,	EXP_NONE,				4 },
	{ "]",		1,	TOKEN_VAR_END,		UNARY_EXPRESSION,		4 },

	{ "*",		1,	TOKEN_MUL,			BINARY_EXPRESSION,		14 },	// Multiplicative
	{ "/",		1,	TOKEN_DIV,			BINARY_EXPRESSION,		14 },	// Multiplicative
	{ "%",		1,	TOKEN_MOD,			BINARY_EXPRESSION,		14 },	// Multiplicative

	{ "+",		1,	TOKEN_ADD,			BINARY_EXPRESSION,		13 },	// Additive
	{ "+",		1,	TOKEN_POS,			UNARY_EXPRESSION,		15 },	// Additive
	{ "-",		1,	TOKEN_SUB,			BINARY_EXPRESSION,		13 },	// Additive
	{ "-",		1,	TOKEN_NEG,			UNARY_EXPRESSION,		15 },	// Additive

	{ "<<<",	3,	TOKEN_LROT,			BINARY_EXPRESSION,		12 },	// Shift
	{ "<<",		2,	TOKEN_LSHIFT,		BINARY_EXPRESSION,		12 },	// Shift
	{ ">>>",	3,	TOKEN_RROT,			BINARY_EXPRESSION,		12 },	// Shift
	{ ">>",		2,	TOKEN_RSHIFT,		BINARY_EXPRESSION,		12 },	// Shift

	{ "<=",		2,	TOKEN_LE,			BINARY_EXPRESSION,		11 },	// Relational
	{ ">=",		2,	TOKEN_GE,			BINARY_EXPRESSION,		11 },	// Relational
	{ "<",		1,	TOKEN_LT,			BINARY_EXPRESSION,		11 },	// Relational
	{ ">",		1,	TOKEN_GT,			BINARY_EXPRESSION,		11 },	// Relational

	{ "==",		2,	TOKEN_EQ,			BINARY_EXPRESSION,		10 },	// Equality
	{ "!=",		2,	TOKEN_NE,			BINARY_EXPRESSION,		10 },	// Equality

	{ "&&",		2,	TOKEN_AND,			BINARY_EXPRESSION,		6 },	// Logical AND
	{ "||",		2,	TOKEN_OR,			BINARY_EXPRESSION,		5 },	// Logical OR

	{ "&",		1,	TOKEN_BITWISE_AND,	BINARY_EXPRESSION,		9 },	// Bitwise AND
	{ "and",	3,	TOKEN_BITWISE_AND,	BINARY_EXPRESSION,		9 },	// Bitwise AND
	{ "^",		1,	TOKEN_BITWISE_XOR,	BINARY_EXPRESSION,		8 },	// Bitwise XOR
	{ "xor",	3,	TOKEN_BITWISE_XOR,	BINARY_EXPRESSION,		8 },	// Bitwise XOR
	{ "|",		1,	TOKEN_BITWISE_OR,	BINARY_EXPRESSION,		7 },	// Bitwise OR
	{ "or",		2,	TOKEN_BITWISE_OR,	BINARY_EXPRESSION,		7 },	// Bitwise OR

	{ "=",		1,	TOKEN_ASSIGN,		BINARY_STMNT_EXP,		3 },	// Assignment

	{ "~",		1,	TOKEN_COMPL,		UNARY_EXPRESSION,		15 },	// Unary Operator
	{ "!",		1,	TOKEN_NOT,			UNARY_EXPRESSION,		15 },	// Unary Operator
	{ "true",	4,	TOKEN_TRUE,			UNARY_EXPRESSION,		15 },	// Unary Operator
	{ "false",	5,	TOKEN_FALSE,		UNARY_EXPRESSION,		15 },	// Unary Operator

	{ "input",	5,	TOKEN_INPUT,		UNARY_STATEMENT,		60 },	// 
	{ "sha256",	6,	TOKEN_SHA256,		BINARY_STATEMENT,		60 }	// Built In Functions
};

extern bool init_token_list(SOURCE_TOKEN_LIST *token_list, size_t size) {
	token_list->token = malloc(size * sizeof(SOURCE_TOKEN));
	token_list->num = 0;
	token_list->size = size;

	if (!token_list->token)
		return false;
	else
		return true;
}

static bool add_token(SOURCE_TOKEN_LIST *token_list, int token_id, char *literal, int line_num) {
		char *str;

	// Increase Token List Size If Needed
	if (token_list->num == token_list->size) {
		token_list->size += 256;
		token_list->token = (SOURCE_TOKEN *)realloc(token_list->token, token_list->size * sizeof(SOURCE_TOKEN));

		if (!token_list->token)
			return false;
	}

	// EPL Tokens
	if (token_id >= 0) {

		// Determine If + And - Binary Or Unary
		if (epl_token[token_id].type == TOKEN_ADD || epl_token[token_id].type == TOKEN_SUB) {
			if (token_list->num == 0 || (token_list->token[token_list->num - 1].exp != UNARY_EXPRESSION && token_list->token[token_list->num - 1].type != TOKEN_CLOSE_PAREN)) {
				token_id++;
			}
		}

		token_list->token[token_list->num].type = epl_token[token_id].type;
		token_list->token[token_list->num].exp = epl_token[token_id].exp;
		token_list->token[token_list->num].literal = NULL;
		token_list->token[token_list->num].prec = epl_token[token_id].prec;
	}
	// Literals
	else if (literal != NULL) {
		str = calloc(1, strlen(literal) + 1);

		if (!str) return false;

		strcpy(str, literal);
		token_list->token[token_list->num].literal = str;
		token_list->token[token_list->num].type = TOKEN_LITERAL;
		token_list->token[token_list->num].exp = UNARY_EXPRESSION;
		token_list->token[token_list->num].prec = -1;
	}
	// Error
	else {
		return false;
	}

	token_list->token[token_list->num].token_id = token_id;
	token_list->token[token_list->num].line_num = line_num;
	token_list->num++;

	return true;
}

extern void delete_token_list(SOURCE_TOKEN_LIST *token_list) {
	int i;

	for (i = 0; i < token_list->num; i++) {
		if (token_list->token[i].literal)
			free(token_list->token[i].literal);
	}

	free(token_list->token);
	token_list->token = NULL;
	token_list->num = 0;
	token_list->size = 0;
}

static int validate_token_list(SOURCE_TOKEN_LIST *token_list) {
	int i, j, len;
	char c;

	for (i = 0; i < token_list->num; i++) {

		// Validate That All Literals Are Numeric
		if (token_list->token[i].type == TOKEN_LITERAL) {

			len = strlen(token_list->token[i].literal);

			for (j = 0; j < len; j++) {
				c = token_list->token[i].literal[j];
				if (!(c >= '0' && c <= '9') && c != '.' && c != '-')
					return i;
			}
		}
	}

	return -1;
}

extern bool get_token_list(char *str, SOURCE_TOKEN_LIST *token_list) {
	char c, literal[100];
	int i, idx, len, token_id, line_num, token_list_sz, literal_idx;

	token_list_sz = sizeof(epl_token) / sizeof(epl_token[0]);

	len = strlen(str);

	idx = 0;
	line_num = 1;
	literal_idx = 0;
	memset(literal, 0, 100);

	while (idx < len) {
		token_id = -1;
		c = str[idx];

		// Remove Whitespace
		if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f') {

			if (literal_idx > 0) {
				add_token(token_list, -1, literal, line_num);
				literal_idx = 0;
				memset(literal, 0, 100);
			}

			// Increment Line Number Counter
			if (c == '\r') {
				line_num++;
			}

			idx++;
			continue;
		}

		// Check For EPL Token
		for (i = 0; i < token_list_sz; i++) {

			if (memcmp(&str[idx], epl_token[i].str, epl_token[i].len) == 0) {
				token_id = i;
				break;
			}

		}

		if (token_id >= 0) {

			if (literal_idx > 0) {
				add_token(token_list, -1, literal, line_num);
				literal_idx = 0;
				memset(literal, 0, 100);
			}

			add_token(token_list, token_id, NULL, line_num);
			idx += epl_token[token_id].len;
		}
		else {
			literal[literal_idx] = c;
			literal_idx++;
			idx++;
		}
	}

//	dump_token_list(token_list);

	idx = validate_token_list(token_list);
	
	if (idx >= 0) {
		applog(LOG_ERR, "Syntax Error - Line: %d  Invalid Operator \"%s\"", token_list->token[idx].line_num, token_list->token[idx].literal);
		return false;
	}

	return true;
}


// Temporary - For Debugging Only
static void dump_token_list(SOURCE_TOKEN_LIST *token_list)
{
	int i;

	fprintf(stdout, "\nNum\tLine\tToken\tToken ID\n");
	printf("--------------------------------\n");
	for (i = 0; i < token_list->num; i++) {
		if (token_list->token[i].type == TOKEN_LITERAL)
			fprintf(stdout, "%d:\t%d\t\"%s\"\t%d\n", i, token_list->token[i].line_num, token_list->token[i].literal, token_list->token[i].type);
		else
			fprintf(stdout, "%d:\t%d\t%s\t%d\n", i, token_list->token[i].line_num, epl_token[token_list->token[i].token_id].str, token_list->token[i].type);
	}
}
