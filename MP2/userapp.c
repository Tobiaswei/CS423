#include "userapp.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int factorial(int n){
  
   int res=1;
   for(int i=2;i<=n;i++){
    res*=i;

   }
   return res;
}


void do_job(void){

      factorial(1000000000);
      factorial(1000000000);

}

void REGISTER(pid_t PID,unsigned long Period,unsigned long ProcessTime){

   FILE *f=fopen("/proc/mp2/status","w");
   fprintf(f, "R,%d,%d,%d", PID,Period,ProcessTime);
   fclose(f);

}

int READ_STATUS(pid_t PID){
  
   FILE *f=fopen("/proc/mp2/status","r");

   char buff[256];

   while(fgets(buff,256,f)){

     if(buff[])

   }


}

void YIELD(pid_t PID){

  FILE *f=fopen("/proc/mp2/status","r");
  fprintf(f, "Y,%d", PID);


}

void DEREGISTER( pid_t PID){

  FILE *f=fopen("/proc/mp2/status","r");
  fprintf(f, "D,%d", PID);

}

// There should be three arguments period process time and number_of_iteations
int main(int argc, char* argv[])
{

  struct timeval start;
  struct timeval end;
  gettimeofday(&start, NULL);
  //factorial(1000000000);
  //factorial(1000000000);
  //factorial(1000000000);
  //factorial(1000000000);
  factorial(1000000000);
  gettimeofday(&end, NULL);


  int pid =getpid();

  int ProcessTime=(int)(end.tv_usec-start.tv_usec)

  int Period=3*ProcessTime;

  int num_iterations=atio(argv[1]);


/*
  if(argc!=4){

     printf("There should be three arguments following as period, process time and num_iterations\n");
     return 0;
  }

  int pid =getpid();

  int Period=atoi(argv[1]); 

  int ProcessTime=atoi(argv[2]);

  int num_iterations=atoi(argv[3]);
*/
    //FILE * file =fopen("/proc/mp2/status","a");

  REGISTER(pid,Period,ProcessTime);

  //int list=READ_STATUS(pid);

  //if(list)

  YIELD(pid);

  int n=0;
  while(num_iterations--){

    printf("get into %d\n", n);

    factorial(1000000000);
    
    printf("finish the %d job\n", n);

    YIELD(pid);

    n++;
     
    }
      
    DEREGISTER(pid);

    execlp("cat","cat","/proc/mp1/status",NULL);

    return 0;
}
