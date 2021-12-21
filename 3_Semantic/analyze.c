/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <string.h>
#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

/* counter for variable memory locations */
int function_decl;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */

static void semanticError(TreeNode * t, char * message)
{ fprintf(listing,"Semantic error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

static void traverse(ScopeList scope, TreeNode * t, int build,
               void (* preProc)(ScopeList, TreeNode*),
               void (* postProc)(ScopeList, TreeNode*) )
{ if (t != NULL)
  { 
    ScopeList newScope;
    if(t->nodekind == StmtK && t->kind.stmt == FunDeclK){
      function_decl = 1;
      if(build){
        newScope = (ScopeList)malloc(sizeof(struct ScopeListRec));
        memset(newScope, 0, sizeof(struct ScopeListRec));
        newScope->scope = copyString(t->attr.name);
        newScope->p = scope;
        scope->child[scope->childcnt++] = newScope;
      }
      else{
        newScope = scope->child[scope->visit++];
        newScope->visit = 0;
      }
    }
    else if(t->nodekind == StmtK && t->kind.stmt == CompoundK){
      if(function_decl)
        function_decl = 0;
      else if(build){
        newScope = (ScopeList)malloc(sizeof(struct ScopeListRec));
        memset(newScope, 0, sizeof(struct ScopeListRec));
        newScope->scope = copyString(scope->scope);
        newScope->p = scope;
        scope->child[scope->childcnt++] = newScope;
      }
      else{
        newScope = scope->child[scope->visit++];
        newScope->visit = 0;
      }
    }
    else
      newScope = scope;
    //printf("scope: %s, newscope: %s\n", scope->scope, newScope->scope);
    //if(scope->p != NULL)printf("pscope: %s, ",scope->p->scope);
    //else printf("pscope: NULL, ");
    //if(newScope->p != NULL)printf("npscope: %s\n",newScope->p->scope);
    //else printf("npscope: NULL\n");
    preProc(newScope, t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(newScope, t->child[i], build, preProc, postProc);
    }
    postProc(newScope, t);
    traverse(scope, t->sibling, build, preProc, postProc);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(ScopeList scope, TreeNode * t)
{ if (t==NULL) return;
  else return;
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode(ScopeList scope, TreeNode * t)
{ 
  Type type;
  memset(&type, 0, sizeof(Type));
  switch (t->nodekind)
  { case StmtK:
      switch(t->kind.stmt)
      { case FunDeclK:
          if(st_lookup_now(tree, t->attr.name) != NULL)
            semanticError(t, "Redefinition Error");
          else{
            type.sym = Function;
            type.ret = t->type;
            TreeNode* arglist = t->child[0];
            while(arglist != NULL){
              type.args[type.argcnt++] = arglist->type;
              arglist = arglist->sibling;
            }
            if(type.argcnt == 1 && type.args[0] == Void)
              type.argcnt = 0; // Void Parameter
            st_insert(tree, t->attr.name, type, t->lineno, tree->location++);
          }
          break;
        case VarDeclK:
          if(st_lookup_now(scope, t->attr.name) != NULL)
            semanticError(t, "Redefinition Error");
          else{
            type.sym = Variable;
            type.ret = t->type;
            st_insert(scope, t->attr.name, type, t->lineno, scope->location++);
          }
          break;
        case ParamK:
          if(st_lookup_now(scope, t->attr.name) != NULL)
            semanticError(t, "Redefinition Error");
          else{
            type.sym = Argument;
            type.ret = t->type;
            //printf("param name: %s\n", t->attr.name);
            st_insert(scope, t->attr.name, type, t->lineno, scope->location++);
          }
          break;
        case CallK:
          //printf("lineno: %d, ident: %s\n",t->lineno, t->attr.name);
          if(st_lookup(scope, t->attr.name) == NULL)
            semanticError(t, "No Such Identifier");
          else
            st_insert(tree, t->attr.name, type, t->lineno, -1);
          break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case IdK:
          //printf("lineno: %d, ident: %s\n",t->lineno, t->attr.name);
          if(st_lookup(scope, t->attr.name) == NULL)
            semanticError(t, "No Such Identifier");
          else
            st_insert(scope, t->attr.name, type, t->lineno, -1);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

void symtabInit(){
  tree = (ScopeList)malloc(sizeof(struct ScopeListRec));
  memset(tree, 0, sizeof(struct ScopeListRec));
  tree->scope = (char*)malloc(sizeof(char)*7);
  strcpy(tree->scope, "global");

  Type type;
  memset(&type, 0, sizeof(Type));
  type.sym = Function;

  type.argcnt = 0;
  type.args[0] = Void;
  type.ret = Integer;
  int h = hash("input");
  BucketList l = tree->hashTable[h];
  l = (BucketList) malloc(sizeof(struct BucketListRec));
  memset(l, 0, sizeof(struct BucketListRec));
  l->scope = copyString(tree->scope);
  l->name = copyString("input");
  l->lines = (LineList) malloc(sizeof(struct LineListRec));
  l->type = type;
  memset(l->lines, 0, sizeof(struct LineListRec));
  l->lines->lineno = 0;
  l->memloc = tree->location++;
  l->lines->next = NULL;
  l->next = tree->hashTable[h];
  tree->hashTable[h] = l;
  
  // assert that hash("input") != hash("output")

  type.argcnt = 1;
  type.args[0] = Integer;
  type.ret = Void;
  h = hash("output");
  l = tree->hashTable[h];
  l = (BucketList) malloc(sizeof(struct BucketListRec));
  memset(l, 0, sizeof(struct BucketListRec));
  l->scope = copyString(tree->scope);
  l->name = copyString("output");
  l->lines = (LineList) malloc(sizeof(struct LineListRec));
  l->type = type;
  memset(l->lines, 0, sizeof(struct LineListRec));
  l->lines->lineno = 0;
  l->memloc = tree->location++;
  l->lines->next = NULL;
  l->next = tree->hashTable[h];
  tree->hashTable[h] = l;
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode* syntaxTree){
  symtabInit();
  traverse(tree, syntaxTree, 1, insertNode, nullProc);
  if(TraceAnalyze)
    printTables(listing);
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(ScopeList scope, TreeNode* t)
{ 
  BucketList l;
  switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { case OpK:
          if ((t->child[0]->type != Integer) ||
              (t->child[1]->type != Integer))
            semanticError(t,"Type Error: Non-Integer in Operation");
          t->type = Integer;
          break;
        case ConstK:
          t->type = Integer;
          break;
        case IdK:
          l = st_lookup(scope, t->attr.name);
          if(l == NULL){
            t->type = Integer;
            break;
          }
          if (l->type.sym == Function)
            semanticError(t,"Type Error: Ambiguous Definition");
          t->type = l->type.ret;
          if(t->child[0] != NULL){
            if(t->child[0]->type != Integer)
              semanticError(t,"Type Error: Invalid Array Access");
            if(t->type != IntegerPtr)
              semanticError(t,"Type Error: Invalid Array Access");
            t->type = Integer;
          }
          break;
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { case IfK:
        case IfElseK:
        case WhileK:
          if (t->child[0]->type != Integer)
            semanticError(t,"Type Error: Condition is not Integer");
          t->type = Void;
          break;
        case AssignK:
          //printf("%d %d\n",t->child[0]->type, t->child[1]->type);
          if (t->child[0]->type != t->child[1]->type)
            semanticError(t,"Type Error: Assigning Incompatible Value");
          t->type = t->child[0]->type;
          break;
        case ReturnK:
          l = st_lookup(tree, scope->scope);
          if(l == NULL){
            t->type = Void;
            break;
          }
          if(l->type.ret != t->child[0]->type)
            semanticError(t,"Type Error: Incompatible Return Type");
          t->type = Void;
          break;
        case NReturnK:
          l = st_lookup(tree, scope->scope);
          if(l == NULL){
            t->type = Void;
            break;
          }
          if(l->type.ret != Void)
            semanticError(t,"Type Error: Returning Value in Void Function");
          t->type = Void;
          break;
        case VarDeclK:
          if(t->type == Void || t->type == VoidPtr)
            semanticError(t,"Type Error: Void Type");
          t->type = Void;
          break;
        case FunDeclK:
        case CompoundK:
        case VoidParamK:
          t->type = Void;
          break;
        case ParamK:
          if(t->type == Void || t->type == VoidPtr)
            semanticError(t,"Type Error: Void Type in Non-Void Function");
          t->type = Void;
          break;
        case CallK:
          l = st_lookup(tree, t->attr.name);
          if(l == NULL){
            t->type = Integer;
            break;
          }
          TreeNode* sib = t->child[0];
          for(int i=0;i<l->type.argcnt;++i){
            if(sib == NULL){
              semanticError(t,"Type Error: Incompatible Arg Num");
              break;
            }
            if(sib->type != l->type.args[i])
              semanticError(t,"Type Error: Incompatible Arg Type");
            sib = sib->sibling;
          }
          if(sib != NULL)
            semanticError(t,"Type Error: Incompatible Arg Num");
          t->type = l->type.ret;
          break;
        default:
          break;
      }
      break;
    default:
      break;

  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(tree, syntaxTree, 0, nullProc, checkNode);
}
