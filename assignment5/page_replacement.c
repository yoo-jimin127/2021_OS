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
	int check[31] = { 0, };
	int pagefault_cnt = 0; // page fault counter
	
	for (int i = 0; i < element_cnt; i++) {
		bool is_refer = false; //reference string이 해당 frame에 존재하는지
	
		// 해당 frame에 page의 존재 여부 확인
		for (int j = 0; j < frame_cnt; j++) {
			if (refer_buf[i] == check[j]) {
				is_refer = true; 
				pagefault_cnt++;
				break;
			}
		}

		if (is_refer) continue; //해당 프레임에 reference string 요소가 있으면 continue

		for (int j = 0; j < frame_cnt; j++) {
			if(!check[j]) {
				check[j] = refer_buf[i];
				is_refer = true;
				pagefault_cnt++;
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

		pagefault_cnt++;
		check[idx] = refer_buf[i];
		printf("%d\t\t%d\t%d\t%d\tF\n", i, check[0], check[1], check[2]);
	}

	printf("Number of page faults : %d times\n", pagefault_cnt);

}

/* ------ FIFO altorithm ------*/
void FIFO_algorithm(int frame_cnt, int element_cnt, int *refer_buf) {
	int *frame_buf = malloc(sizeof(int) * frame_cnt); //frame buf 동적할당
	char *fault_check = malloc(sizeof(char) * element_cnt); //page fault의 발생여부를 저장하는 배열
	char *level_print = malloc(sizeof(char) * 300); //각 단계에서의 page replacement 상태를 출력하는 메시지
	char *tmp_print = malloc(sizeof(char) * 10); //문자열 저장을 위한 임시 버퍼

	int pagefault_cnt = 0; //page fault counter
	int currIdx = 0; //page reference string을 삽입하는 인덱스

	for (int i = 0; i < frame_cnt; i++) {
		frame_buf[i] = -1; //frame_buf를 -1으로 초기화
	}
	memset(fault_check, ' ', sizeof(fault_check)); //fault_check 배열 초기화

	for (int i = 0; i < element_cnt; i++) { //page reference string만큼 반복
		memset(level_print, 0, sizeof(level_print));

		if(CheckExist(frame_buf, frame_cnt, refer_buf[i]) == -1) { //비어있는 frame을 발견한 경우
			frame_buf[currIdx] = refer_buf[i]; //currIdx의 위치에 해당 reference string을 넣음
			currIdx = (currIdx + 1) % frame_cnt; //currIdx의 값 증가
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

/* ------ LRU altorithm ------*/
void LRU_algorithm(int frame_cnt, int element_cnt, int *refer_buf) {
	int *frame_buf = malloc(sizeof(int) * frame_cnt); //frame buf 동적할당
	char *fault_check = malloc(sizeof(char) * element_cnt); //page fault의 발생 여부를 저장하는 배열
	char *level_print = malloc(sizeof(char) * 300); //각 단계에서 page replacement 의 상태를 출력하는 메시지
	char *tmp_print = malloc(sizeof(char) * 10); //문자열 저장을 위한 임시 버퍼

	int pagefault_cnt = 0; //page fault counter
	int currIdx = 0; //current index number
	int totalCurr = 0; //가장 오래 참조되지 않은 element를 사용하기 위해 idx 값 조절 용도로 사용되는 변수
	
	for (int i = 0; i < frame_cnt; i++) {
		frame_buf[i] = -1; //frame_buf를 -1으로 초기화
	}

	memset(fault_check, ' ', sizeof(fault_check)); //fault_check 배열 초기화

	for (int i = 0; i < element_cnt; i++) { //page reference string만큼 반복
		memset(level_print, 0, sizeof(level_print)); //단계별로 출력되는 메시지버퍼 초기화

		if (CheckExist(frame_buf, frame_cnt, refer_buf[i]) != -1) { //refer_buf[i]의 메모리가 page frame상에 존재하는 경우
			int tmp_mem = frame_buf[currIdx]; //참조된 page의 값을 임시 저장
			for (int j = currIdx; j < totalCurr - 1; j++) {
				frame_buf[j] = frame_buf[j+1]; //참조된 페이지는 그 다음 참조될 페이지로 선택되지 않도록 뒤로 옮기는 작업
			}
			frame_buf[totalCurr - 1] = tmp_mem; 
		}

		else { //page fault가 발생하며 refer_buf[i]의 메모리가 페이지 상에 올라가는 경우
			if (frame_cnt == totalCurr) { //page replacement가 발생하는 경우(빈 frame 존재 X)
				for (int j = 0; j < frame_cnt; j++) {
					frame_buf[j - 1] = frame_buf[j]; //각 page frame에 저장된 내용을 앞으로 한칸씩 이동시키는 작업 우선 수행
				}
				frame_buf[frame_cnt - 1] = refer_buf[i]; //가장 뒤에 위치한 frame에 refer_buf[i] 인덱스의 메모리 삽입
				fault_check[i] = 'F'; //해당 i번째 element에서의 page fault 발생여부 체크
			}

			else { //빈 frame이 존재하는 경우
				frame_buf[totalCurr] = refer_buf[i]; //갱신된 totalCurr번째 인덱스에 메모리 삽입
				fault_check[i] = 'F'; //해당 i번째 element에서의 page fault 발생여부 체크
				totalCurr++; // totalCurr의 값 업데이트
			}

			pagefault_cnt++; //pagefault_cnt 증가
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
