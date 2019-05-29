#include "sched.h"
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/slab.h>

void init_myrr_rq (struct myrr_rq *myrr_rq)
{
	// 런큐 초기화
	printk(KERN_INFO "***[MYRR] Myrr class is online \n");
	myrr_rq->nr_running = 0;
	INIT_LIST_HEAD(&myrr_rq->queue);
}

static void update_curr_myrr(struct rq *rq)
{
	struct task_struct *cur = rq->curr; 	// cur : 현재 task
	struct myrr_rq *myrr_rq = &rq->myrr;

	// 현재 task의 update_num 값 증가
	cur->myrr.update_num++;
	printk(KERN_INFO "***[MYRR] update_curr_myrr pid = %d, update_num = %d\n",
				cur->pid, cur->myrr.update_num);

	// 만약 현재 task가 3번넘게 업데이트 되었다면 -> 다음 task에게 cpu 넘겨줘야함
	if(cur->myrr.update_num > 3)
	{
		// update_num 값 초기화
		cur->myrr.update_num = 0;
		// 현재 타스크는 런큐 맨 마지막에 넣어주고, 맨 앞의 타스크를 dequeue
		list_del_init(&cur->myrr.run_list);
		list_add_tail(&cur->myrr.run_list, &myrr_rq->queue);
		// 리스케쥴
		resched_curr(rq);
	}
	
}

static void enqueue_task_myrr(struct rq *rq, struct task_struct *p, int flags)
{
	// 큐의 끝에 새 타스크 추가
	list_add_tail(&p->myrr.run_list, &rq->myrr.queue);
	// running 타스크 개수 추가
	rq->myrr.nr_running++;

	printk(KERN_INFO "***[MYRR] enqueue: success cpu=%d, nr_running=%d, pid=%d\n",
				cpu_of(rq), rq->myrr.nr_running, p->pid);
}

static void dequeue_task_myrr(struct rq *rq, struct task_struct *p, int flags)
{
	//TODO
	// 런큐 비었으면 아무것도 안함
	if( !list_empty(&rq->myrr.queue) )
	{
		// 큐 맨 앞 타스크 삭제
		list_del_init(&p->myrr.run_list);
		// running 타스크 개수 감소
		rq->myrr.nr_running--;

		printk(KERN_INFO "\t***[MYRR] dequeue: success cpu=%d, nr_running=%d, pid=%d\n",	
				cpu_of(rq), rq->myrr.nr_running, p->pid);
	
	}
	else
	{
	}
}

static void check_preempt_curr_myrr(struct rq *rq, struct task_struct *p, int flags){}
struct task_struct *pick_next_task_myrr(struct rq *rq, struct task_struct *prev)
{
	struct task_struct *next_p = NULL;
	struct sched_myrr_entity *next_se = NULL;
	struct myrr_rq *myrr_rq = &rq->myrr;

	// 큐 비어있으면 NULL 리턴
	if( !myrr_rq->nr_running ) 
	{
		return NULL;
	}

	if(prev->sched_class != &myrr_sched_class)
	{
		printk(KERN_INFO "***[MYRR] pick_next_task: other class came in.. prev->pid=%d\n"
						, prev->pid);
		// 기존 task를 put
		put_prev_task(rq, prev);
	}

	// 큐에서 다음요소를 담고 있는 sched_myrr_entity를 next_se에 저장
	next_se = container_of(myrr_rq->queue.next, struct sched_myrr_entity, run_list);
	// next_se를 요소로 가지는 task 찾아서 next_p에 저장
	next_p = container_of(next_se, struct task_struct, myrr);

	printk(KERN_INFO "\t***[MYRR] pick_next_task: cpu=%d, prev->pid=%d, next_p->pid=%d, \
					nr_running=%d\n", cpu_of(rq), prev->pid, next_p->pid, myrr_rq->nr_running);

	return next_p;
}

static void put_prev_task_myrr(struct rq *rq, struct task_struct *p)
{
	printk(KERN_INFO "\t***[MYRR] put_prev_task: do nothing, p->pid=%d\n", p->pid);	
}
static int select_task_rq_myrr(struct task_struct *p, int cpu, int sd_flag, int flags)
{
	return task_cpu(p);
}
static void set_curr_task_myrr(struct rq *rq){}
static void task_tick_myrr(struct rq *rq, struct task_struct *p, int queued)
{
	update_curr_myrr(rq);	
}
static void prio_changed_myrr(struct rq *rq, struct task_struct *p, int oldprio) {}
static void switched_to_myrr(struct rq *rq, struct task_struct *p)
{
	resched_curr(rq);
}

const struct sched_class myrr_sched_class={
	.next = &fair_sched_class,
	.enqueue_task = enqueue_task_myrr,
	.dequeue_task = dequeue_task_myrr,
	.check_preempt_curr = check_preempt_curr_myrr,
	.pick_next_task = pick_next_task_myrr,
	.put_prev_task = put_prev_task_myrr,
#ifdef CONFIG_SMP
	.select_task_rq = select_task_rq_myrr,
#endif
	.set_curr_task = set_curr_task_myrr,
	.task_tick = task_tick_myrr,
	.prio_changed = prio_changed_myrr,
	.switched_to = switched_to_myrr,
	.update_curr = update_curr_myrr,
};

