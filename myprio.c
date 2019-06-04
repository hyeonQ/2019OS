#include "sched.h"
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/slab.h>

void init_task_myprio(struct task_struct *p)
{
	struct sched_myprio_entity *se = &p->myprio;

	p->sched_class = &myprio_sched_class;
	se->age = 0;
	printk(KERN_INFO "***[MYPRIO] init_task_myprio: priority=%d, p->pid=%d\n, age=%d",
				se->prio, p->pid, se->age);
}

void init_myprio_rq (struct myprio_rq *myprio_rq)
{
	printk(KERN_INFO "***[MYPRIO] Mypriority class is online \n");
	myprio_rq->nr_running = 0;
	INIT_LIST_HEAD(&myprio_rq->queue);
}

static void update_curr_myprio(struct rq *rq)
{
	/*
	struct task_struct *cur = rq->curr;
	struct task_struct *pos_p = NULL;
	struct myprio_rq *myprio_rq = &rq->mystride;
	struct list_head *pos;
	struct list_head *head = &rq->myprio.queue;
	struct list_head *curptr = &cur->myprio.run_list;
	struct sched_myprio_entity *pos_se = NULL;
	unsigned int is_resched = 0;
	int i, min_pass_idx = 0, min_pass;

	printk(KERN_INFO "***[MYSTRD] update_curr: update_curr starting!\n");
		
	// update curr tasks' pass and make myprio_rq sorted by pass
	cur->myprio.pass += cur->myprio.stride;
	
	min_pass = cur->myprio.pass;
	pos = &myprio_rq->queue;

	for(i=0; i<myprio_rq->nr_running; i++){
		pos = pos->next;
		pos_se = container_of(pos, struct sched_myprio_entity, run_list);
		pos_p = container_of(pos_se, struct task_struct, myprio);
		
		printk(KERN_INFO "\t\t***[MYSTRD] update_curr: curr->pid=%d, pass=%d, p->pid=%d, pass=%d\n",
							cur->pid, cur->myprio.pass, pos_p->pid, pos_se->pass);
			
		if(min_pass > pos_se->pass){
			min_pass = pos_se->pass;
			min_pass_idx = i;
			is_resched = 1;
		}
	}

	//if any task has smaller pass than curr task's pass
	if(is_resched){
		printk(KERN_INFO "\t***[MYSTRD] update_curr: curr doesn't have minimum pass\n");
		
		for(i=0; i<min_pass_idx; i++){
			pos = myprio_rq->queue.next;
			pos_se = container_of(pos, struct sched_myprio_entity, run_list);
			
			list_del_init(&pos_se->run_list);
			list_add_tail(&pos_se->run_list, &myprio_rq->queue);
		}

		resched_curr(rq);
	}
	printk(KERN_INFO "***[MYSTRD] update_curr: update_curr end!\n");
	//  resched	*/
}

static void enqueue_task_myprio(struct rq *rq, struct task_struct *p, int flags)
{
	struct myprio_rq *myprio_rq = &rq->myprio;
	struct sched_myprio_entity *front_se = NULL;

	struct list_head *pos = NULL;
	struct task_struct *pos_p = NULL;
	struct sched_myprio_entity *pos_se = NULL;
	
	int is_enqueued=0;
	// TODO
	if(!rq->myprio.nr_running){
		printk(KERN_INFO "***[MYPRIO] enqueue first one: success cpu=%d, nr_running=%d, pid=%d\n",
				cpu_of(rq), rq->myprio.nr_running, p->pid);
		list_add_tail(&p->myprio.run_list, &rq->myprio.queue);
	} else {
		for(pos = myprio_rq->queue.next; pos != &myprio_rq->queue; pos = pos->next){
			pos_se = container_of(pos, struct sched_myprio_entity, run_list);
			pos_p = container_of(pos_se, struct task_struct, myprio);

			if(p->myprio.prio > pos_se->prio) {
				printk(KERN_INFO "***[MYPRIO] enqueue middle of queue: success cpu=%d, \
				nr_running=%d, pid=%d, next_prio=%d\n",
				 cpu_of(rq), myprio_rq->nr_running, p->pid, pos_se->prio);
				__list_add(&p->myprio.run_list, pos->prev, pos);
				is_enqueued=1;
				break;
			}
		}
		if(!is_enqueued){
			list_add_tail(&p->myprio.run_list, &myprio_rq->queue);
		}
	}
	rq->myprio.nr_running++;

}

static void dequeue_task_myprio(struct rq *rq, struct task_struct *p, int flags)
{
	//TODO

	if( !list_empty(&rq->myprio.queue) )
	{
		list_del_init(&p->myprio.run_list);
		rq->myprio.nr_running--;

		printk(KERN_INFO "\t***[MYPRIO] dequeue: success cpu=%d, nr_running=%d, pid=%d\n",	
				cpu_of(rq), rq->myprio.nr_running, p->pid);
	
	}
	else
	{
	}
}

static void check_preempt_curr_myprio(struct rq *rq, struct task_struct *p, int flags){}
struct task_struct *pick_next_task_myprio(struct rq *rq, struct task_struct *prev)
{
	struct task_struct *next_p = NULL;
	struct sched_myprio_entity *next_se = NULL;
	struct myprio_rq *myprio_rq = &rq->myprio;

	struct list_head *pos = NULL;
	struct task_struct *pos_p = NULL;
	struct sched_myprio_entity *pos_se = NULL;

	int i;

	//TODO
	if( !myprio_rq->nr_running ) 
	{
		return NULL;
	}

	if(prev->sched_class != &myprio_sched_class)
	{
		printk(KERN_INFO "***[MYPRIO] pick_next_task: other class came in.. prev->pid=%d\n",
																				 prev->pid);
		put_prev_task(rq, prev);
	}
	
	pos = &myprio_rq->queue;
	printk(KERN_INFO "\t***[MYPRIO] pick_next_task: show queue\n");

	for(i=0; i<myprio_rq->nr_running; i++){
		pos = pos->next;
		pos_se = container_of(pos, struct sched_myprio_entity, run_list);
		pos_p = container_of(pos_se, struct task_struct, myprio);

		printk(KERN_INFO "\t\t***[MYPRIO] pick_next_task: pos->pid=%d, pos->prio=%d, age=%d\n",
						pos_p->pid, pos_se->prio, pos_se->age);
	}

	//TODO
	next_se = container_of(myprio_rq->queue.next, struct sched_myprio_entity, run_list);
	next_p = container_of(next_se, struct task_struct, myprio);

	printk(KERN_INFO "\t***[MYPRIO] pick_next_task: cpu=%d, prev->pid=%d, next_p->pid=%d, nr_running=%d, next_se->prio=%d\n",
		 cpu_of(rq), prev->pid, next_p->pid, myprio_rq->nr_running, next_se->prio);

	return next_p;
}

static void put_prev_task_myprio(struct rq *rq, struct task_struct *p)
{
	printk(KERN_INFO "\t***[MYPRIO] put_prev_task: do nothing, p->pid=%d\n", p->pid);	
}
static int select_task_rq_myprio(struct task_struct *p, int cpu, int sd_flag, int flags)
{
	return task_cpu(p);
}
static void set_curr_task_myprio(struct rq *rq){}
static void task_tick_myprio(struct rq *rq, struct task_struct *p, int queued)
{
	update_curr_myprio(rq);
}

static void prio_changed_myprio(struct rq *rq, struct task_struct *p, int oldprio) {}
static void switched_to_myprio(struct rq *rq, struct task_struct *p)
{
	resched_curr(rq);
}

const struct sched_class myprio_sched_class={
	.next = &fair_sched_class,
	.enqueue_task = enqueue_task_myprio,
	.dequeue_task = dequeue_task_myprio,
	.check_preempt_curr = check_preempt_curr_myprio,
	.pick_next_task = pick_next_task_myprio,
	.put_prev_task = put_prev_task_myprio,
#ifdef CONFIG_SMP
	.select_task_rq = select_task_rq_myprio,
#endif
	.set_curr_task = set_curr_task_myprio,
	.task_tick = task_tick_myprio,
	.prio_changed = prio_changed_myprio,
	.switched_to = switched_to_myprio,
	.update_curr = update_curr_myprio,
};

