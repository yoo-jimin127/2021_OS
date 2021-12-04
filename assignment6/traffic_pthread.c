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

/* ====== << global variable declaration >> ====== */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //mutex initialization
int ncount = 0; //share resource by thread
int result = 0; //store return value at pthread_join()
pthread_cond_t mainThr_cond = PTHREAD_COND_INITIALIZER; //pthread condition var initializer (main pthread)
pthread_cond_t cond[4] = { PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER }; //pthread condition var initialization (each 4 pthreads) 

int userInput = 0; //user's input at program init screen
int *startList; // start position list by userInput
int passCar = 0; // arriveed vehicle
int *waitCar; // waiting vehicle array
int movingCar = 0; // moving Vehicle
int movingIdx = 0; // moving vehicle's array index

int *arriveCheck; // vehicle's arriveed check array
int totalTime = 0; //total time(ticks)
int iterNum = 0; //iterator used to occupy pthread
int arriveCnt = 0; //arrived car's count

/* ====== << function declaration >> ====== */
void status_print(); //status print each tick
void *thr1_function(void *data); //<p1> thread function
void *thr2_function(void *data); //<p2> thread function
void *thr3_function(void *data); //<p3> thread function
void *thr4_function(void *data); //<p4> thread function

int main (void) {
	int pointCnt[4] = { 0, }; //count each start point
	int diff = 0; //time difference store variable 
	int flag = 0; //check time flag

	/* ====== << definition related to pthread >> ====== */ 
	int thr_id; //thread id
	pthread_t p_thread[THREAD_NUM]; //thread ID storage (each 4 threads)
	pthread_attr_t attr; //set of thread attributes
	char p1[] = "thread_p1";
	char p2[] = "thread_p2";
	char p3[] = "thread_p3";
	char p4[] = "thread_p4";
	char pM[] = "thread_m";
	int status;

	printf("please input <Total number of vehicles> (range: 10 ~ 15) : ");
	scanf("%d", &userInput);
	printf("\n");

	/* ====== << memory allocation part >> ====== */
	startList = (int *)malloc(sizeof(int) * userInput); //memory allocation size by userInput
	waitCar = (int *)malloc(sizeof(int) * userInput);
	arriveCheck = (int *)malloc(sizeof(int) * userInput);

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

	/* ====== << create 4 threads progress >> ====== */
	pthread_mutex_lock(&mutex); //pthread mutex lock instruction

	thr_id = pthread_create(&p_thread[0], NULL, thr1_function, (void *)p1); //create thread1
	if (thr_id < 0) {
		perror("thread1 create error : "); // if error occurs, print error
		exit(1); //abnormal end
	}
	pthread_cond_wait(&mainThr_cond, &mutex); //wait main thread's condition variable

	thr_id = pthread_create(&p_thread[1], NULL, thr2_function, (void *)p2); //create thread2
	if (thr_id < 0) {
		perror("thread create error : "); // if error occurs, print error
		exit(1); //abnormal end
	}
	pthread_cond_wait(&mainThr_cond, &mutex); //wait main thread's condition variable

	thr_id = pthread_create(&p_thread[2], NULL, thr3_function, (void *)p3); //create thread3
	if (thr_id < 0) {
		perror("thread create error : "); // if error occurs, print error
		exit(1); //abnormal end
	}
	pthread_cond_wait(&mainThr_cond, &mutex); //wait main thread's codition variable
	
	thr_id = pthread_create(&p_thread[3], NULL, thr4_function, (void *)p4); //create thread4
	if (thr_id < 0) {
		perror("thread create error : "); // if error occurs, print error
		exit(1); //abnormal end
	}
	pthread_cond_wait(&mainThr_cond, &mutex); //wait main thread's condition variable
	
	pthread_mutex_unlock(&mutex); //pthead mutex unlock instruction

	/* ====== << mutex occupy & thread operation >> ====== */
	while(1) {
		pthread_mutex_lock(&mutex); //pthred mutex lock instruction
		
		passCar = 0; //arrived car's value initialize	
		totalTime++; //update tick(total time)
		if (totalTime >= userInput) iterNum = userInput;
		else iterNum = totalTime;
		
		/* ====== << CASE 1. no moving Car >> ====== */
		if (movingCar == 0) {
			for (int i = 0; i < iterNum; i++) {
				if (arriveCheck[i] == 0) {
					movingIdx = i; //update moving car's array index
					movingCar = startList[i];
					break;
				}
			}
		}
		pthread_cond_signal(&cond[movingCar - 1]); // wake waiting thread : movingCar idx cond
		pthread_cond_wait(&mainThr_cond, &mutex); // wait main thread's condition variable

		/* ====== << CASE 2. exist waiting car >> ====== */
		if (waitCar[movingIdx] == 2) {
			flag = 0;

			for (int i = 0; i < iterNum; i++) {
				diff = movingCar - startList[i];
				if (diff < 0) diff *= (-1);
				if ((diff == 2) && (arriveCheck[i] == 0)) {
					movingCar = startList[i];
					movingIdx = i;
					flag = 1;
					break;
				}
			}

			if (flag == 1) {
				pthread_cond_signal(&cond[movingCar - 1]);
				pthread_cond_wait(&mainThr_cond, &mutex);
			}

			else {
				movingCar = 0;
				movingIdx = 0;
			}
		}

		status_print();

		if (arriveCnt == userInput) {
			passCar = 0;
			totalTime++;

			status_print();

			break;
		}
		
		pthread_mutex_unlock(&mutex);
	}

	/* pthread_cond_signal(&cond[0]);
	pthread_cond_signal(&cond[1]);
	pthread_cond_signal(&cond[2]);
	pthread_cond_signal(&cond[3]); */

	/* ====== << pthread end >> ====== */
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

/* ====== << status print each ticks >> ======= */
void status_print() {
	printf("tick : ");
	printf("%d\n", totalTime);
	printf("===============================\n");

	printf("Passed Vehicle\n");
	printf("Car ");
	if (passCar == 0) printf("\n");
	else printf("%d\n", passCar);

	printf("Waiting Vehicle\n");
	printf("Car ");
	for (int i = 0; i < iterNum; i++) {
		if (waitCar[i] == 0) printf("%d ", startList[i]);
	}
	printf("\n===============================\n");
}

/* ====== << <P1> thread function >> ====== */
void *thr1_function(void *data) {
	pthread_t tid; //thread id
	tid = pthread_self();
	
	while(1) {
		pthread_mutex_lock(&mutex); //pthread mutex lock instruction
		
		pthread_cond_signal(&mainThr_cond); //wake main thread's condition variable
		pthread_cond_wait(&cond[0], &mutex); //wait first thread's condition variable

		/* ====== << CASE 1. all cars arrived >> ====== */
		if (arriveCnt == userInput) {
			pthread_mutex_unlock(&mutex); //make mutex unlock status
			break;
		}

		/* ====== << CASE 2. no all cars arrived >> ====== */
		waitCar[movingIdx]++; //make movingCar array index's status -> waiting
		
		if (waitCar[movingIdx] == 2) { //when waitCar[movingIdx] == 2 -> pass that car
			arriveCheck[movingIdx] = 1;
			passCar = 1;
			
			arriveCnt++;
		}

		pthread_mutex_unlock(&mutex); //pthread mutex unlock
	}

	return NULL;
}

/* ====== << <P2> thread function >> ====== */
void *thr2_function(void *data) {
	pthread_t tid; //thread id
	tid = pthread_self();

	while(1) {
		pthread_mutex_lock(&mutex); //pthread mutex lock instruction

		pthread_cond_signal(&mainThr_cond); //wake main thread's condition variable
		pthread_cond_wait(&cond[1], &mutex); //wait first thread's condition variable

		/* ====== << CASE 1. all cars arrived >> ====== */
		if (arriveCnt == userInput) {
			pthread_mutex_unlock(&mutex); //make mutex unlock status
			break;
		}

		/* ====== << CASE 2. no all cars arrived >> ====== */
		waitCar[movingIdx]++; //make movingCar array index's status -> waiting

		if (waitCar[movingIdx] == 2) { //when waitCar[movingIdx] == 2 -> pass that car
			arriveCheck[movingIdx] = 1;
			passCar = 2;

			arriveCnt++;
		}

		pthread_mutex_unlock(&mutex); //pthread mutex unlock
	}

	return NULL;
}

/* ====== << <P3> thread function >> ====== */
void *thr3_function(void *data) {
	pthread_t tid; //thread id
	tid = pthread_self();

	while(1) {
		pthread_mutex_lock(&mutex); //pthread mutex lock instruction

		pthread_cond_signal(&mainThr_cond); //wake main thread's condition variable
		pthread_cond_wait(&cond[2], &mutex); //wait first thread's condition variable

		/* ====== << CASE 1. all cars arrived >> ====== */
		if (arriveCnt == userInput) {
			pthread_mutex_unlock(&mutex); //make mutex unlock status
			break;
		}

		/* ====== << CASE 2. no all cars arrived >> ====== */
		waitCar[movingIdx]++; //make movingCar array index's status -> waiting

		if (waitCar[movingIdx] == 2) { //when waitCar[movingIdx] == 2 -> pass that car
			arriveCheck[movingIdx] = 1;
			passCar = 3;

			arriveCnt++;
		}

		pthread_mutex_unlock(&mutex); //pthread mutex unlock
	}

	return NULL;
}

/* ====== << <P4> thread function >> ====== */
void *thr4_function(void *data) {
	pthread_t tid; //thread id
	tid = pthread_self();

	while(1) {
		pthread_mutex_lock(&mutex); //pthread mutex lock instruction

		pthread_cond_signal(&mainThr_cond); //wake main thread's condition variable
		pthread_cond_wait(&cond[3], &mutex); //wait first thread's condition variable
		
		/* ====== << CASE 1. all cars arrived >> ====== */
		if (arriveCnt == userInput) {
			pthread_mutex_unlock(&mutex); // wake main thread's condition variable
			break;
		}

		/* ====== << CASE 2. no all cars arrived >> ====== */
		waitCar[movingIdx]++; //make movingCar array index's status -> waiting

		if (waitCar[movingIdx] == 2) { //when waitCar[movingIdx] == 2 -> pass that car
			arriveCheck[movingIdx] = 1;
			passCar = 4;

			arriveCnt++;
		}

		pthread_mutex_unlock(&mutex); //pthread mutex unlock
	}

	return NULL;
}
