#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/delay.h>

static struct semaphore waitingRoom;
static struct semaphore barberChair;
static struct semaphore barberPillow;
static struct semaphore seatBelt;
static struct semaphore killsem;

static int customerCount=5;
static int allDone=0;
static struct task_struct *bthread;
static struct task_struct *cthread[5];

int barber(void *data)
{
	while (allDone < customerCount) {
		printk(KERN_INFO "The barber is sleeping.");
		down(&barberPillow);
		printk(KERN_INFO "The barber is cutting hair");
		msleep(1000);
		printk(KERN_INFO "The barber has finished cutting hair.");
		up(&seatBelt);
		allDone++;
	}
	printk(KERN_INFO "The barber is going home for the day.");
	up(&killsem);
	do_exit(0);
	return 0;
}

int customer(void* n)
{
	int num = (int)n;
	num++;
	printk(KERN_INFO "Customer %d arrived at barber shop.", num);
	down(&waitingRoom);
	printk(KERN_INFO "Customer %d entering waiting room.", num);
	down(&barberChair);
	up(&waitingRoom);
	printk(KERN_INFO "Customer %d waking the barber.", num);
	up(&barberPillow);
	down(&seatBelt);
	printk(KERN_INFO "Customer %d leaving barber shop. Value: %d", num, allDone);
	up(&barberChair);
	do_exit(0);
	return 0;
}

asmlinkage long sys_sleepingbarber(void)
{
	sema_init(&waitingRoom, customerCount);
	sema_init(&barberChair, 1);
	sema_init(&barberPillow, 0);
	sema_init(&seatBelt, 0);
	sema_init(&killsem, 0);

	printk(KERN_INFO "Creating barber thread");
	bthread = kthread_create(barber, NULL, "bthread");
	wake_up_process(bthread);

	printk(KERN_INFO "Creating customer threads");
	int i;
	for(i=0; i<customerCount; i++)
	{
		char s[9] = "cthread0";
		s[7] = (char)(32+i);
		cthread[i] = kthread_create(customer, (void*)i, s);
		wake_up_process(cthread[i]);
	}
	down(&killsem);
	for(i=0; i<customerCount; i++)
	{
		kthread_stop(cthread[i]);
	}
//	up(&barberPillow);
	kthread_stop(bthread);
	return 0;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("K163807|K163760");
MODULE_DESCRIPTION("Solution to sleeping barber problem (System Call)");
