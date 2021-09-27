#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <signal.h>
#include <utmp.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <ncurses.h>
#include <curses.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#define MAX 1024

struct tm *t;

//��� ������
int tasks, run, slp, stop, zombie;
double us, sy, ni, id, wa, hi, si, st;
double memtotal, memfree, memused, membuff_cache;
double swaptotal, swapfree, swapused, swapavail_mem;


//�ɼ� ����
int start_row = 0, start_col = 0;
int begin[MAX] = {0, 8, 17, 21, 25, 33, 40, 47, 48, 53, 59, 69};
int option; //���� �ɼ�
int uptime_line; //uptime_line ǥ�� ����
int delay; //refresh �ʴ���
int option_c; //��� ���� ǥ��/ ��ǥ��
char string[MAX];
int option_i; //���� ���μ��� ǥ������ �ʴ� �ɼ�
int print_cnt; //print ���� Ƚ��
int print_max; //print �ִ� ���� ��

int print_row; //result�迭�� ������ �� ��

int special_pid; //Ư�� pid�� ����ϴ� ��� pid ���� 
int pids[MAX]; //��� pid �ִ� �Լ�

int option_b; //������� �ɼ�

int special_user; //Ư�� user�� ����ϴ� ��� user�� ��
char users[MAX]; //��� user �ִ� �Լ�
char blank[MAX];
char blank2[MAX];
int kill_pid; //signal�� ���� pid
//cpu���� ����
int current_cpu[9];
int before_cpu[9];

struct winsize win;


typedef struct {
	unsigned long PID;
	char USER[MAX];
	char PR[3];
	char NI;
	long long VIRT;
	long long RES;
	long long SHR;
	char S;
	double CPU;
	double MEM;
	long long TIME;
	char COMMAND[MAX];
} proc;

const char *proc_path = "/proc";
proc procs[MAX];

char result[MAX][MAX];

void print_1();
void print_2();
void get_data();
void sort_by_cpu();
void sort_by_time();
int get_ch();
void sort_by_mem();

long long get_value(const char* str) { //string���κ��� �������� ��� �Լ�
	long long ret = 0;
	for(int i = 0; i < MAX; i++) {
		if(isdigit(str[i])) { //������ ��� 10�� ���ϰ� ���Ѵ�.
			ret = ret*10 + ((long long)str[i] - '0');
		}
	}

	return ret;
}

void handler(int signo) { //SIGALRM �ڵ鷯 �Լ�
	if(print_max == print_cnt) {
		endwin();
		exit(0);
	}
	print_cnt++;
	memset(result, 0, sizeof(result));
	memset(procs, 0, sizeof(procs));
	print_row = 0;
	erase();
	get_data();
	//printf("%d\n", delay);
	if(option == 'P')
		sort_by_cpu();
	else if(option == 'T')
		sort_by_time();
	else if(option == 'M')
		sort_by_mem();
	if(!option_b) {
		print_1();
		refresh();
	}
	else {
		print_2();
	}
	alarm(delay);
}

int get_delay(const char* s) {
	int ret = 0;
	for(int i =0; i < strlen(s); i++) {
		if(isdigit(s[i])) {
			ret = ret * 10 + (s[i] - '0');
		}
	}
	return ret;
}

void swap(proc *a, proc *b) {
	proc tmp;
	memcpy(&tmp, a, sizeof(proc));
	memcpy(a, b, sizeof(proc));
	memcpy(b, &tmp, sizeof(proc));
}

double round(double a) { //�Ҽ� ù°�ڸ����� �ݿø��ϴ� �Լ�
	int tmp = a * 10;
	double ret = (double)tmp / 10;
	return ret;
}

int get_user() { //USER�� ���� ���ϴ� �Լ�
	struct utmp *user;
	setutent();
	int ret = 0;
	while((user = getutent()) != NULL)
		if(user->ut_type == USER_PROCESS)
			ret++;
	endutent();

	return ret;
}

long long get_uptime() { //UPTIME�� �ʴ����� �ٲٴ� �Լ�
	char buffer[MAX];
	memset(buffer, 0, sizeof(buffer));

	int fd;
	if((fd = open("/proc/uptime", O_RDONLY)) < 0) {
		fprintf(stderr, "/proc/uptime open error\n");
		exit(1);
	}

	read(fd, buffer, MAX);
	long long ret;

	int idx = 0;
	while(buffer[idx] != ' ')
		idx++;

	memset(buffer + idx, 0, sizeof(char) *(MAX - idx));

	ret = atoll(buffer);
	close(fd);

	return ret;
}

void get_loadavg(char* loadavg) { //loadavg�� string���� �����ϴ� �Լ�
	int fd;
	if((fd = open("/proc/loadavg", O_RDONLY)) <0) {
		fprintf(stderr, "/proc/loadavg open error\n");
		exit(1);
	}
	if(read(fd, loadavg, 14) == 0) {
		fprintf(stderr, "/proc/loadavg read error\n");
		exit(1);
	}
	close(fd);
}

void get_cpu_info() { //proc/stat���κ��� cpu������ �޴� ��
	int fd;
	int total = 0;
	int tmp_us, tmp_sy, tmp_ni, tmp_id, tmp_wa, tmp_hi, tmp_si, tmp_st;
	char tmp_stat[MAX]; //stat���� ���� ����
	memset(tmp_stat, 0, sizeof(tmp_stat));
	if((fd = open("/proc/stat", O_RDONLY)) < 0) {
		fprintf(stderr, "/proc/stat file open error\n");
		exit(1);
	}
	read(fd, tmp_stat, MAX);

	memset(current_cpu, 0, sizeof(current_cpu));

	char *ptr = strtok(tmp_stat, " ");
	//CPU���� �б�
	for(int i = 0; i < 8; i++) {
		ptr = strtok(NULL, " ");
		current_cpu[i] = atoi(ptr);
		current_cpu[8] += current_cpu[i];
	}

	//������� ���
	us = (double)(current_cpu[0] - before_cpu[0]) / (double)(current_cpu[8] - before_cpu[8]) *100;
	ni = (double)(current_cpu[1] - before_cpu[1]) / (double)(current_cpu[8] - before_cpu[8]) *100;
	sy = (double)(current_cpu[2] - before_cpu[2]) / (double)(current_cpu[8] - before_cpu[8]) *100;
	id = (double)(current_cpu[3] - before_cpu[3]) / (double)(current_cpu[8] - before_cpu[8]) *100;
	wa = (double)(current_cpu[4] - before_cpu[4]) / (double)(current_cpu[8] - before_cpu[8]) *100;
	hi = (double)(current_cpu[5] - before_cpu[5]) / (double)(current_cpu[8] - before_cpu[8]) *100;
	si = (double)(current_cpu[6] - before_cpu[6]) / (double)(current_cpu[8] - before_cpu[8]) *100;
	st = (double)(current_cpu[7] - before_cpu[7]) / (double)(current_cpu[8] - before_cpu[8]) *100;

	memcpy(before_cpu, current_cpu, sizeof(before_cpu));
}

void get_mem_info() {//proc/meminfo�κ��� ������ ��� �Լ�
	int fd;

	if((fd = open("/proc/meminfo", O_RDONLY)) < 0) { //proc/meminfo ���� ����
		fprintf(stderr, "/proc/meminfo file open error\n");
		exit(1);
	}
	char mem_tmp[MAX]; //���Ϸκ��� �о�� ����
	memset(mem_tmp, 0, MAX);
	if(read(fd, mem_tmp, MAX) == 0) { //proc/meminfo ���� �б�
		fprintf(stderr, "/proc/meminfo file read error\n");
		exit(1);
	}

	close(fd);

	char mem[MAX][MAX];
	memset(mem, 0, sizeof(mem));
	int i = 0;
	char *ptr = strtok(mem_tmp, "\n"); //���� ���� �Ľ�
	while(ptr != NULL) {
		strcpy(mem[i++], ptr);
		ptr = strtok(NULL, "\n"); //�������� ���� �Ľ�
	}

	//�Ľ��� ���� int������ �ٲٱ�
	int mtotal = get_value(mem[0]); 
	int mfree = get_value(mem[1]); 
	int	buffers = get_value(mem[3]);
	int SReclaimable = get_value(mem[23]);
	int cache = get_value(mem[4]);
	int mused = mtotal - mfree - buffers - cache - SReclaimable;
	int stotal = get_value(mem[14]);
	int sfree = get_value(mem[15]);
	int mavail = get_value(mem[2]);

	//header 4�� ���� ����
	memtotal = (double)mtotal / 1024;
	memfree = (double)mfree / 1024;
	memused = (double)mused / 1024;
	membuff_cache = (double)(buffers + cache + SReclaimable) / 1024;
	swaptotal = (double)stotal / 1024;
	swapfree = (double)sfree / 1024;
	swapused = (double)(stotal - sfree) / 1024;
	swapavail_mem = (double)mavail / 1024;

}

void get_proc_stat(char* proc_stat_path, int index) { //proc/pid/stat���� ������ ��� ��
	//fprintf(stderr, "%s\n", proc_stat_path);
	int fd;
	char stat_tmp[MAX];
	memset(stat_tmp, 0, MAX);
	if((fd = open(proc_stat_path, O_RDONLY)) < 0) { //�ش� process�� stat �о����
		fprintf(stderr, "process stat file open error\n");
		exit(1);
	}
	if(read(fd, stat_tmp, MAX) == 0) { //stat���� �б�
		fprintf(stderr, "stat file read error\n");
		exit(1);
	}
	close(fd); //stat���� �ݱ�

	char *ptr = strtok(stat_tmp, " "); //������ �������� stat ���� �ڸ���
	int i = 0;

	char stats[MAX][MAX];
	memset(stats, 0, sizeof(stats));
	while(ptr != NULL) { //������ �������� stat ���� �ڸ���
		strcpy(stats[i++],ptr);
		ptr = strtok(NULL, " ");
	}
	procs[index].S = stats[2][0]; //S����
	switch(stats[2][0]) { //cpu���� ���
		case 'R':
			run++;
			break;
		case 'S':
		case 'I':
			slp++;
			break;
		case 'T':
		case 't':
			stop++;
			break;
		case 'Z':
			zombie++;
			break;
	}

	struct stat statbuf; //process stat����ü
	stat(proc_stat_path, &statbuf); //�ش� stat �б�
	struct passwd *upasswd = getpwuid(statbuf.st_uid); //uid�о�����

	//username �б�
	for(int i = 0; i < MAX; i++) {
		if(upasswd->pw_name[i] != '\0') {
			procs[index].USER[i] = upasswd->pw_name[i];
		}
		else {
			break;
		}
	}
	strncpy(procs[index].PR, stats[17], 3); //PR����
	procs[index].NI = atoi(stats[18]); //NI����

	long long utime = atoll(stats[13]);
	long long stime = atoll(stats[14]);
	long long uptime = get_uptime();
	int hertz = (int)sysconf(_SC_CLK_TCK);

	double tic = (double)(utime + stime) / hertz;
	procs[index].CPU = (double)tic / uptime * 100;//%CPU���ϱ�
	procs[index].CPU = round(procs[index].CPU); //�Ҽ��� ù°�ڸ����� �ݿø�
	procs[index].TIME = (double)(utime + stime) / ((double)hertz / 100); //TIME+���ϱ�

	//COMMAND����
	if(option_c) {
		i = 1;
		procs[index].COMMAND[0] = '[';
		while(stats[1][i] != ')') {
			procs[index].COMMAND[i] = stats[1][i];
			i++;
		}
		procs[index].COMMAND[i] = ']';
	}
	else {
		i = 0;
		while(stats[1][i+1] != ')') {
			procs[index].COMMAND[i] = stats[1][i+1];
			i++;
		}
	}
}

void get_proc_status(const char* proc_status_path, int index) { //proc/pid/status���� ������ ��� ��
	int fd;
	char status_tmp[MAX];
	if((fd = open(proc_status_path, O_RDONLY)) < 0) {//proc/pid/status ���� ����
		fprintf(stderr, "/proc/pid/status file open error\n");
		exit(1);
	}
	if(read(fd, status_tmp, MAX) == 0) { //��� �� �б�
		fprintf(stderr, "/proc/pid/status file read error\n");
		exit(1);
	}
	close(fd);

	char *ptr = strtok(status_tmp, "\n"); //���� ������ ��ū�� �ڸ���
	int i = 0;
	char status[MAX][MAX];
	memset(status, 0, sizeof(status));
	while(ptr != NULL) { //2���� �迭 status�� �ึ�� ����
		strcpy(status[i++], ptr);
		ptr = strtok(NULL, "\n");
	}
	procs[index].VIRT = get_value(status[17]); //VIRT�� str -> integer�� ��ȯ
	procs[index].RES = get_value(status[21]); //RES�� str -> integer�� ��ȯ
	procs[index].SHR = get_value(status[23]) + get_value(status[24]); //SHR�� str -> integer�� ��ȯ
	procs[index].MEM = (double)procs[index].RES / (memtotal * 1024) * 100;
}

void get_cmdline(const char* path, int index) { //proc/pid/cmdline���� ������ ���
	//fprintf(stderr, "%s\n", path);
	int fd;
	if((fd = open(path, O_RDONLY)) < 0) { //proc/pid/cmdline ���� ����
		fprintf(stderr, "/proc/pid/cmdline file open error\n");
		exit(1);
	}
	if(read(fd, procs[index].COMMAND, MAX) < 0) { //proc/pid/cmdline ���� �б�
		fprintf(stderr, "/proc/pid/cmdline file read error\n");
		exit(1);
	}
	close(fd);
}

void get_procs() { //pid�� Ȯ���ϰ� process �������� �������� �Լ�
	DIR *proc_dir; ///proc���丮 ������
	struct dirent *dp; //proc���丮 ��Ʈ�� ������
	if((proc_dir = opendir(proc_path)) == NULL) { //dir open
		fprintf(stderr, "/proc open error\n");
		exit(1);
	}

	while((dp = readdir(proc_dir)) != NULL) { //���� ���ϵ��� �ϳ��� �д´�.
		if(isdigit(dp->d_name[0])) { //���� ������ ���ڷ� �����ϴ� ���(process������ ���)
			procs[tasks].PID = atoi(dp->d_name);
			//printf("%s\n", dp->d_name);
			tasks++;
		}
	}

	char proc_stat_path[MAX]; //prod/pid/sta t���
	char proc_status_path[MAX]; //proc/pid/status ���
	char proc_cmdline_path[MAX]; //proc/pid/cmdline ���
	for(int i = 0; i < tasks; i++) {
		memset(proc_stat_path, 0, MAX);
		memset(proc_status_path, 0, MAX);
		memset(proc_cmdline_path, 0, MAX);
		sprintf(proc_stat_path, "%s/%ld/stat", proc_path, procs[i].PID);
		sprintf(proc_status_path, "%s/%ld/status", proc_path, procs[i].PID);
		sprintf(proc_cmdline_path, "%s/%ld/cmdline", proc_path, procs[i].PID);
		if(access(proc_stat_path, F_OK) == 0) //������ ������ ���� ����
			get_proc_stat(proc_stat_path, i); //proc/pid/stat�� ������ ��´�.
		if(access(proc_status_path, F_OK) == 0) //������ ������ ���� ����
			get_proc_status(proc_status_path, i); //proc/pid/status�� ������ ��´�.
		if(option_c && access(proc_cmdline_path, F_OK) == 0) get_cmdline(proc_cmdline_path, i);
	}
	closedir(proc_dir);
}


void sort_by_cpu() { //%CPU������ ����
	for(int i = 0; i < tasks-1; i++) {
		int mnum = i;
		for(int j = i+1; j < tasks; j++) {
			if(procs[mnum].CPU < procs[j].CPU) {
				mnum = j;
			}
			else if(procs[mnum].CPU == procs[j].CPU) { //��뷮�� ���� ��� PID�� ���� ������ ����
				if(procs[mnum].PID > procs[j].PID) mnum = j;
			}
		}
		if(mnum != i) swap(&procs[mnum], &procs[i]);
	}
}

void sort_by_time() { //TIME+������ ����
	for(int i = 0; i < tasks-1; i++) {
		int mnum = i;
		for(int j = i+1; j < tasks; j++) {
			if(procs[mnum].TIME < procs[j].TIME) {
				mnum = j;
			}
			else if(procs[mnum].TIME == procs[j].TIME) {
				if(procs[mnum].PID > procs[j].PID) mnum = j;
			}
		}
		if(mnum != i) swap(&procs[mnum], &procs[i]);
	}
}

void sort_by_mem() { //%MEM������ ����
	for(int i = 0; i < tasks-1; i++) {
		int mnum = i;
		for(int j = i+1; j < tasks; j++) {
			if(procs[mnum].MEM < procs[j].MEM) {
				mnum = j;
			}
			else if(procs[mnum].MEM == procs[j].MEM) {
				if(procs[mnum].PID > procs[j].PID) mnum = j;
			}
		}
		if(mnum != i) swap(&procs[mnum], &procs[i]);
	}
}

void get_data() {//�����͸� �о���� �Լ�
	tasks = 0, run = 0, slp = 0, stop = 0, zombie = 0;
	FILE* fp;
	struct tm *t; //�����ð� �ҷ���
	time_t tim = time(NULL);
	t = localtime(&tim);
	int user = get_user(); //user �� �б�
	long long uptime = get_uptime(); //uptime �б�
	int uptime_h = uptime / 3600;
	int uptime_m = (uptime - (uptime_h * 3600)) / 60;
	char loadavg[15];
	get_loadavg(loadavg);
	loadavg[14] = '\0';
	get_mem_info();
	get_cpu_info();
	get_procs();

	if(ioctl(0, TIOCGWINSZ, (char*)&win) < 0) {
		fprintf(stderr, "ioctl error\n");
		exit(1);
	}

	memset(result, '\0', sizeof(result));
	//head ����
	if(!uptime_line) {
		sprintf(result[print_row++], "top - %02d:%02d:%02d up  %2d:%02d,%3d user,  load average: %s", t->tm_hour, t->tm_min, t->tm_sec, uptime_h, uptime_m, user, loadavg);
	}
	sprintf(result[print_row++], "Tasks: %d total,   %d running, %d sleeping,   %d stopped,   %d zombie", tasks, run, slp, stop, zombie);
	sprintf(result[print_row++], "%%Cpu(s):  %.1f us,  %.1f sy,  %.1f ni,  %.1f id,  %.1f wa,  %.1f hi,  %.1f si,  %.1f st", us, sy, ni, id, wa, hi, si, st);
	sprintf(result[print_row++], "MiB Mem :   %.1f total,   %.1f free,   %.1f used,   %.1f buff/cache", memtotal, memfree, memused, membuff_cache);
	sprintf(result[print_row++], "MiB Swap:   %.1f total,   %.1f free,   %.1f used.   %.1f avail Mem", swaptotal, swapfree, swapused, swapavail_mem);
	strcpy(result[print_row++], "");
	snprintf(result[print_row++],win.ws_col, "%7s %-8s %3s %3s %7s %6s %6s %c %4s %4s   %7s %s%s",
			"PID", "USER", "PR", "NI", "VIRT", "RES", "SHR", 'S', "%CPU", "%MEM", "TIME+", "COMMAND", blank);
}

bool ispid(int pid) { //����ؾ��ϴ� pid���� �Ǵ�
	if(special_pid == 0) return true; //����ؾ��ϴ� pid�� ���� 0�� ��� ���
	bool ret = false;
	for(int i = 0; i < special_pid; i++) {
		if(pids[i] == pid) {
			ret = true;
			break;
		}
	}
	return ret;
}

bool isuser(char* str) { //����ؾ��ϴ� user���� �Ǵ�
	if(special_user == 0) return true; //����ؾ��ϴ� user�� ���� 0�� ��� ���
	bool ret = false;
	if(!strcmp(users, str)) {
		ret = true;
	}

	return ret;
}

void print_1() { //������尡 �ƴ� �� ����� ����ϴ� �Լ�
	//�͹̳� ũ�� ���ϱ�
	if(ioctl(0, TIOCGWINSZ, (char*)&win) < 0) {
		fprintf(stderr, "ioctl error\n");
		exit(1);
	}

	char tm[8]; //TIME ���ڿ� �����Լ�
	for(int i = start_row; i < tasks; i++) {
		if(option_i && procs[i].CPU < 0.1) continue;
		memset(tm, 0, sizeof(tm));//TIME�� ���ڿ��� ��Ÿ����
		int min = procs[i].TIME / 6000;
		int sec = (procs[i].TIME - (min *6000)) / 100;
		int rest = (procs[i].TIME - (min *6000) - (sec * 100));
		sprintf(tm, "%d:%02d.%02d", min, sec, rest);
		char username[MAX];
		memcpy(username, procs[i].USER, MAX);
		if(username[7] != '\0') {
			username[7] = '+'; //�̸��� �ʹ� �� ��� �ڿ� '+'�� ǥ��
			memset(username + 8, '\0', MAX - 8);
		}
		if(ispid(procs[i].PID) && isuser(procs[i].USER)) {//����ϰ��� �ϴ� pid�� ���
			snprintf(result[print_row++], win.ws_col, "%7ld %-8s %3s %3d %7lld %6lld %6lld %c %4.1lf %4.1lf   %7s %s", 
					procs[i].PID, username, procs[i].PR, procs[i].NI, procs[i].VIRT, procs[i].RES, 
					procs[i].SHR, procs[i].S, procs[i].CPU, procs[i].MEM, tm, procs[i].COMMAND);
		}
	}

	//���� ���
	if(!uptime_line) {
		for(int i = 0; i < 6; i++)
			mvprintw(i, 0, "%s", result[i]);
		attron(A_REVERSE);
		mvprintw(6, 0, "%s", result[6] + begin[start_col]);
		attroff(A_REVERSE);
		for(int i = 7; i <= win.ws_row; i++)
			mvprintw(i, 0, "%s", result[i] + begin[start_col]);
	}
	else {
		for(int i = 0; i < 5; i++)
			mvprintw(i, 0, "%s", result[i]);
		attron(A_REVERSE);
		mvprintw(5, 0, "%s", result[5] + begin[start_col]);
		attroff(A_REVERSE);
		for(int i = 6; i <= win.ws_row; i++)
			mvprintw(i, 0, "%s", result[i] + begin[start_col]);
	}
}


void print_2() { //���� ��� ����� ����ϴ� �Լ�
	printf("\n");
	//�͹̳� ũ�� ���ϱ�
	if(ioctl(0, TIOCGWINSZ, (char*)&win) < 0) {
		fprintf(stderr, "ioctl error\n");
		exit(1);
	}


	char tm[8]; //TIME ���ڿ� �����Լ�
	for(int i = start_row; i < tasks; i++) {
		if(option_i && procs[i].CPU < 0.1) continue;
		memset(tm, 0, sizeof(tm));//TIME�� ���ڿ��� ��Ÿ����
		int min = procs[i].TIME / 6000;
		int sec = (procs[i].TIME - (min *6000)) / 100;
		int rest = (procs[i].TIME - (min *6000) - (sec * 100));
		sprintf(tm, "%d:%02d.%02d", min, sec, rest);
		char username[MAX];
		memcpy(username, procs[i].USER, MAX);
		if(username[7] != '\0') {
			username[7] = '+'; //�̸��� �ʹ� �� ��� �ڿ� '+'�� ǥ��
			memset(username + 8, '\0', MAX - 8);
		}
		if(ispid(procs[i].PID) && isuser(procs[i].USER)) {//����ϰ��� �ϴ� pid�� ���
			snprintf(result[print_row++], win.ws_col, "%7ld %-8s %3s %3d %7lld %6lld %6lld %c %4.1lf %4.1lf   %7s %s", 
					procs[i].PID, username, procs[i].PR, procs[i].NI, procs[i].VIRT, procs[i].RES, 
					procs[i].SHR, procs[i].S, procs[i].CPU, procs[i].MEM, tm, procs[i].COMMAND);
		}
	}

	//���� ���
	for(int i = 0; i < print_row; i++)
		printf("%s\n", result[i]);
}

void operation_d() { //d�ɼ��� �����ϴ� �Լ�
	alarm(0); //alarm ���
	char str[MAX];
	memset(str, 0, MAX);
	int idx = 0;
	int c;
	int chk = 0;
	if(!uptime_line) { //uptime_line�� �ִ� ���
		mvaddstr(5, 0, blank2);
		mvaddstr(5, 0, "Change delay from 3.0 to "); 
	}
	else { //uptime_line�� ���� ���
		mvaddstr(4, 0, blank2);
		mvaddstr(4, 0, "Change delay from 3.0 to ");
	}
	while((c = getch()) != '\n') {
		if(c == 8) { //backspace�� ���
			delch();
			continue;
		}
		if(c == 27) { //esc�� ���� ���
			chk = 1;
			break;
		}
		if(c != -1)
			str[idx++] = c;
	}
	if(!chk) {//esc�� ���� ��� ���
		delay = atoi(str);
	}
	alarm(3);
	raise(SIGALRM);
}

void operation_u() {
	alarm(0); //alarm ���
	char str[MAX];
	memset(str, 0, MAX);
	int idx = 0;
	char c;
	int chk = 0;
	if(!uptime_line) { //uptime_line�� �ִ� ���
		mvaddstr(5, 0, blank2);
		mvaddstr(5, 0, "Which user (blank for all) "); 
	}
	else { //uptime_line�� ���� ���
		mvaddstr(4, 0, blank2);
		mvaddstr(4, 0, "Which user (blank for all) ");
	}
	char tmp[10];
	while((c = getch()) != '\n') {
		if(c == 8) { //backspace�� ���
			idx--;
			delch();
			continue;
		}
		if(c == 27) { //esc�� ���� ���
			chk = 1;
			break;
		}
		if(c != -1)
			str[idx++] = c;
	}
	if(!chk) { //esc�� ������ �ʾ��� ��
		strncpy(users, str, MAX); //����ϰ��� �ϴ� user�� ����
		mvaddstr(5, 0, users);
		if(idx == 0) //�ƹ��͵� �Է��� ���� ���� ���
			special_user = 0;
		else
			special_user = 1;
	}
	alarm(delay);
	raise(SIGALRM);

}


void operation_k() { //k�ɼ��� �����ϴ� �Լ�
	alarm(0); //alarm ���
	char str[MAX];
	char sig[10];
	char tmp[MAX];
	char trash[MAX];
	memset(str, 0, MAX);
	memset(sig, 0, 10);
	int idx = 0;
	int c;
	int chk = 0;
	memset(tmp, 0, MAX);
	sprintf(tmp, "PID to signal/kill [default pid = %ld] ", procs[0].PID);
	if(!uptime_line) { //uptime_line�� �ִ� ���
		mvaddstr(5, 0, blank2);
		mvaddstr(5, 0, tmp); //default�� ���� ���� �ִ� Process
	}
	else { //uptime_line�� ���� ���
		mvaddstr(4, 0, blank2);
		mvaddstr(4, 0, tmp);
	}
	while((c = getch()) != '\n') {
		if(c == 8) { //backspace�� ���
			idx--;
			delch();
			continue;
		}
		if(c == 27) { //esc�� ���� ���
			chk = 1;
			break;
		}
		if(c != -1)
			str[idx++] = c;
	}
	if(!chk) {//esc�� ������ ���� ���
		if(idx == 0) //�ƹ��͵� �Է��� ���� ���� ���
			kill_pid = procs[0].PID;
		else
			kill_pid = atoi(str);

		memset(tmp, 0, MAX);
		sprintf(tmp, "Send pid %d signal [15/sigterm] ", kill_pid);
		if(!uptime_line) { //uptime_line�� �ִ� ���
			mvaddstr(5, 0, blank2);
			mvaddstr(5, 0, tmp);
		}
		else { //uptime_line�� ���� ���
			mvaddstr(4, 0, blank2);
			mvaddstr(4, 0, tmp);
		}
		idx = 0;
		tcflush(0, TCIFLUSH);
		while((c = getch()) != '\n') {
			if(c == 8) { //backspace�� ���
				idx--;
				delch();
				continue;
			}
			if(c == 27) { //esc�� ���� ���
				chk = 1;
				break;
			}
			if(c != -1)
				sig[idx++] = c;
		}
		if(!chk) { //esc�� ������ ���� ���
			int signal = 0;
			for(int i = 0; i < idx; i++) {
				if(isdigit(sig[i])) {
					signal = signal*10 + (sig[i] - '0');
				}
			}
			if(signal == 9) {
				kill(kill_pid, SIGKILL);
			}
		}
	}
	alarm(delay);
	raise(SIGALRM);
}



void start_status() { //ncurses �ʱ� ����
	noecho(); //echo ����
	curs_set(0); //Ŀ�� �Ⱥ��̰� ��
	initscr(); //��� ������ �ʱ�ȭ
	halfdelay(10); //0.1�ʸ��� ����
	keypad(stdscr, true);
}

int get_ch() {
	int ret;
	struct termios buf, save;
	tcgetattr(0, &save);
	buf = save;
	buf.c_lflag &= ~ICANON;
	buf.c_lflag &= ~ECHO;
	tcsetattr(0, TCSAFLUSH,	&buf);
	ret = getchar();
	tcsetattr(0, TCSAFLUSH, &save);
	return ret;
}


int main(int argc, char **argv) {
	if(ioctl(0, TIOCGWINSZ, (char*)&win) < 0) {
		fprintf(stderr, "ioctl error\n");
		exit(1);
	}

	for(int i = 12; i < MAX; i++) begin[i] = begin[i-1] + 8;
	//�ʱ� �ɼ� ����
	signal(SIGALRM, handler); //alarm �ñ׳ο� ������ handler�Լ��� ����
	option = 'P';
	uptime_line = 0;
	delay = 3;
	option_c = 0;
	option_i = 0;
	print_max = -1;
	special_pid = 0;
	option_b = 0;
	special_user = 0;
	kill_pid = 0;
	memset(blank2, ' ', 70);

	//���� �� �ɼ� parsing
	for(int i = 1; i < argc; i++) {
		if(!strcmp(argv[i], "-n")) { //���� Ƚ�� ���ѿɼ�
			print_max = atoi(argv[i+1]);
			if(print_max == 0) { //�ùٸ��� ���� ���ڸ� �Է��� ��� ����ó�� �� ����
				fprintf(stderr, "top: bad iterations argument '0'\n");
				endwin();
				exit(0);
			}
		}
		if(!strcmp(argv[i], "-p")) { //Ư�� PID�� ���
			int pid = atoi(argv[i+1]);
			pids[special_pid] = pid;
			special_pid++; //��� pid ���� ����
		}
		if(!strcmp(argv[i], "-b")) { //��� process���� �����ϱ⸸ �ϴ� ���
			option_b = 1;
		}
	}

	get_data();
	sort_by_cpu();
	if(!option_b) {
		start_status();
		print_1();
		refresh();
	}
	else {
		print_2();
	}
	print_cnt = 1;
	alarm(delay);
	//3�ʸ��� ����
	while(1) {
		if(ioctl(0, TIOCGWINSZ, (char*)&win) < 0) {
			fprintf(stderr, "ioctl error\n");
			exit(1);
		}
		memset(blank, ' ', win.ws_col);


		if(print_max != -1 && print_max == print_cnt) break;
		int input = getch();
		int sum = input;
		if(input == 'q') { //����
			break;
		}
		else if(input == 'P') { //CPU�� ����
			option = 'P';
			raise(SIGALRM);
		}
		else if(input == 'M') { //MEM�� ����
			option = 'M';
			raise(SIGALRM);
		}
		else if(input == 'T') { //TIME�� ����
			option = 'T';
			raise(SIGALRM);
		}

		else if(input == 'l') { //uptime_line����/ǥ��
			if(uptime_line)
				uptime_line = 0;
			else
				uptime_line = 1;
			raise(SIGALRM);
		}

		else if(input == ' ' || input == 27) { //space�� esc�� ���� ��� refresh
			raise(SIGALRM);
		}

		else if(input == 'c') { //��� ���� ǥ��/ ��ǥ��
			if(option_c) option_c = 0;
			else option_c = 1;
			raise(SIGALRM);
		}

		else if(input == 'i') {
			if(option_i) option_i = 0;
			else option_i = 1;
			raise(SIGALRM);
		}

		else if(input == 'd') { //delay�� ����
			operation_d();
		}

		else if(input == 'u' || input == 'U') { //Ư�� user ����
			operation_u();
		}

		else if(input == 'k') { //kill�Ϸ��� pid ���
			operation_k();
		}
		else { //����Ű�� �Է��ϴ� ���
			if(sum == KEY_UP) {//�� ����Ű�� ���
				start_row--;
				if(start_row < 0) start_row = 0;
				raise(SIGALRM);
			}
			else if(sum == KEY_DOWN) { //�Ʒ� ����Ű�� ���
				start_row++;
				if(start_row == tasks) start_row = tasks-1;
				raise(SIGALRM);
			}
			else if(sum == KEY_LEFT) { //���� ����Ű�� ���
				start_col--;
				if(start_col < 0) start_col = 0;
				raise(SIGALRM);
			}
			else if(sum == KEY_RIGHT) { //������ ����Ű�� ���
				start_col++;
				raise(SIGALRM);
			}
		}
	}

	endwin();
	return 0;
}
