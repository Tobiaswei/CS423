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

void do_job(int num){

 while(num--){

    factorial(100000000);
  }

}


void REGISTER(pid_t PID,int Period,int ProcessTime){

   FILE *f=fopen("/proc/mp2/status","w");
   printf("The regitser  value is %d,%d,%d\n",PID,Period,ProcessTime);
   fprintf(f, "R,%d,%d,%d", PID,Period,ProcessTime);
   fclose(f);

}

int READ_STATUS(pid_t PID){
  
   FILE *f=fopen("/proc/mp2/status","r");

   char buff[256];

   pid_t curr_pid;

   printf("checking the register pid\n");

   while(fgets(buff,256,f)){

     sscanf(buff,"%d%*s",&curr_pid);

     if(curr_pid==PID){
            fclose(f);

            return 1;
      }

   }
  fclose(f);
  printf("PID: %d falied register\n",PID);
  return 0;

}

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
  
   unsigned long t0,t1;

   int pid =getpid();

   int num=atoi(argv[2]);

   t0=get_usec();
  
  // factorial(100000000);

   do_job(num);

   t1=get_usec();
  
   srand(time(NULL));

   int ProcessTime=(int)(t1-t0)/1000;

   int random=rand()%3+3;

   int Period= random*ProcessTime;

   int num_iterations=atoi(argv[1]);

   printf("random number is %d\n", random);

   printf("The value of is %d,%d,%d",pid,Period, ProcessTime);


   REGISTER(pid,Period,ProcessTime);

  if(READ_STATUS(pid)==0){
              
        return 0;
   }

  YIELD(pid);

  int n=0;
  
   while(num_iterations--){

    printf("pid : %d  get into %dth circle \n",pid,n);

    //factorial(100000000);
    do_job(num);
    
    printf("pid: %d  finish the %dth circle\n", pid,n);

    YIELD(pid);

    n++;
     
    }
      
    DEREGISTER(pid);

    execlp("cat","cat","/proc/mp2/status",NULL);

    return 0;
}
