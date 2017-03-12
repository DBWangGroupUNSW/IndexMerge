

#include "header.h"
#include <sys/time.h>
#include "mgdidx.h"


char g_version[]=VERSION;


using namespace std;

void print_version(){
  fprintf(stderr, "Version: %s\n", g_version);
  exit(0);
}


void print_usage(){
  fprintf(stderr, "usage: <-i index file names>\n");
  fprintf(stderr, "       <-g group member that want to show>\n");
  fprintf(stderr, "       <-d show document ids>\n");
  fprintf(stderr, "       <-z filter zeros>\n");
  print_version();
  exit(0);
}





extern int bitmods[];
static char b[100];


const char* byte_to_binary( int x, int depth )
{
  int z, y;
  for (z = bitmods[depth-1], y=0; z>0; z>>=1, y++)
  {
    b[y] = ( ((x & z) == z) ? 49 : 48);
  }
  b[y+1] = '\0';
  return b;
}



int main(int argc, char* argv[])
{
  int c;
  char *infile_name = NULL; 
  merged_index_t *midx = NULL;
  char * groupm;  
  int showdoc = 0;
  int filtezero = 0;

  while ((c = getopt(argc,argv, "i:g:dhzv")) != -1)
    switch (c){
      case 'i':
        infile_name = optarg;
        break;
      case 'g':
        groupm = optarg;
        break;
      case 'd':
        showdoc = 1;
        break;        
      case 'z':
        filtezero = 1;
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

  midx = build_merged_index_from_files(infile_name);
  if ( midx == NULL ){
    cerr << "... INDEX LOADING ERROR ..." <<endl;
    return -1;
  }
  

  dict_node_t pnode;
  int token_pos;
  int group_depth;
  int group_pos;
  int group_list[100];
#ifdef MD5SIGN
  create_sign_md5(groupm, strlen(groupm),
              &pnode.sign1, &pnode.sign2);
#else
  create_sign_bitwise(groupm, &pnode.sign1, &pnode.sign2);
#endif
  if (dict_search(midx->token_dict, &pnode) 
       == RT_HASH_DICT_SEARCH_SUCC ){
    token_pos = (int) pnode.pointer;
  }else{
    cout << "----- Token " << groupm << " Not find in index -----" << endl;
    exit(-1);
  }
  group_pos = midx->token_list[token_pos].cluster_pos;
  group_depth = midx->token_list[token_pos].cluster_depth;
  
  for ( int i = 0; i < midx-> tok_num; i++ ){
    if (midx->token_list[i].cluster_pos == group_pos){
      group_list[midx->token_list[i].token_order] = i;
    }
  }

  cout << "------Dump group contains " << groupm << " with depth " << group_depth << " .-----" << endl;

  for ( int i = 0; i < group_depth; i++ ) {    
    cout << "\"" << midx->token_list[group_list[i]].token << "\" ";
  }
  cout << endl;


  int levels[MAX_MERGE_CLUSTER_SIZE];
  for ( int i = 0; i < MAX_MERGE_CLUSTER_SIZE; i++ )
    levels[i] = 0;


  for ( int i = 1; i < bitmods[group_depth]; i++ ){
    int t = 0;
    int idx_pos = midx->cluster_array[group_pos + i - 1];
    int idx_len = (midx->cluster_array[group_pos + i] - idx_pos)>>2;
    if ( filtezero  && idx_len == 0 )
      continue;
    cout << byte_to_binary(i, group_depth) << " ";
    idx_pos = idx_pos >> 2;
    cout << " " << idx_len << " { ";

    for ( int p = 0; p < group_depth; p ++){
      if ( (i & bitmods[p]) > 0 ){
        t ++;
        cout << "\"" << midx->token_list[group_list[p]].token << "\" ";
      }
    }
    cout << "}";

    if ( showdoc){
      cout << "  [";
      for ( int t = 0; t <  + idx_len; t++){
        cout << midx->idx_buf[idx_pos + t] << ", ";
      }
      cout << "]";
    }
    levels[t-1] += (idx_len * (t - 1));
    cout << endl;
  }

  int totalsave = 0;
  for ( int i = 0; i < MAX_MERGE_CLUSTER_SIZE; i++ ){
    totalsave += levels[i];
  }

  cerr << "Group Level info: Total Save " << totalsave << endl;
  for ( int i = 0; i < group_depth; i++ ){
    fprintf(stderr, "Level %d save: %d Portion: %.2f%s \n", i, levels[i], (levels[i] * 100.0) / totalsave, "%" );
  }

  return 0;
}


