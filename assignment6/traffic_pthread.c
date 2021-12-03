#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define THREAD_NUM 4
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //mutex initialization
int share_count = 0; //share resource by thread

void *t_function(void *data); //thread function

int main (void) {
	int userInput = 0; //user's input at program init screen
	int *startList; // start position list by userInput

	/* ------ << definition related to pthread >> ------ */ 
	int thr_id; //thread id
	pthread_t p_thread[THREAD_NUM]; //
	pthread_attr_t attr; //set of thread attributes
	char p1[] = "thread_1";
	char p2[] = "thread_2";
	char p3[] = "thread_3";
	char p4[] = "thread_4";
	char pM[] = "thread_m";
	int status;

	printf("please input <Total number of vehicles> (range: 10 ~ 15) : ");
	scanf("%d", &userInput);
	printf("\n");

	startList = malloc(sizeof(int) * userInput); //memory allocation size by userInput
	
	srand((unsigned)time(NULL)); //initialize random set
	memset(startList, 0, userInput); //initialize startList array
	
	//make random list by userInput
	for (int i = 0; i < userInput; i++) {
		int randPos = (rand() % 4) + 1; //make random element
		startList[i] = randPos; //randomPos insert
	}

	printf("Total number of vehicles : %d\n", userInput); //print userInput
	printf("Start point : "); //print startList element
	for (int i = 0; i < userInput; i++) {
		if (i == (userInput - 1)) {
			printf("%d\n", startList[i]);
			break;
		}
		printf("%d ", startList[i]);
	}

	thr_id = pthread_create(&p_thread[0], NULL, t_function, (void *)p1);
	if (thr_id < 0) {
		perror("thread create error : ");
		exit(0);
	}

	thr_id = pthread_create(&p_thread[1], NULL, t_function, (void *)p2);
	if (thr_id < 0) {
		perror("thread create error : ");
		exit(0);
	}

	thr_id = pthread_create(&p_thread[2], NULL, t_function, (void *)p3);
	if (thr_id < 0) {
		perror("thread create error : ");
		exit(0);
	}
	
	thr_id = pthread_create(&p_thread[3], NULL, t_function, (void *)p4);
	if (thr_id < 0) {
		perror("thread create error : ");
		exit(0);
	}
	
	t_function((void *)pM);

	pthread_join(p_thread[0], (void **)&status);
	pthread_join(p_thread[1], (void **)&status);
	pthread_join(p_thread[2], (void **)&status);
	pthread_join(p_thread[3], (void **)&status);
	
	return 0;
}

void *t_function(void *data) {
	pid_t pid; //process id
	pthread_t tid; //thread id

	pid = getpid();
	tid = pthread_self();

	char *thread_name = (char*)data;

	int i = 0;

	while(i < 4) {
		printf("[%s] pid: %u, tid: %x --- %d\n", thread_name, (unsigned int)pid, (unsigned int)tid, i);
		i++;
		sleep(1);
	}
	

}

