#include "header.h"
#include <sys/time.h>
#include "mgdidx.h"
#include "greedymerge.h"
#include "binio.h"
#include "index.h"

char g_version[]=VERSION;

using namespace std;

extern int n;
extern int total_index_size;
extern int total_benifit;
extern int m;



void print_version(){
  fprintf(stderr, "Version: %s\n", g_version);
  exit(0);
}


void print_usage(){
  fprintf(stderr, "usage: <-b binary input file name>\n");
  fprintf(stderr, "       <-o output index file>\n");
  fprintf(stderr, "       <-m highest merge order>\n");
  print_version();
  exit(0);
}


int main(int argc, char* argv[])
{
  int c;
  char *infile_name = NULL; 
  char *outindex_name = NULL;
  merge_cluster_t *mcls = NULL;
  int cls_len;
  int order = MAX_MERGE_CLUSTER_SIZE;
 // merged_index_t *midx;


  while ((c = getopt(argc,argv, "hvb:l:o:m:")) != -1)
    switch (c){
      case 'b':
        infile_name = optarg;
        break;
      case 'o':
        outindex_name = optarg;
        break;
      case 'm':
        order = atoi(optarg);
        break;
      case 'h':
        print_usage();
        break;	  
      case 'v':
        print_version();
        break;
      case '?':
        if ( optopt == 'b' || optopt == 'o' || optopt == 'm' )
          cerr << "Error: Option -" << optopt << "requires an argument." << endl;
        else if ( isprint(optopt))
          cerr << "Error: Unknown Option -" << optopt << endl;
        else
          cerr << "Error: Unknown Option character" <<endl;
        return 1;
      default:
        print_usage();
    }  

  if ( infile_name == NULL ){
    print_usage();
    exit(-1);
  }
  
  cerr << "... LOADING DATASET ..." << endl;
  readTokenBinary(infile_name);

  mcls = new merge_cluster_t[n];    

  timeval timeStart, timeEnd;
  gettimeofday(&timeStart, NULL);
  cerr << "=== BEGIN JOIN (TIMER STARTED) ===" << endl;
  generate_index();           // Need to generate index to do top-k join.


  // Try to find a right function for order.

  if( order >= 2 && order <= MAX_MERGE_CLUSTER_SIZE ){
    cls_len = greedy_merge(mcls, n, order);    
  }else{
    cerr << "invalid order parameter." << endl;
    return -1;
  }

  dump_merge_clusters(mcls, cls_len);
  dump_merge_pairs(mcls, cls_len);
  //  If user defined output file, then output index.
  if (outindex_name != NULL){
    build_merged_index_from_deep_merge(mcls, cls_len, outindex_name);
  }  

  cerr << "=== END JOIN (TIMER STOPPED) ===" << endl;
  gettimeofday(&timeEnd, NULL);
  cerr << "Total Running Time: " << setiosflags(ios::fixed) 
       << setprecision(3) << double(timeEnd.tv_sec - timeStart.tv_sec) + double(timeEnd.tv_usec - timeStart.tv_usec) / 1e6 << endl;

  cerr << endl;
  return 0;
}


