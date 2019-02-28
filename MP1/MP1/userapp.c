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
void cat_function(){

printf("The cat /proc/mp1/status is following \n");


FILE * file=fopen("/proc/mp1/status","r");

char buff[2048];

ssize_t bytes=read(fileno(file),(void*)buff,2048);
buff[bytes-1]='\0';
printf("%s",buff);

fclose(file);


}


int main(int argc, char* argv[])
{
      int pid =getpid();

      FILE * file =fopen("/proc/mp1/status","a");
      
     if(file){
       fprintf(file,"%d\0",pid);
       
        printf("My pid is %d\n",pid);
        fclose(file);
        
      }else{
        printf("proc_file not create properly!");
      }
      factorial(1000000000);
      factorial(1000000000);
      factorial(1000000000);
      factorial(1000000000);
     // cat_function();
      execlp("cat","cat","/proc/mp1/status",NULL);
      return 0;
}
