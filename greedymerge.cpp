#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include "header.h"
#include "index.h"
#include "binio.h"
#include "greedymerge.h"


using namespace std;

typedef set<int> mergeset_t;

extern int vector_id[STRING_NUM]; //record id within original txt file

extern elem_index* list[TOKEN_NUM]; //inverted lists
extern int indexElemNum, indexTokenNum; //stats for inverted lists
extern int indexEnd[ELEM_NUM]; //start/end pointer for inverted lists


extern int count[STRING_NUM];
extern int overlap[STRING_NUM][2];
extern int overnum;
extern int threshold;// = 0;              // This is the threshold to limit the minimal benifit. 


extern int total_index_size;// = 0;
extern int total_benifit;//= 0;

extern mergeset_t mergeset;

extern char *tokstr[STRING_NUM];
extern int limit;

extern int vector_id[STRING_NUM]; //record id within original txt file
extern elem_index* list[TOKEN_NUM]; //inverted lists
extern int indexElemNum, indexTokenNum; //stats for inverted lists
extern int indexStart[ELEM_NUM], indexEnd[ELEM_NUM]; //start/end pointer for inverted lists   


int m = MAX_MERGE_CLUSTER_SIZE;
int last_pos;

int tok_counter[STRING_NUM];
int tok_filter[STRING_NUM];

//  list for token information.
token_info_t *token_info;
int info_len = 0;
int *top_one_queue;
int top_one_queue_size = 0;


void dump_merge_clusters(merge_cluster_t *mcls, int cls_len)
{
  int i, j;  
  for (i = 0; i < cls_len; i++){
    fprintf(stdout, "Cluster: %d save space: %d topk robbered %d with %d tokens: ", 
            i, mcls[i].saving, mcls[i].robbered, mcls[i].depth);
    for (j = 0; j < mcls[i].depth; j++){
      fprintf(stdout, " %d|%s|%d", mcls[i].tokens[j], 
              tokstr[mcls[i].tokens[j]], len[mcls[i].tokens[j]]);      
    }
    fprintf(stdout, "\n");
  }
}



static int calc_overlap(int x, int y){
  int i,j;
  int ov = 0;
  i = j = ov = 0 ;

  while (i < len[x] && j < len[y]){
    if ( token [x][i] < token[x][j] ){
      i ++;
    }else if ( token [x][i] > token[x][j] ){
      j ++;
    }else{
      i ++; 
      j ++;
      ov ++;
    }
  }
  return ov;
}


void dump_merge_pairs(merge_cluster_t *mcls, int cls_len)
{
  int i, j, k, x, y, t;  
  for (i = 0; i < cls_len; i++){
    if ( mcls[i].depth <= 1 ){
      fprintf(stdout, "DumpPairs: ALONE %d %s %d\n", mcls[i].tokens[0], 
            tokstr[mcls[i].tokens[0]], len[mcls[i].tokens[0]]);      
    }else{      
      for (j = 0; j < mcls[i].depth-1; j++){
        x = mcls[i].tokens[j];
        for (k = j + 1; k < mcls[i].depth; k++){          
          y = mcls[i].tokens[k];
          if ( x > y ) {t=x; x=y; y=t;} // swep
          t = calc_overlap(x,y);
          fprintf(stdout, "DumpPairs: Pair |%d %s %d| <-- %d --> |%d %s %d|\n", x ,tokstr[x], len[x], 
                t, y ,tokstr[y], len[y]);
        }
      }
    }
  }
}


// top one queue insert it
inline void  top_one_queue_insert(int x){
  int i;
  for ( i = ++top_one_queue_size; i / 2 > 0 && 
          token_info[top_one_queue[i/2]].topov 
          < token_info[x].topov; i/=2)
    top_one_queue[i] = top_one_queue[i/2];
  top_one_queue[i] = x;
}


// top one queue delete it
inline int top_one_queue_delete_max(){
  int i, child, minele, lastele;

  if ( top_one_queue_size < 1)
    return -1;

  minele = top_one_queue[1];
  lastele = top_one_queue[top_one_queue_size --];

  
  for ( i = 1; i * 2 <= top_one_queue_size; i = child ){
    child = i * 2;
    if ( child != top_one_queue_size && 
         token_info[top_one_queue[child+1]].topov
         > token_info[top_one_queue[child]].topov)
      child ++;
    if ( token_info[top_one_queue[lastele]].topov 
         < token_info[top_one_queue[child]].topov )
      top_one_queue[i] = top_one_queue[child];
    else
      break;      
  }
  top_one_queue[i] = lastele;
  return minele;
}


inline int double_binary_search(int *list, int offset, int max, int key)
{
  int left = offset;
  int right, mid;
  int step = 1;
  int pos;
  
  // Use binary search to find in the right direction.
  while ( step + left < max && list[step + left] < key )
    step  = step << 1;
  
  // if we find a good step.
  if ( step + left >= max ){
    if ( list [ max - 1] >= key ) {
      right  = max -1;
    }else{
      return -1;
    }
  }else{
    right = left + step;
  }

  pos = right;
  while ( right > left ){
    mid = ( right + left ) >> 1;
    if ( key < list[mid] ){
      pos = mid;
      right = mid - 1;
    }else if ( key > list[mid] ){
      pos = mid + 1;
      left = mid + 1;
    }else{
      return mid;
    }    
  }
  return pos;  
}


// Use merge skip idea to fast verify two inverted list and return the overlap number. 
// if the rest of the list is less than max - currov. then return current ov. and. 
// stop proceesing.
inline int merge_skip_verify(int x, int y, int max )
{
  int *listx, *listy;
  int lenx, leny;
  int posx = 0, posy = 0;
  int ov = 0;
  int ret;

  listx = token[x];
  listy = token[y];
  lenx = len[x];
  leny = len[y];
  
  while ( posx < lenx && posy < leny ){
    if ( listx[posx] == listy[posy] ){
      ov ++;
      posx ++;
      posy ++;
    }else{
      // not equal.
      if ( listx[posx] > listy[posy] ){
        ret = double_binary_search( listy, posy + 1, leny, listx[posx]);
        if ( ret == -1 ){
          break;
        }else{
          posy = ret;
        }
        
        // add filter to return faster
        if (leny - posy < max - ov)
          return ov;
      }else{
        ret = double_binary_search( listx, posx + 1, lenx, listy[posy]);
        if ( ret == -1 ){
          break;
        }else{
          posx = ret;
        }

        // add filter to return faster
        if (lenx - posx < max - ov) 
          return ov;  
      }
    }
  }
  return ov;
}






inline int top_one_overlap_bfs(int id, int upper)
{
  int max = id - 1;
  int l,x,t,j,g;
  int s = token_info[id].size;
  int maxov = 0;
  memset(tok_counter, 0, sizeof(int)*(id));
  memset(tok_filter, 0, sizeof(int)*(id));

#ifndef PRODUCT
  fprintf(stderr, "Process top one Overlap of Id %d with upper bound %d\n", id, upper);
#endif

  if ( id == 0 ) {
    token_info[id].top = -1;
    token_info[id].topov = 0;
  }
  
  l = len[id];
  for (j = 0; j < l; j++ )
  {
    t = token[id][j];
    for ( x = 0; x < indexEnd[t]; x++ ){
      g = token_info[list[t][x].str].gid;
      if ( len[g] <= maxov ){
        if ( g < id && token_info[g].size + s <= m){
          if ( tok_filter[g] != t ){
            tok_filter[g] = t;
            if(++tok_counter[g] > maxov){
              maxov = tok_counter[g];
              max = g;
            }
          }
        }
      }else{
        // This need to use def over merged groups.
        break;
      }
    }
  }

  if (tok_counter[max] < 1){
    token_info[id].top = -1;
    token_info[id].topov = 0;
  }else{
    token_info[id].top = max;
    token_info[id].topov = tok_counter[max];
  }
  return token_info[id].top;
}




// top one overlapping calculation.
inline int top_one_overlap_dfs(int id, int upper)
{
  int max = id - 1;
  int maxov = 0;
  int l,x,t,j,g;
  //int s = token_info[id].size;

  if ( token_info[id].mode == GROUP_MEMBER_MODE ||
       token_info[id].mode == CLOSED_GROUP_MODE )
    return 0;

  memset(tok_counter, 0, sizeof(int)*(id));

#ifndef PRODUCT
  fprintf(stderr, "Process top one Overlap of Id %d with upper bound %d\n", id, upper);
#endif

  if ( id == 0 ) {
    token_info[id].top = -1;
    token_info[id].topov = 0;
  }

  l = len[id];
  for (j = 0; j < l; j++ )
  {
    t = token[id][j];
    for ( x = 0; x < indexEnd[t]; x++ ){
      g = token_info[list[t][x].str].gid;
      if ( g < id && tok_counter[g] == 0 && token_info[g].mode != CLOSED_GROUP_MODE && 
              token_info[g].size + token_info[id].size < m)
      {
        if ( len[g] > maxov ){          
          tok_counter[g] = merge_skip_verify(g, id, maxov);          
          if ( tok_counter[g] > maxov ){
            max = g;
            maxov = tok_counter[g];
          }
        }else{        
          // Add a filter. This one need other assumption.
          break;
        }
      }
    }
  }

  if (tok_counter[max] < 1){
    token_info[id].top = -1;
    token_info[id].topov = 0;
  }else{
    token_info[id].top = max;
    token_info[id].topov = tok_counter[max];
  }
  return token_info[id].topov;
}




// Find a resonable one in top one field.
inline int find_a_reasonable_current_top_one()
{
  int top;
  int token_head, token_tail;

  while (top_one_queue_size > 0){
    top = top_one_queue[1];
    token_head = top;
    token_tail = token_info[top].top;    
    if ( token_info[token_head].mode == SINGLE_TOKEN_MODE ||
         token_info[token_head].mode == OPEN_GROUP_MODE ){
      if ((token_info[token_tail].mode == SINGLE_TOKEN_MODE ||
           token_info[token_tail].mode == OPEN_GROUP_MODE) && 
          (token_info[token_head].size + token_info[token_tail].size <= m)){
        // They are all good, and ready to merge. 
        return token_head;
      }else{
        // pop that one out and recalculate it's top one.
        // Token_info is qualified, but the next one is not qualified.
        top_one_queue_delete_max();
        top_one_overlap_bfs(top, 0);

#ifdef DEBUG    
        fprintf(stderr, "Robbered Top one of %d is %d ovlp is %d\n", 
                top, token_info[top].top, token_info[top].topov);
#endif
        if ( token_info[top].topov > OVERLAP_LOWER_BAND )
          top_one_queue_insert ( top );
      }
    }else{
      // not a valueable one.
      top_one_queue_delete_max();
    }
  }
  return -1;
}


inline int search_two_tokens(){
  // Use this one to adaptablly calculate the largest overlap. 
  
  int top;
  int ov;
  int maxov = 0;
  
  top = find_a_reasonable_current_top_one();
  if ( top == -1 )
    maxov = 0;
  else
    maxov = token_info[top].topov;
  
  // Do insert non -calculated top one.
  while ( last_pos >= 0 && len[last_pos] > maxov){
    // calculate the overlap of this one. 
    ov = top_one_overlap_bfs(last_pos, maxov );
    if ( ov > maxov ){
      maxov = ov;
    }
    if ( ov >= OVERLAP_LOWER_BAND ){
      top_one_queue_insert (last_pos);
    }
    last_pos --;
  }
  return top_one_queue_delete_max();
}

// Greedlly find the high_merge clusters.
int greedy_merge(merge_cluster_t * mcls, int cls_size, int depth){
  int i,j;
  int p ;
  int topa,topb;  
  int pa,pb, pm;
  int saving;

  m = depth;

  // make new token info structure;
  // init data structure.
  token_info = new token_info_t[3*n];
  top_one_queue = new int[3*n];
  top_one_queue_size = 0;
  info_len = n;  
  last_pos = n - 1;

  // init token_info.
  for ( i = 0; i < n; i++ ){
    token_info[i].mode = SINGLE_TOKEN_MODE;
    token_info[i].size = 1;
    token_info[i].top = -1;
    token_info[i].topov = 0;
    token_info[i].saving = 0;
    token_info[i].gid = i;
    token_info[i].members[0] = i;
  }
  
  while ( (p = search_two_tokens()) > 0 ){
    // Ok find a merge pair, go merge them together.
    // Output it for debugging

#ifdef DEBUG
    fprintf(stderr, "Find a merge pair:  %d <--> %d  Saving: %d \n",
           p, token_info[p].top, token_info[p].topov);
#endif

    topa = p;
    topb = token_info[p].top;
    saving = token_info[p].topov;
    
    // First to merge two inverted list together.
    len[info_len] = len[topa] + len[topb] - token_info[p].topov;
    token[info_len] = new int[len[info_len]];
    pa = pb = pm = 0;

    // TODO: we need a faster merge function..
    while ( pa < len[topa] && pb < len[topb] ){
      if ( token[topa][pa] < token[topb][pb] ){
        token[info_len][pm++] = token[topa][pa++];
      }else if ( token[topa][pa] > token[topb][pb]){
        token[info_len][pm++] = token[topb][pb++];
      }else{
        token[info_len][pm++] = token[topa][pa++];
        pb++;
      }
    }
    while (pa < len[topa]){
      token[info_len][pm++] = token[topa][pa++];
    }
    while (pb < len[topb]){
      token[info_len][pm++] = token[topb][pb++];
    }

    // Merge saving information
    token_info[info_len].saving = token_info[topa].saving + token_info[topb].saving + saving;

    // Change the mode of members.
    for(int i = 0; i < token_info[topa].size; i++ ){
      token_info[token_info[topa].members[i]].gid = info_len;
      token_info[info_len].members[i] = token_info[topa].members[i];
    }
    for(int i = 0; i < token_info[topb].size; i++ ){
      token_info[token_info[topb].members[i]].gid = info_len;
      token_info[info_len].members[token_info[topa].size + i] 
        = token_info[topb].members[i];
    }

    // They become a member of a new group
    token_info[topa].mode = GROUP_MEMBER_MODE;
    token_info[topb].mode = GROUP_MEMBER_MODE;


    // then generate a slote to put it into.
    token_info[info_len].size = token_info[topa].size + token_info[topb].size;
    if ( token_info[info_len].size >= m ){
      // group info_len finished it's merge and continuel
      token_info[info_len].mode = CLOSED_GROUP_MODE;
//       for(int i = 0; i < token_info[info_lne].size; i ++ ){
//         token_info[token_info[info_len].members[i]].mod = CLOSED_TOKEN_MODE;
//       }
    }else{
      token_info[info_len].mode = OPEN_GROUP_MODE;
      
      // Calculate the top one overlap. 
      top_one_overlap_bfs(info_len, 0);

#ifdef DEBUG
      fprintf(stderr, "Merged Top one of %d is %d ovlp is %d\n", info_len, 
              token_info[info_len].top, token_info[info_len].topov);      
#endif
      // Insert into 
      if (token_info[info_len].topov > OVERLAP_LOWER_BAND){
        top_one_queue_insert(info_len);
      }
    }
    info_len ++;    
  }
  
  // Build merged cluster information.
  int mid = 0;
  for ( i = info_len; i >= 0; i-- ){
    if( token_info[i].mode == SINGLE_TOKEN_MODE ||
        token_info[i].mode == OPEN_GROUP_MODE || 
        token_info[i].mode == CLOSED_GROUP_MODE ){
      mcls[mid].depth = token_info[i].size;
      for ( j = 0; j < token_info[i].size; j ++ )
        mcls[mid].tokens[j] = token_info[i].members[j];
      mcls[mid].saving = token_info[i].saving;   
      mcls[mid].robbered = 0;
      mid ++;
    } 
  }
  return mid;
}






