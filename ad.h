#ifndef _AD_H
#define _AD_H

#include "vm.h"

// the domain analysis

struct Symbol;typedef struct Symbol Symbol;

typedef enum{		// base type
	TB_INT,TB_DOUBLE,TB_CHAR,TB_VOID,TB_STRUCT
	}TypeBase;

typedef struct{		// the type of a symbol
	TypeBase tb;
	Symbol *s;		// for TB_STRUCT, the struct's symbol
	int n;          // array dimension
	}Type;

// returns the size of type t in bytes
int typeSize(Type *t);

typedef enum{		// symbol's kind
	SK_VAR,SK_PARAM,SK_FN,SK_STRUCT
	}SymKind;

struct Symbol{
	const char *name;
	SymKind kind;
	Type type;
	Symbol *owner;
	Symbol *next;
	union{
		int varIdx;
		void *varMem;
		int paramIdx;
		Symbol *structMembers;
		struct{
			Symbol *params;
			Symbol *locals;
			void(*extFnPtr)();
			Instr *instr;
			}fn;
		};
	};

typedef struct Domain {
    Symbol *symbols;
    struct Domain *parent;
} Domain;

extern Domain *symTable;

void freeSymbols(Symbol *list);
Symbol *newSymbol(const char *name, SymKind kind);
Symbol *dupSymbol(Symbol *symbol);
Symbol *addSymbolToList(Symbol **list, Symbol *s);
int symbolsLen(Symbol *list);
void freeSymbol(Symbol *s);
Domain *pushDomain();
void dropDomain();
void showNamedType(Type *t, const char *name);
void showSymbol(Symbol *s);
void showDomain(Domain *d, const char *name);
Symbol *findSymbolInDomain(Domain *d, const char *name);
Symbol *findSymbol(const char *name);
Symbol *addSymbolToDomain(Domain *d, Symbol *s);
Symbol *addExtFn(const char *name, void(*extFnPtr)(), Type ret);
Symbol *addFnParam(Symbol *fn, const char *name, Type type);
int typeBaseSize(Type *t);
int typeSize(Type *t);
void printAllSymbols();

#endif