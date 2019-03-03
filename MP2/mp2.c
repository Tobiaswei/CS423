#define LINUX
#include<asm/uaccess.h>//(copy_from_user)
#include <linux/module.h>
#include <linux/kernel.h>
#include<linux/slab.h>// kmalloc;
#include "mp2_given.h" //get_cpu_time;
#include <linux/list.h>
#include <linux/proc_fs.h> 
#include <linux/time.h>
#include<linux/string.h>
//#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
//#include <linux/vmalloc.h>
//kthread lib
#include <linux/sched.h>
#include <linux/kthread.h>

#define FILENAME "status"
#define DIRECTORY "mp2"

#define SLEEP 0
#define RUN 1
#define READY 2

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yuguang2");
MODULE_DESCRIPTION("CS-423 MP2");

static spinlock_t my_lock;
#define DEBUG 1
static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_entry;

LIST_HEAD(new_list);// LIST HEAD

struct mp2_task_struct{

  struct task_struct * task;// PCB is defined in task_struct 

  struct list_head my_list;// Linux kernel list 

  struct timer_list task_timer; // timer for each task

  unsigned int task_state;

  uint64_t next_period;  ///not clear about this parameter!
   
  unsigned int  pid;

  unsigned long relative_period; //period

  unsigned long slice;  //effective computational time of each period 

};
struct mp2_task_struct * runnning_task;// global variable for current running task!
struct mp2_task_struct * old_task;// gloabal variable for last running task!
struct mp2_task_struct * next_task;
//static struct workqueue_struct *my_wq;
struct task_struct * dispatcher;
struct kmem_cache * kcache;

void my_timer_callback(unsigned long data);

void registration(unsigned int pid, unsigned long period,unsigned slice){

        struct mp2_task_struct *obj;
       // obj= (struct mp2_task_struct *) kmalloc(sizeof(struct mp2_task_struct),GFP_KERNEL);
        obj=(struct mp2_task_struct *)kmem_cache_alloc(kcache,GFP_KERNEL);
        obj->pid=pid;
        obj->relative_period=period;
        obj->slice=slice;

        obj->task=find_task_by_pid(pid);

        set_task_state(obj->task, TASK_INTERRUPTIBLE);

        obj->task_state=SLEEP;
               //timer initalization 
        setup_timer(&obj->task_timer,my_timer_callback,obj);

        obj->next_period=jiffies;//msecs_to_jiffies(obj->relative_period);

        spin_lock(&my_lock);

        list_add(&obj->my_list,&new_list);

        spin_unlock(&my_lock);

}
void deregistrarion(void){

     static struct list_head *pos,*q;
     //static struct mp2_task_struct head;

    list_for_each_safe(pos,q,&new_list){
        static struct mp2_task_struct *tmp;  
        tmp=list_entry(pos,struct mp2_task_struct,my_list);
        del_timer(&(tmp->task_timer));
        list_del(pos);
        kmem_cache_free(kcache,tmp);
    }


}

ssize_t mp2_write(struct file* flip, const char __user *buff,

                       unsigned long len,void *data)
{
  printk("get into write!");  
  
  char * command;

  command= (char *) kmalloc(len+1,GFP_KERNEL);

  unsigned int pid;
    
  unsigned long period;

  unsigned long slice;

  char *token[10];

  int i=0;

  if(copy_from_user(command,buff,len)){
     
           return -ENOSPC;
    }

  char * delim=","; //split by ','

    
  while(token[i++]=strsep(&command,delim)){
  
    if(DEBUG) printk("token %d : %s\n",i-1,token[i-1]);

   }

   if(strcmp(token[0],"R")==0){
           
        sscanf(token[1],"%lu",&pid);

        sscanf(token[2],"%lu",&period);

        sscanf(token[3],"%lu",&slice);

        if(DEBUG) printk("Registration:the value of pid is %lu period is %lu computation time is %lu\n",pid, period,slice);

         registration(pid,period,slice);// registration

   }

  else if(strcmp(token[0],"Y")==0){

         sscanf(token[1],"%lu",&pid);

         if(DEBUG) printk("YIELD: the value of pid is %lu\n",pid);

   }
  else if(strcmp(token[0],"D")==0){
           
         sscanf(token[1],"%lu",&pid);

         if(DEBUG) printk("Deregistrarion: the value of pid is %lu\n",pid);

   }else{
          
          printk("No related option");
       
   }
   
    //kstrtoint(pid_,10,&_pid);
  
    return len;
}

static ssize_t mp2_read(struct file *file ,char __user *buffer, size_t count,loff_t *pos)
{ 

  if(* pos>0) return 0;
  struct mp2_task_struct* my_obj;
  spin_lock(&my_lock);
  list_for_each_entry(my_obj,&new_list,my_list){
       char my_buff[64];
        sprintf(my_buff,"%lu : %lu  :%lu\n",my_obj->pid,my_obj->relative_period,my_obj->slice);
        if(DEBUG) printk("pid is %lu period is %lu computation time %lu\n",my_obj->pid,my_obj->relative_period,my_obj->slice);
        if(DEBUG) printk("my_buff is %s\n",my_buff);

       if(copy_to_user(buffer + *pos ,my_buff,strlen(my_buff)+1)){
        return -EFAULT;      
        }

       *pos+=64;
   
      }
   spin_unlock(&my_lock);
      
   return  *pos;
}
void yeild(unsigned int pid){
       
       struct mp2_task_struct* my_obj;

       list_for_each_entry(my_obj,&new_list,my_list){
      
         if(my_obj->pid==pid){
                
               break;
         }
        
      }
      if(obj->next_period < jiffies) 
      {
       obj->next_period=jiffies+msecs_to_jiffies(obj->relative_period);
       mod_timer(&(runnning_task->task_timer),runnning_task->next_period);
       obj->task_state=READY;
       wake_up_process(obj->task);
      }
     else{
       if(runnning_task->pid!=pid)
         {    printk("error!");
              return;
         }      
       mod_timer(&(runnning_task->task_timer),runnning_task->next_period);

       runnning_task->next_period=runnning_task->next_period+msecs_to_jiffies(runnning_task->relative_period);

       runnning_task->task_state=SLEEP;
      
       set_task_state(runnning_task->task,TASK_UNINTERRUPTIBLE);
      }
       wake_up_process(dispatcher);
       schedule();

}
static int dispatch_function(void){

while (1){

  set_current_state(TASK_INTERRUPTIBLE);
  schedule();
  
  if(kthread_should_stop()) return 0;
  
  old_task=runnning_task;
  struct mp2_task_struct* my_obj;
  unsigned long least_period=INT_MAX;

  spin_lock(&my_lock);
  list_for_each_entry(my_obj,&new_list,my_list){

    if(my_obj->task_state==READY){
        if(my_obj->relative_period<least_period){

            least_period=my_obj->relative_period;
            next_task=my_obj;     
         }
      }
    }
    spin_unlock(&my_lock);
    
    runnning_task=next_task;

    struct sched_param sparam;
    wake_up_process(runnning_task->task);
    sparam.sched_priority=99;
    sched_setscheduler(runnning_task->task, SCHED_FIFO, &sparam);

    struct sched_param sparam2;
    sparam2.sched_priority=0;
    sched_setscheduler(old_task->task,SCHED_NORMAL,&sparam2);
}
   return 0;
}

void my_timer_callback(unsigned long data)
{
   struct mp2_task_struct *obj;

   obj->task_state=READY;
   wake_up_process(obj->task);

   wake_up_process(dispatcher);
   schedule();   
   
}

static const struct file_operations mp2_file={

 .owner=THIS_MODULE,
 .read=mp2_read,
 .write=mp2_write,

};

// mp1_init - Called when module is loaded
int __init mp2_init(void)
{
   #ifdef DEBUG
   printk(KERN_ALERT "MP2 MODULE LOADING\n");
   #endif
   spin_lock_init(&my_lock);
   int ret;
   // Insert your code here ...
   proc_dir=proc_mkdir(DIRECTORY,NULL);
   
   proc_entry=proc_create(FILENAME,0666,proc_dir,&mp2_file);  

   kcache=kmem_cache_create("kcache",sizeof(mp2_task_struct),0,SLAB_HWCACHE_ALIGN,NULL);

   dispatcher=kthread_create(&dispatch_function, NULL,"dispatcher function");
   
   printk(KERN_ALERT "MP2 MODULE LOADED\n");
   return 0;   
}

// mp1_exit - Called when module is unloaded
void __exit mp2_exit(void)
{
   #ifdef DEBUG
   printk(KERN_ALERT "MP2 MODULE UNLOADING\n");
   #endif  
  // Insert your code here ...
    //kthread_stop(dispatcher);
 
    static struct list_head *pos,*q;

   proc_remove(proc_entry);
   proc_remove(proc_dir);
   list_for_each_safe(pos,q,&new_list){
  
      static struct mp2_task_struct *tmp; 
  
      tmp=list_entry(pos,struct mp2_task_struct,my_list);
     
      del_timer(&(tmp->task_timer));

      list_del(pos);
    
      kmem_cache_free(kcache,tmp);
    }
    if(kcache) kmem_cache_destroy(kcache);
   printk(KERN_ALERT "MP2 MODULE UNLOADED\n");
}// Register init and exit funtions
module_init(mp2_init);
module_exit(mp2_exit);
