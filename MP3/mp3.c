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

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_entry;

static void my_wq_function(struct work_struct *work);

DECLARE_DELAYED_WORK(delayed_work,my_wq_function);
//struct work_struct * work=NULL;
static struct workqueue_struct *my_wq=NULL; //workqueu declaration 
struct kmem_cache * kcache=NULL;

unsigned long * buff; // kernel shared buffer

int size_list=0;

int index=0;

LIST_HEAD(new_list);//LIST marco to locate the head of the list

typedef struct mp3_task_struct{

   struct task_struct * task;// PCB is defined in task_struct 

   struct list_head my_list;

   int  pid;

   unsigned long major_count;

   unsigned long minor_count;

   unsigned long cpu_usage;

}mp3_task_struct;

static void my_wq_function(struct work_struct *work);//declare of the workqueue function 
void static _init_queue(void){

  my_wq=create_workqueue("my_queue");
}
/*
void static _delete_queue(void){

  if(my_wq!=NULL){

      flush_workqueue(my_wq);

      destroy_workqueue(my_wq);

      my_wq=NULL;

  }

}*/

void Registration(int pid){

  if(DEBUG) printk("PID : %d register\n",pid);

  if(size_list==0){

   if(DEBUG) printk("The first process reg!\n");

  // struct delayed_work * work=(struct delayed_work *)kmalloc(sizeof(struct delayed_work),GFP_KERNEL);
  // INIT_DELAYED_WORK((struct delayed_work*)work,my_wq_function);

    queue_delayed_work(my_wq,&delayed_work,msecs_to_jiffies(1000/20));

  }

   size_list++;

   struct mp3_task_struct *obj;

   obj=(struct mp3_task_struct *)kmem_cache_alloc(kcache,GFP_KERNEL);

   obj->pid=pid;

   obj->task=find_task_by_pid(pid);

   obj->major_count=0;

   obj->minor_count=0;

   obj->cpu_usage=0;

   spin_lock(&my_lock);

   list_add(&obj->my_list,&new_list);

   spin_unlock(&my_lock);


}

void Unregistration(int pid){

    if(DEBUG) printk("PID : %d Unregister\n",pid);

    static struct list_head *pos,*q;

    spin_lock(&my_lock);

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
    spin_unlock(&my_lock);


    if(size_list==0){
     //delete the work;
      // _delete_queue();
       cancel_delayed_work(&delayed_work);

       flush_workqueue(my_wq);
    }

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

    if(command[0]=='R'){

        sscanf(command,"R %d",&pid);

        Registration(pid);

      }

    if(command[0]=='U'){

        sscanf(command,"U %d",&pid);

        Registration(pid);

    }

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
  
    buff[index++]=jiffies;
    buff[index++]=min_sum;
    buff[index++]=maj_sum;
    buff[index++]=cpu_sum;

    if(DEBUG) printk("1.%lu 2.%lu 3%lu 4%lu\n",jiffies, min_sum,maj_sum,cpu_sum);
   //flush_workqueue(my_wq);

  // struct work_struct * work_next=(struct work_struct *)kmalloc(sizeof(struct work_struct),GFP_KERNEL);
   
   // INIT_DELAYED__WORK((struct delayed_work*)work,my_wq_function);

    queue_delayed_work(my_wq, &delayed_work,msecs_to_jiffies(1000/20));
   
}

static const struct file_operations mp3_file={

 .owner=THIS_MODULE,
 .read=mp3_read,
 .write=mp3_write,

};

//character device driver callback functions
static int my_open(struct inode *inode, struct file *file){
  if(DEBUG) printk(KERN_INFO "MP3 OPEN\n");
  return 0;
}

static int my_release(struct inode *inode, struct file *file){
  if(DEBUG) printk(KERN_INFO "MP3 RELEASE\n");
  return 0;
}

static int my_mmap(struct file *filp, struct vm_area_struct *vma){

  unsigned long * buff_add=buff;

  unsigned long pfn;

  unsigned long map_start_addr=vma->vm_start;

  int i;

  for( i=0;i<PAGE_NUM;i++){

        pfn=vmalloc_to_pfn(buff_add);

       // buff_add+=PAGE_S/sizeof(unsigned long);
        buff_add+=PAGE_SIZE;

        int ret=remap_pfn_range(vma,map_start_addr,pfn,PAGE_SIZE,vma->vm_page_prot);

        // map_start_addr+=(unsigned long)((vma->vm_end)-(vma->vm_start))/PAGE_NUM;
         map_start_addr+=PAGE_SIZE;

        //if(DEBUG && i%32==0) printk()
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
 .open=my_open,
 .release=my_release,
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

   memset(buff,0,PAGE_NUM*PAGE_SIZE);

   _init_queue();

   //void cdev_init(struct cdev * cdev ,struct file *fops);

   //int cdev_add(struct cdev *dev,dev_t num,unsigned int count);

   register_chrdev(100,"mp3",&my_fops);

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

    vfree(buff);

    unregister_chrdev(100,"mp3");

    /*spin_lock(&my_lock);

    list_for_each_safe(pos,q,&new_list){
  
       static struct mp3_task_struct *tmp; 
  
       tmp=list_entry(pos,struct mp3_task_struct,my_list);

       list_del(pos);
    
       kmem_cache_free(kcache,tmp);
    }

    spin_unlock(&my_lock);
    */
   // cancel_delayed_work(&delayed_work);
     flush_workqueue(my_wq);

     destroy_workqueue(my_wq);

   if(kcache) kmem_cache_destroy(kcache);
   // kthread_stop(dispatcher);
   printk(KERN_ALERT "MP3 MODULE UNLOADED\n");
}// Register init and exit funtions

module_init(mp3_init);
module_exit(mp3_exit);
