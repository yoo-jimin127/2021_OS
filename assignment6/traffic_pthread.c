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
#define MAX_ARR_SIZE 15
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //mutex initialization
int ncount = 0; //share resource by thread
int result = 0; //store return value at pthread_join()

int totalTime = 0; //total time(ticks)
int passCar = 0; // passed vihicle
int waitCar[MAX_ARR_SIZE] = { 0, }; // waiting vehicle array
int *startList; // start position list by userInput
int userInput = 0; //user's input at program init screen

void *t_function(void *data); //thread function

int main (void) {
	int pointCnt[4] = { 0, }; //count each start point

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
		exit(1);
	}

	thr_id = pthread_create(&p_thread[1], NULL, t_function, (void *)p2);
	if (thr_id < 0) {
		perror("thread create error : ");
		exit(1);
	}

	thr_id = pthread_create(&p_thread[2], NULL, t_function, (void *)p3);
	if (thr_id < 0) {
		perror("thread create error : ");
		exit(1);
	}
	
	thr_id = pthread_create(&p_thread[3], NULL, t_function, (void *)p4);
	if (thr_id < 0) {
		perror("thread create error : ");
		exit(1);
	}
	
	t_function((void *)pM);

	pthread_join(p_thread[0], (void **)&status);
	pthread_join(p_thread[1], (void **)&status);
	pthread_join(p_thread[2], (void **)&status);
	pthread_join(p_thread[3], (void **)&status);

	printf("Number of vehicles passed from each start point\n");
	for (int i = 0; i < userInput; i++) {
		if (startList[i] == 1) pointCnt[0]++;
		else if (startList[i] == 2) pointCnt[1]++;
		else if (startList[i] == 3) pointCnt[2]++;
		else if (startList[i] == 4) pointCnt[3]++;
	}

	for (int i = 0; i < 4; i++) {
		printf("P%d : %d times\n", i+1, pointCnt[i]);
	}

	printf("Total time : %d ticks\n", totalTime);
	
	
	return 0;
}

void *t_function(void *data) {
	pid_t pid; //process id
	pthread_t tid; //thread id

	pid = getpid();
	tid = pthread_self();

	char *thread_name = (char*)data;
	
	pthread_mutex_lock(&mutex);
	while(1) {
		totalTime++;

		printf("tick : %d\n", totalTime);
		printf("===============================\n");
		printf("Passed Vehicle\n");
		printf("Car %d\n", passCar);
		printf("Waiting Vehicle\n");
		printf("Car ");
		for (int i = 0; i < MAX_ARR_SIZE; i++) {
			if (waitCar[i] == 0) break;
			printf("%d ", waitCar[i]);
		}
		printf("\n===============================\n");
		sleep(1);

	}
	pthread_mutex_unlock(&mutex);
	
}

