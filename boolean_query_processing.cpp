#ifndef __BOOLEAN_QUERY_PROCESSING_H__
#define __BOOLEAN_QUERY_PROCESSING_H__

#define MAX_QUERY_TOKEN_NUM 1024
#define MAX_QUERY_STRING_LEN 1024*10
#define MAX_QUERY_TOKEN_LEN 128
#define MAX_QUERY_RESULT_LEN 1024*1024
#define INVERTED_LIST_BUF_SIZE 1024*1024*10


#include <string>
#include <queue>
#include <vector>
#include <string.h>
#include "boolean_query_processing.h"
#include "vanidx.h"
#include "mgdidx.h"
#include "boolean_queue.h"
#include "header.h"
#include "greedymerge.h"



using namespace std;


/* Data array to keep result of query processing */
/* return the length of query processing         */
unsigned int result_set[MAX_QUERY_RESULT_LEN];

extern int bitemods[];

/* Data array to keep and parsing query token list */
static char query_token_list[MAX_QUERY_TOKEN_NUM][MAX_QUERY_TOKEN_LEN];

and_boolean_queue *andq = NULL;

/* Data array to keep token id  */

int boolean_query_parsing(string s)
{
    char cstr[MAX_QUERY_STRING_LEN];
    char sep[] = " \t";
    char *brkb;
    char *token;
    int  i=0;
    
    strcpy(cstr, s.c_str());
    
    for (token = strtok_r(cstr, sep, &brkb); token;
         token = strtok_r(NULL, sep, &brkb)){      
      strcpy(query_token_list[i], token);
      i++;
    }
    return i;
}




/* Process query based on vanilla index structures */
/* simplly parsing and retrival and merge          */
int vanilla_query_processing(string &query, vanilla_index_t *vidx){
  int token_num;
  dict_node_t pnode;
  single_boolean_queue *sgq;

  int token_pos;
  int rst_num = 0;
  
  clear_van_buf();
  token_num = boolean_query_parsing(query);
  if ( token_num == 0 )         /* no result */
    return 0;
  if ( andq == NULL )
    andq = new and_boolean_queue();
 
  andq->initialize(0, NULL, token_num);
  
  for ( int i = 0; i < token_num; i++ ){ 
#ifdef MD5SIGN
    create_sign_md5(query_token_list[i], strlen(query_token_list[i]),
                    &pnode.sign1, &pnode.sign2);
#else
    create_sign_bitwise(query_token_list[i], &pnode.sign1, &pnode.sign2);
#endif
    if (dict_search(vidx->token_dict, &pnode) 
        == RT_HASH_DICT_SEARCH_SUCC ){
      token_pos = (int) pnode.pointer;
      
      sgq = vanilla_ivlist_retrieval(vidx, token_pos);
      andq->add(sgq);
    }
  }
  andq->pop();
  while(andq->top!=QUE_END_MARK){
    result_set[rst_num++] = andq->top;
    andq->pop();
  }
  andq->destroy();
  return rst_num;
}


// token entry heap
static token_entry_t *token_heap[MAX_QUERY_TOKEN_NUM];
static int token_heap_size = 0;

inline void token_heap_insert(token_entry_t *tep)
{
  int i;
  for ( i = ++token_heap_size; i / 2 > 0 && token_heap[ i / 2 ]->cluster_pos 
          > tep->cluster_pos; i /= 2 )
    token_heap[ i ] = token_heap[ i / 2 ];
  token_heap[ i ] = tep;
}

inline void token_heap_delete_min()
{
  int i, child;
  token_entry_t *minele, *lastele;
  
  if ( token_heap_size == 0 )
    return;
  
  minele = token_heap[1];
  lastele = token_heap[token_heap_size --];
  
  for ( i = 1; i * 2 <= token_heap_size; i = child ){
    child = i * 2;
    if ( child != token_heap_size && token_heap[ child + 1 ]->cluster_pos
         < token_heap[ child ]->cluster_pos )
      child ++;
    if (lastele->cluster_pos > token_heap[child]->cluster_pos)
      token_heap[i] = token_heap[child];
    else
      break;
  }
  token_heap[i] = lastele;
  return;
}





/* Process query based on merged index structures   */
/* need to parsing covered set and retrival all the */
/* underline intersection list */
int merged_query_processing(string &query, merged_index_t *midx){
  int token_num;
  dict_node_t pnode;
  int token_pos;
  int rst_num = 0;
  int cluster_pos;
  int depth;
  int order;
  int mod;
  const token_entry_t *tep;
  merge_boolean_queue *mbqp;
  
  token_num = boolean_query_parsing(query);
  if ( token_num == 0 )         /* no result */
    return 0;

  clear_mge_buf();
  token_heap_size = 0;
  
  for ( int i = 0; i < token_num; i++ ){ 
#ifdef MD5SIGN
    create_sign_md5(query_token_list[i], strlen(query_token_list[i]),
                    &pnode.sign1, &pnode.sign2);
#else
    create_sign_bitwise(query_token_list[i], &pnode.sign1, &pnode.sign2);
#endif
    if (dict_search(midx->token_dict, &pnode) 
        == RT_HASH_DICT_SEARCH_SUCC ){
      token_pos = (int) pnode.pointer;
      token_heap_insert(&(midx->token_list[token_pos]));
    }
  }

  if ( andq == NULL )
    andq = new and_boolean_queue();
  andq->initialize(0, NULL, token_num);

  
  // iterate all the token entry in the queue;
  while (token_heap_size > 0){
    tep = token_heap[1];
    cluster_pos = tep->cluster_pos;
    depth = tep->cluster_depth;
    order = tep->token_order;
    mod = bitmods[order];
    token_heap_delete_min();
    while (token_heap_size > 0){
      tep = token_heap[1];
      if (cluster_pos == tep->cluster_pos){
        mod = mod | bitmods[tep->token_order];
      }else{
        break;
      }
      token_heap_delete_min();
    }

    mbqp = merged_index_retrieval(cluster_pos, depth, mod, midx);

    if (mbqp != NULL){
      andq->add(mbqp);
    }
  }
  
  andq->pop();
  while(andq->top!=QUE_END_MARK){
    result_set[rst_num++] = andq->top;
    andq->pop();
  }
  andq->destroy();
  return rst_num;
}

#endif
