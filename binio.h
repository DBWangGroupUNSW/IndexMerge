#ifndef __IO_H__
#define __IO_H__

#include "header.h"

extern int limit;                // limitation of length
extern int n;                      // number of records
extern int vector_id[STRING_NUM];  // record id in original txt file
extern int len[STRING_NUM];        // record length
extern int maxlen;                 // The max len in every token list.
extern int *token[STRING_NUM];     // record content
extern int freq[TOKEN_NUM];        // frequency of tokens with the document
extern int tokenNum;               // total number of tokens
extern int klen[STRING_NUM];       // topk number
extern int *topk[STRING_NUM];      // topk information
extern int *topkov[STRING_NUM];    // top k over lapping
extern char *tokstr[STRING_NUM];   // token string
extern int tokstrlen[STRING_NUM];  // token string len



void readTokenBinary(char *filename);

void outputTokenBinary(char *filename);


#endif
