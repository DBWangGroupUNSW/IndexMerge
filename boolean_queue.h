#ifndef __BOOLEAN_QUEUE_H__
#define __BOOLEAN_QUEUE_H__

#include <queue>
#include <vector>

using namespace std;


#define QUE_END_MARK 0xffffffff
#define BQ_PQ_MIN_SIZE  (3)
#define BQ_PQ_MIN_DATA  (-32767)
#define BQ_PQ_MAX_DATA  (0xffffffff)



// boolean_queue is the root class of all the
// other boolean_queue classes. boolean_queue 
// is designed to use priority queue (heap) to
// merge queues.
class boolean_queue{
  public:
  unsigned int top;                  /* Keep the top data */
  virtual void pop(){};                        /* pop the top data  */
  virtual void destroy(){};                     /* destroy objects   */
};


typedef boolean_queue*  bq_pq_element_type;

typedef struct _bq_heap_struct_t {
    int capacity;
    int size;
    bq_pq_element_type *elements;
} bq_heap_struct_t;

typedef bq_heap_struct_t* bq_priority_queue;

bq_priority_queue bq_pq_initialize(int);
void bq_pq_destroy(bq_priority_queue);
void bq_pq_make_empty(bq_priority_queue);
void bq_pq_insert(bq_pq_element_type, bq_priority_queue);
bq_pq_element_type bq_pq_delete_min(bq_priority_queue);
bq_pq_element_type bq_pq_find_min(bq_priority_queue);
int bq_pq_is_empty(bq_priority_queue);
int bq_pq_is_full(bq_priority_queue);



/* Sub-class of boolean_queue.                */
/* This queue calculate the intersection of   */
/* all the queues in this queus               */
/* every time it pop out one element which is */
/* the minimal element all queue have         */
class and_boolean_queue: public boolean_queue{
  private: 
  bq_priority_queue heap_queue;          /* priority queue to keep minimal heap */
  int len;                      /* sub queue list it have */  

  public:
  and_boolean_queue();
  /* constructor of and_boolean_queue            */
  /* Thsi function build a boolean_queue object  */
  /* from a list of boolean_queue objects        */
  and_boolean_queue(int, boolean_queue*, int);
  void initialize(int, boolean_queue*, int);

  /* add a boolean_queue into this queue */
  void add(boolean_queue*);
  void pop();
  void destroy();
};


/* Sub-class of boolean_queue.                    */
/* This queue is to calculate a union queu of its */
/* members.  */
class or_boolean_queue: public boolean_queue{
  private:
  bq_priority_queue heap_queue;
  int len;
  
  public:
  or_boolean_queue(int, boolean_queue *, int);

  /* add a boolean_queue into this queue */
  void add(boolean_queue*);
  void pop();
  void destroy();
};


/* Sub-class of boolean_queue.                    */
/* Merged index queue   */
/* members.  */
class merge_boolean_queue: public boolean_queue{
  private:
  int len;
  int *ivdata;
  int pos[1<<MAX_MERGE_CLUSTER_SIZE];
  int endp[1<<MAX_MERGE_CLUSTER_SIZE];
  int heap[1<<MAX_MERGE_CLUSTER_SIZE];
  int heap_size;
  void heap_insert(int x);
  int heap_delete_min();
  
  public:
  merge_boolean_queue();
  merge_boolean_queue(int *indata, int mod, int depth, int *group_array);
  void initialize(int *indata, int mod, int depth, int *group_array);

  void pop();
  void destroy();
};

class single_boolean_queue: public boolean_queue{
  private:
  int len;
  int pos;
  unsigned int *ivlist;

  public:
  single_boolean_queue();
  single_boolean_queue(int, unsigned int*);
  void initialize(int, unsigned int*);
  void pop();  
  void destroy();
};




#endif
