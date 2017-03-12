#include <iostream>
#include "header.h"
#include "dict.h"
#include "vanidx.h"
#include "boolean_queue.h"
#include "boolean_query_processing.h"




extern int maxlen;
extern int n; //number of records
extern int *token[STRING_NUM]; //record content
extern int len[STRING_NUM]; //record lengt
extern char *tokstr[STRING_NUM]; // token strings. 
extern int tokstrlen[STRING_NUM]; // token string length.



#ifndef MEM_ACCESS
static unsigned int van_idx_buf[STRING_NUM];
static int buf_pos = 0;
#endif


static single_boolean_queue *single_bqs[MAX_QUERY_TOKEN_LEN];
static int single_bqs_pos;


void clear_van_buf()
{
//  #if
#ifndef MEM_ACCESS
  buf_pos = 0;
#endif
  single_bqs_pos = 0;
}




// Vanilla inverted index retrieval
single_boolean_queue *vanilla_ivlist_retrieval(vanilla_index_t *vidx, int token_pos){
  single_boolean_queue * bqp;
  int idx_pos, idx_len;
  unsigned int *buffers;
  
  // if no one is here, create new.
  if ( single_bqs[single_bqs_pos] == NULL )
    single_bqs[single_bqs_pos] = new single_boolean_queue();

  bqp = single_bqs[single_bqs_pos++];
  idx_pos = vidx->token_list[token_pos].idx_pos;
  idx_len = (vidx->token_list[token_pos+1].idx_pos - idx_pos) >> 2;

#ifdef MEM_ACCESS
  buffers = &(vidx->idx_buf[idx_pos>>2]);
#else
  vidx->io_times++;
  vidx->io_len += idx_len;
  buffers = van_idx_buf + buf_pos;
  fseek(vidx->idx_fp, idx_pos, SEEK_SET);
  fread(buffers, sizeof(int), idx_len, vidx->idx_fp);
  buf_pos += idx_len;
#endif
  
  bqp->initialize(idx_len, buffers);
  return bqp;
}


vanilla_index_t *build_vanilla_index_from_files(char *filename)
{
  FILE *fp;
  char namebuf[1024];
  char tokbuf[1024];
  int tok_num,index_size; 
  unsigned int tok_len, idx_pos;
  vanilla_index_t *vidx;
  dict_node_t pnode;


  /* Read filename.n file to read into the basic info */
  snprintf (namebuf, 1024, "%s.n", filename);
  if( (fp = fopen(namebuf, "r"))==NULL ){
    fprintf(stderr, "Error: %s not exist\n", namebuf);
    return NULL;
  }
  if ( fscanf(fp, "toknum = %d index_size = %d", 
              &tok_num, &index_size) != 2 ){
    fprintf(stderr, "Error: %s is not in the right format\n", namebuf);
    return NULL;
  }
  fclose(fp);

  vidx = new vanilla_index_t;
  vidx->tok_num = tok_num;
  vidx->index_size = index_size;

  /* Read token list file to build up a token hash array */
  snprintf (namebuf, 1024, "%s.tok", filename);
  if( (fp = fopen(namebuf, "rb"))==NULL ){
    fprintf(stderr, "Error: %s not exist\n", namebuf);
    delete vidx;
    return NULL;
  }

  // Generate token entry
  vidx->token_list = new vanilla_token_entry_t[tok_num+1];
  vidx->token_dict = dict_create(tok_num * 3 + 11);
  //vidx->idx_fp = NULL;

  // Read all the tokens into syste.
  for ( int i = 0; i < tok_num; i++ ){
    fread(&tok_len, sizeof(int), 1, fp);
    fread(tokbuf, sizeof(char), tok_len, fp);
    tokbuf[tok_len] = '\0';
    vidx->token_list[i].tok_len = tok_len;
    vidx->token_list[i].token = new char[tok_len+1];
    strncpy(vidx->token_list[i].token, tokbuf, tok_len);
    vidx->token_list[i].token[tok_len] = '\0';
    fread(&idx_pos, sizeof(int), 1, fp);
    vidx->token_list[i].idx_pos = idx_pos;
    
    // inserted it into hash dict.
#ifdef MD5SIGN
    create_sign_md5(tokbuf, tok_len, &pnode.sign1, &pnode.sign2);
#else
    create_sign_bitwise(tokbuf, &pnode.sign1, &pnode.sign2);
#endif
    pnode.pointer = i;
    dict_add(vidx->token_dict, &pnode, 1);
  }

  // make one more for length;
  vidx->token_list[tok_num].idx_pos = vidx->index_size;
  
  fclose(fp);    

  /* Read cluster array list file into memory*/
  snprintf (namebuf, 1024, "%s.idx", filename);
  if( (fp = fopen(namebuf, "rb"))==NULL ){
    fprintf(stderr, "Error: %s not exist\n", namebuf);
    goto ERR;
  }
  
  vidx->io_times = 0;
  vidx->io_len = 0;

#ifdef MEM_ACCESS
  vidx->idx_buf = new unsigned int[(vidx->index_size)>>2];
  fread ( vidx->idx_buf, sizeof(int), (vidx->index_size)>>2, fp);
  fclose (fp);
#else
  vidx->idx_fp = fp;
#endif

  return vidx;
  
  ERR:
  destory_vanilla_index(vidx);
  return NULL;
}


int build_vanilla_index(char *filename)
{
  vanilla_index_t  *vidx;
  FILE *idxfp, *infofp, *tokfp;
  dict_node_t pnode;
  char namebuf[1024];
  
  vidx = new vanilla_index_t;
  strcpy (vidx->file_name, filename);
  vidx->tok_num = n;
  vidx->token_dict = dict_create(vidx->tok_num * 3 + 1);
  vidx->token_list = new vanilla_token_entry_t[n];
  vidx->index_size = 0;
  
  snprintf(namebuf, 1024, "%s.idx", filename);
  if( (idxfp = fopen(namebuf, "wb")) == NULL ){
    fprintf(stderr, "Error: %s can not create\n", namebuf);
    goto FREE;
  }
  //vidx->idx_fp = idxfp;

  snprintf(namebuf, 1024, "%s.tok", filename);
  if( (tokfp = fopen(namebuf, "wb")) == NULL ){
    fprintf(stderr, "Error: %s can not create\n", namebuf);
    goto FREE;
  }
  
  for ( int i = 0; i < n; i++ ){
    vidx->token_list[i].tok_len = tokstrlen[i];
    vidx->token_list[i].token = new char[tokstrlen[i]];
    
    strncpy(vidx->token_list[i].token, tokstr[i],
            vidx->token_list[i].tok_len);
    // inserted it into hash dict.
#ifdef MD5SIGN
    create_sign_md5(tokstr[i], tokstrlen[i],
                    &pnode.sign1, &pnode.sign2);
#else
    create_sign_bitwise(vidx->token_list[i].token,
                        &pnode.sign1, &pnode.sign2);
#endif
    pnode.pointer = vidx->tok_num;
    dict_add(vidx->token_dict, &pnode, 1);
    vidx->token_list[i].idx_pos = vidx->index_size;
    fwrite(token[i], sizeof(int), len[i], idxfp);
    vidx->index_size += sizeof(int) * len[i];
    
    fwrite(&vidx->token_list[i].tok_len, 
           sizeof(int), 1, tokfp);
    fwrite(vidx->token_list[i].token, sizeof(char),
           vidx->token_list[i].tok_len, tokfp);
    fwrite(&vidx->token_list[i].idx_pos,
           sizeof(int), 1, tokfp);
  }
  fclose(idxfp);
  fclose(tokfp);

  snprintf(namebuf, 1024, "%s.n", filename);
  if( (infofp = fopen(namebuf, "w")) == NULL ){
    fprintf(stderr, "Error: %s can not create\n", namebuf);
    goto FREE;
  }
  fprintf(infofp, "toknum = %d index_size = %d\n",
          vidx->tok_num, vidx->index_size);
  fclose(infofp);

  return vidx->index_size;

  FREE:
  return -1;
}


/* Free vanilla index, release all memory */
int destory_vanilla_index( vanilla_index_t * vidx){
  int i;
  if (vidx != NULL){
    if (vidx->token_list != NULL ){
      for (i = 0; i < vidx->tok_num; i++)
        if ( vidx->token_list[i].token != NULL )
          delete vidx->token_list[i].token;
      delete vidx->token_list;
    }
    
    if(vidx->token_dict != NULL)
      dict_destory(vidx->token_dict);
    
#ifdef MEM_ACCESS
    delete [] vidx->idx_buf;
#else
    if(vidx->idx_fp != NULL )
      fclose (vidx->idx_fp);
#endif
    
    delete vidx;
  }
  return 0;
}

