#include "userapp.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>


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

void REGISTER(pid_t PID,int Period,int ProcessTime){

   FILE *f=fopen("/proc/mp2/status","w");
   printf("The regitser  value is %d,%d,%d\n",PID,Period,ProcessTime);
   fprintf(f, "R,%d,%d,%d", PID,Period,ProcessTime);
   fclose(f);

}
/*
int READ_STATUS(pid_t PID){
  
   FILE *f=fopen("/proc/mp2/status","r");

   char buff[256];

   while(fgets(buff,256,f)){

     if(buff[])

   }


}
*/

void YIELD(pid_t PID){

  FILE *f=fopen("/proc/mp2/status","w");
  printf("yield process pid is %d\n",PID);
  fprintf(f, "Y,%d", PID);
  fclose(f);

}

void DEREGISTER( pid_t PID){

  FILE *f=fopen("/proc/mp2/status","w");
  printf("Deregister process id is %d\n",PID);
  fprintf(f, "D,%d", PID);
  fclose(f);

}

unsigned long get_usec(){

  struct timeval time;

  while(gettimeofday(&time,NULL));

  return time.tv_sec*1000000+time.tv_usec;

}

// There should be three arguments period process time and number_of_iteations
int main(int argc, char* argv[])

{

if(argc!=4){

     printf("There should be three arguments following as period, process time and num_iterations\n");
     return 0;
  }


  
 unsigned long t0,t1;

 //  
  int pid =getpid();

  int Period=atoi(argv[1]); 

  int ProcessTime=atoi(argv[2]);

  int num_iterations=atoi(argv[3]);

///  int ProcessTime;

  while(t1-t0>0){

     t0=get_usec();
  
     factorial(100000000);

     t1=get_usec();

  }
  
  srand(time(NULL));

  int random=rand()%5+5;

  int Period= random*ProcessTime;

  int num_iterations=atoi(argv[1]);

  printf("The value of is %d,%d,%d",pid,Period, ProcessTime);

   int pid =getpid();

/
  REGISTER(pid,Period,ProcessTime);

  //int list=READ_STATUS(pid);

  //if(list)

  YIELD(pid);

  int n=0;
  
   while(num_iterations--){

    printf("get into %d\n", n);

    factorial(100000000);
    
    printf("finish the %d job\n", n);

    YIELD(pid);

    n++;
     
    }
      
    DEREGISTER(pid);

    execlp("cat","cat","/proc/mp2/status",NULL);

    return 0;
}
