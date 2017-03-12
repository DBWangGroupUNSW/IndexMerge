#ifndef __HIGH_MERGE_H__
#define __HIGH_MERGE_H__

#include "header.h"

#define USELESS_MODE 0         // token slot no use

#define SINGLE_TOKEN_MODE 1    // a token is pointed by this.
#define GROUP_MEMBER_MODE 2    // a token is a group member.
#define OPEN_GROUP_MODE 3      // a group is open for merge.
#define CLOSED_GROUP_MODE 4    // a group is closed for merge;
#define CLOSED_TOKEN_MODE 5    // a group is closed for merge;



typedef struct _token_info_t{
  int mode;   // Single token,group member, on going group or finished group.
  int size;   // size of this group, if it is a group.
  int saving; // sving of space in this token.
  int top;      // top one.
  int topov;    // top one over lap number.
  int gid;  // group id is the index in the token_info_array
  int members[MAX_MERGE_CLUSTER_SIZE];  // members list is the place for envy.
} token_info_t;


/* This defines merge cluster. */
typedef struct _merge_cluster_t{
  int depth;                    /* How many cluster member */
  int tokens[MAX_MERGE_CLUSTER_SIZE]; /* Token ids in this cluster */
  int saving;                         /* Savings on the cluster */
  int robbered;
}merge_cluster_t;

/* Function for dump out merge clusters. */
void dump_merge_clusters(merge_cluster_t *mcls, int cls_len);

void dump_merge_pairs(merge_cluster_t *mcls, int cls_len);

int greedy_merge(merge_cluster_t * mcls, int cls_size, int depth);

#endif
