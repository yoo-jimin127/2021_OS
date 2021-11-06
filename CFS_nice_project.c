#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <wait.h>

struct sched_attr {
	uint32_t size; //size of struct
	
	uint32_t sched_policy; //SCHED_OTHER, SCHED_FIFO, SCHED_RR
	int32_t sched_nice; //nice value
	uint32_t sched_priority; //priority value
	
	uint64_t sched_flags;
	uint64_t sched_runtime;
	uint64_t sched_deadline;
	uint64_t sched_period;
};

//change scheduling attribute function
static int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags) {
	return syscall(SYS_sched_setattr, pid, attr, flags);
}

int main (void) {
	long long num = 1;
	struct sched_attr attr;
	int result = 0; //store change scheduling attribute return value
	pid_t pid; //process id
	pid_t pids[21];
	pid_t pid_child;
	int status; //wait() status

	memset(&attr, 0, sizeof(attr)); //initialize attr
	attr.size = sizeof(struct sched_attr); //set attr's size

	for (int i = 0; i < 21; i++) {
		pids[i] = fork(); // make child process from parent process
		//sleep(1);
	
		if (pids[i] == -1) { // fork() error
			printf("fork() error at [%d] loof\n", i);
			exit(0);
		}
		
		else if (pids[i] == 0) { //child process
			if (i >= 0 && i <= 6) { // process group A
				printf("[process group A] <%d process> begin (parent process id: %d)\n", getpid(), getppid()); 
				memset(&attr, 0, sizeof(attr)); //initialize attr
				attr.size = sizeof(struct sched_attr); //set attr's size

				attr.sched_nice = 19; // set nice value [A group: high nice]
				attr.sched_policy = SCHED_OTHER;

				result = sched_setattr(getpid(), &attr, 0);
				if (result == -1) {
					perror("ERROR: error occur at calling sched_setattr <group A>.\n");
				}

				for (long long a = 0; a < 100000; a++) {
					for (long long b = 0; b < 20000; b++) {
						num = a * b;
					}
				}
			}

			else if (i >= 7 && i <= 13) { // process group B
				printf("[process group B] <%d process> begin (parent process id: %d)\n", getpid(), getppid()); 
				memset(&attr, 0, sizeof(attr)); //initialize attr
				attr.size = sizeof(struct sched_attr); //set attr's size
				
				attr.sched_nice = 10; // set nice value [B group: middle nice]
				attr.sched_policy = SCHED_OTHER;

				result = sched_setattr(getpid(), &attr, 0);
				if (result == -1) {
					perror("ERROR: error occurs at calling sched_setattr <group B>.\n");
				}
				//printf("b\n");

				for (long long a = 0; a < 100000; a++) {
					for (long long b = 0; b < 20100; b++) {
						num = a * b;
					}
				}
			}

			else if (i >= 14 && i <= 20) { // process group C
				printf("[process group C] <%d process> begin (parent process id: %d)\n", getpid(), getppid()); 
				memset(&attr, 0, sizeof(attr)); //initialize attr
				attr.size = sizeof(struct sched_attr); //set attr's size
				
				attr.sched_nice = 5; // set nice value [C group: low nice]
				attr.sched_policy = SCHED_OTHER;

				result = sched_setattr(getpid(), &attr, 0);
				if (result == -1) {
					perror("ERROR: error occurs at calling sched_setattr <group C>.\n");
				}
				//printf("c\n");

				for (long long a = 0; a < 100000; a++) {
					for (long long b = 0; b < 20200; b++) {
						num = a * b;
					}
				} 
			}
	
			printf("<%d process> end\n", getpid());
			exit(1);
		}
	}
		
	for (int i = 0; i < 21; i++) {
		pid_child = wait(&status);
	}
	
	
	return 0;
}

