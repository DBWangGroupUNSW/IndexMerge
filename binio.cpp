#include "header.h"

extern int verifyAccess;

int limit=0; // limitation of length
int n; //number of records
int vector_id[STRING_NUM]; //record id in original txt file
int len[STRING_NUM]; //record length
int maxlen;          // The max len in every token list.
int *token[STRING_NUM]; //record content
int freq[TOKEN_NUM]; //frequency of tokens with the document
int tokenNum; //total number of tokens
int klen[STRING_NUM]; // topk number
int *topk[STRING_NUM]; //topk information
int *topkov[STRING_NUM]; //top k over lapping
char *tokstr[STRING_NUM]; // 
int tokstrlen[STRING_NUM];
double alpha; //threshold (jaccard/cosine)


void readTokenBinary(char *filename)
{
	int i;
	int totalLen = 0;
	FILE *fp;
	fp = fopen(filename, "rb");
	tokenNum = -1, n = 0;
    maxlen = 0;
	while (fread(vector_id + n, sizeof(int), 1, fp) == 1) {
        fread(tokstrlen + n, sizeof(int), 1, fp);
		fread(len + n, sizeof(int), 1, fp);
        if (len[n]>maxlen) maxlen = len[n];
        fread(klen + n, sizeof(int), 1, fp);
        if ( klen[n] == 0 ){
            topk[n] = NULL;
            topkov[n] = NULL;
        }else{
            topk[n] = new int[klen[n]];
            topkov[n] = new int[klen[n]];
        }
        tokstr[n] = new char[tokstrlen[n]];
        fread(tokstr[n] , sizeof(char), tokstrlen[n], fp);
		totalLen += len[n];
		token[n] = new int[len[n]];
		for (i = 0; i < len[n]; i++) {
			fread(token[n] + i, sizeof(int), 1, fp);
			if (token[n][i] > tokenNum) tokenNum = token[n][i];
		}
        
        for (i = 0; i < klen[n]; i++) {
            fread(topk[n] + i, sizeof(int), 1, fp);
            fread(topkov[n] + i, sizeof(int), 1, fp);
        }
        if ( len[n] < limit ){
            if(topk[n] != NULL) {delete[] topk[n];topk[n]=NULL;}
            if(topkov[n] != NULL) {delete[] topkov[n];topk[n]=NULL;}
            if(tokstr[n] != NULL) {delete[] tokstr[n];tokstr[n]=NULL;}
            if(token[n] != NULL) {delete[] token[n];token[n]=NULL;}            
        }else{
            ++n;
        }
	}
	fclose(fp);
	++tokenNum;
	std::cerr << "# Records: " << n << std::endl;	
	std::cerr << "# Average Size: " << double(totalLen) / n << std::endl;
}

void outputTokenBinary(char *filename)
{
	int i,m;
	FILE *fp;
	fp = fopen(filename, "wb");
    
    for ( m = 0; m < n; m ++){
        fwrite (vector_id + m, sizeof(int), 1, fp);
        fwrite (tokstrlen + m, sizeof(int), 1, fp);
        fwrite (len + m, sizeof(int), 1, fp);
        fwrite (klen + m, sizeof(int), 1, fp);
        
        fwrite (tokstr[m], sizeof(char), tokstrlen[m], fp);

		for (i = 0; i < len[m]; i++) {
			fwrite(token[m] + i, sizeof(int), 1, fp);
		}
        
        for (i = 0; i < klen[m]; i++) {
            fwrite(topk[m] + i, sizeof(int), 1, fp);
            fwrite(topkov[m] + i, sizeof(int), 1, fp);
        }
	}
	fclose(fp);
}
