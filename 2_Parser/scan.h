/****************************************************/
/* File: scan.h                                     */
/* The scanner interface for the TINY compiler      */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SCAN_H_
#define _SCAN_H_

/* MAXTOKENLEN is the maximum size of a token */
#define MAXRECURSION 2000
#define MAXTOKENLEN 40

/* tokenString array stores the lexeme of each token */
extern char tokenString[2][MAXTOKENLEN+1];
extern char* savedNameStack[MAXRECURSION];
extern int savedLineNoStack[MAXRECURSION];
extern int curTokenPos;
extern int curStackTop;
extern int curLineTop;

/* function getToken returns the 
 * next token in source file
 */
TokenType getToken(void);

#endif
