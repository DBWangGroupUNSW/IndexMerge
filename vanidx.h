#ifndef __VANILLA_INDEX_H__
#define __VANILLA_INDEX_H__

#include "header.h"
#include "dict.h"
#include "boolean_queue.h"

/* This header file defines merged index structue */
/* Tnad the function of build and access those    */
/* index structure */


/* This part describe the index structure         */
/* In order for fast super set retrieval          */




typedef struct _vanilla_token_entry_t{
  int tok_len;
  char *token;
  int idx_pos;
}vanilla_token_entry_t;

typedef struct _vanilla__index_t{
  char   file_name[256];       /* Index file name */
  int    tok_num;              /* Total token number */
  dict_t *token_dict;      /* hash dict for tokens */
  vanilla_token_entry_t *token_list;
  int    index_size;           /* Index file size */
  int io_times;
  long long io_len;
#ifdef MEM_ACCESS
  unsigned int    *idx_buf;
#else
  FILE    *idx_fp;             /* index file pointer */
#endif

}vanilla_index_t;


/* This function is for read index from index files */
vanilla_index_t *build_vanilla_index_from_files(char *filename);

/* This function is for build a merged index */
int build_vanilla_index(char *filename);

/* Free merged index, release all memory */
int destory_vanilla_index( vanilla_index_t * midx);

/* Vanilla inverted index retrieval */
single_boolean_queue *vanilla_ivlist_retrieval(vanilla_index_t *vidx, int token_pos);



void clear_van_buf();

/* Create a empty  */


#endif

