#ifndef __BOOLEAN_QUERY_PROCESSING_H__
#define __BOOLEAN_QUERY_PROCESSING_H__

#include "vanidx.h"
#include "mgdidx.h"

#define MAX_QUERY_TOKEN_NUM 1024
#define MAX_QUERY_STRING_LEN 1024*10
#define MAX_QUERY_TOKEN_LEN 128
#define MAX_QUERY_RESULT_LEN 1024*1024


/* parsing the query through segementation         */
int boolean_query_parsing(string s, char token_list[MAX_QUERY_TOKEN_NUM][MAX_QUERY_TOKEN_LEN]);


/* Process query based on vanilla index structures */
/* simplly parsing and retrival and merge          */
int vanilla_query_processing(string &query, vanilla_index_t *vidx);


/* Process query based on merged index structures   */
/* need to parsing covered set and retrival all the */
/* underline intersection list */
int merged_query_processing(string &query, merged_index_t *midx);


#endif
