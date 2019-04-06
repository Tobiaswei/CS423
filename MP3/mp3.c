#define LINUX

#include <asm/uaccess.h>//(copy_from_user)
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>// kmalloc;
#include <linux/sched.h>
#include "mp3_given.h" //get_cpu_time;
#include <linux/list.h>
#include <linux/proc_fs.h> 
#include <linux/time.h>
#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#define FILENAME "status"
#define DIRECTORY "mp3"

#define DEBUG 1

#define PAGE_NUM 128

#define MAX_NUM_SAMPLES (20*600)

#define NUM_ITEMS 4   

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yuguang2");
MODULE_DESCRIPTION("CS-423 MP3");

static spinlock_t my_lock;// spin_lock
// delcare of proce_dirctory
static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_entry;
// declare of   work_queue function and work
static void my_wq_function(struct work_struct *work);

DECLARE_DELAYED_WORK(delayed_work,my_wq_function);

static struct workqueue_struct *my_wq=NULL; //workqueu declaration 
//declare of slab cache
struct kmem_cache * kcache=NULL;
//kernel shared buffer
unsigned long * buff;
// size of the list
int size_list=0;
// current index of shared buffer
int index=0;
// start point of kernel list
LIST_HEAD(new_list);//LIST marco to locate the head of the list

typedef struct mp3_task_struct{

   struct task_struct * task;// PCB is defined in task_struct 

   struct list_head my_list;

   int  pid;

   unsigned long major_count;

   unsigned long minor_count;

   unsigned long cpu_usage;

}mp3_task_struct;
// create the workqueue

void static _init_queue(void){

  my_wq=create_workqueue("my_queue");
}

// registration function
void Registration(int pid){

  if(DEBUG) printk("PID : %d register\n",pid);

   size_list++;//increment the size

  //allocate slab memory for mp3_task_struct

   struct mp3_task_struct *obj;

   obj=(struct mp3_task_struct *)kmem_cache_alloc(kcache,GFP_KERNEL);

   //Initialize the each attributes in the agumented task_struct
   obj->pid=pid;

   obj->task=find_task_by_pid(pid);

   obj->major_count=0;

   obj->minor_count=0;

   obj->cpu_usage=0;

   spin_lock(&my_lock);
//add it into list
   list_add(&obj->my_list,&new_list);
// if it is the first time to register 
   if(size_list==1){
   //delay the work by 1000/20 !
    queue_delayed_work(my_wq,&delayed_work,msecs_to_jiffies(1000/20));
    if(DEBUG) printk("workqueue create!");
    }

   spin_unlock(&my_lock);


}

void Unregistration(int pid){

    if(DEBUG) printk("PID : %d Unregister\n",pid);

    static struct list_head *pos,*q;

    spin_lock(&my_lock);
//clear and free memory when unreg
    list_for_each_safe(pos,q,&new_list){

    static struct mp3_task_struct *tmp;  

    tmp=list_entry(pos,struct mp3_task_struct,my_list);

    if(tmp->pid==pid)
       {

        list_del(pos);

        kmem_cache_free(kcache,tmp);

        size_list--;
    
       }  
    }
  // IF the linked list is empty clear the unfinished work 
    if(size_list==0){
     //delete the work;
          cancel_delayed_work(&delayed_work);

          flush_workqueue(my_wq);
    }  
  spin_unlock(&my_lock);

}

ssize_t mp3_write(struct file* flip, const char __user *buff,

                       unsigned long len,void *data)
{

    char * command;

    command= (char *) kmalloc(len+1,GFP_KERNEL);

    int pid;

    if(copy_from_user(command,buff,len)){
     
         return -ENOSPC;
    }
// choose register or unregister
    if(command[0]=='R'){

        sscanf(command,"R %d",&pid);

        Registration(pid);

      }

    if(command[0]=='U'){

        sscanf(command,"U %d",&pid);

        Unregistration(pid);

    }
    kfree(command);
    return len;
}

static ssize_t mp3_read(struct file *file ,char __user *buffer, size_t count,loff_t *pos)
{ 

 if(* pos>0) return 0;

  struct mp3_task_struct* my_obj;

  spin_lock(&my_lock);
  //read from list
  list_for_each_entry(my_obj,&new_list,my_list){

        char my_buff[64];

        sprintf(my_buff,"Registered Process: %d \n",my_obj->pid);

        if(copy_to_user(buffer + *pos ,my_buff,strlen(my_buff)+1)){
              return -EFAULT;      
        }

       *pos+=64;
   
      }

   spin_unlock(&my_lock);
      
   return  *pos;
}

static void my_wq_function(struct work_struct *work){
  
  struct mp3_task_struct* my_obj;

  unsigned long major_c;
  unsigned long minor_c;
  unsigned long stime;
  unsigned long utime;
  unsigned long maj_sum=0,min_sum=0,cpu_sum=0;
  spin_lock(&my_lock);
  //read from list
  list_for_each_entry(my_obj,&new_list,my_list){

   //for each registered process accmulate the min_page fault and major pagefault and cpu_utilization

      if(index>MAX_NUM_SAMPLES*NUM_ITEMS)
      {
         printk("Save up to maximun number of samples\n");

         return;
      }

      int ret=get_cpu_use(my_obj->pid,&minor_c,&major_c,&utime,&stime);

      if(ret==-1)
      {
        printk("Cannot get process: %d profile information\n",my_obj->pid);
        continue;
       }
      
      maj_sum+=major_c;
      min_sum+=minor_c;
      cpu_sum+=(utime+stime);

      }

   spin_unlock(&my_lock);
  // write the accumlated data into shared kernel buffer;

   spin_lock(&my_lock);

    buff[index++]=jiffies;
    buff[index++]=min_sum;
    buff[index++]=maj_sum;
    buff[index++]=cpu_sum;

   spin_unlock(&my_lock);
    if(DEBUG) printk("1.%lu 2.%lu 3%lu 4%lu\n",jiffies, min_sum,maj_sum,cpu_sum);
   // push another delayed work into workqueue keep peridically tracking the usage 
   queue_delayed_work(my_wq, &delayed_work,msecs_to_jiffies(1000/20));
   
}

static const struct file_operations mp3_file={

 .owner=THIS_MODULE,
 .read=mp3_read,
 .write=mp3_write,

};

//character device driver callback functions
static int my_open(struct inode *inode, struct file *file){
}

static int my_release(struct inode *inode, struct file *file){
}
// implementation of the MMAP
static int my_mmap(struct file *filp, struct vm_area_struct *vma){

  if(DEBUG) printk("MMAP function!\n");

  char * buff_add; //        pay attention there should be char *

  unsigned long pfn;  // page frame number

  unsigned long map_start_addr=vma->vm_start; // virtual memory
  int i=0;
// all the staff type shoudl be transferred into char*!!!!!!!!!
  buff_add=(char *)buff;

/*iterate all 128 page frames one by one 
the core of the MMAP
 1.get the vmalloc or kmalloc allocated memory in pysical

 2.remap it into user spacer/user virtual memory however, the physical has to be continous 

    the feature of remap_pfn_range only map continous phyiscal memory
    the kmalloc allocate contionus however the vmalloc not contigious in phyiscla memory but page continous !

*/
  for(;i<PAGE_NUM;i++){

       printk("The %d \n",i);
 
        pfn=vmalloc_to_pfn(buff_add);

        buff_add+=PAGE_SIZE;//buff_add(char*)+ PAGES_SIZE just fit ecah byte
      int ret=remap_pfn_range(vma,map_start_addr,pfn,PAGE_SIZE,vma->vm_page_prot);
         map_start_addr+=PAGE_SIZE;
         
        if (ret < 0) 
        {
             printk("could not map the address area\n");

             return -1;
        }

  }

  return 0;

}

static const struct file_operations my_fops={

 .owner=THIS_MODULE,
 .open=NULL,
 .release=NULL,
 .mmap=my_mmap,

};

// mp1_init - Called when module is loaded
int __init mp3_init(void)
{
   #ifdef DEBUG
   printk(KERN_ALERT "MP3 MODULE LOADING\n");
   #endif
   spin_lock_init(&my_lock);
   
   // Insert your code here ...
   proc_dir=proc_mkdir(DIRECTORY,NULL);
   
   proc_entry=proc_create(FILENAME,0666,proc_dir,&mp3_file); 

   //Initialization of the kernel Buffer
   buff=(unsigned long*)vmalloc(PAGE_NUM*PAGE_SIZE);

 //  memset(buff,0,PAGE_NUM*PAGE_SIZE);
   int count=0;
   for(;count<MAX_NUM_SAMPLES*NUM_ITEMS;count++){
     buff[count]=-1;
    }
   _init_queue();

   //void cdev_init(struct cdev * cdev ,struct file *fops);

   //int cdev_add(struct cdev *dev,dev_t num,unsigned int count);
   register_chrdev(100,"node",&my_fops);

  //initializiation of kcache  
   kcache=kmem_cache_create("kcache",sizeof(struct mp3_task_struct),0,SLAB_HWCACHE_ALIGN,NULL);

   printk(KERN_ALERT "MP3 MODULE LOADED\n");

   return 0;    
}

// mp1_exit - Called when module is unloaded
void __exit mp3_exit(void)
{
    #ifdef DEBUG
    printk(KERN_ALERT "MP3 MODULE UNLOADING\n");
   #endif  
  // Insert your code here ...

   static struct list_head *pos,*q;

    proc_remove(proc_entry); //rm the entry

    proc_remove(proc_dir);// rm the directory 

    vfree(buff); // free the shared kernel buff 

    unregister_chrdev(100,"node");
  //free each element in the linked list
    spin_lock(&my_lock);

    list_for_each_safe(pos,q,&new_list){
  
       static struct mp3_task_struct *tmp; 
  
       tmp=list_entry(pos,struct mp3_task_struct,my_list);

       list_del(pos);

       kmem_cache_free(kcache,tmp);
    }

    spin_unlock(&my_lock);
// destory the workqueue!
    flush_workqueue(my_wq);
    destroy_workqueue(my_wq);
//destroy the kcache!
    if(kcache) kmem_cache_destroy(kcache);
    printk(KERN_ALERT "MP3 MODULE UNLOADED\n");
}// Register init and exit funtions

module_init(mp3_init);
module_exit(mp3_exit);
