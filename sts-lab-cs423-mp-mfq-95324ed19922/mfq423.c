/*
 * Multi-Level Feedback Queue scheduling class
 */

#ifdef CONFIG_SCHED_MFQ423_POLICY  

#include <linux/sched.h>

#include "sched.h"

void init_mfq423_rq(struct mfq423_rq *mfq423_rq) {

}
  
static void enqueue_task_mfq423(struct rq *rq, struct task_struct *p, int flags)
{

}

static void dequeue_task_mfq423(struct rq *rq, struct task_struct *p, int flags)
{

}

static void check_preempt_curr_mfq423(struct rq *rq, struct task_struct *p, int wake_flags)
{

}

static struct task_struct *
pick_next_task_mfq423(struct rq *rq, struct task_struct *prev) {
  return NULL;
}

/*                                                                                                                                                                        
 * All the scheduling class methods:                                                                                                                                      
 */
const struct sched_class mfq423_sched_class = {

  .next                   = &idle_sched_class,
  .enqueue_task           = enqueue_task_mfq423,
  .dequeue_task           = dequeue_task_mfq423,
  .check_preempt_curr     = check_preempt_curr_mfq423,
  .pick_next_task         = pick_next_task_mfq423,
};

#endif
