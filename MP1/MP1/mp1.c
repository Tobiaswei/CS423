#define LINUX
#include<asm/uaccess.h>//(copy_from_user)
#include <linux/module.h>
#include <linux/kernel.h>
#include<linux/slab.h>// kmalloc;
#include "mp1_given.h" //get_cpu_time;
#include <linux/list.h>
#include <linux/proc_fs.h> 
#include <linux/time.h>
#include<linux/string.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
//#include <linux/vmalloc.h>
#define FILENAME "status"
#define DIRECTORY "mp1"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yuguang2");
MODULE_DESCRIPTION("CS-423 MP1");
static spinlock_t my_lock;
#define DEBUG 1
static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_entry;
LIST_HEAD(new_list);
struct list_node{

   struct list_head my_list;
   unsigned long cpu_time;
   int  pid;

};
static struct timer_list my_timer;
static struct workqueue_struct *my_wq;

ssize_t mp1_write(struct file* flip, const char __user *buff,

                       unsigned long len,void *data)
{
   struct list_node *obj;
   obj= (struct list_node *) kmalloc(sizeof(struct list_node),GFP_KERNEL); 
   int _pid;

   char * pid_;
   pid_= (char *) kmalloc(len+1,GFP_KERNEL);
   unsigned long _cpu_time;
   
   if(copy_from_user(pid_,buff,len)){
     
           return -ENOSPC;
      }
   kstrtoint(pid_,10,&_pid);

    printk("the value of pid is %d",_pid);
    obj->cpu_time=0;
    obj->pid=_pid;
    spin_lock(&my_lock);
    list_add(&obj->my_list,&new_list);
    spin_unlock(&my_lock);
    kfree(pid_);
    return len;
}

static ssize_t mp1_read(struct file *file ,char __user *buffer, size_t count,loff_t *pos)
{ 

  if(* pos>0) return 0;
  struct list_node* my_obj;
  spin_lock(&my_lock);
  list_for_each_entry(my_obj,&new_list,my_list){
       char my_buff[64];
        sprintf(my_buff,"%d : %lu\n",my_obj->pid,my_obj->cpu_time);
        if(DEBUG) printk("pid is %d cpu time is %lu\n",my_obj->pid,my_obj->cpu_time);
        if(DEBUG) printk("my_buff is %s\n",my_buff);

       if(copy_to_user(buffer + *pos ,my_buff,strlen(my_buff)+1)){
        return -EFAULT;      
        }

       *pos+=64;
   
      }
   spin_unlock(&my_lock);
      
   return  *pos;
}

static void my_wq_function(struct work_struct *work){
  
  struct list_node* my_obj;
//  spin_lock(&my_lock); 
 
   static struct list_head *pos,*q;
  list_for_each_safe(pos,q,&new_list){

   unsigned long _cpu_time=0;
 
   my_obj=list_entry(pos,struct list_node,my_list);
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

static const struct file_operations mp1_file={

 .owner=THIS_MODULE,
 .read=mp1_read,
 .write=mp1_write,

};

void my_timer_callback(unsigned long data)
{
//   int ret;
    
   struct work_struct *work=(struct work_struct *)kmalloc(sizeof(struct work_struct),GFP_KERNEL);
   INIT_WORK((struct work_struct*)work,my_wq_function);
   queue_work(my_wq,(struct work_struct*)work);

   setup_timer(&my_timer,my_timer_callback,0);
   mod_timer(&my_timer,jiffies+msecs_to_jiffies(5000));
}
// mp1_init - Called when module is loaded
int __init mp1_init(void)
{
   #ifdef DEBUG
   printk(KERN_ALERT "MP1 MODULE LOADING\n");
   #endif
   spin_lock_init(&my_lock);
   int ret;
   // Insert your code here ...
   proc_dir=proc_mkdir(DIRECTORY,NULL);
   
   proc_entry=proc_create(FILENAME,0666,proc_dir,&mp1_file);
   
   my_wq=create_workqueue("my_queue");

   setup_timer(&my_timer,my_timer_callback,0);
   ret=mod_timer(&my_timer,jiffies+msecs_to_jiffies(5000));
   if(ret) printk("Error in mod_timer\n");
   printk(KERN_ALERT "MP1 MODULE LOADED\n");
   return 0;   
}

// mp1_exit - Called when module is unloaded
void __exit mp1_exit(void)
{
   #ifdef DEBUG
   printk(KERN_ALERT "MP1 MODULE UNLOADING\n");
   #endif  
  // Insert your code here ...
   static struct list_head *pos,*q;
   static struct list_node head;
  // static struct list_node * tmp;
   del_timer(&my_timer);
   flush_workqueue(my_wq);
   destroy_workqueue(my_wq);
   proc_remove(proc_entry);
   proc_remove(proc_dir);
   list_for_each_safe(pos,q,&new_list){
   static struct list_node *tmp;  
    tmp=list_entry(pos,struct list_node,my_list);
    list_del(pos);
    kfree(tmp);
    }
   printk(KERN_ALERT "MP1 MODULE UNLOADED\n");
}// Register init and exit funtions
module_init(mp1_init);
module_exit(mp1_exit);
