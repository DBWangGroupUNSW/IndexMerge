#include "header.h"
#include <sys/time.h>
#include "mgdidx.h"
#include "vanidx.h"
#include "boolean_query_processing.h"
#include "usage.h"




char g_version[]=VERSION;

extern unsigned int result_set[MAX_QUERY_RESULT_LEN];
extern char __usage_information[1024];



using namespace std;

void print_version(){
  fprintf(stderr, "Version: %s\n", g_version);
  exit(0);
}


void print_usage(){
  fprintf(stderr, "usage: <-i index file names>\n");
  fprintf(stderr, "       <-m use merged inverted list ( default )>\n");
  fprintf(stderr, "       <-n use vanilla inverted list >\n");
  print_version();
  exit(0);
}



void output_result(int id, int result_num, string &query){
  cout << id << " Query: " << query << " result num: " << result_num
       << " list:";
  for ( int i = 0; i < result_num; i++ ){
    cout << " " << result_set[i];
  }
  cout << endl;
}



int main(int argc, char* argv[])
{
  int c;
  char *infile_name = NULL; 
  merged_index_t *midx = NULL;
  vanilla_index_t *vidx = NULL;
  int vanilla = 0;
  string query;
  int query_result_num;
  int i;


  while ((c = getopt(argc,argv, "hvmni:")) != -1)
    switch (c){
      case 'i':
        infile_name = optarg;
        break;
      case 'n':
        vanilla = 1;
        break;
      case 'm':
        vanilla = 0;
        break;
      case 'h':
        print_usage();
        break;	  
      case 'v':
        print_version();
        break;
      case '?':
        if ( optopt == 'i' )
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

  cerr << "... LOADING INDEX ..." << endl;
  
  if ( vanilla ){
    vidx = build_vanilla_index_from_files(infile_name);
    if ( vidx == NULL ){
      cerr << "... INDEX LOADING ERROR ..." <<endl;
      return -1;
    }
  }else{
    midx = build_merged_index_from_files(infile_name);
    if ( midx == NULL ){
      cerr << "... INDEX LOADING ERROR ..." <<endl;
      return -1;
    }
  }
  
  timeval timeStart, timeEnd;
  gettimeofday(&timeStart, NULL);
  ResetUsage();
  cerr << "=== BEGIN QUERIES (TIMER STARTED) ===" << endl;
  i = 0;

  if ( vanilla ){
    // Do query processing on vanilla index here.
    
    while (cin){
      getline(cin, query);
      i ++;
      // call the query processing function.
      query_result_num = vanilla_query_processing(query, vidx);
      output_result (i, query_result_num, query );   
    }
  }else{
    // do qery processing on merged index here.
    
    while (cin){
      getline(cin, query);
      i ++;
      // call the query processing function.
      query_result_num = merged_query_processing(query, midx);
      output_result (i,  query_result_num, query );
    }
  }
  
  cerr << "=== END QUERIES (TIMER STOPPED) ===" << endl;
  ShowUsage();
  gettimeofday(&timeEnd, NULL);
  int io_times;
  long long io_len;

  cerr << "Rusage "  << __usage_information <<endl;
  if ( vanilla ){
    io_times = vidx->io_times;
    io_len = vidx->io_len;
  }else{
    io_times = midx->io_times;
    io_len = midx->io_len;    
  }
    
    
  cerr << "IO times: " << io_times << "  IO byte numbers:  " << io_len*4 << endl;
  cerr << "Total Running Time: " << setiosflags(ios::fixed) 
       << setprecision(3) << double(timeEnd.tv_sec - timeStart.tv_sec) 
       + double(timeEnd.tv_usec - timeStart.tv_usec) / 1e6 << endl;
  
  
  cerr << endl;
  return 0;
}


