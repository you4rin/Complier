/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

/* SIZE is the size of the hash table */
#define SIZE 211

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */

typedef struct LineListRec
   { int lineno;
     struct LineListRec * next;
   } * LineList;

/* The record in the bucket lists for
 * each variable, including name, 
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */

typedef enum {Function, Variable, Argument} SymbolType;

typedef struct Type{
    SymbolType sym;
    int argcnt; // only for function. 0 if void function
    ExpType args[SIZE]; 
    ExpType ret; // ret is type of Variable/Argument;
}Type;

typedef struct BucketListRec{ 
    char* scope;
    char* name;
    Type type;
    LineList lines;
    int memloc ; /* memory location for variable */
    struct BucketListRec * next;
}* BucketList;

/* the hash table */

typedef struct ScopeListRec{
    char* scope;
    BucketList hashTable[SIZE];
    struct ScopeListRec* p;
    int childcnt;
    struct ScopeListRec* child[SIZE];
    int location;
    int visit;
}* ScopeList;

ScopeList tree;

int hash(char * key);

void st_insert(ScopeList scope, char * name, Type type, int lineno, int loc);

/* Function st_lookup returns the memory 
 * location of a variable or NULL if not found
 */
BucketList st_lookup(ScopeList scope, char * name);

BucketList st_lookup_now(ScopeList scope, char * name);

ScopeList st_get_scope(char* scope);

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printTables(FILE * listing);

#endif
