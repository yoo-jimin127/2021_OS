#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

void OPT_algorithm(int frame_cnt, int element_cnt, int *refer_buf);
void FIFO_algorithm(int frame_cnt, int element_cnt, int *refer_buf);
void LRU_algorithm(int frame_cnt, int element_cnt, int *refer_buf);
void SecondChance_algorithm(int frame_cnt, int element_cnt, int *refer_buf);
int CheckExist(int *frame_buf, int frame_cnt, int refer_string);

int main (void) {
	/* ------ 파일 관련 변수 ------ */
	FILE *fp = NULL; //파일포인터
	char inputbuf[61]; // input.txt 파일 저장 공간

	/* ------ page frame 관련 변수 ------ */
	char frame_cnt_str[5];
	int frame_cnt = 0; // page frame 저장 변수
	char refer_strbuf[65]; //page frame 제외한 page reference string 저장소
	int refer_buf[31] = { 0, }; // page reference string를 정수형으로 저장할 배열
	int element_cnt = 0; //정수형 배열의 인덱스 카운터 변수
	
	/* ------ 메인 기능 관련 변수 ------ */
	char option[50];

	if ((fp = fopen("input.txt", "r")) == NULL) { //file open
		fprintf(stderr, "input.txt fopen() error\n");
		exit(1);
	}

	fread(inputbuf, sizeof(inputbuf), 1, fp); // input.txt 읽는 작업
	fclose(fp); //file close

	/* ------ used method &  page reference string 출력 ------ */
	printf("Used method : ");
	scanf("%s", option);

	char *ptr = strtok(inputbuf, "\n"); //frame_cnt & page reference string 토큰화
	strcpy(frame_cnt_str, ptr); // 토큰으로 저장된 frame number info 저장
	frame_cnt = atoi(frame_cnt_str); //frame_cnt string -> int
	
	ptr = strtok(NULL, "\n");
	strncpy(refer_strbuf, ptr, strlen(ptr)); // frame_cnt 제외한 page reference string 저장
	int temp = strlen(refer_strbuf);
	char *intptr = strtok(refer_strbuf, " "); //공백자를 기준으로 토큰화(문자열 배열 -> 정수형 배열)
	
	while(intptr != NULL) {
		refer_buf[element_cnt] = atoi(intptr);
		intptr = strtok(NULL, " ");
		element_cnt++;
	} 

	printf("page reference string : %s\n", ptr);
	printf("\n");
	printf("\tframe   1\t2\t3\tpage fault\n");
	printf("time\n");
	
	/* ------ 옵션에 따른 알고리즘 기법 적용 ------ */
	if (strcmp(option, "OPT") == 0) {
		OPT_algorithm(frame_cnt, element_cnt, refer_buf);
	}

	else if (strcmp(option, "FIFO") == 0) {
		FIFO_algorithm(frame_cnt, element_cnt, refer_buf);
	}

	else if (strcmp(option, "LRU") == 0) {
		LRU_algorithm(frame_cnt, element_cnt, refer_buf);
	}

	else if (strcmp(option, "Second-Chance") == 0) {
		SecondChance_algorithm(frame_cnt, element_cnt, refer_buf);
	}

	else {
		printf("You can use only <OPT, FIFO, LRU, Second-Chance>\n");
	}

	return 0;
}

/* ------ optimal altorithm ------*/
void OPT_algorithm(int frame_cnt, int element_cnt, int *refer_buf) {
int *frame_buf = malloc(sizeof(int) * frame_cnt); // frame_buf 동적할당
	char *fault_check = malloc(sizeof(char) * element_cnt); //pagefault의 발생여부를 저장하는 배열
	char *level_print = malloc(sizeof(char) * 300); //각 단계에서의 page replacement 상태를 출력하는 메시지
	char *tmp_print = malloc(sizeof(char) * 10); //문자열 저장을 위한 임시 버퍼

	int pagefault_cnt = 0; //page fault counter
	int currIdx = 0; //page reference string을 삽입하는 인덱스
	int selectedIdx = 0; //OPT 알고리즘에 의해 선택된 인덱스
	int longest_frameNum = 0; //앞으로 가장 오랫동안 참조되지 않을 frame의 index number
	int frame_distance = 0; //각 frame별 참조되지 않는 시간을 저장하는 변수
	
	for (int i = 0; i < frame_cnt; i++) {
		frame_buf[i] = -1; //frame_buf를 -1으로 초기화
	}
	
	memset(fault_check, ' ', sizeof(fault_check)); //fault_check 배열 초기화

	for (int i = 0; i < element_cnt; i++) {
		memset(level_print, 0, sizeof(level_print));
		longest_frameNum = 0; //가장 오랫동안 참조되지 않은 frame의 index number 초기화
		
		if (frame_cnt > currIdx) { //currIdx의 값이 frame_cnt보다 작은 경우(비어있는 frame 존재)
			if(CheckExist(frame_buf, frame_cnt, refer_buf[i]) == -1) { //비어있는 frame을 발견한 경우		
				frame_buf[currIdx] = refer_buf[i]; //refer_buf[i]의 메모리를 currIdx 값에 저장
				pagefault_cnt++; //pagefault_cnt 증가
				fault_check[i] = 'F'; //해당 i번째 element에서의 page fault 발생여부 체크
				currIdx++;
			}

			else { //page fault가 발생하지 않는 경우
				sprintf(level_print,"%d\t\t", i+1);
				if (frame_buf[0] == -1) sprintf(tmp_print, " \t");
		         else sprintf(tmp_print, "%d\t", frame_buf[0]);
		         strcat(level_print, tmp_print);
		         memset(tmp_print, 0, sizeof(tmp_print));
 
				if (frame_buf[1] == -1) sprintf(tmp_print, " \t");
		         else sprintf(tmp_print, "%d\t", frame_buf[1]);
		         strcat(level_print, tmp_print);
		         memset(tmp_print, 0, sizeof(tmp_print));
 
		         if (frame_buf[2] == -1) sprintf(tmp_print, " \t");
		         else sprintf(tmp_print, "%d\t", frame_buf[2]);
		         strcat(level_print, tmp_print);
		         memset(tmp_print, 0, sizeof(tmp_print));
		         sprintf(tmp_print, "%c", fault_check[i]);
 
		         strcat(level_print, tmp_print);
		         printf("%s\n", level_print);

				continue;
			}
		}

		else { //OPTIMAL Page replacement 발생
			if(CheckExist(frame_buf, frame_cnt, refer_buf[i]) != -1) { //빈 frame이 존재하지 않는 경우
				sprintf(level_print,"%d\t\t", i+1);
                if (frame_buf[0] == -1) sprintf(tmp_print, " \t");
                else sprintf(tmp_print, "%d\t", frame_buf[0]);
				strcat(level_print, tmp_print);
                memset(tmp_print, 0, sizeof(tmp_print));
 
                if (frame_buf[1] == -1) sprintf(tmp_print, " \t");
                else sprintf(tmp_print, "%d\t", frame_buf[1]);
                strcat(level_print, tmp_print);
                memset(tmp_print, 0, sizeof(tmp_print));
 
                if (frame_buf[2] == -1) sprintf(tmp_print, " \t");
                else sprintf(tmp_print, "%d\t", frame_buf[2]);
                strcat(level_print, tmp_print);
                memset(tmp_print, 0, sizeof(tmp_print));
                sprintf(tmp_print, "%c", fault_check[i]);
 
                strcat(level_print, tmp_print);
                printf("%s\n", level_print);

				continue;
			}

			for (int j = 0; j < frame_cnt; j++) { //page fault가 발생하는 경우
				frame_distance = CalcFrameLongest(i, element_cnt, frame_buf[j], refer_buf);
				if (longest_frameNum < frame_distance) { //현재 longest_frameNum이 계산된 frame_distance 값보다 큰 경우
					longest_frameNum = frame_distance; //longest_frameNum 값 갱신
					selectedIdx = j; //OPT 알고리즘에 의해 갱신된 index num 설정
				}
			}

			frame_buf[selectedIdx] = refer_buf[i]; //선택된 인덱스의 프레임에 메모리 저장
			pagefault_cnt++; //pagefault의 카운터 증가
			fault_check[i] = 'F'; //해당 i번째 element에서의 page fault 발생여부 체크
		}

		sprintf(level_print,"%d\t\t", i+1);

		if (frame_buf[0] == -1) sprintf(tmp_print, " \t");
		else sprintf(tmp_print, "%d\t", frame_buf[0]);
		strcat(level_print, tmp_print);
		memset(tmp_print, 0, sizeof(tmp_print));
		
		if (frame_buf[1] == -1) sprintf(tmp_print, " \t");
		else sprintf(tmp_print, "%d\t", frame_buf[1]);
		strcat(level_print, tmp_print);
		memset(tmp_print, 0, sizeof(tmp_print));
		
		if (frame_buf[2] == -1) sprintf(tmp_print, " \t");
		else sprintf(tmp_print, "%d\t", frame_buf[2]);
		strcat(level_print, tmp_print);
		memset(tmp_print, 0, sizeof(tmp_print));
		sprintf(tmp_print, "%c", fault_check[i]);
		
		strcat(level_print, tmp_print);
		printf("%s\n", level_print);
	}

	printf("Number of page faults : %d times\n", pagefault_cnt);
}

/* ------ FIFO altorithm ------*/
void FIFO_algorithm(int frame_cnt, int element_cnt, int *refer_buf) {
	int *frame_buf = malloc(sizeof(int) * frame_cnt); //frame buf 동적할당
	char *fault_check = malloc(sizeof(char) * element_cnt); //page fault의 발생여부를 저장하는 배열
	int pagefault_cnt = 0; //page fault counter
	int currIdx = 0; //page reference string을 삽입하는 인덱스

	for (int i = 0; i < frame_cnt; i++) {
		frame_buf[i] = 0; //frame_buf를 0으로 초기화
		frame_check[i] = 0; //비어있음을 의미하도록 frame_check의 모든 원소를 0으로 초기화
	}
	memset(fault_check, ' ', sizeof(fault_check)); //fault_check 배열 초기화

	for (int i = 0; i < element_cnt; i++) { //pagr reference string만큼 반복
		if(CheckExist(frame_buf, frame_cnt, refer_buf[i]) == -1) { //비어있는 frame을 발견한 경우
			frame_buf[currIdx] = refer_buf[i]; //currIdx의 위치에 해당 reference string을 넣음
			currIdx = (currIdx + 1) % frame_cnt; //currIdx의 값 증가
			pagefault_cnt++;
			fault_check[i] = 'F';
		}
		printf("%d\t\t%d\t%d\t%d\t%c\n", i+1, frame_buf[0], frame_buf[1], frame_buf[2], fault_check[i]);
	}

	printf("Number of page faults : %d times\n", pagefault_cnt);
}

/* ------ LRU altorithm ------*/
void LRU_algorithm(int frame_cnt, int element_cnt, int *refer_buf) {

}

/* ------ Second Chance altorithm ------*/
void SecondChance_algorithm(int frame_cnt, int element_cnt, int *refer_buf) {

}

int CheckExist (int *frame_buf, int frame_cnt, int refer_string) {
	for (int i = 0; i < frame_cnt; i++) {
		if (frame_buf[i] == refer_string) {
			return i;
		}
	}
		
		return -1;
}
