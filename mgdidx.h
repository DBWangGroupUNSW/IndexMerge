#ifndef __MERGED_INDEX_H__
#define __MERGED_INDEX_H__

#include <queue>
#include <vector>
#include "dict.h"
#include "greedymerge.h"
#include "boolean_queue.h"



using namespace std;


/* This header file defines merged index structue */
/* Tnad the function of build and access those    */
/* index structure */


/* This part describe the index structure         */
/* In order for fast super set retrieval          */




typedef struct _token_entry_t{
  int tok_len;                  /* length of the token string*/
  char *token;                  /* token string */
  int cluster_pos;              /* the  start positon of cluster in cluster_array */
  int cluster_depth;          /* the depth of the culster it in */
  int token_order;            /* the order of this token in its cluster */
}token_entry_t;


typedef struct _merged_index_t{
  char   file_name[256];       /* Index file name */
  int    tok_num;              /* Total token number */
  int    cluster_num;          /* Total cluster number */
  int    cluster_array_len;    /* The length of cluster array */
  dict_t  *token_dict;     /* Hash table for map token to
                                  cluster id */
  token_entry_t *token_list;   /* list to store tokens */
  int    index_size;           /* Index file size */
  int    *cluster_array;       /* Cluster array locate ivlist */  
  
  int io_times;
  int io_len;
  
#ifdef MEM_ACCESS
  int    *idx_buf;
#else
  FILE    *idx_fp;             /* index file pointer */
#endif
  
}merged_index_t;

/* This data structure is used. showed be replaced.  */
static int bitmods[]={1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};


/* This function is for read index from index files */
merged_index_t *build_merged_index_from_files(char *filename);


/* This function is for build a merged index */
int build_merged_index_from_deep_merge(merge_cluster_t *mcls,
                                       int cls_len, char *filename);

// A Deep merge procedure to merge all the inverted
// list in the same cluster. and generate a merged
// cluster id;
int deep_merge(merge_cluster_t *mcls, int **merged_array, 
               int *before_len, int *after_len );

/* Free merged index, release all memory */
int destory_merged_index( merged_index_t * midx);

void clear_mge_buf();

merge_boolean_queue *merged_index_retrieval(int cls_pos, int depth, int mod, merged_index_t *midx);


/* Create a empty  */


#endif
