#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <signal.h>
#include <utmp.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <termios.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>


#define MAX 1024
#define ll long long

typedef struct { //ps 명령어에 해당하는 옵션에 대한 구조체
	bool a; //모든 프로세스에 대한 ps 명령결과 출력
	bool e; //커널 프로세스를 제외한 모든 프로세스 출력
	bool f; //풀 포맷으로 출력
	//bool l; //긴 포맷으로 출력
	bool p; //특정한 PID를 지정
	bool r; //현재 실행중인 프로세스를 기준으로 출력
	bool u; //프로세스를 가지고 있는 소유자 기준으로 출력
	bool x; //데몬 프로세스처럼 터미널에 종속되지 않는 프로세스 출력
	bool bar; //옵션을 입력할 때에 - 가 있는지의 여부를 확인
	bool no; //옵션의 유무를 확인
} opt;

typedef struct {
	char USER[MAX]; // username을 저장하기 위한 배열
	uid_t UID; // user ID
	pid_t PID; // 프로세스 아이디
	pid_t PPID; // PPID
	char PRI[MAX]; //프라이머리 키 값
	char NI[MAX]; 
	double CPU; //CPU 값 저장
	double MEM; //메모리 값 저장
	ll VSZ; 
	ll RSS; 
	char TTY[MAX]; //터미널 & 콘솔 저장
	char STAT[MAX]; //stat 저장 변수
	unsigned long START; // 시작
	unsigned long TIME; //시간 저장 변수
	char COMMAND[MAX];
	char CMD[MAX]; // CMD의 값을 저장하기 위한 배열
	int ttyNr; //터미널 tty
} proc;

pid_t mypid; //본인의 pid
uid_t myuid; //본인의 uid
char mytty[MAX]; //본인의 터미널(콘솔) 정보
int tasks;
double memtotal; //전체 메모리
opt option; //option들
proc procs[MAX]; //process 정보들
int special_pid = 0;
pid_t pids[MAX];

ll get_value(const char* str) { //string 값을 매개변수로 받아 정수값을 리턴하는 함수
	ll ret = 0; //리턴값을 저장한 변수 ret
	for(int i = 0; i < MAX; i++) { //MAX번 만큼의 반복
		if(isdigit(str[i])) { // 인자로 받은 str의 i번째 인덱스 값이 문자이면
			ret = ret *10 + ((ll) str[i] - '0'); //ret값에 string 값을 정수로 변환한 값을 넣음
		}
	}
	return ret; //ret 변수의 저장 값 리턴
}

double round(double a) { //소수 첫째자리에서 반올림하는 함수
	int tmp = a *10; 
	double ret = (double)tmp / 10; //소수 첫째 자리를 반올림해 리턴

	return ret;
}

ll get_uptime() { //Uptime을 초단위로 바꾸는 함수
	char buffer[MAX]; //버퍼 생성
	int idx = 0;
	int fd; //file descriptor
	ll ret;
	
	memset(buffer, 0, sizeof(buffer)); //버퍼 메모리의 값 초기화

	if((fd = open("/proc/uptime", O_RDONLY)) < 0) { // proc/uptime 파일을 읽기 전용 모드로 open하여 해당 파일 디스크립터는 fd 변수에 저장
		fprintf(stderr, "/proc/uptime file open error\n"); // 파일 오픈 과정에서 오류 발생 시 예외처리
		exit(1);
	}

	read(fd, buffer, MAX); ///proc/uptime의 값을 읽음

	while(buffer[idx] != ' ') idx++; //첫 번째 토큰을 읽음
	memset(buffer + idx, 0, sizeof(char) * (MAX - idx)); //뒷부분은 널로 채움
	ret = atoll(buffer); //버퍼의 값을 long long 형으로 바꾸어 ret 변수에 저장
	close(fd); // 파일 close

	return ret; //ret 값 리턴
}

void get_mem_info() { ///proc/meminfo로부터 memtotal을 얻음
	int fd; //파일 디스크립터
	char mem_tmp[MAX]; //파일로부터 읽어올 버퍼
	char mem[MAX][MAX]; //파싱 정보 저장
	int i = 0;

	if((fd = open("/proc/meminfo", O_RDONLY)) < 0) { //proc/meminfo 파일 열기
		fprintf(stderr, "/proc/meminfo file open error\n");
		exit(1);
	}

	memset(mem_tmp, 0, MAX); //메모리 임시 공간인 mem_tmp의 값 초기화
	if(read(fd, mem_tmp, MAX) == 0) { //proc/meminfo 파일을 읽는 작업
		fprintf(stderr, "/proc/meminfo file read error\n");
		exit(1);
	}

	close(fd); //파일 close

	memset(mem, 0, sizeof(mem)); //mem 크기만큼 초기화
	char *ptr = strtok(mem_tmp, "\n"); //개행단위로 읽은 정보 파싱(토큰화)
	while(ptr != NULL) { //ptr의 값이 NULL이기 전까지 반복
		strcpy(mem[i++], ptr); //메모리의 i++ 인덱스의 ptr의 토큰 값을 복사해 저장
		ptr = strtok(NULL, "\n"); //개행으로 정보 파싱
	}

	int mtotal = get_value(mem[0]); //파싱한 정보를 정수형으로 바꿔주는 작업
	memtotal = (double)mtotal / 1024; // memtotal의 값에 mtotal을 MiB단위로 바꾸어 저장
}

ll get_VmLck(int pid) { //VmLck를 얻는 함수
	char path[MAX]; //경로를 저장할 문자열 배열
	char tmp[MAX]; //읽은 정보 저장할 버퍼
	char status[MAX][MAX]; //파싱결과 저장
	int fd;
	int i = 0;
	ll ret = 0;

	memset(path, 0, MAX); //메모리 초기화 작업
	memset(tmp, 0, MAX);
	memset(status, 0, sizeof(status));

	sprintf(path, "/proc/%d/status", pid); //경로 지정


	if((fd = open(path, O_RDONLY)) < 0) { //파일을 읽기 전용모드로 open
		fprintf(stderr, "%s file open errorn\n", path);
		exit(1); //파일 open 과정에서의 예외처리
	}

	if(read(fd, tmp, MAX) == 0) { //파일을 MAX만큼 읽어옴
		fprintf(stderr, "%s file read error\n", path);
		exit(1); //파일 read 과정에서의 예외처리
	}

	char *ptr = strtok(tmp, "\n");
	while(ptr != NULL) { //개행을 기준으로 tmp에 저장된 값을 토큰화시키는 작업
		strcpy(status[i++], ptr);
		ptr = strtok(NULL, "\n");
	}

	for(int j = 0; j < i; j++) { //반복문을 통해 VmLck가 존재하는 경우를 찾는 작업 
		if(!strncmp(status[j], "VmLck", 5)) { //VmLck를 찾은 경우
			ret = get_value(status[j]); //해당 값을 ret에 넣어줌
			break;
		}
	}
	
	close(fd);
	return ret;
}

ll get_VSZ(int pid) { //VSZ를 얻기 위한 작업을 진행하는 함수
	char path[MAX]; //경로를 저장하기 위한 문자열 배열
	char tmp[MAX]; //읽은 정보 저장할 버퍼
	char status[MAX][MAX]; //파싱결과 저장
	int fd;
	int i = 0;
	ll ret = 0;
	
	memset(path, 0, MAX); //메모리 초기화
	memset(tmp, 0, MAX);
	memset(status, 0, sizeof(status));

	sprintf(path, "/proc/%d/status", pid); //경로 지정


	if((fd = open(path, O_RDONLY)) < 0) {
		fprintf(stderr, "%s file open errorn\n", path);
		exit(1);
	}

	if(read(fd, tmp, MAX) == 0) {
		fprintf(stderr, "%s file read error\n", path);
		exit(1);
	}

	char *ptr = strtok(tmp, "\n"); //개행을 기준으로 토큰화
	while(ptr != NULL) {
		strcpy(status[i++], ptr);
		ptr = strtok(NULL, "\n");
	}

	for(int j = 0; j < i; j++) { //VmSize가 있는 경우 이를 저장
		if(!strncmp(status[j], "VmSize", 6)) { //VmSize를 찾은 경우
			ret = get_value(status[j]);
			break;
		}
	}

	close(fd);
	return ret;
}

void get_environ(int index) { //ps 명령어와 유사하게 작동할 환경을 세팅하기 위한 함수
	
	if(procs[index].UID != myuid) {
		return; //procs의 UID와 본인의 uid가 다를 경우 이에 접근할 권한이 없으므로 return하며 종료
	}

	char path[MAX];
	char buffer[MAX];
	int fd;
	
	memset(path, 0, MAX);
	memset(buffer, 0, MAX);
	
	sprintf(path, "/proc/%d/environ", procs[index].PID);
	
	if((fd = open(path, O_RDONLY)) < 0) {
		fprintf(stderr, "%s open error\n", path);
		exit(1);
	}

	if(read(fd, buffer, MAX) == 0) {
		fprintf(stderr, "%s file read error\n", path);
		exit(1);
	}

	for(int i = 0; i < MAX; i++) { //버퍼 사이에 존재하는 null 값을 공백으로 바꿔주는 작업
		if(buffer[i] == '\0') {
			if(buffer[i+1] == '\0') {
				break;
			}
			else {
				buffer[i] = ' ';
			}
		}
	}

	if(strlen(buffer)) { //환경변수가 존재하는 경우를 찾아 이를 COMMAND 뒤에 이어 붙이는 작업
		strcat(procs[index].COMMAND, " ");
		strncat(procs[index].COMMAND, buffer, MAX/2); //procs의 index번째의 COMMAND 값에 buffer의 내용을 붙임
	}
}

void get_tty(int index) { // TTY 획득을 위한 명령어
	char fdpath[MAX];			//0번 fd에 대한 절대 경로

	memset(fdpath, '\0', MAX);
	sprintf(fdpath, "/proc/%d/fd/0", procs[index].PID);

	if(access(fdpath, F_OK) < 0){	//fd 0이 없을 경우
		DIR *dp;
		char nowPath[MAX];
		struct dirent *dentry;
		struct stat statbuf;
		
		if((dp = opendir("/dev")) == NULL){		// 터미널 찾기 위해 /dev 디렉터리 open
			fprintf(stderr, "/dev directory open error\n");
			exit(1);
		}

		while((dentry = readdir(dp)) != NULL){	// /dev 디렉터리 탐색
			memset(nowPath, 0, MAX);	// 현재 탐색 중인 파일 절대 경로
			sprintf(nowPath, "/dev/%s", dentry->d_name);

			if(stat(nowPath, &statbuf) < 0){	// stat 획득
				fprintf(stderr, "stat error for %s\n", nowPath);
				exit(1);
			}
			if(!S_ISCHR(statbuf.st_mode))		//문자 디바이스 파일이 아닌 경우 skip
				continue;
			else if(statbuf.st_rdev == procs[index].ttyNr){	//문자 디바이스 파일의 디바이스 ID가 ttyNr과 같은 경우
				strcpy(procs[index].TTY, dentry->d_name);	//tty에 현재 파일명 복사
				break;
			}
		}
		closedir(dp);

		if(!strlen(procs[index].TTY))					// /dev에서도 찾지 못한 경우
			strcpy(procs[index].TTY, "?");				//nonTerminal
	}

	else{
		char symLinkName[MAX];
		memset(symLinkName, 0, MAX);
		
		if(readlink(fdpath, symLinkName, MAX) < 0){
			fprintf(stderr, "readlink error for %s\n", fdpath);
			exit(1);
		}

		if(!strcmp(symLinkName, "/dev/null"))		//symbolic link로 가리키는 파일이 /dev/null일 경우
			strcpy(procs[index].TTY, "?");					//nonTerminal
		else
			sscanf(symLinkName, "/dev/%s", procs[index].TTY);	//그 외의 경우 tty 획득

	}

	if(procs[index].PID == mypid)
		strcpy(mytty, procs[index].TTY);

	return;

}


void get_proc_stat(const char* path, int index) { //proc의 stat 값을 가져오기 위한 작업을 진행하는 함수
	int fd;
	char stat_tmp[MAX];
	struct stat statbuf; //USER 정보를 읽는 작업
	stat(path, &statbuf);

	struct passwd *upasswd = getpwuid(statbuf.st_uid);
	strcpy(procs[index].USER, upasswd->pw_name);
	if(procs[index].USER[8] != '\0') {
		procs[index].USER[7] = '+';
		procs[index].USER[8] = '\0';
	}
	char stat[MAX][MAX];

	//UID읽기
	procs[index].UID = upasswd->pw_uid; //UID를 읽는 작업

	memset(stat_tmp, 0, MAX);
	memset(stat, 0, sizeof(stat));

	if((fd = open(path, O_RDONLY)) < 0) {
		fprintf(stderr, "/proc/pid/stat file open error\n");
		exit(1);
	}

	if(read(fd, stat_tmp, MAX) == 0) {
		fprintf(stderr, "/proc/pid/stat file read error\n");
		exit(1);
	}

	close(fd);

	char *ptr = strtok(stat_tmp, " "); //공백 기준으로 token 자르기
	int i = 0;
	while(ptr != NULL) {
		strcpy(stat[i++], ptr);
		ptr = strtok(NULL, " ");
	}

	procs[index].CPU = atoll(stat[3]); //PPID를 구하는 작업

	ll utime = atoll(stat[13]); //cpu를 구하는 작업
	ll stime = atoll(stat[14]);
	ll uptime = get_uptime();
	int hertz = (int)sysconf(_SC_CLK_TCK);

	double tic = (double)(utime + stime) /hertz;
	procs[index].CPU = (double)tic / uptime * 100;
	procs[index].CPU = round(procs[index].CPU);

	procs[index].TIME = (utime + stime) / hertz; //TIME을 구하는 작업

	i = 0; //CMD를 구하는 작업
	while(stat[1][i+1] != ')') {
		procs[index].CMD[i] = stat[1][i+1];
		i++;
	}

	char stat_str[MAX]; //STAT을 구하는 작업
	memset(stat_str, 0, MAX);
	strcpy(stat_str, stat[2]);

	if(procs[index].PID == atoi(stat[5]))
		strcat(stat_str, "s");

	if(atoi(stat[18]) > 0)
		strcat(stat_str, "N");
	else if(atoi(stat[18]) < 0)
		strcat(stat_str, "<");

	if(get_VmLck(procs[index].PID) != 0)
		strcat(stat_str, "L");

	if(atoi(stat[19]) > 1)
		strcat(stat_str, "l");

	if(getpgid(procs[index].PID) == atoi(stat[7]))
		strcat(stat_str, "+");

	strcpy(procs[index].STAT, stat_str);
	strcpy(procs[index].NI, stat[18]); //NI 저장
	strncpy(procs[index].PRI, stat[17], 3); //PRI 저장

	procs[index].START = time(NULL) - uptime + ((unsigned long)atoi(stat[21]) / hertz); //START를 구하는 작업
	procs[index].ttyNr = atoi(stat[6]); //ttyNr을 구하는 작업
}

void get_proc_status(const char* path, int index) { //proc/pid/status 파일 읽기
	int fd;
	char status_tmp[MAX];
	char status[MAX][MAX];
	memset(status_tmp, 0, MAX);
	memset(status, 0, sizeof(status));

	if((fd = open(path, O_RDONLY)) < 0) { //파일 오픈 작업
		fprintf(stderr, "%s file open error\n", path);
		exit(1);
	}

	if(read(fd, status_tmp, MAX) == 0) { //파일을 읽는 작업
		fprintf(stderr, "%s file read error\n", path);
		exit(1);
	}


	char *ptr = strtok(status_tmp, "\n");
	int i = 0;
	while(ptr != NULL) { //status_tmp의 내용을 개행문자 기준으로 토큰화하는 작업
		strcpy(status[i++], ptr);
		ptr = strtok(NULL, "\n");
	}

	ll res = get_value(status[20]);
	procs[index].MEM = (double)res / (memtotal * 1024) * 100;
	procs[index].VSZ = get_VSZ(procs[index].PID);
	procs[index].RSS = get_value(status[21]);
	
	close(fd);
}

void get_proc_cmdline(const char* path, int index) {//proc/pid/cmdline 파일 읽기
	int fd;

	if((fd = open(path, O_RDONLY)) < 0) {
		fprintf(stderr, "/proc/pid/cmdline file open error\n");
		exit(1);
	}

	if(read(fd, procs[index].COMMAND, MAX) < 0) {
		fprintf(stderr, "/proc/pid/cmdline file read error\n");
		exit(1);
	}

	for(int i = 0; i < MAX; i++) { //두번 연속 널이 나올 경우 종료 그렇지 않으면 공백으로 처리
		if(procs[index].COMMAND[i] == '\0') {
			if(procs[index].COMMAND[i+1] == '\0') break;
			else procs[index].COMMAND[i] = ' ';
		}
	}
	if(!strlen(procs[index].COMMAND)) { //없는 경우
		sprintf(procs[index].COMMAND, "[%s]", procs[index].CMD);
	}
	close(fd);
}


void get_procs() { //pid를 확인하고 process 정보들을 가져오는 함수

	DIR *proc_dir; //proc디렉토리 포인터
	struct dirent *dp; //proc디렉토리 엔트리 포인터
	char stat_path[MAX];
	char status_path[MAX];
	char cmdline_path[MAX];

	if((proc_dir = opendir("/proc")) == NULL) { //proc directory open
		fprintf(stderr, "/proc open error\n");
		exit(1);
	}

	while((dp = readdir(proc_dir)) != NULL) { //하위 파일들을 하나씩 읽는다.
		if(isdigit(dp->d_name[0])) { //하위폴더가 숫자일 경우(process 폴더인 경우)
			procs[tasks++].PID = atol(dp->d_name); //pid저장
		}
	}

	for(int i = 0; i < tasks; i++) {
		memset(stat_path, 0, MAX);
		memset(status_path, 0, MAX);
		memset(cmdline_path, 0, MAX);
		sprintf(stat_path, "/proc/%d/stat", procs[i].PID);
		sprintf(status_path, "/proc/%d/status", procs[i].PID);
		sprintf(cmdline_path, "/proc/%d/cmdline", procs[i].PID);
		
		if(access(stat_path, F_OK) == 0) { //예외처리 파일이 존재할 때 열기
			get_proc_stat(stat_path, i);
		}

		if(access(status_path, F_OK) == 0) {
			get_proc_status(status_path, i);
		}

		if(access(status_path, F_OK) == 0) {
			get_proc_cmdline(cmdline_path, i);
		}

		get_tty(i);
		
		if(option.e) get_environ(i);
	}

	closedir(proc_dir);
}

bool isSpecial(pid_t p) { //pid를 인자로 받아 특수한 경우인지 확인하는 함수
	bool ret = false;
	for(int i = 0; i < special_pid; i++) {
		if(pids[i] ==  p) {
			ret = true;
			break;
		}
	}
	return ret;
}

void print_data() { //데이터들을 불러오는 함수

	struct winsize win; //터미널의 사이즈
	if(ioctl(0, TIOCGWINSZ, (char*)&win) < 0) {
		fprintf(stderr, "ioctl error\n");
		exit(1);
	}

	//ps 명령어 입력 시 첫 줄의 내용 출력
	if(!(option.bar && option.f) && option.u)
		printf("USER    ");

	if((option.bar && option.f))
		printf("UID     ");

	printf("    PID");

	if((option.bar && option.f))
		printf("    PPID");

	if(!option.u &&(option.bar && option.f))
		printf(" STIME");

	if(option.u)
		printf(" %%CPU");

	if(option.u)
		printf(" %%MEM");
	
	if(option.u)
		printf("    VSZ");
	
	if(option.u)
		printf("   RSS");
	
	printf(" TTY    ");
	
	if(option.u || option.r || option.x || option.f || option.a || option.p)
		printf("STAT");
	
	if(option.u)
		printf(" START");
	
	if(option.bar || option.no)
		printf("      TIME");
	
	else 
		printf("   TIME");
	
	if(option.r || option.u || option.x || option.e || option.f)
		printf(" COMMAND\n");
	
	else
		printf(" CMD\n");

	int strlen; //string 길이
	
	for(int index = 0; index < tasks; index++) {
		strlen = 0;

		//옵션에 따라 프로세스를 선택적으로 출력하는 작업
		if(option.r) { //현재 running중인 프로세스만 표시
			if(procs[index].STAT[0] != 'R') continue;
		}
		
		if(option.a) {
			if(option.u) {
				if(!option.x) {
					if(option.p) { //aup일 경우
						if(!isSpecial(procs[index].PID) && !strcmp(procs[index].TTY, "?")) continue; //특정 pid도 아니고 tty가 ?일 경우 건너뛰기
					}
					else { //au일 경우 tty가 ?가 아닌 것만 출력
						if(!strcmp(procs[index].TTY, "?")) continue;
					}
				}
				//ax는 전부 출력
			}

			else {
				if(!option.x) { //a, ap일 경우
					if(!isSpecial(procs[index].PID) && !strcmp(procs[index].TTY, "?")) continue;
				}
			}
		}

		else {
			if(option.u) {
				if(!option.x) {
					if(option.p) { //up일 경우
						if(!isSpecial(procs[index].PID)) continue; //u는 무시됨
					}
					else { //u일 경우
						if(!strcmp(procs[index].TTY, "?")) continue; //tty가 ?일 경우 건너뜀
					}
				}

				else {
					if(option.p) { //uxp일 경우
						//같은 소유자와 지정한 pid만  출력
						if(!isSpecial(procs[index].PID) && (procs[index].UID != myuid)) continue;
					}

					else { //ux일 경우
						if(procs[index].UID != myuid) continue; //다른 소유자일 경우 건너뜀
					}
				}
			}

			else {
				if(!option.x) {
					if(option.p) { //p일 경우
						if(!isSpecial(procs[index].PID)) continue; //지정한 pid만 출력
					}
					else { //no option인 경우
						if(strcmp(procs[index].TTY, mytty) != 0) continue; //같은 tty만 출력
					}
				}

				else {
					if(option.p) { //xp인 경우
						//같은 소유자와 지정한 pid만 출력
						if(!isSpecial(procs[index].PID) && procs[index].UID != myuid) continue;
					}

					else { //x인 경우
						if(procs[index].UID != myuid) continue; //같은 소유자만 출력
					}
				}
			}
		}

		//출력되는 정보를 구별하여 필터링
		if(!(option.bar && option.f) && option.u) {
			printf("%-8s", procs[index].USER);
			strlen += 8;
		}

		if((option.bar && option.f)) {
			printf("%-8s", procs[index].USER);
			strlen += 8;
		}

		printf(" %6d", procs[index].PID);
		strlen += 7;
		
		if((option.bar && option.f)) {
			printf(" %6d", procs[index].PPID);
			strlen += 7;
		}
		
		if(option.u) {
			printf(" %4.1f", procs[index].CPU);
			strlen += 5;
		}

		if(option.u) {
			printf(" %4.1f", procs[index].MEM);
			strlen += 5;
		}

		if(option.u) {
			printf(" %6lld", procs[index].VSZ);
			strlen += 7;
		}

		if(option.u) {
			printf(" %5lld", procs[index].RSS);
			strlen += 8;
		}

		printf(" %-7s", procs[index].TTY);
		strlen += 10;
		
		if(option.a ||option.u || option.r || option.x || option.f || option.p) {
			printf("%-4s", procs[index].STAT);
			strlen += 4;
		}
		
		if(option.u) {
			struct tm *t;
			char starttime[16];
			memset(starttime, 0, 16);
			t = localtime(&procs[index].START);
			if(time(NULL) - procs[index].START < 24 * 3600) {
				strftime(starttime, 16, "%H:%M", t);
			}
			
			else if(time(NULL) - procs[index].START < 7 * 24 * 3600) {
				strftime(starttime, 16, "%b %d", t);
			}
			
			else {
				strftime(starttime, 16, "%y", t);
			}
			
			printf(" %s", starttime);
			strlen += 6;

		}

		struct tm *T = localtime(&procs[index].TIME);
		if(option.bar || option.no) {
			if(procs[index].TIME == 0)
				printf("  00:00:00");
			else
				printf("  %2d:%02d:%02d", T->tm_hour, T->tm_min, T->tm_sec);
			strlen += 10;
		}
		
		else {
			printf("  %2d:%02d", T->tm_min, T->tm_sec);
			strlen += 7;
		}
		
		char cmdstr[MAX];
		memset(cmdstr, 0, MAX);
		if(option.r || option.u || option.x || !option.bar || option.e) {
			strcpy(cmdstr, " ");
			strcat(cmdstr, procs[index].COMMAND);
		}
		
		else {
			strcpy(cmdstr, " ");
			strcat(cmdstr, procs[index].CMD);
		}
		
		for(int i = 0; i < win.ws_col-strlen-1; i++)
			putc(cmdstr[i], stdout);
		putc('\n', stdout);
	}
}

int main(int argc, char** argv) {
	mypid = getpid();
	myuid = getuid();
	option.bar = FALSE;
	option.a = FALSE;
	option.e = FALSE;
	option.f = FALSE;
	option.p = FALSE;
	option.no = FALSE;
	option.r = FALSE;
	option.u = FALSE;
	option.x = FALSE;
	if(argc == 1) option.no = TRUE;
	else {
		option.no = FALSE;
		for(int i = 1; i < argc; i++) { //옵션 설정
			int idx = 0;
			while(argv[i][idx] != '\0') {
				if(argv[i][idx] == '-') option.bar = TRUE;
				if(argv[i][idx] == 'a') {
					option.a = TRUE;
				}
				if(argv[i][idx] == 'e') option.e = TRUE;
				if(argv[i][idx] == 'f') option.f = TRUE;
				if(argv[i][idx] == 'p') {
					option.p = TRUE;
					i++;
					while(i < argc) {
						if(!isdigit(argv[i][0])) break;
						pids[special_pid++] = atoi(argv[i]);
						i++;
					}
					i--;
					break;
				}
				if(argv[i][idx] == 'r') option.r = TRUE;
				if(argv[i][idx] == 'u') option.u = TRUE;
				if(argv[i][idx] == 'x') option.x = TRUE;
				idx++;
			}
		}
	}
	get_mem_info();
	get_procs();
	print_data();
}
