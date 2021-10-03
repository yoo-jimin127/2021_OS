typedef struct
{
    char la_min1[8], la_min5[8], la_min15[8];
} LOADAVG;

typedef struct
{
    double cpu_total, cpu_user, cpu_nice, cpu_system, cpu_idle;
    double cpu_wa, cpu_hi, cpu_si, cpu_st;
} CPU_USING;

typedef struct
{
    int pid, prior, nice;
    long utime, stime;
    char username[32], stat[16], comm[256], runningtime[6];
    double mem, cpu;
    long vsize, rss, shr;
} PS;

typedef struct
{
    int task_count;
    int running, sleeping, stopped, zombie;
    PS *ps;
} TASKS;

typedef struct
{
    long mem_total, mem_free, mem_used, mem_bufca, mem_avail;
    long swap_total, swap_free, swap_used;
}MEM_SWAP;

char *getNowtime();
char *getRunningtime();
int getUsercount();
LOADAVG getLoadavg();
CPU_USING getCpuUsing();
TASKS getPS();
void getCmdline(char *file, char *buf);
MEM_SWAP getMemSwap();
static struct termios init_set, new_set;
static int pk_ch = -1;
void init_key();
void close_key();
int _kbhit();
int _getch();