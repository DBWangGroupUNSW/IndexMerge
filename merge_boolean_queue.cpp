#include <stdlib.h>
#include <stdio.h>
#include "mgdidx.h"
#include "header.h"
#include "boolean_queue.h"

extern int bitmods[];

inline void merge_boolean_queue::heap_insert(int x){
  int i;
  for ( i = ++heap_size; i / 2 > 0 && ivdata[pos[heap[i/2]]] > ivdata[pos[x]]; i /= 2)
    heap[i] = heap[i/2];
  heap[i] = x;
}



inline int merge_boolean_queue::heap_delete_min(){
  int i, child;  
  int minele, lastele;
  if (heap_size < 1)
    return -1;

  minele = heap[1];
  lastele = heap[heap_size--];


  for ( i = 1; i * 2 <= heap_size; i = child){
    child = i * 2;
    if ( child != heap_size && ivdata[pos[heap[child + 1]]] 
         < ivdata[pos[heap[child]]] )
      child ++;
    if (ivdata[pos[lastele]] > ivdata[pos[heap[child]]]){
      heap[i] = heap[child];
    }else
      break;
  }
  heap[i] = lastele;
  return minele;
}

merge_boolean_queue::merge_boolean_queue(){
  len = 0; 
  heap_size = 0;
  ivdata = NULL;
}

// Constructor of merge boolean queue
merge_boolean_queue::merge_boolean_queue(int *indata, int mod, 
                                         int depth, int *group_array)
{
  ivdata = NULL;
  initialize(indata, mod, depth, group_array);
}


void merge_boolean_queue::initialize(int *indata, int mod, int depth, int *group_array){  
  int base = group_array[mod-1];
  len = 0;
  ivdata = indata;
  top = 0;
  for ( int i = mod; i < bitmods[depth]; i++ ){
    if ( (i & mod) == mod ){ // find a match.
      if ( group_array[i] - group_array[i-1] > 0){
        pos[len] = (group_array[i-1] - base) >> 2;
        endp[len] = (group_array[i] - base) >> 2;
        len ++;
      }
    }
  }
  
  heap_size = 0;
  // build up a heap queue
  for ( int i = 0; i < len; i++ ){
    heap_insert(i);
  }
//  pop();
  if(heap_size > 0)
    top = ivdata[pos[heap[1]]];
  else
    top = QUE_END_MARK;
  return;
}


void merge_boolean_queue::pop(){
  int minp;

  if ( top == QUE_END_MARK ){
    return;
  }
  
  minp = heap_delete_min();
  pos[minp]++;
  if(pos[minp] < endp[minp])
    heap_insert(minp);

  if(heap_size > 0)
    top = ivdata[pos[heap[1]]];
  else
    top = QUE_END_MARK;

  return;
}

void merge_boolean_queue::destroy(){
  if (ivdata){
    ivdata = NULL;
  }
  len = 0; 
  heap_size = 0;
}




