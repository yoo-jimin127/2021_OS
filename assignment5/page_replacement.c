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
	int check[31] = { 0, };
	int pagefault_cnt = 0; // page fault counter
	printf("element_cnt : %d\n", element_cnt);
	
	bool is_refer = false;
	for (int i = 0; i < element_cnt; i++) {
		
		// 해당 frame에 page의 존재 여부 확인
		for (int j = 0; j < frame_cnt; j++) {
			if (refer_buf[i] == check[j]) {
				is_refer = true; 
				break;
			}
		}

		if (is_refer) continue; //해당 프레임에 reference string 요소가 있으면 continue

		for (int j = 0; j < frame_cnt; j++) {
			if(!check[j]) {
				check[j] = refer_buf[i];
				is_refer = true;
				break;
			}
		}

		if (is_refer) continue; //해당 프레임에 reference string 요소가 있으면 continue

		int idx, deviceIdx = -1;

		for (int j = 0; j < frame_cnt; j++) {
			int lastUsedIdx = 0;
			int next = i + 1;
			//앞으로 사용되지 않는 refer string의 경우 lastUsedIdx가 가장 크기에 제일 먼저 빠짐.
			//앞으로 사용되는 refer string 중 가장 나중에 쓰이는 기기의 lastUsedIdx 찾음.
			while (next < element_cnt && check[j] != refer_buf[next]) {
				lastUsedIdx++;
				next++;
			}

			if (lastUsedIdx > deviceIdx) {
				idx = j;
				deviceIdx = lastUsedIdx;
			}
		}
		
	}

}

/* ------ FIFO altorithm ------*/
void FIFO_algorithm(int frame_cnt, int element_cnt, int *refer_buf) {

}

/* ------ LRU altorithm ------*/
void LRU_algorithm(int frame_cnt, int element_cnt, int *refer_buf) {

}

/* ------ Second Chance altorithm ------*/
void SecondChance_algorithm(int frame_cnt, int element_cnt, int *refer_buf) {

}
