#include "sched.h"
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/slab.h>

void init_task_mystride(struct task_struct *p)
{
	struct sched_mystride_entity *se = &p->mystride;

	p->sched_class = &mystride_sched_class;
	se->stride = 1000 / (se->ticket);
	se->pass = se->stride;

	printk(KERN_INFO "***[MYSTRD] init_task_mystrd: ticket=%d, stride=%d, pass=%d, p->pid=%d\n",
				se->ticket, se->stride, se->pass, p->pid);
}

void init_mystride_rq (struct mystride_rq *mystride_rq)
{
	printk(KERN_INFO "***[MYSTRD] Mystride class is online \n");
	mystride_rq->nr_running = 0;
	INIT_LIST_HEAD(&mystride_rq->queue);
}

static void update_curr_mystride(struct rq *rq)
{
	struct task_struct *cur = rq->curr;
	struct task_struct *pos_p = NULL;
	struct mystride_rq *mystride_rq = &rq->mystride;
	struct list_head *pos;
	struct list_head *head = &rq->mystride.queue;
	struct list_head *curptr = &cur->mystride.run_list;
	struct sched_mystride_entity *pos_se = NULL;
	unsigned int is_resched = 0;
	int i, min_pass_idx = 0, min_pass;

	printk(KERN_INFO "***[MYSTRD] update_curr: update_curr starting!\n");
		
	// update curr tasks' pass and make mystride_rq sorted by pass
	cur->mystride.pass += cur->mystride.stride;
	
	min_pass = cur->mystride.pass;
	pos = &mystride_rq->queue;

	for(i=0; i<mystride_rq->nr_running; i++){
		pos = pos->next;
		pos_se = container_of(pos, struct sched_mystride_entity, run_list);
		pos_p = container_of(pos_se, struct task_struct, mystride);
		
		printk(KERN_INFO "\t\t***[MYSTRD] update_curr: curr->pid=%d, pass=%d, p->pid=%d, pass=%d\n",
							cur->pid, cur->mystride.pass, pos_p->pid, pos_se->pass);
			
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
			pos = mystride_rq->queue.next;
			pos_se = container_of(pos, struct sched_mystride_entity, run_list);
			
			list_del_init(&pos_se->run_list);
			list_add_tail(&pos_se->run_list, &mystride_rq->queue);
		}

		resched_curr(rq);
	}
	printk(KERN_INFO "***[MYSTRD] update_curr: update_curr end!\n");
	//  resched	
}

static void enqueue_task_mystride(struct rq *rq, struct task_struct *p, int flags)
{
	// TODO
	list_add_tail(&p->mystride.run_list, &rq->mystride.queue);
	rq->mystride.nr_running++;

	printk(KERN_INFO "***[MYSTRD] enqueue: success cpu=%d, nr_running=%d, pid=%d\n",
				cpu_of(rq), rq->mystride.nr_running, p->pid);
}

static void dequeue_task_mystride(struct rq *rq, struct task_struct *p, int flags)
{
	//TODO

	if( !list_empty(&rq->mystride.queue) )
	{
		list_del_init(&p->mystride.run_list);
		rq->mystride.nr_running--;

		printk(KERN_INFO "\t***[MYSTRD] dequeue: success cpu=%d, nr_running=%d, pid=%d\n",	
				cpu_of(rq), rq->mystride.nr_running, p->pid);
	
	}
	else
	{
	}
}

static void check_preempt_curr_mystride(struct rq *rq, struct task_struct *p, int flags){}
struct task_struct *pick_next_task_mystride(struct rq *rq, struct task_struct *prev)
{
	struct task_struct *next_p = NULL;
	struct sched_mystride_entity *next_se = NULL;
	struct mystride_rq *mystride_rq = &rq->mystride;

	//TODO
	if( !mystride_rq->nr_running ) 
	{
		return NULL;
	}

	if(prev->sched_class != &mystride_sched_class)
	{
		printk(KERN_INFO "***[MYSTRD] pick_next_task: other class came in.. prev->pid=%d\n",
																				 prev->pid);
		put_prev_task(rq, prev);
	}

	//TODO
	next_se = container_of(mystride_rq->queue.next, struct sched_mystride_entity, run_list);
	next_p = container_of(next_se, struct task_struct, mystride);

	printk(KERN_INFO "\t***[MYSTRD] pick_next_task: cpu=%d, prev->pid=%d, next_p->pid=%d, nr_running=%d\n", cpu_of(rq), prev->pid, next_p->pid, mystride_rq->nr_running);

	return next_p;
}

static void put_prev_task_mystride(struct rq *rq, struct task_struct *p)
{
	printk(KERN_INFO "\t***[MYSTRD] put_prev_task: do nothing, p->pid=%d\n", p->pid);	
}
static int select_task_rq_mystride(struct task_struct *p, int cpu, int sd_flag, int flags)
{
	return task_cpu(p);
}
static void set_curr_task_mystride(struct rq *rq){}
static void task_tick_mystride(struct rq *rq, struct task_struct *p, int queued)
{
	update_curr_mystride(rq);
}

static void prio_changed_mystride(struct rq *rq, struct task_struct *p, int oldprio) {}
static void switched_to_mystride(struct rq *rq, struct task_struct *p)
{
	resched_curr(rq);
}

const struct sched_class mystride_sched_class={
	.next = &fair_sched_class,
	.enqueue_task = enqueue_task_mystride,
	.dequeue_task = dequeue_task_mystride,
	.check_preempt_curr = check_preempt_curr_mystride,
	.pick_next_task = pick_next_task_mystride,
	.put_prev_task = put_prev_task_mystride,
#ifdef CONFIG_SMP
	.select_task_rq = select_task_rq_mystride,
#endif
	.set_curr_task = set_curr_task_mystride,
	.task_tick = task_tick_mystride,
	.prio_changed = prio_changed_mystride,
	.switched_to = switched_to_mystride,
	.update_curr = update_curr_mystride,
};

