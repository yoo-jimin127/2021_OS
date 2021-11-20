#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

void OPT_algorithm(int frame_cnt, char *refer_buf);
void FIFO_algorithm(int frame_cnt, char *refer_buf);
void LRU_algorithm(int frame_cnt, char *refer_buf);
void SecondChance_algorithm(int frame_cnt, char *refer_buf);

int main (void) {
	/* ------ about file ------ */
	FILE *fp = NULL; //file pointer
	char inputbuf[61]; // input.txt content storage

	/* ------ about page frame ------ */
	char frame_cnt_str[5];
	int frame_cnt = 0;
	char refer_strbuf[61]; //except page frame, page reference string storage
	
	/* ------ about main function ------ */

	char option[50];

	if ((fp = fopen("input.txt", "r")) == NULL) { //file open
		fprintf(stderr, "input.txt fopen() error\n");
		exit(1);
	}

	fgets(inputbuf, sizeof(inputbuf), fp); // read input.txt
	fclose(fp); //file close

	/* ------ used method &  page reference string print ------ */
	printf("Used method : ");
	scanf("%s", option);

	char *ptr = strtok(inputbuf, "\n"); //tokenize frame_cnt & page reference string
	strcpy(frame_cnt_str, ptr); // string copy frame number info token
	frame_cnt = atoi(frame_cnt_str); //frame_cnt string -> int
	printf("frame_cnt: %d\n", frame_cnt);
	
	while (ptr != NULL) {
		ptr = strtok(NULL, "\n");
		printf("ptr: %s\n", ptr);
		strcpy(refer_strbuf, ptr);
		printf("refer_strbuf : %s\n", refer_strbuf);
	}

	printf("page reference string : %s\n", refer_strbuf);
	
	/* ------ apply algorithm method by option ------ */
	if (strcmp(option, "OPT") == 0) {
		OPT_algorithm(frame_cnt, &refer_strbuf);
	}

	else if (strcmp(option, "FIFO") == 0) {
		FIFO_algorithm(frame_cnt, &refer_strbuf);
	}

	else if (strcmp(option, "LRU") == 0) {
		LRU_algorithm(frame_cnt, &refer_strbuf);
	}

	else if (strcmp(option, "Second-Chance") == 0) {
		SecondChance_algorithm(frame_cnt, &refer_strbuf);
	}

	else {
		printf("You can use only <OPT, FIFO, LRU, Second-Chance>\n");
	}

	return 0;
}

/* ------ optimal altorithm ------*/
void OPT_algorithm(int frame_cnt, char *refer_buf) {

}

/* ------ FIFO altorithm ------*/
void FIFO_algorithm(int frame_cnt, char *refer_buf) {

}

/* ------ LRU altorithm ------*/
void LRU_algorithm(int frame_cnt, char *refer_buf) {

}

/* ------ Second Chance altorithm ------*/
void SecondChance_algorithm(int frame_cnt, char *refer_buf) {

}
