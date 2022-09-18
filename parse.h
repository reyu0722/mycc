#pragma once

typedef enum
{
	EXT_FUNC,
	EXT_FUNCDECL,
	EXT_GVAR,
	EXT_ENUM,
	EXT_STRUCT,
	EXT_TYPEDEF,
} ExternalKind;

typedef struct External External;
struct External
{
	ExternalKind kind;
	String *name;
	Node *code[100];
	int offsets[6];
	int size;
	StringLiteral *literals;
	int stack_size;
	_Bool is_variadic;
};

_Bool at_eof();
External *external();
