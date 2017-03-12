#include <sys/time.h>
#include <math.h>
#include <stdlib.h>
#include <cstdlib>
#include <stdio.h>
#include <time.h>
#include "header.h"
#include "binio.h"


char g_version[]=VERSION;

using namespace std;

extern int n;
extern char *tokstr[STRING_NUM];
extern int tokstrlen[STRING_NUM];
extern int len[STRING_NUM];
extern int *token[STRING_NUM];
extern int klen[STRING_NUM]; // topk number
extern int *topk[STRING_NUM]; //topk information
extern int *topkov[STRING_NUM]; //top k over lapping


void print_version(){
  fprintf(stderr, "Version: %s\n", g_version);
  exit(0);
}


void print_usage(){
  fprintf(stderr, "usage: <-b binary input file name>\n");
  fprintf(stderr, "       <-m maximal query tokens>\n");
  print_version();
  exit(0);
}


int main(int argc, char* argv[])
{
  int c;
  char *infile_name = NULL;
  int order = MAX_MERGE_CLUSTER_SIZE;

  while ((c = getopt(argc,argv, "hvb:m:")) != -1)
    switch (c){
      case 'b':
        infile_name = optarg;
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

  timeval timeStart, timeEnd;
  gettimeofday(&timeStart, NULL);
  cerr << "=== BEGIN JOIN (TIMER STARTED) ===" << endl;

  srand ( time(NULL) );

  int filter[1024];

  // for every token, generate a number of tokens related to their 
  // token length.
  for ( int i = 0; i < n ; i++ ){
    int q = (int)log10(len[i]) + 1;
    while ( q > 0 ){
      q --;
      memset(filter, 0, sizeof(int)*1024);
      cout << tokstr[i];
      for ( int j = 0; j < order; j++ ){
        int k = ((int)rand());
        k = k % klen[i];
        if (filter[k] == 0){
          filter[k] = 1;
          cout << " " << tokstr[topk[i][k]];
        }
      }
      cout << endl;
    }
  }

  cerr << "=== END JOIN (TIMER STOPPED) ===" << endl;
  gettimeofday(&timeEnd, NULL);
  cerr << "Total Running Time: " << setiosflags(ios::fixed) 
       << setprecision(3) << double(timeEnd.tv_sec - timeStart.tv_sec) + double(timeEnd.tv_usec - timeStart.tv_usec) / 1e6 << endl;
  cerr << endl;

  return 0;
}


