#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include <string>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "header.h"


char g_version[]=VERSION;
using namespace std;

typedef list<int> listtype;
typedef list<int>::iterator listtypeit;
typedef map<string, listtype> maptype;
typedef map<string, listtype>::iterator maptypeit;

maptype invert;
typedef struct _order_node_t{
    int id;
    int len;
}order_node_t;

// We generate a token list as a stop words.
// but We eliminate the duplicate did from 
// the inverted list to simplify this problem
// to a certain level.

int tokenize_stop(string s, int did)
{
    char cstr[STRING_LEN];
    char sep[] = " _.,?!\t";
    char *brkb;
    char *token;

  
    strcpy(cstr, s.c_str());

    for (token = strtok_r(cstr, sep, &brkb);
         token;
         token = strtok_r(NULL, sep, &brkb)){
        maptypeit mapit = invert.find(token);
        if (mapit != invert.end()){
            listtype *ivlistp = &mapit->second;
            if(ivlistp->back() != did)
                ivlistp->push_back(did);
        }else{
            listtype *ivlistp = new listtype();
            ivlistp->push_back(did);
            invert[token] = *ivlistp;
        }
    }
    return 0;
}


// Most time we use q-gram as tokenizing method. 
// This method will generate too much redundency.
// We also eliminate duplicate index did to simplify
// our problem. 

int tokenize_qgram(string s, int did, int q)
{
    char token[STRING_LEN];
    int len = s.length();
    int l = 0;
  
    for (int i = 0; i < len - q; i++){
        l = s.copy(token, q, i);
        token[l] = '\0'; 
        maptypeit mapit = invert.find(token);
        if (mapit != invert.end()){
            listtype *ivlistp = &mapit->second;
            if(ivlistp->back() != did)
                ivlistp->push_back(did);
        }else{
            listtype *ivlistp = new listtype();
            ivlistp->push_back(did);
            invert[token] = *ivlistp;
        }
    }
    return 0;
}





int cmpOrder(const void *a, const void *b)
{
    if (((order_node_t*)a)->len < ((order_node_t*)b)->len)
        return -1;
    else if (((order_node_t*)a)->len == ((order_node_t*)b)->len 
             && ((order_node_t*)a)->id < ((order_node_t*)b)->id)
        return -1;
    else
        return 1;
}

int output_invert(char *filename){
    int ntoken = invert.size();
    listtype ** tivlist = new listtype* [ntoken];
    string * tstrlist = new string[ntoken];
    int i = 0;

    for (maptypeit mapit = invert.begin(); mapit != invert.end(); mapit ++){
        if ( mapit->second.size() < 5 ) continue;	 
        tivlist[i] = &mapit->second; // make a map into a list
        tstrlist[i] = mapit->first;
        i ++;
    }
    ntoken = i;
  
    order_node_t *order = new order_node_t[ntoken];
    for (i = 0; i < ntoken; i++) {order[i].id = i; order[i].len = tivlist[i]->size();}
  
    qsort(order, ntoken, sizeof(order_node_t), cmpOrder);
  
    for (i = 0; i < ntoken; i++){
        cout << "tid_" << i+1 << " "<< order[i].len << " "<< tstrlist[order[i].id];
        for (listtypeit listit=tivlist[order[i].id]->begin();
             listit != tivlist[order[i].id]->end();
             listit ++){
            cout << " " << *listit;
        }
        cout << endl;
    }

    FILE *fp = fopen(filename, "wb");    
    for(i = 0; i < ntoken; i++){
        int tid = order[i].id;
        int slen = tstrlist[tid].length() + 1;
        int p  = 0;
        char sbuf[1024];
        strcpy (sbuf, tstrlist[tid].c_str());
        sbuf[slen] = '\0';
        if (order[i].len == 0) continue;
        int k = i + 1;

        fwrite(&k, sizeof(int), 1, fp);  // Token number
        fwrite(&slen, sizeof(int), 1, fp);  // Token string length 
        fwrite(&order[i].len, sizeof(int), 1, fp);  // Token inverted list length 
        fwrite(&p, sizeof(int), 1, fp);      // Token Top-k list length
        fwrite(sbuf, sizeof(char), slen, fp); // Token string content

        for (listtypeit listit=tivlist[tid]->begin();       
             listit != tivlist[tid]->end();                 
             listit ++){          
            k = *listit;
            //	  cout << "output k " << k;
            fwrite(&k, sizeof(int), 1, fp);
        }                                	
    }

    fclose(fp);


    delete [] tivlist;
    delete [] tstrlist;
    delete [] order;

    return 0;
}

void print_version(){
  fprintf(stderr, "Version: %s\n", g_version);
  exit(0);
}


void print_usage(){
  fprintf(stderr, "usage: <-q <qgram length> defaut is tokenize>\n");
  fprintf(stderr, "       <-o <output index name>>\n");
  print_version();
  exit(0);
}


int main(int argc, char* argv[])
{
  int q = 0;
  char *output = NULL;
  char c;
  string i;
  int n = 0;

  while ((c = getopt(argc,argv, "hvq:o:")) != -1)
    switch (c){
      case 'q':
        q = atoi(optarg);
        break;
      case 'o':
        output = optarg;
        break;
      case 'h':
        print_usage();
        break;	  
      case 'v':
        print_version();
        break;
      case '?':
        if ( optopt == 'q' || optopt == 'o' )
          cerr << "Error: Option -" << optopt << "requires an argument." << endl;
        else if ( isprint(optopt))
          cerr << "Error: Unknown Option -" << optopt << endl;
        else
          cerr << "Error: Unknown Option character" <<endl;
        return 1;
      default:
        print_usage();
    }

  // output name check.
  if ( output == NULL ){
    cerr << "Need out put file name" <<endl;
    print_usage();
  }

  while (cin){
    getline(cin, i);
    n++;
    if ( q <= 0 )
      tokenize_stop(i,n);
    else
      tokenize_qgram(i,n,q);
    if ( n % 10000 == 0 )
      cerr << n << endl;
  }
  cerr << " output now" <<endl;
  output_invert(output);
  return 0;
}

