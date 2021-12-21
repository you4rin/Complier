/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "util.h"

/* the hash function */
int hash(char * key)
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

/* the list of line numbers of the source 
 * code in which a variable is referenced
 */


/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert(ScopeList scope, char * name, Type type, int lineno, int loc){ 
  int h = hash(name);
  ScopeList curScope = scope;
  BucketList l;
  if(loc>=0){
    l = scope->hashTable[h];
    while ((l != NULL) && (strcmp(name,l->name) != 0))
      l = l->next;
  }
  else{
    while(curScope != NULL){
      l = curScope->hashTable[h];
      while((l != NULL) && (strcmp(name,l->name) != 0))
        l = l->next;
      if(l != NULL)
        break;
      curScope = curScope->p;
    }
  }
  if (l == NULL) /* variable not yet in table */
  { 
    l = (BucketList) malloc(sizeof(struct BucketListRec));
    memset(l, 0, sizeof(struct BucketListRec));
    l->scope = copyString(scope->scope);
    l->name = copyString(name);
    l->type = type;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    memset(l->lines, 0, sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
    l->next = scope->hashTable[h];
    scope->hashTable[h] = l; }
  else /* found in table, so just add line number */
  { LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    memset(t->next, 0, sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
} /* st_insert */

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
BucketList st_lookup(ScopeList scope, char * name){
  int h = hash(name);
  while(scope != NULL){
    BucketList l = scope->hashTable[h];
    while ((l != NULL) && (strcmp(name,l->name) != 0))
      l = l->next;
    if (l != NULL)
      return l;
    scope = scope->p;
  }
  return NULL;
}

BucketList st_lookup_now(ScopeList scope, char * name){
  int h = hash(name);
  BucketList l = scope->hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) return NULL;
  else return l;
}

ScopeList st_get_scope(char* scope){
  for(int i=0;i<tree->childcnt;++i){
    if(strcmp(tree->child[i]->scope, scope) == 0)
      return tree->child[i];
  }
  return NULL;
}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */

void printSymtab(FILE* listing, ScopeList scope){
  for(int i=0;i<SIZE;++i)
  { if (scope->hashTable[i] != NULL)
    { BucketList l = scope->hashTable[i];
      while (l != NULL)
      { LineList t = l->lines;
        fprintf(listing,"%-13s  ",l->name);
        switch(l->type.sym){
          case Function:
            fprintf(listing,"Function     ");
            break;
          default:
            fprintf(listing,"Variable     ");
            break;
        }
        switch(l->type.ret){
          case Void:
            fprintf(listing,"void           ");
            break;
          case Integer:
            fprintf(listing,"int            ");
            break;
          case VoidPtr:
            fprintf(listing,"void[]         ");
            break;
          case IntegerPtr:
            fprintf(listing,"int[]          ");
            break;
        }
        fprintf(listing,"%-12s  ",l->scope);
        fprintf(listing,"%-8d  ",l->memloc);
        while (t != NULL)
        { fprintf(listing,"%4d ",t->lineno);
          t = t->next;
        }
        fprintf(listing,"\n");
        l = l->next;
      }
    }
  }
  if(scope == tree)
    fprintf(listing, "value          Variable     int            output        0            0\n");
  for(int i=0;i<scope->childcnt;++i){
    printSymtab(listing, scope->child[i]);
  }
}

void printFunctab(FILE* listing, ScopeList scope){
  for(int i=0;i<SIZE;++i){ 
    if (scope->hashTable[i] != NULL)
    { BucketList l = scope->hashTable[i];
      while (l != NULL)
      { 
        switch(l->type.sym){
          case Function:
            fprintf(listing,"%-13s  ", l->name);
            break;
          default:
            l = l->next;
            continue;
        }
        switch(l->type.ret){
          case Void:
            fprintf(listing,"void           ");
            break;
          case Integer:
            fprintf(listing,"int            ");
            break;
          case VoidPtr:
            fprintf(listing,"void[]         ");
            break;
          case IntegerPtr:
            fprintf(listing,"int[]          ");
            break;
        }
        if(l->type.argcnt == 0)
          fprintf(listing, "                void");
        fprintf(listing, "\n");
        if(strcmp(l->name, "input") == 0){
          l = l->next;
          continue;
        }
        else if(strcmp(l->name, "output") == 0){
          fprintf(listing, "-              -              value           int\n");
          l = l->next;
          continue;
        }
        ScopeList funcscope = NULL;
        for(int j=0;j<scope->childcnt;++j){
          //printf("%s %s\n", scope->child[j]->scope, l->name);
          if(strcmp(scope->child[j]->scope, l->name) == 0){
            funcscope = scope->child[j];
            break;
          }
        }
        if(funcscope == NULL) // should not happen
          fprintf(listing, "Failed to find scope %s\n", l->name);
        if(strcmp(l->name, "output") != 0){
          for(int j=0;j<SIZE;++j){
            if (funcscope->hashTable[j] != NULL){
              BucketList nl = funcscope->hashTable[j];
              while(nl != NULL){
                if(nl->type.sym != Argument){
                  nl = nl->next;
                  continue;
                }
                fprintf(listing, "-              -              ");
                fprintf(listing, "%-14s  ", nl->name);
                switch(nl->type.ret){
                  case Void:
                    fprintf(listing,"void          \n");
                    break;
                  case Integer:
                    fprintf(listing,"int           \n");
                    break;
                  case VoidPtr:
                     fprintf(listing,"void[]        \n");
                     break;
                  case IntegerPtr:
                    fprintf(listing,"int[]         \n");
                    break;
                }
                nl = nl->next;
              }
            }
          }
        }
        l = l->next;
      }
    }
  }
}

void printGlobtab(FILE* listing, ScopeList scope){
  for(int i=0;i<SIZE;++i)
  { if (scope->hashTable[i] != NULL)
    { BucketList l = scope->hashTable[i];
      while (l != NULL)
      { 
        fprintf(listing, "%-13s  ", l->name);
        switch(l->type.sym){
          case Function:
            fprintf(listing,"Function     ");
            break;
          default:
            fprintf(listing,"Variable     ");
            break;
        }
        switch(l->type.ret){
          case Void:
            fprintf(listing,"void           \n");
            break;
          case Integer:
            fprintf(listing,"int            \n");
            break;
          case VoidPtr:
            fprintf(listing,"void[]         \n");
            break;
          case IntegerPtr:
            fprintf(listing,"int[]          \n");
            break;
        }
        
        l = l->next;
      }
    }
  }
}

void printScopetab(FILE* listing, ScopeList scope, int depth){
  int exist = 0;
  if(depth != 0){
    for(int i=0;i<SIZE;++i)
    { if (scope->hashTable[i] != NULL)
      { BucketList l = scope->hashTable[i];
        while (l != NULL)
        { 
          exist = 1;
          fprintf(listing, "%-12s  ", scope->scope);
          fprintf(listing, "%-12d  ", depth);
          fprintf(listing, "%-13s  ", l->name);
          switch(l->type.ret){
            case Void:
              fprintf(listing,"void           \n");
              break;
            case Integer:
              fprintf(listing,"int            \n");
              break;
            case VoidPtr:
              fprintf(listing,"void[]         \n");
              break;
            case IntegerPtr:
              fprintf(listing,"int[]          \n");
              break;
          }
          
          l = l->next;
        }
      }
    }
  }
  else
    exist = 1;
  //printf("childs: %d\n",scope->childcnt);
  if(exist)
    fprintf(listing, "\n");
  for(int i=0;i<scope->childcnt;++i)
    printScopetab(listing, scope->child[i], depth + 1);
}

void printTables(FILE* listing)
{ 
  fprintf(listing,"\n\n< Symbol Table >\n");
  fprintf(listing," Symbol Name   Symbol Kind   Symbol Type    Scope Name   Location  Line Numbers\n");
  fprintf(listing,"-------------  -----------  -------------  ------------  --------  ------------\n");
  printSymtab(listing, tree);
  fprintf(listing,"\n\n< Functions >\n");
  fprintf(listing,"Function Name   Return Type   Parameter Name  Parameter Type\n");
  fprintf(listing,"-------------  -------------  --------------  --------------\n");
  printFunctab(listing, tree);
  fprintf(listing,"\n\n< Global Symbols >\n");
  fprintf(listing," Symbol Name   Symbol Kind   Symbol Type\n");
  fprintf(listing,"-------------  -----------  -------------\n");
  printGlobtab(listing, tree);
  fprintf(listing,"\n\n< Scopes >\n");
  fprintf(listing," Scope Name   Nested Level   Symbol Name   Symbol Type\n");
  fprintf(listing,"------------  ------------  -------------  -----------\n");
  fprintf(listing,"output        1             value          int        \n");
  printScopetab(listing, tree, 0);
} 
