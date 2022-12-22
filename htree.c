/*
 * htree.c
 * 
 * Created by Sadia Khan
 * Compile by: gcc htree.c –o htree -Wall -Werror -std=gnu99 -pthread
 * Notes:
 *     This is a program that computes and prints the hash value of a given file
 */


#include <stdio.h>     
#include <stdlib.h>   
#include <stdint.h>  
#include <inttypes.h>  
#include <errno.h>     // for EINTR
#include <fcntl.h>     
#include <unistd.h>    
#include <sys/types.h>
#include <sys/stat.h>
#include "common.h"
#include "common_threads.h"
#include <sys/mman.h>
#include <string.h>


//struct for creating tree
struct parameters{
	int root;
	int threads;
	uint32_t result;
};

void *tree(void *arg);

// Print out the usage of the program and exit.
void Usage(char*);
uint32_t jenkins_one_at_a_time_hash(const uint8_t* , uint64_t );

//some variables
#define BSIZE 4096
uint32_t* htable;
uint64_t nmblocks;
uint8_t *arr; 
int32_t fd;
uint32_t hash;



int 
main(int argc, char** argv) 
{
  uint32_t nblocks;

  // input checking 
  if (argc != 3)
    Usage(argv[0]);

  // open input file
  fd = open(argv[1], O_RDWR);
  if (fd == -1) {
    perror("open failed");
    exit(EXIT_FAILURE);
  }
  // use fstat to get file size
  struct stat buf;
  if (fstat(fd, &buf)== -1){
      perror("fstat failed");
  }
  
  size_t size = buf.st_size;
  
  
  // calculate nblocks which is number of blocks
  nblocks=size/BSIZE;
  printf("no. of blocks = %u\n", nblocks);

  //get threads
  int index;
  index=atoi(argv[2]);
  printf("no. of threads = %d \n",index);

  //calculate blocks per thread
  nmblocks=nblocks/index;
  printf("no. of blocks per thread = %"PRIu64"\n", nmblocks);
  
  //define things for thread creation
  struct parameters par;
  par.root=0;
  par.threads=index;
  
  //create thread and wait for it to return to get final hash
  pthread_t p1;
  
  double start = GetTime();
  Pthread_create(&p1, NULL, tree, &par);
  Pthread_join(p1, NULL);
  hash=par.result;
  double end = GetTime();

  //print hash and time out
  
  printf("hash value = %u \n", hash);
  printf("time taken = %f \n", (end - start));
  close(fd);
  return EXIT_SUCCESS;
 
}

//this is the hashing function
//it computes hash value of blocks we pass to it 
//also need pass length of blocks
uint32_t 
jenkins_one_at_a_time_hash(const uint8_t* key, uint64_t length) 
{
  uint64_t i = 0;
  uint32_t hash = 0;

  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

//usage 
void 
Usage(char* s) 
{
  fprintf(stderr, "Usage: %s filename num_threads \n", s);
  exit(EXIT_FAILURE);
}

//this creates our thread tree which hashes blocks and returns final hash value
void* 
tree(void* arg) 
{
 
	//setting up data from parameters
  struct parameters* par = (struct parameters*) arg;
  //left child
  struct parameters par1;
  par1.root=par->root*2+1;
  par1.threads=par->threads;
	
  struct parameters* par1new= &par1;
  
  //right child
  struct parameters par2;
  par2.root=par->root*2+2;
  par2.threads=par->threads;
	
  struct parameters* par2new= &par2;
  
  //calculate which blocks for each thread
  uint64_t blocks;
  blocks=par->root*nmblocks;
  
  uint32_t hash1;  

  //left child
  if(par1.root<par->threads){

	  //create threads
	  pthread_t p2;
	  
	  Pthread_create(&p2, NULL, tree, par1new);
	  
	  //read blocks + hashing values
	  arr=mmap(NULL, nmblocks*BSIZE, PROT_READ, MAP_PRIVATE, fd, blocks*BSIZE);
	  hash1=jenkins_one_at_a_time_hash(arr, nmblocks*BSIZE);
	  par->result=hash1;
	  
	  
	  //right child
	  if(par2.root<par->threads){
		pthread_t p3;
		Pthread_create(&p3, NULL, tree, par2new);
  	    arr=mmap(NULL, nmblocks*BSIZE, PROT_READ, MAP_PRIVATE, fd, blocks*BSIZE);
  	    hash1=jenkins_one_at_a_time_hash(arr, nmblocks*BSIZE);
  	    par->result=hash1;
		Pthread_join(p3,NULL);
		
	  } 
	  
	  Pthread_join(p2, NULL);
	  
	  //getting ready to concatinate + allocate to heap 
	  char *left=(char *)malloc(sizeof(par1.result)+1);
	  char *right=(char *)malloc(sizeof(par2.result)+1);
	  char *other=(char *)malloc(sizeof(par->result)+1);
	  sprintf(right, "%u", par2.result);
	  sprintf(left, "%u", par1.result);
	  sprintf(other, "%u", par->result);	  
	  
	  char *combine=(char *)malloc(sizeof(left)+sizeof(right)+sizeof(other)+1);
	  combine=strcat(other, left);
	  if(par2.result != 0){
		  combine=strcat(combine, right);
	  	
	  }
	  
	  //hashing results
 	 hash1=jenkins_one_at_a_time_hash((uint8_t*)combine, strlen(combine));
 	 par->result=hash1;
	  
		
  }
	
 else{
	 //last thread so exit happens
    	 arr=mmap(NULL, nmblocks*BSIZE, PROT_READ, MAP_PRIVATE, fd, blocks*BSIZE);
    	 hash1=jenkins_one_at_a_time_hash(arr, nmblocks*BSIZE);
    	 par->result=hash1;
	     pthread_exit(NULL);
  } 
  
  

  return NULL;
}

