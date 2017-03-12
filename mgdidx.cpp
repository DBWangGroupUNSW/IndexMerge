#include <iostream>
#include "header.h"
#include "dict.h"
#include "mgdidx.h"
#include "greedymerge.h"
#include "boolean_queue.h"
#include "boolean_query_processing.h"


extern int maxlen;
extern int n; //number of records
extern int *token[STRING_NUM]; //record content
extern int len[STRING_NUM]; //record lengt
extern char *tokstr[STRING_NUM]; // token strings. 
extern int tokstrlen[STRING_NUM]; // token string length.
extern int bitmodx[];


#ifndef MEM_ACCESS
static int mge_idx_buf[STRING_NUM];
static int buf_pos = 0;
#endif

static merge_boolean_queue *merge_bqs[MAX_QUERY_TOKEN_LEN];
static int merge_bqs_pos;

void clear_mge_buf()
{
//  #if
#ifndef MEM_ACCESS
  buf_pos = 0;
#endif
  merge_bqs_pos = 0;
}


merge_boolean_queue *merged_index_retrieval(int cls_pos, int depth, int mod, merged_index_t *midx)
{
  int idx_len;
  int idx_pos;
  merge_boolean_queue *mbqp;
  int *buffers;
  
  if ( merge_bqs[merge_bqs_pos] == NULL )
    merge_bqs[merge_bqs_pos] = new merge_boolean_queue();
  
  mbqp = merge_bqs[merge_bqs_pos++];

  idx_pos = midx->cluster_array[cls_pos + mod - 1];
  idx_len = (midx->cluster_array[cls_pos + bitmods[depth]] - idx_pos)>>2;
              

  if ( idx_len == 0 )
    return NULL;
  else{
#ifdef MEM_ACCESS
    buffers = &(midx->idx_buf[idx_pos>>2]);
#else
    midx->io_times ++;
    midx->io_len += idx_len;
    buffers = mge_idx_buf + buf_pos;
    fseek(midx->idx_fp, idx_pos, SEEK_SET);
    fread(buffers, sizeof(int), idx_len, midx->idx_fp);
    buf_pos += idx_len;
#endif
    mbqp->initialize(buffers, mod, depth, &midx->cluster_array[cls_pos]);
  }
  return mbqp;
}



merged_index_t *build_merged_index_from_files(char *filename)
{
  FILE *fp;
  char namebuf[1024];
  char tokbuf[1024];
  int tok_num, cluster_num, cluster_array_len,index_size; 
  unsigned int tok_len, cluster_depth, tok_order, cluster_pos;
  merged_index_t *midx;
  dict_node_t pnode;

  /* Read filename.n file to read into the basic info */
  snprintf (namebuf, 1024, "%s.n", filename);
  if( (fp = fopen(namebuf, "r"))==NULL ){
    fprintf(stderr, "Error: %s not exist\n", namebuf);
    return NULL;
  }
  if ( fscanf(fp, "toknum = %d cluster_num = %d cluster_array_len = %d index_size = %d", 
              &tok_num, &cluster_num, &cluster_array_len, &index_size) != 4 ){
    fprintf(stderr, "Error: %s is not in the right format\n", namebuf);
    return NULL;
  }
  fclose(fp);

  midx = new merged_index_t;
  midx->tok_num = tok_num;
  midx->cluster_num = cluster_num;
  midx->cluster_array_len = cluster_array_len;
  midx->index_size = index_size;

  /* Read token list file to build up a token hash array */
  snprintf (namebuf, 1024, "%s.tok", filename);
  if( (fp = fopen(namebuf, "rb"))==NULL ){
    fprintf(stderr, "Error: %s not exist\n", namebuf);
    delete midx;
    return NULL;
  }

  // Generate token entry
  midx->token_list = new token_entry_t[tok_num];
  midx->cluster_array = new int[cluster_array_len+1];  
  midx->token_dict = dict_create(tok_num * 3 + 11);
//  midx->idx_fp = NULL;

  // Read all the tokens into syste.
  for ( int i = 0; i < tok_num; i++ ){
    fread(&tok_len, sizeof(int), 1, fp);
    fread(tokbuf, sizeof(char), tok_len, fp);
    tokbuf[tok_len] = '\0';
    midx->token_list[i].tok_len = tok_len;
    midx->token_list[i].token = new char[tok_len+1];
    strncpy(midx->token_list[i].token, tokbuf, tok_len);
    midx->token_list[i].token[tok_len] = '\0';
    fread(&cluster_pos, sizeof(int), 1, fp);
    fread(&cluster_depth, sizeof(int), 1, fp);
    fread(&tok_order, sizeof(int),1, fp);
    midx->token_list[i].cluster_pos = cluster_pos;
    midx->token_list[i].cluster_depth = cluster_depth;
    midx->token_list[i].token_order = tok_order;
    
    // inserted it into hash dict.
#ifdef MD5SIGN
    create_sign_md5(tokbuf, tok_len, &pnode.sign1, &pnode.sign2);
#else
    create_sign_bitwise(tokbuf, &pnode.sign1, &pnode.sign2);
#endif
    pnode.pointer = i;
    dict_add(midx->token_dict, &pnode, 1);
  }
  
  fclose(fp);
    
  /* Read cluster array list file into memory*/
  snprintf (namebuf, 1024, "%s.cls", filename);
  if( (fp = fopen(namebuf, "rb"))==NULL ){
    fprintf(stderr, "Error: %s not exist\n", namebuf);
    goto ERR;
  }
  fread(midx->cluster_array, sizeof(int), cluster_array_len, fp);
  midx->cluster_array[cluster_array_len] = midx->index_size;
  fclose(fp);

  /* Read cluster array list file into memory*/
  snprintf (namebuf, 1024, "%s.idx", filename);
  if( (fp = fopen(namebuf, "rb"))==NULL ){
    fprintf(stderr, "Error: %s not exist\n", namebuf);
    goto ERR;
  }
  
  midx->io_times = 0;
  midx->io_len = 0;

#ifdef MEM_ACCESS
  midx->idx_buf = new int[(midx->index_size)>>2];
  fread(midx->idx_buf, sizeof(int), (midx->index_size)>>2, fp);
  fclose(fp);
#else
  midx->idx_fp = fp;
#endif
  return midx;
  
  ERR:
  destory_merged_index(midx);
  return NULL;
}


#define MAX_DID 0x7fffffff


// A Deep merge procedure to merge all the inverted
// list in the same cluster. and generate a merged
// cluster id;

int deep_merge(merge_cluster_t *mcls, int **merged_array, 
               int *before_len, int *after_len ){
  int merged_len;
  int depth;
  int minimal[MAX_MERGE_CLUSTER_SIZE];
  int curr[MAX_MERGE_CLUSTER_SIZE];
  int flag = 1;
  int order = 0;
  int min;
  int before = 0;
  int after = 0;

  depth = mcls->depth;
  if ( depth == 1 ){
    merged_array[0][0]=0;
    merged_array[1][0]=len[mcls->tokens[0]];
    
    for ( int i = 0; i < merged_array[1][0]; i++ ){
      merged_array[1][i+1] = token[mcls->tokens[0]][i];
    }
    *before_len = merged_array[1][0];
    *after_len = merged_array[1][0];
    return 0;
  }

  merged_len = bitmods[depth];
  for( int i = 0; i < depth; i++ ){
    curr[i] = 1;
    minimal[i] = token[mcls->tokens[i]][0];
    before = before + len[mcls->tokens[i]];
  }
  
  for( int i = 0; i < merged_len; i++){
    merged_array[i][0] = 0;
  }

  while (flag){
    flag = 0;
    min = minimal[0];
    for ( int i = 1; i < depth; i++ ){
      if ( minimal[i] < min ){
        min = minimal[i];
      }
    }
    if ( min == MAX_DID )  break;
    order = 0;
    for ( int i = 0; i< depth; i++ ){
      if ( minimal [i] == min ){
        order =  order | bitmods[i];
        flag = 1; 
        if ( curr[i] < len[mcls->tokens[i]] ){
          minimal[i] = token[mcls->tokens[i]][curr[i]];
          curr[i] ++;
        }else{
          minimal[i] = MAX_DID;
        }        
      }
    }
    if (flag){
      after ++;
      merged_array[order][merged_array[order][0] + 1] = min;
      merged_array[order][0]++;
    }
  }
  
  *before_len = before;
  *after_len = after;

  return before - after;
}


/* This function is for build a merged index */
int build_merged_index_from_deep_merge(merge_cluster_t *mcls, int cls_len, char *filename)						
{
  int **merged_array;
  int merged_array_len = 1 << (MAX_MERGE_CLUSTER_SIZE + 1);
  int cluster_array_num = (int) (merged_array_len / MAX_MERGE_CLUSTER_SIZE * n);
  merged_index_t  *midx;
  FILE *idxfp, *clsfp, *infofp, *tokfp;
  dict_node_t pnode;
  char namebuf[1024];
  int merged_len; 
  int before_len;
  int after_len;
  int saving;
  
  merged_array = new int* [merged_array_len];
  
  for ( int i = 0; i < merged_array_len; i++ ){
    merged_array[i] = new  int[maxlen + 2];    
  }
  
  midx = new merged_index_t;
  strcpy (midx->file_name, filename);
  midx->tok_num = 0;
  midx->cluster_num = cls_len;
  midx->cluster_array_len = 0;
  midx->token_dict = dict_create(midx->tok_num * 3 + 1);
  midx->cluster_array = new int [cluster_array_num];
  midx->token_list = new token_entry_t[n];
  midx->index_size = 0;
  
  snprintf(namebuf, 1024, "%s.idx", filename);
  if( (idxfp = fopen(namebuf, "wb")) == NULL ){
    fprintf(stderr, "Error: %s can not create\n", namebuf);
    goto FREE;
  }
  //midx->idx_fp = idxfp;
  
  for ( int i = 0; i < cls_len; i++ ){
    if ( i == 122 )
      fprintf(stderr, "Process cluster %d .\n", i );
    fprintf(stderr, "Process cluster %d .\n", i );
    saving = deep_merge( &mcls[i], merged_array, 
                         &before_len, &after_len);
    merged_len = bitmods[mcls[i].depth];
    for ( int j = 0; j < mcls[i].depth; j++ ){
      midx->token_list[midx->tok_num].tok_len = 
        tokstrlen[mcls[i].tokens[j]];
      midx->token_list[midx->tok_num].token = new 
        char[midx->token_list[midx->tok_num].tok_len];
      strncpy(midx->token_list[midx->tok_num].token, 
              tokstr[mcls[i].tokens[j]],
              midx->token_list[midx->tok_num].tok_len);
      midx->token_list[midx->tok_num].cluster_depth = 
        mcls[i].depth;
      midx->token_list[midx->tok_num].token_order = j;      
      midx->token_list[midx->tok_num].cluster_pos = 
        midx->cluster_array_len;
      
      // inserted it into hash dict.
#ifdef MD5SIGN
      create_sign_md5(tokstr[mcls[i].tokens[j]],
                      tokstrlen[mcls[i].tokens[j]],
                      &pnode.sign1, &pnode.sign2);
#else
      create_sign_bitwise(tokstr[mcls[i].tokens[j]],
                          &pnode.sign1, &pnode.sign2);
#endif

      pnode.pointer = midx->tok_num;
      dict_add(midx->token_dict, &pnode, 1);
      midx->tok_num ++;
    }

    for ( int j = 1; j < merged_len; j++ ){
      if ( merged_array[j][0] > 0 )
        fwrite(&merged_array[j][1], sizeof(int), merged_array[j][0], idxfp);

      midx->cluster_array[midx->cluster_array_len] = 
        midx->index_size;
      midx->cluster_array_len ++;
      midx->index_size += sizeof(int) * merged_array[j][0];      
    }
  }
  midx->cluster_array[midx->cluster_array_len] = midx->index_size;  
  midx->cluster_array_len ++;
  fclose(idxfp);

  snprintf(namebuf, 1024, "%s.tok", filename);
  if( (tokfp = fopen(namebuf, "wb")) == NULL ){
    fprintf(stderr, "Error: %s can not create\n", namebuf);
    goto FREE;
  }

  //write them into files. 
  for ( int i = 0; i < midx->tok_num; i++ ){
    fwrite(&midx->token_list[i].tok_len, 
            sizeof (int), 1, tokfp );
    fwrite(midx->token_list[i].token, sizeof(char), 
            midx->token_list[i].tok_len, tokfp );
    fwrite(&midx->token_list[i].cluster_pos, 
           sizeof(int), 1, tokfp);
    fwrite(&midx->token_list[i].cluster_depth, 
           sizeof(int), 1, tokfp);
    fwrite(&midx->token_list[i].token_order, 
           sizeof(int), 1, tokfp);    
  }
  fclose(tokfp);


  snprintf(namebuf, 1024, "%s.cls", filename);
  if( (clsfp = fopen(namebuf, "wb")) == NULL ){
    fprintf(stderr, "Error: %s can not create\n", namebuf);
    goto FREE;
  }
  
  fwrite(midx->cluster_array, sizeof(int),
         midx->cluster_array_len, clsfp);
  fclose(clsfp);
  
  
  snprintf(namebuf, 1024, "%s.n", filename);
  if( (infofp = fopen(namebuf, "w")) == NULL ){
    fprintf(stderr, "Error: %s can not create\n", namebuf);
    goto FREE;
  }
  fprintf(infofp, "toknum = %d cluster_num = %d cluster_array_len = %d index_size = %d\n",
          midx->tok_num, midx->cluster_num, midx->cluster_array_len, midx->index_size);
  fclose(infofp);
  

  snprintf(namebuf, 1024, "%s.idx", filename);
  if( (idxfp = fopen(namebuf, "rb")) == NULL ){
    fprintf(stderr, "Error: %s can not create\n", namebuf);
    goto FREE;
  }
  
  FREE:
  for (int i = 0; i< merged_array_len; i++ ){
    delete merged_array[i];
  }
  delete merged_array;

  return midx->index_size;
}



/* Free merged index, release all memory */
int destory_merged_index( merged_index_t * midx){
  int i;

  if (midx != NULL){
    if (midx->token_list != NULL ){
      for (i = 0; i < midx->tok_num; i++)
        if ( midx->token_list[i].token != NULL )
          delete midx->token_list[i].token;
      delete midx->token_list;
    }
    
    if(midx->token_dict != NULL)
      dict_destory(midx->token_dict);
    
    if(midx->cluster_array != NULL)
      delete midx->cluster_array;

#ifdef MEM_ACCESS
    delete [] midx->idx_buf;
#else
    if(midx->idx_fp != NULL )
      fclose (midx->idx_fp);
#endif

    delete midx;
  }
  return 0;
}
