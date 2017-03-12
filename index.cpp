#include <math.h>
#include "header.h"
#include "index.h"


extern int n; //number of records
extern int tokenNum; //total number of tokens

extern int *token[STRING_NUM]; //record content
extern int len[STRING_NUM]; //record length
extern int freq[TOKEN_NUM]; //frequency of token within the document

extern int vector_id[STRING_NUM]; //record id within original txt file

elem_index* list[TOKEN_NUM]; //inverted lists
int indexElemNum, indexTokenNum; //stats for inverted lists
int indexStart[ELEM_NUM], indexEnd[ELEM_NUM]; //start/end pointer for inverted lists



//generating inverted lists
void generate_index()
{
  int i, j, tok;
  indexElemNum = 0;
  
  memset(freq, 0, tokenNum * sizeof(int));
  for (i = 0; i < n; i++) {	
	for (j = 0; j < len[i]; j++) 
	  ++freq[token[i][j]];
  }
  
  //inserting into inverted lists
  for (i = 0; i < tokenNum; i++)
	if (freq[i] >= FREQ_LIMIT) //FREQ_LIMIT is 2 here to avoid inserting widow tokens
	  list[i] = new elem_index[freq[i]];

  memset(indexStart, -1, tokenNum * sizeof(int));
  memset(indexEnd, -1, tokenNum * sizeof(int));


  // Need to indexing in a reverse order.
  // Log changes made in version 2 by jianbin Qin.
  for (i = n-1; i >= 0; i--) 
	for (j = 0; j < len[i]; j++) {
	  tok = token[i][j];
	  if (freq[tok] < FREQ_LIMIT) continue;
	  if (indexStart[tok] < 0) indexStart[tok] = indexEnd[tok] = 0;
	  list[tok][indexEnd[tok]].str = i;
	  ++indexEnd[tok];
	  ++indexElemNum;
	}

  for (i = 0, j = 0; i < tokenNum; i++) if (freq[i]) ++j;
  std::cerr << "# Distinct Indexed Tokens: " << j << std::endl;
  std::cerr << "# Inverted Index Entries: " << indexElemNum << std::endl;

}


void freeToken()
{
  int i;
  for (i = 0; i < n; i++)
    delete [] token[i];
  return;
}

void freeIndex()
{
  int i;
  for (i = 0; i < tokenNum; i++)
    if (indexEnd[i] >= 0)
      delete [] list[i];
}
