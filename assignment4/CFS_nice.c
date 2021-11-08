#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <wait.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

struct sched_attr{
	uint32_t size;
	uint32_t sched_policy;
	uint64_t sched_flags;
	int32_t sched_nice;

	uint32_t sched_priority;

	uint64_t sched_runtime;
	uint64_t sched_deadline;
	uint64_t sched_period;
};

static int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags)
{
	return syscall(SYS_sched_setattr, pid, attr, flags);
}

void delay(int count)
{
	int i, j, k;
	int *data;

	data = (int *)malloc(sizeof(int) * 1024 * 10);

	if(data <= 0)
		printf("error on memory(delay)\n");

	for(i=0;i<count;i++)
	{
		for(k=0;k<100;k++)
			for(j=0;j<500;j++)
				data[j] = j*k;
	}

}	

int main(void)
{
	//part of scheduling
	struct sched_attr attr;

	memset(&attr, 0, sizeof(attr));
	attr.size = sizeof(struct sched_attr);

	//CFS Scheduler
	attr.sched_policy = SCHED_OTHER;

	//part of process
	int pids[22];
	int rets[22];
	int ret, state;

	printf("====================\n");
	for(int i=1;i<22;i++)
	{
		ret = fork();

		if(ret > 0)
		{
			pids[i] = ret;
			printf(" %d process begins\n", ret);
		}

		else if(ret == 0)
		{
			if(i < 8)
				attr.sched_nice = 19;
			else if(i < 15)
				attr.sched_nice = 0;
			else if(i < 22)
				attr.sched_nice = -20;

			sched_setattr(getpid(), &attr, 0);
			delay(5000);
			printf(" %d process ends\n", getpid());
			exit(0);
		}

		else
		{
			perror("fork error \n");
			exit(0);
		}
	}
		
	printf("=====================\n");
	printf("1~7's Nice value   = 19\n8~14's Nice value  = 0\n15~21's Nice value = -20\n");
	printf("=====================\n");

	for(int i=1;i<22;i++)
	{
		rets[i] = wait(&state);
	}

	printf("==All processes end==\n");
		
	return 0;
}
