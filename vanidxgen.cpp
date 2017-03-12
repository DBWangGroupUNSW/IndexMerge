#include "header.h"
#include <sys/time.h>
#include "vanidx.h"
#include "binio.h"
#include "greedymerge.h"


char g_version[]=VERSION;

using namespace std;

extern int limit;
extern int threshold;

extern int total_index_size;
extern int total_benifit;
extern int n;

extern char *tokstr[STRING_NUM];
extern int tokstrlen[STRING_NUM];
extern int len[STRING_NUM];
extern int *token[STRING_NUM];


void print_version(){
  fprintf(stderr, "Version: %s\n", g_version);
  exit(0);
}


void print_usage(){
  fprintf(stderr, "usage: <-b binary input file name>\n");
  fprintf(stderr, "       <-o output index file>\n");
  print_version();
  exit(0);
}


int main(int argc, char* argv[])
{
  int c;
  char *infile_name = NULL; 
  char *outindex_name = NULL;

  while ((c = getopt(argc,argv, "hvb:o:")) != -1)
    switch (c){
      case 'b':
        infile_name = optarg;
        break;
      case 'o':
        outindex_name = optarg;
        break;
      case 'h':
        print_usage();
        break;	  
      case 'v':
        print_version();
        break;
      case '?':
        if ( optopt == 'b' || optopt == 'o')
          cerr << "Error: Option -" << optopt << "requires an argument." << endl;
        else if ( isprint(optopt))
          cerr << "Error: Unknown Option -" << optopt << endl;
        else
          cerr << "Error: Unknown Option character" <<endl;
        return 1;
      default:
        print_usage();
    }  

  if ( infile_name == NULL || outindex_name == NULL ){
    print_usage();
    exit(-1);
  }
  
  cerr << "... LOADING DATASET ..." << endl;
  readTokenBinary(infile_name);

  timeval timeStart, timeEnd;
  gettimeofday(&timeStart, NULL);
  cerr << "=== BEGIN JOIN (TIMER STARTED) ===" << endl;

  build_vanilla_index(outindex_name);

  cerr << "=== END JOIN (TIMER STOPPED) ===" << endl;
  gettimeofday(&timeEnd, NULL);
  cerr << "Total Running Time: " << setiosflags(ios::fixed) 
       << setprecision(3) << double(timeEnd.tv_sec - timeStart.tv_sec) 
          + double(timeEnd.tv_usec - timeStart.tv_usec) / 1e6 << endl;

  cerr << endl;

  return 0;
}


