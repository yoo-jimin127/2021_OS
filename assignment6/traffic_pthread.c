#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

int main (void) {
	int userInput = 0; //user's input at program init screen
	int *startList; // start position list by userInput

	printf("please input <Total number of vehicles> (range: 10 ~ 15) : ");
	scanf("%d", &userInput);

	startList = malloc(sizeof(int) * userInput); //memory allocation size by userInput
	
	srand((unsigned)time(NULL)); //initialize random set
	memset(startList, 0, userInput); //initialize startList array
	
	//make random list by userInput
	for (int i = 0; i < userInput; i++) {
		int randPos = (rand() % 4) + 1; //make random element
		startList[i] = randPos; //randomPos insert
	}

	printf("Total number of vehicles : %d\n", userInput);
	printf("Start point : ");
	for (int i = 0; i < userInput; i++) {
		if (i == (userInput - 1)) {
			printf("%d\n", startList[i]);
			break;
		}
		printf("%d ", startList[i]);
	}

	return 0;
}
