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
#include <linux/workqueue.h>
#include <linux/spinlock.h>
//#include <linux/vmalloc.h>
#define FILENAME "status"
#define DIRECTORY "mp2"

#define SLEEP 0;
#define RUN 1;
#define READY 2;

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

  unsigned long relative_period;

  unsigned long slice;

};

static struct workqueue_struct *my_wq;

void registration(unsigned int pid, unsigned long period,unsigned slice){

        struct mp2_task_struct *obj;
        obj= (struct mp2_task_struct *) kmalloc(sizeof(struct mp2_task_struct),GFP_KERNEL);

        obj->pid=pid;
        obj->relative_period=period;
        obj->slice=slice

        obj->task=find_task_by_pid(pid);

        set_task_state(obj->task, TASK_INTERRUPTIBLE);

        obj->task_state=SLEEP;

        spin_lock(&my_lock);

        list_add(&obj->my_list,&new_list);

        spin_unlock(&my_lock);

}

void deregistrarion(){

     static struct list_head *pos,*q;
     //static struct mp2_task_struct head;

    list_for_each_safe(pos,q,&new_list){
        static struct mp2_task_struct *tmp;  
        tmp=list_entry(pos,struct mp2_task_struct,my_list);
        list_del(pos);
        kfree(tmp);
    }


}

ssize_t mp2_write(struct file* flip, const char __user *buff,

                       unsigned long len,void *data)
{
 
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

    while(token[i++]=strsep(&command,delim)){}

   if(token[0]=="R"){
           
        sscanf(token[1],"%lu",&pid);

        sscanf(token[2],"%lu",&period);

        sscanf(token[3],"%lu",&slice);

        if(DEBUG) printk("Registration:the value of pid is %lu period is %lu computation time is %lu",pid,
            period,slice);

          registration(pid,period,slice);// registration

   }

   if(token[0]=="Y"){

         sscanf(token[1],"%lu",&pid);

         if(DEBUG) printk("YIELD: the value of pid is %lu",pid);

   }
   if(token[0]=="D"){
           
         sscanf(token[1],"%lu",&pid);

         if(DEBUG) printk("Deregistrarion: the value of pid is %lu",pid);

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
/*
static void my_wq_function(struct work_struct *work){
  
  struct mp2_task_struct* my_obj;
//  spin_lock(&my_lock); 
 
   static struct list_head *pos,*q;
  list_for_each_safe(pos,q,&new_list){

   unsigned long _cpu_time=0;
 
   my_obj=list_entry(pos,struct mp2_task_struct,my_list);
   if(get_cpu_use(my_obj->pid,&_cpu_time)==-1){
   if(DEBUG) printk("pid is invalid\n");
   list_del(&(my_obj->my_list));
   kfree(my_obj);    
   
     }
 else{
    spin_lock(&my_lock);
    my_obj->cpu_time=_cpu_time;
     spin_unlock(&my_lock);
    if(DEBUG) printk("pid is %d cpu-time is %lu\n",my_obj->pid,my_obj->cpu_time);
      } 

    }
 // spin_unlock(&my_lock);      
}

void my_timer_callback(unsigned long data)
{
//   int ret;
    
   struct work_struct *work=(struct work_struct *)kmalloc(sizeof(struct work_struct),GFP_KERNEL);
   INIT_WORK((struct work_struct*)work,my_wq_function);
   queue_work(my_wq,(struct work_struct*)work);

   setup_timer(&my_timer,my_timer_callback,0);
   mod_timer(&my_timer,jiffies+msecs_to_jiffies(5000));
}
*/
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
   
   //my_wq=create_workqueue("my_queue");

  // setup_timer(&my_timer,my_timer_callback,0);
   //ret=mod_timer(&my_timer,jiffies+msecs_to_jiffies(5000));
   //if(ret) printk("Error in mod_timer\n");
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
   static struct list_head *pos,*q;

   //del_timer(&my_timer);
   //flush_workqueue(my_wq);
   //destroy_workqueue(my_wq);
   proc_remove(proc_entry);
   proc_remove(proc_dir);
   list_for_each_safe(pos,q,&new_list){
   static struct mp2_task_struct *tmp;  
    tmp=list_entry(pos,struct mp2_task_struct,my_list);
    list_del(pos);
    kfree(tmp);
    }
   printk(KERN_ALERT "MP2 MODULE UNLOADED\n");
}// Register init and exit funtions
module_init(mp2_init);
module_exit(mp2_exit);
