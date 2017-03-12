#include <stdlib.h>
#include <stdio.h>
#include "header.h"
#include "boolean_queue.h"



using namespace std;




and_boolean_queue::and_boolean_queue(){
  len = 0;
  heap_queue = NULL;
  top = QUE_END_MARK;
}



/* constructor of and_boolean_queue            */
/* Thsi function build a boolean_queue object  */
/* from a list of boolean_queue objects        */
and_boolean_queue::and_boolean_queue(int l, boolean_queue *bq_list, int t){
  len = l;
  heap_queue = bq_pq_initialize(t);
  for (int i = 0; i < len; i++){
    bq_pq_insert(&bq_list[i], heap_queue);
  }
  top = 0;
}


/* constructor of and_boolean_queue            */
/* Thsi function build a boolean_queue object  */
/* from a list of boolean_queue objects        */
void and_boolean_queue::initialize(int l, boolean_queue *bq_list, int t){
  len = l;
  heap_queue = bq_pq_initialize(t);
  for (int i = 0; i < len; i++){
    bq_pq_insert(&bq_list[i], heap_queue);
  }
  top = 0;
}


/* add a boolean_queue into this queue */
void and_boolean_queue::add(boolean_queue *bq){
  bq_pq_insert(bq, heap_queue);
  len ++;
}

/* pop out the top one and find the next intersection    */

/* of all the queue lists. if one of the queus runs over */
/* the whole list will be destroied cause of no more     */
/* answer. */
void and_boolean_queue::pop(){
  int count;                  /* count the exist time. */
  unsigned int last;                   /* last one */
  boolean_queue *bq;

  if(top == QUE_END_MARK)
    return;
  
  if(bq_pq_is_empty(heap_queue)){
    top  = QUE_END_MARK;
    return;
  }

  last = bq_pq_find_min(heap_queue)->top; 
  count = 1;
  while(heap_queue->size > 0){
    bq = bq_pq_delete_min(heap_queue);
    bq->pop();
    if(bq->top == QUE_END_MARK){
      bq->destroy();

      while  (heap_queue->size > 0){
        bq = bq_pq_delete_min(heap_queue);
        if ( bq->top == last )
          count++;
        bq->destroy();
      }
      if (count >=len){
        top = last;
      }else{
        top = QUE_END_MARK;
      }
      return;
    }
    bq_pq_insert(bq, heap_queue);    
    bq = bq_pq_find_min(heap_queue);    
    if(bq->top == last){
      count++;
    }else{
      if(count >= len){
        top = last;
        return;
      }
      count = 1;
      last=bq->top;
    }
  }
  top = QUE_END_MARK;
  return;
}

void and_boolean_queue::destroy(){
  boolean_queue *bq;
  if(heap_queue!=NULL){
      while(!bq_pq_is_empty(heap_queue)){
          bq = bq_pq_delete_min(heap_queue);
          bq->destroy();
      }
      bq_pq_destroy(heap_queue);
  }
  heap_queue = NULL;
}




/* constructor of and_boolean_queue            */
/* Thsi function build a boolean_queue object  */
/* from a list of boolean_queue objects        */
or_boolean_queue::or_boolean_queue(int l, boolean_queue *bq_list, int t){
  len = l;
  top = 0;
  heap_queue = bq_pq_initialize(t);
  for (int i = 0; i < len; i++){
    bq_pq_insert(&bq_list[i], heap_queue);
  }
}

/* add a boolean_queue into this queue */
void or_boolean_queue::add(boolean_queue *bq){
  bq_pq_insert(bq, heap_queue);
  len ++;
}




/* pop out the top one and find the next intersection    */
/* of all the queue lists. if one of the queus runs over */
/* the whole list will be destroied cause of no more     */
/* answer. */
void or_boolean_queue::pop(){
  boolean_queue *bq;

  if(top == QUE_END_MARK)
    return;

  if(!bq_pq_is_empty(heap_queue)){
    bq = bq_pq_delete_min(heap_queue);
    top = bq->top;
    bq->pop();
    
    if(bq->top == QUE_END_MARK){
      bq->destroy();
      return;
    }else{
      bq_pq_insert(bq, heap_queue);
      return;
    }
  }
  top = QUE_END_MARK;
  return;
}



void or_boolean_queue::destroy(){
  boolean_queue *bq;
  while(!bq_pq_is_empty(heap_queue)){
    bq = bq_pq_delete_min(heap_queue);
    bq->destroy();
  }
  bq_pq_destroy(heap_queue);
}



single_boolean_queue::single_boolean_queue()
{
  ivlist = NULL;
  len = 0;
  pos = 0;
}


void single_boolean_queue::initialize(int l, unsigned int *list)
{
  len = l;
  pos = 0;
  top = 0;
  ivlist = list;
  pop();
}

single_boolean_queue::single_boolean_queue(int l, unsigned int *list)
{
  len = l;
  pos = 0;
  top = 0;
  ivlist = list;
  pop();
}

void single_boolean_queue::pop(){
  if (pos < len){
    top = ivlist[pos++];      
  }else{ 
    top = QUE_END_MARK;
  }
}

void single_boolean_queue::destroy(){
  ivlist = NULL;
  len = 0;
  pos = 0;
  top = QUE_END_MARK;
}


