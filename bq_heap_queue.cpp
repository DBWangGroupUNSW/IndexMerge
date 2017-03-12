#include <stdlib.h>
#include <stdio.h>
#include "header.h"
#include "boolean_queue.h"



using namespace std;


// Implemnt a heap queue.
bq_priority_queue bq_pq_initialize(int maxelements) {
    bq_priority_queue queue;

    if (maxelements < BQ_PQ_MIN_SIZE)
        maxelements = BQ_PQ_MIN_SIZE;
    
    queue = new bq_heap_struct_t;
    if (queue == NULL){
      fprintf(stderr, "Out of space\n");
      return NULL;
    }
    
    queue->elements = new bq_pq_element_type[maxelements + 1];

    if (queue->elements == NULL){
      fprintf(stderr, "Out of space\n");
      return NULL;
    }
    
    queue->capacity = maxelements;
    queue->size = 0;
    queue->elements[ 0 ] = NULL;

    return queue;
}

void bq_pq_make_empty(bq_priority_queue queue) {
  queue->size = 0;
}


void bq_pq_insert(bq_pq_element_type X, bq_priority_queue queue) {
  int i;
  
  if (bq_pq_is_full(queue)) {
    fprintf(stderr, "Priority queue is full\n");
    return;
  }
  
  for (i = ++queue->size; i / 2 > 0 && queue->elements[ i / 2 ]->top > X->top; i /= 2)
    queue->elements[ i ] = queue->elements[ i / 2 ];
  queue->elements[ i ] = X;
}


bq_pq_element_type bq_pq_delete_min(bq_priority_queue queue) {
    int i, child;
    bq_pq_element_type minelement, lastelement;

    if (bq_pq_is_empty(queue)) {
      fprintf(stderr, "Priority queue is empty\n");
      return queue->elements[ 0 ];
    }

    minelement = queue->elements[ 1 ];
    lastelement = queue->elements[ queue->size-- ];
    
    for (i = 1; i * 2 <= queue->size; i = child) {
      child = i * 2;
      if (child != queue->size && queue->elements[ child + 1 ]->top
          < queue->elements[ child ] -> top)
        child++;
      
      if (lastelement->top > queue->elements[ child ]->top)
        queue->elements[ i ] = queue->elements[ child ];
      else
        break;
    }
    queue->elements[ i ] = lastelement;
    return minelement;
}

bq_pq_element_type bq_pq_find_min(bq_priority_queue queue) {
    if (!bq_pq_is_empty(queue))
        return queue->elements[ 1 ];
    fprintf(stderr, "Priority queue is empty\n");
    return queue->elements[ 0 ];
}

int bq_pq_is_empty(bq_priority_queue queue) {
  return queue->size == 0;
}

int bq_pq_is_full(bq_priority_queue queue) {
  return queue->size == queue->capacity;
}

void bq_pq_destroy(bq_priority_queue queue) {
  free(queue->elements);
  free(queue);
}
