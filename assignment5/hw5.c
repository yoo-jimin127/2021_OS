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
int CalcFrameLongest(int idx, int element_cnt, int frame_element, int *refer_buf);

int main (void) {
	/* ------ ���� ���� ���� ------ */
	FILE *fp = NULL; //����������
	char inputbuf[61]; // input.txt ���� ���� ����

	/* ------ page frame ���� ���� ------ */
	char frame_cnt_str[5];
	int frame_cnt = 0; // page frame ���� ����
	char refer_strbuf[65]; //page frame ������ page reference string �����
	int refer_buf[31] = { 0, }; // page reference string�� ���������� ������ �迭
	int element_cnt = 0; //������ �迭�� �ε��� ī���� ����
	
	/* ------ ���� ��� ���� ���� ------ */
	char option[50];

	if ((fp = fopen("input.txt", "r")) == NULL) { //file open
		fprintf(stderr, "input.txt fopen() error\n");
		exit(1);
	}

	fread(inputbuf, sizeof(inputbuf), 1, fp); // input.txt �д� �۾�
	fclose(fp); //file close

	/* ------ used method &  page reference string ��� ------ */
	printf("Used method : ");
	scanf("%s", option);

	char *ptr = strtok(inputbuf, "\n"); //frame_cnt & page reference string ��ūȭ
	strcpy(frame_cnt_str, ptr); // ��ū���� ����� frame number info ����
	frame_cnt = atoi(frame_cnt_str); //frame_cnt string -> int
	
	ptr = strtok(NULL, "\n");
	strncpy(refer_strbuf, ptr, strlen(ptr)); // frame_cnt ������ page reference string ����
	int temp = strlen(refer_strbuf);
	char *intptr = strtok(refer_strbuf, " "); //�����ڸ� �������� ��ūȭ(���ڿ� �迭 -> ������ �迭)
	
	while(intptr != NULL) {
		refer_buf[element_cnt] = atoi(intptr);
		intptr = strtok(NULL, " ");
		element_cnt++;
	} 

	printf("page reference string : %s\n", ptr);
	printf("\n");
	printf("\tframe   1\t2\t3\tpage fault\n");
	printf("time\n");
	
	/* ------ �ɼǿ� ���� �˰��� ��� ���� ------ */
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
	int *frame_buf = malloc(sizeof(int) * frame_cnt); // frame_buf �����Ҵ�
	char *fault_check = malloc(sizeof(char) * element_cnt); //pagefault�� �߻����θ� �����ϴ� �迭
	char *level_print = malloc(sizeof(char) * 300); //�� �ܰ迡���� page replacement ���¸� ����ϴ� �޽���
	char *tmp_print = malloc(sizeof(char) * 10); //���ڿ� ������ ���� �ӽ� ����

	int pagefault_cnt = 0; //page fault counter
	int currIdx = 0; //page reference string�� �����ϴ� �ε���
	int selectedIdx = 0; //OPT �˰��� ���� ���õ� �ε���
	int longest_frameNum = 0; //������ ���� �������� �������� ���� frame�� index number
	int frame_distance = 0; //�� frame�� �������� �ʴ� �ð��� �����ϴ� ����
	
	for (int i = 0; i < frame_cnt; i++) {
		frame_buf[i] = -1; //frame_buf�� -1���� �ʱ�ȭ
	}
	
	memset(fault_check, ' ', sizeof(fault_check)); //fault_check �迭 �ʱ�ȭ

	for (int i = 0; i < frame_cnt; i++) {
		memset(level_print, 0, sizeof(level_print));
		longest_frameNum = 0; //���� �������� �������� ���� frame�� index number �ʱ�ȭ
		
		if (frame_cnt > currIdx) { //currIdx�� ���� frame_cnt���� ���� ���(����ִ� frame ����)
			if(CheckExist(frame_buf, frame_cnt, refer_buf[i]) == -1) { //����ִ� frame�� �߰��� ���		
				frame_buf[currIdx] = refer_buf[i]; //refer_buf[i]�� �޸𸮸� currIdx ���� ����
				pagefault_cnt++; //pagefault_cnt ����
				fault_check[i] = 'F'; //�ش� i��° element������ page fault �߻����� üũ
				currIdx++;
			}

			else { //page fault�� �߻����� �ʴ� ���
				continue;
			}
		}

		else { //OPTIMAL Page replacement �߻�
			if(CheckExist(frame_buf, frame_cnt, refer_buf[i]) != -1) { //�� frame�� �������� �ʴ� ���
				continue;
			}

			for (int j = 0; j < frame_cnt; j++) { //page fault�� �߻��ϴ� ���
				frame_distance = CalcFrameLongest(i, element_cnt, frame_buf[j], refer_buf);
				if (longest_frameNum < frame_distance) { //���� longest_frameNum�� ���� frame_distance ������ ū ���
					longest_frameNum = frame_distance; //longest_frameNum �� ����
					selectedIdx = j; //OPT �˰��� ���� ���ŵ� index num ����
				}
			}

			frame_buf[selectedIdx] = refer_buf[i]; //���õ� �ε����� �����ӿ� �޸� ����
			pagefault_cnt++; //pagefault�� ī���� ����
			fault_check[i] = 'F'; //�ش� i��° element������ page fault �߻����� üũ
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
	int *frame_buf = malloc(sizeof(int) * frame_cnt); //frame buf �����Ҵ�
	char *fault_check = malloc(sizeof(char) * element_cnt); //page fault�� �߻����θ� �����ϴ� �迭
	char *level_print = malloc(sizeof(char) * 300); //�� �ܰ迡���� page replacement ���¸� ����ϴ� �޽���
	char *tmp_print = malloc(sizeof(char) * 10); //���ڿ� ������ ���� �ӽ� ����

	int pagefault_cnt = 0; //page fault counter
	int currIdx = 0; //page reference string�� �����ϴ� �ε���

	for (int i = 0; i < frame_cnt; i++) {
		frame_buf[i] = -1; //frame_buf�� -1���� �ʱ�ȭ
	}
	memset(fault_check, ' ', sizeof(fault_check)); //fault_check �迭 �ʱ�ȭ

	for (int i = 0; i < element_cnt; i++) { //page reference string��ŭ �ݺ�
		memset(level_print, 0, sizeof(level_print));

		if(CheckExist(frame_buf, frame_cnt, refer_buf[i]) == -1) { //����ִ� frame�� �߰��� ���
			frame_buf[currIdx] = refer_buf[i]; //currIdx�� ��ġ�� �ش� reference string�� ����
			currIdx = (currIdx + 1) % frame_cnt; //currIdx�� �� ����
			pagefault_cnt++; //pagefault�� ī���� ����
			fault_check[i] = 'F'; //�ش� i��° element������ page fault �߻����� üũ
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
	int *frame_buf = malloc(sizeof(int) * frame_cnt); //frame buf �����Ҵ�
	char *fault_check = malloc(sizeof(char) * element_cnt); //page fault�� �߻� ���θ� �����ϴ� �迭
	char *level_print = malloc(sizeof(char) * 300); //�� �ܰ迡�� page replacement �� ���¸� ����ϴ� �޽���
	char *tmp_print = malloc(sizeof(char) * 10); //���ڿ� ������ ���� �ӽ� ����

	int pagefault_cnt = 0; //page fault counter
	int currIdx = 0; //current index number
	int totalCurr = 0; //���� ���� �������� ���� element�� ����ϱ� ���� idx �� ���� �뵵�� ���Ǵ� ����
	
	for (int i = 0; i < frame_cnt; i++) {
		frame_buf[i] = -1; //frame_buf�� -1���� �ʱ�ȭ
	}

	memset(fault_check, ' ', sizeof(fault_check)); //fault_check �迭 �ʱ�ȭ

	for (int i = 0; i < element_cnt; i++) { //page reference string��ŭ �ݺ�
		memset(level_print, 0, sizeof(level_print)); //�ܰ躰�� ��µǴ� �޽������� �ʱ�ȭ

		if (CheckExist(frame_buf, frame_cnt, refer_buf[i]) != -1) { //refer_buf[i]�� �޸𸮰� page frame�� �����ϴ� ���
			int tmp_mem = frame_buf[currIdx]; //������ page�� ���� �ӽ� ����
			for (int j = currIdx; j < totalCurr - 1; j++) {
				frame_buf[j] = frame_buf[j+1]; //������ �������� �� ���� ������ �������� ���õ��� �ʵ��� �ڷ� �ű�� �۾�
			}
			frame_buf[totalCurr - 1] = tmp_mem; 
		}

		else { //page fault�� �߻��ϸ� refer_buf[i]�� �޸𸮰� ������ �� �ö󰡴� ���
			if (frame_cnt == totalCurr) { //page replacement�� �߻��ϴ� ���(�� frame ���� X)
				for (int j = 0; j < frame_cnt; j++) {
					frame_buf[j - 1] = frame_buf[j]; //�� page frame�� ����� ������ ������ ��ĭ�� �̵���Ű�� �۾� �켱 ����
				}
				frame_buf[frame_cnt - 1] = refer_buf[i]; //���� �ڿ� ��ġ�� frame�� refer_buf[i] �ε����� �޸� ����
				fault_check[i] = 'F'; //�ش� i��° element������ page fault �߻����� üũ
			}

			else { //�� frame�� �����ϴ� ���
				frame_buf[totalCurr] = refer_buf[i]; //���ŵ� totalCurr��° �ε����� �޸� ����
				fault_check[i] = 'F'; //�ش� i��° element������ page fault �߻����� üũ
				totalCurr++; // totalCurr�� �� ������Ʈ
			}

			pagefault_cnt++; //pagefault_cnt ����
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

/* ------ Second Chance altorithm ------ */
void SecondChance_algorithm(int frame_cnt, int element_cnt, int *refer_buf) {
	int *frame_buf = malloc(sizeof(int) * frame_cnt); //frame buf ���� �Ҵ�
	char *fault_check = malloc(sizeof(char) * element_cnt); //page fault�� �߻� ���θ� �����ϴ� �迭
	char *level_print = malloc(sizeof(char) * 300); //�� �ܰ迡���� page replacement ���¸� ����ϴ� �޽���
	char *tmp_print = malloc(sizeof(char) * 10); //���ڿ� ������ ���� �ӽ� ����

	int pagefault_cnt = 0; //page fault counter
	int currIdx = 0; //page reference string�� �����ϴ� �ε���

	for (int i = 0; i < frame_cnt; i++) {
		frame_buf[i] = -1; //frame_buf�� -1���� �ʱ�ȭ
	}

	memset(fault_check, ' ', sizeof(fault_check));
}

/* ------ frame�� empty ���� Ȯ�� �Լ� ------ */
int CheckExist (int *frame_buf, int frame_cnt, int refer_string) {
	for (int i = 0; i < frame_cnt; i++) {
		if (frame_buf[i] == refer_string) {
			return i;
		}
	}
		
		return -1;
}

/* ------ OPT �˰���: ������ ���� ���� �������� ���� framenum ��� �Լ� ------ */
int CalcFrameLongest (int idx, int element_cnt, int frame_element, int *refer_buf) {
	int frame_distance = 0; //�� frame�� element�� ���� �������� �ɸ��� �ð� ���� ����
	for (int i = idx + 1; i < element_cnt; i++) {
		if (frame_element != refer_buf[i]) {
			frame_distance++;
		}

		else {
			break;
		}
	}
		return frame_distance++;
}
