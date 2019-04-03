# CS 423: Operating Systems Design
# MP 2.5: Multi-Level Feedback Queue Scheduler

##0. Disclaimer

One of the learning objectives of CS 423 is to teach students practical kernel programming skills. Designing programming assignments that enable this goal is often challenging, particularly when simultaneously attempting to create assignments that are also _accessible_ and allow students to explore kernel programming concepts. Some operating systems courses avoid this problem by using "toy" operating systems for their assignments, but this deprives students of the opportunity to understand and navigate the massive code bases of commodity monolithic kernels. In contrast, our approach provides this opportunity, but requires great care and engineering effort when designing (and completing!) machine problems.
  
This is an exploratory extra-credit assignment.  _The level of effort and expertise required to complete this assignment is currently unknown._ The instructions for getting started on this assignment are not yet complete, and once they are complete they will likely be incorrect.
Further, this assignment is to be completed concurrently with MPs 3 and 4, the latter of which happens to be significantly more challenging than all prior assignments. I only recommend that you pursue this extra credit assignment if 1) you submitted MPs 1 and 2 with time to spare, 2) you are committed to completed MPs 3 and 4 with time to spare, and 3) you have generally found the machine problems in this course to be unchallenging and underwhelming.
  
##1. Goals and Overview

* Understand how to safely modify and extend complex kernel subsystems
* Understand how to correctly implement _optional_ kernel features
* Understand the functions and interface of the Linux Scheduler API
* Design and implement a fully-operational Linux Scheduler
* Leverage red-black trees for fast search and deletion of tasks from the ready list
* Design and implement applications to benchmark your scheduler in user space
* Create and validate a set of instructions (i.e., an Implementation Overview) for this assignment

##2. Introduction

_Instructor Note: In progress. You should be able to advance to instruction 15 in the implementation overview before getting into the details of Multi-Level Feedback Queues though._

##3. Developmental Setup

In this assignment, you will again work on the provided Virtual Machine. You will develop your scheduler module for the linux-4.4.0-netid kernel that you compiled as part of MP0.  Again, you will have full access and control of your Virtual Machine,  you will be able to turn it on, and off using the VMWare vSphere Console.  Inside your Virtual Machine you are free to install any editor or software like Gedit, Emacs and Vim. 

Development in this assignment is different from previous machine problems.  We will insert our scheduler into the kernel at build-time and add kernel configuration parameters to enable or disable our scheduler.  We will be working off of the source code that you downloaded during MP0; to ensure you have a clean copy of the kernel source (which we will also use in MP4), create a new git branch (e.g., "mfq423") for your changes. Also, before starting this assignment, please take a snapshot of your vm in working condition. 
The development you will do in this machine problem will affect your kernel's boot procedure and bugs in your code are likely to prevent your machine from booting. Furthermore, if you have access to a more powerful machine on which you would like to carry on the compilation of the kernel and the production of the .deb packages, you are allowed to do that as long as a copy of your code ends up on your virtual machine and the VM is running the correct kernel at the time of the grading. Finally, you are encouraged to discuss design ideas and bugs in Piazza. Piazza is a great tool for collective learning. However, please refrain from posting large amounts of code. Two or three lines of code are fine. High-level pseudo-code is also fine.

##4. Problem Description

_Instructor Note: In progress. You should be able to advance to instruction 15 in the implementation overview before getting into the details of Multi-Level Feedback Queues though._

##5. Implementation Overview

In this section we will guide you through the implementation process.

1. _Go finish MP3!_
2. The features we add to to kernel in this assignment are intended to be configurable and optional, 
  so we will need to add a new kernel configuration option. 
  While our module will not actually be architecture-specific, most scheduling parameters are, 
  so we will follow that convention and add a new configuration in `./arch/x86/kconfig`. 
  Insert the following codeblock above `menu "Processor Type and Features"`:
  


        menu "CS423 Experimental Schedulers"
        config SCHED_MFQ423_POLICY
          bool "CS423 MFQ scheduling policy"
          default n
          ---help---
          Experimental multi-level feedback queue scheduler for CS 423: Operating Systems Design
        endmenu



3. Now we need to define a  global constant for our scheduling policy. 
  Open `./include/uapi/linux/sched.h` and add the following code after `SCHED_DEADLINE` is defined. 
  Notice the check for `CONFIG_SCHED_MFQ423_POLICY`; 
  this needs to wrap _every change_ that we make to the kernel's existing code or else it will not compile with `SCHED_MFQ423_POLICY=n`.
  _From this point forward we will assume you are adding this check to all of your code (i.e., we will not display it in the code blocks)_.

        #ifdef CONFIG_SCHED_MFQ423_POLICY        
        #define SCHED_MFQ423            7
        #endif

4. Let's create a priority range for our MFQ scheduler. 
  The existing schedulers use priorities 0-140, so let's reserve 141-144 for MFQ.
  To do, so, add the following lines to `./include/linux/sched/prio.h` after `DEFAULT_PRIO` is defined.
  
        #define	MFQ423_NUM_QUEUES     	4
        #define	MFQ423_MAX_PRIO  	    MAX_PRIO + MFQ423_NUM_QUEUES

5. The kernel scheduling subsystem will use each task's priority to determine the scheduling class 
  that it is associated with. To facilitate this, we need to define two helper functions that will allow
  the kernel to quickly check if the task's priority maps to our MFQ scheduler. 
  Create a new file `./include/linux/sched/mfq423.h` and add the following code. 
  Afterwards, include your new header file in `./kernel/sched/sched.h`.      
  
        #ifndef _SCHED_MFQ423_H
        #define _SCHED_MFQ423_H

        #include <linux/sched/prio.h>

        static inline int mfq423_prio(int prio)
        {
            /* Normally, maximum Priority for user processes is MAX_PRIO-1                                                                                                     
               Our scheduler's priorities will range from 140 to 143 (MAX_MFQ423_PRIO-1) */
            if (unlikely(prio >= MAX_PRIO && prio < MFQ423_MAX_PRIO))
                return 1;
            return 0;
        }

        static inline int mfq423_task(struct task_struct *p)
        {
            return mfq423_prio(p->prio);
        }

        #endif /* _SCHED_MFQ423_H */

6. While we're still in `./kernel/sched/sched.h`, notice that there are a few helper functions defined for determining the active policy (`static inline int *_policy(int policy)`). Let's define one for our MFQ here too. **(Intructor note: Unclear if this step is needed.)**

        static inline int mfq423_policy(int policy)
        {
                return policy == SCHED_MFQ423;
        }


7. Continuing to look through `./kernel/sched/sched.h`, notice `struct rq`, which is the data structure responsible for maintaining the per-processor runqueue. Notice that there is a per-scheduler `struct *_rq` inside of this data structure; we'll need to define one for our MFQ. Inside of `struct rq`, add the field `struct mfq423_rq mfq423;`, which we can define back in `./include/linux/sched/mfq423.h` as follows:

        struct mfq423_rq {
            /* Contents TBD */
        };

8. We will need to modify a number of data structures declased in `./include/linux/sched.h`. Add the parameter `u64 shed_mfq423_priority;` to `struct sched_attr`. **(Intructor note: Unclear if this step is needed.)**

9. Also in `./include/linux/sched.h`, we need to add a new field to `struct task_struct`. First define the struct as shown in the code below, then add it `struct mfq423_task mfq423_task;` to an appropriate location in `struct task_struct`. **(Intructor note: Unclear if this step is needed.)**

        struct mfq423_task{
            unsigned int id;
            /* Other contents TBD */
        };


10. Tasks are able to have their scheduling class changed, 
  which is facilitated by the `__setscheduler` function in `./kernel/sched/core.c.`
  Add a case in the if statements for our scheduler, MFQ423:
  
        else if (mfq423_prio(p->prio))
                p->sched_class = &mfq423_sched_class;

11. Also in `./kernel/sched/core.c`, we need to initialize our scheduler for each processor. 
  Inside of `sched_init`, add the following line within the `for_each_possible_cpu(i)` loop,
  e.g., behind the `init_dl_rq` call. We will define `init_mfq423_rq` later:
  
        init_mfq423_rq(&rq->mfq423);

12. We haven't yet defined `mfq423_sched_class` and `init_mfq423_rq`, but even once we do it won't be visible in `core.c` unless we add the following lines somewhere in `kernel/sched/sched.h`. Look for the obvious place to insert each by looking for the associated code from the other scheduling classes.

        extern const struct sched_class mfq423_sched_class;
        extern void init_mfq423_rq(struct mfq423_rq *mfq423_rq);

13. When selecting a task to run next on the processor, 
   the core scheduler will iterate over each scheduling module's ready list in a specified order (i.e., STOP->DEADLINE->RT->FAIR->IDLE).
   Each scheduler points to the next in the list within their `struct sched_class`.
   Let's insert MFQ423 between FAIR and IDLE by modifying `./kernel/sched/fair.c`. 
   Change `.next = &fair_sched_class,` to `.next = &mfq423_sched_class,` when our scheduler is enabled.
   
14. We're ready to copy in the provided skeleton file where most of your code will go.  Move the `mfq423.c` file you received with this assignment to location `./kernel/sched/`. We also need to make sure the make system actually compiles our code; to do so, edit `./kernel/sched/Makefile` to include `mfq423.o` in `obj-y`.
     
15. At this point your code should compile both with and without the MFQ423 scheduling class enabled in the kernel configuration. **(Instructor Note: I think the kernel will actually be operational at this point, so long as we don't try to run a task with the MFQ423 scheduler. Does someone want to check?)**     
     
##6. Software Engineering

Your code should include comments where appropriate. 
It is not a good idea to repeat what the function does using pseudo-code,
 but instead  provide a high-level overview of the function including any preconditions and postconditions of the algorithm. 
Some functions might have as few as one line comments, while some others might have a longer paragraph. 
Also, your code must be split  into small functions, even if these functions contain no parameters. 
This is a common situation in kernel modules because most of the  variables are declared as global, 
  including but not limited to data structures, state variables, locks, timers and threads.
An important problem in kernel code readability is to know if a function holds the lock for a data structure or not;
  different conventions are used. 
A common convention is to start the function with the character '\_' if the function does not hold the lock of a data structure.
In kernel coding, performance is a very important issue;
  usually the code uses macros and preprocessor commands extensively.
Making proper use  of macros and identifying possible situations where they should be used is important in kernel programming.
Finally, in kernel programming, the use of the goto statement is a common practice. 
A good example of this, is the implementation of the Linux scheduler function schedule(). 
In this case, the use of the goto statement improves readability and/or performance. 
''Spaghetti code'' is never a good practice!

##7. Submission Instructions
_in progress..._

##8. Grading Criteria

The below criteria will be used as the basis for awarding up to 2.5 extra credit points to your final grade:

| Criteria                                                 	| Points 	|
|----------------------------------------------------------	|--------	|
| Kernel compiles successfully with MFQ423 option enabled  	| 5      	|
| Kernel compiles successfully with MFQ423 option disabled 	| 5      	|
| _in progress..._                                         	| 90     	|
|----------------------------------------------------------	|--------	|
| **Total**                                                	| 100    	|

_Up to an additional 2.5 points extra credit_ will be awarded at the discretion of the instructor
for students that make substantive corrections or additions to these instructions (in the form of pull requests).
These points will also be awarded for students that make additional contributions to administering this assignment,
  e.g., by designing an effective user space benchmark that will allow us to test the implementation of the scheduler.
  
I would also note that if you contribute significantly to this project and ace the assignment, I will be very happy
  to write you an effusively positive letter of recommendation based on your participation in this course.

##References