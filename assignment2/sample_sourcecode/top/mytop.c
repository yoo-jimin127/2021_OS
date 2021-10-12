#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <ncurses.h>
#include <utmp.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include "mytop.h"

int main(void)
{
    time_t oldtime = 0;
    struct winsize trmn_size;
    int i, start_line = 0, end_line, max_line = 999;

    if (ioctl(0, TIOCGWINSZ, (char *)&trmn_size) < 0)
        printf("TIOCGWINSZ error");
    if (trmn_size.ws_row < 8)
        end_line = 0;
    else
        end_line = trmn_size.ws_row - 8;

    init_key();
    while (1)
    {
        if (_kbhit())
        {                      
            int key = _getch();

            if (key == 91 || key == 0)
            {
                key = _getch();
                switch (key)
                {
                case 65:
                    if (start_line > 0)
                    {
                        start_line--;
                        end_line--;
                    }
                    oldtime = 0;
                    break;
                case 66:
                    if (end_line < max_line)
                    {
                        end_line++;
                        start_line++;
                    }
                    oldtime = 0;
                    break;
                }
            }
            else if (key == 'q' || key == 'Q')
            {
                close_key();
                exit(0);
            }
        }
        time_t nowtime_sec;
        time(&nowtime_sec);

        if (oldtime == 0)
            oldtime = nowtime_sec;
        else if ((nowtime_sec - oldtime) == 3)
            oldtime = nowtime_sec;
        else
            continue;

        char *nowtime;
        nowtime = malloc(sizeof(char) * 32);
        nowtime = getNowtime();
        printf("top - %s ", nowtime);

        char *os_runningtime;
        os_runningtime = malloc(sizeof(char) * 32);
        os_runningtime = getRunningtime();
        printf("up  %s,  ", os_runningtime);

        int user_count;
        user_count = getUsercount();
        printf("%d user,  ", user_count);

        LOADAVG LA;
        LA = getLoadavg();
        printf("load average: %s, %s, %s\n", LA.la_min1, LA.la_min5, LA.la_min15);

        TASKS tasks;
        tasks.ps = (PS *)malloc(sizeof(PS) * tasks.task_count);
        tasks = getPS();
        printf("Tasks: %4d total, %4d running, %4d sleeping, %4d stopped, %4d zombie\n",
               tasks.task_count, tasks.running, tasks.sleeping, tasks.stopped, tasks.zombie);

        CPU_USING cpu_using;
        cpu_using = getCpuUsing();
        printf("%%Cpu(s)  %.1f us,  %.1f sy,  %.1f ni,  %.1f id,  %.1f wa  %.1f hi  %.1f si  %.1f st\n",
               cpu_using.cpu_user, cpu_using.cpu_nice, cpu_using.cpu_system, cpu_using.cpu_idle,
               cpu_using.cpu_wa, cpu_using.cpu_hi, cpu_using.cpu_si, cpu_using.cpu_st);

        MEM_SWAP mem_swap;
        mem_swap = getMemSwap();
        printf("KiB Mem : %7ld total, %7ld free, %7ld used, %7ld buff/cashe\n", mem_swap.mem_total, mem_swap.mem_free, mem_swap.mem_used, mem_swap.mem_bufca);
        printf("KiB Swap: %7ld total, %7ld free, %7ld used, %7ld avail Mem\n\n", mem_swap.swap_total, mem_swap.swap_free, mem_swap.swap_used, mem_swap.mem_avail);

        printf("  PID USER        PR   NI    VIRT     RES    SHR  S  %%CPU  %%MEM    TIME+ COMMAND\n");
        for (i = start_line; i < end_line; i++)
        {
            tasks.ps[i].cpu = (tasks.ps[i].utime + tasks.ps[i].stime) / cpu_using.cpu_total * 100;
            tasks.ps[i].mem = tasks.ps[i].rss * 1000 / mem_swap.mem_total;
            printf("%5d %-8s %5d %4d ", tasks.ps[i].pid, tasks.ps[i].username, tasks.ps[i].prior, tasks.ps[i].nice);
            printf("%7ld %7ld %6ld  %s", tasks.ps[i].vsize, tasks.ps[i].rss, tasks.ps[i].shr, tasks.ps[i].stat);
            printf("%5.1f %5.1f  %s %s\n", tasks.ps[i].cpu, tasks.ps[i].mem / 10.0, tasks.ps[i].runningtime, tasks.ps[i].comm);
        }
        max_line = tasks.task_count;
        free(tasks.ps);
    }
    close_key();

    exit(0);
}

char *getNowtime()
{
    time_t t;
    struct tm tm;
    char *nowtime;
    nowtime = malloc(sizeof(char) * 32);

    time(&t);
    tm = *localtime(&t);
    sprintf(nowtime, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);

    return nowtime;
}

char *getRunningtime()
{
    FILE *fp;
    time_t t;
    struct tm tm;
    char os_starttime[32], *os_runningtime;
    double os_runningtime_tmp;
    int os_runningtime_sec;
    os_runningtime = malloc(sizeof(char) * 32);

    fp = fopen("/proc/uptime", "r");
    fscanf(fp, "%s", os_starttime);
    os_runningtime_tmp = atof(os_starttime);
    os_runningtime_sec = (int)os_runningtime_tmp;
    fclose(fp);
    t = os_runningtime_sec;
    tm = *localtime(&t);
    sprintf(os_runningtime, "%02d:%02d", tm.tm_hour - 9, tm.tm_min);

    return os_runningtime;
}

int getUsercount()
{                        
    struct utmp *utmpfp;
    setutent();
    int user_count = 0;
    while ((utmpfp = getutent()) != NULL)
        if (utmpfp->ut_type == USER_PROCESS)
            user_count++;
    return user_count;
}

LOADAVG getLoadavg()
{
    FILE *fp;
    LOADAVG LA;
    memset(&LA, 0, sizeof(LOADAVG));

    fp = fopen("/proc/loadavg", "r");
    fscanf(fp, "%s %s %s", LA.la_min1, LA.la_min5, LA.la_min15);
    fclose(fp);

    return LA;
}

CPU_USING getCpuUsing()
{
    FILE *fp;
    CPU_USING cpu_using;
    memset(&cpu_using, 0, sizeof(CPU_USING));

    fp = fopen("/proc/stat", "r");
    char cpu_tmp[16];
    long cpu_user, cpu_nice, cpu_system, cpu_idle;
    long cpu_wa, cpu_hi, cpu_si, cpu_st;
    fscanf(fp, "%s %ld %ld %ld", cpu_tmp, &cpu_user, &cpu_nice, &cpu_system);
    fscanf(fp, "%ld %ld %ld %ld %ld", &cpu_idle, &cpu_wa, &cpu_hi, &cpu_si, &cpu_st);
    fclose(fp);

    cpu_using.cpu_total = (double)cpu_user + cpu_nice + cpu_system + cpu_idle + cpu_wa + cpu_hi + cpu_si + cpu_st;
    cpu_using.cpu_user = (double)cpu_user / cpu_using.cpu_total * 100;
    cpu_using.cpu_nice = (double)cpu_nice / cpu_using.cpu_total * 100;
    cpu_using.cpu_system = (double)cpu_system / cpu_using.cpu_total * 100;
    cpu_using.cpu_idle = (double)cpu_idle / cpu_using.cpu_total * 100;
    cpu_using.cpu_wa = (double)cpu_wa / cpu_using.cpu_total * 100;
    cpu_using.cpu_hi = (double)cpu_hi / cpu_using.cpu_total * 100;
    cpu_using.cpu_si = (double)cpu_si / cpu_using.cpu_total * 100;
    cpu_using.cpu_st = (double)cpu_st / cpu_using.cpu_total * 100;

    return cpu_using;
}

TASKS getPS()
{
    TASKS tasks;
    memset(&tasks, 0, sizeof(TASKS));
    tasks.ps = (PS *)malloc(sizeof(PS) * 1000);
    PS ps;

    FILE *fp;
    DIR *dir;
    struct dirent *dir_file;
    struct stat filestat;
    struct passwd *pwd;

    int i;
    char ps_temp[256], tempPath[256];
    char ps_vsize_tmp[32], ps_rss_tmp[32];

    char ps_starttime[32];
    int ps_runningtime_hour, ps_runningtime_min;
    long ps_time;

    struct winsize trmn_size;

    if (ioctl(0, TIOCGWINSZ, (char *)&trmn_size) < 0)
        printf("TIOCGWINSZ error");

    dir = opendir("/proc");

    while ((dir_file = readdir(dir)) != NULL)
    {
        lstat(dir_file->d_name, &filestat);

        if (!S_ISDIR(filestat.st_mode))
            continue;

        ps.pid = atoi(dir_file->d_name);
        if (ps.pid <= 0)
            continue;

        tasks.task_count++;

        sprintf(tempPath, "/proc/%d/stat", ps.pid);
        fp = fopen(tempPath, "r");

        for (i = 0; i < 2; i++) // 1 2
            fscanf(fp, "%s", ps_temp);

        fscanf(fp, "%s", ps.stat);
        if (!strcmp(ps.stat, "R"))
            tasks.running++;
        else if (!strcmp(ps.stat, "S"))
            tasks.sleeping++;
        else if (!strcmp(ps.stat, "T"))
            tasks.stopped++;
        else if (!strcmp(ps.stat, "Z"))
            tasks.zombie++;

        fscanf(fp, "%s", ps_temp);
        fscanf(fp, "%s", ps_temp);

        fscanf(fp, "%s", ps_temp);

        fscanf(fp, "%s", ps_temp);
        fscanf(fp, "%s", ps_temp);

        for (i = 0; i < 5; i++)
            fscanf(fp, "%s", ps_temp);

        fscanf(fp, "%ld", &ps.utime);
        fscanf(fp, "%ld", &ps.stime);
        ps_time = ps.utime + ps.stime;
        ps_time = ps_time / sysconf(_SC_CLK_TCK);
        ps_runningtime_hour = ps_time / 60;
        ps_runningtime_min = ps_time % 60;
        sprintf(ps.runningtime, "%02d:%02d.%02d", ps_runningtime_hour, ps_runningtime_min, (int)(ps_runningtime_min * 100 / 60));

        for (i = 0; i < 2; i++)
            fscanf(fp, "%s", ps_temp);

        fscanf(fp, "%d", &ps.prior);
        fscanf(fp, "%d", &ps.nice);

        fscanf(fp, "%s", ps_temp);
        fscanf(fp, "%s", ps_temp);

        fscanf(fp, "%s", ps_starttime);

        fscanf(fp, "%s", ps_vsize_tmp);
        fscanf(fp, "%s", ps_rss_tmp);
        ps.vsize = atol(ps_vsize_tmp) / 1024;
        ps.rss = atol(ps_rss_tmp) * 4;
        fclose(fp);

        sprintf(tempPath, "/proc/%d/status", ps.pid);
        fp = fopen(tempPath, "r");
        for (i = 0; i < 18; i++)
            fscanf(fp, "%s", ps_temp);
        char pwd_tmp[16];
        fscanf(fp, "%s", pwd_tmp);
        for (i = 0; i < 41; i++)
            fscanf(fp, "%s", ps_temp);
        fscanf(fp, "%ld", &ps.shr); // rss file
        fclose(fp);
        pwd = getpwuid(atoi(pwd_tmp));
        sprintf(ps.username, "%s", pwd->pw_name);

        sprintf(tempPath, "/proc/%d/comm", ps.pid);

        getCmdline(tempPath, ps.comm); // /proc/pid/comm
        ps.comm[strlen(ps.comm) - 1] = '\0';

        if (trmn_size.ws_col > 76 && strlen(ps.comm) > (trmn_size.ws_col - 76))
            ps.comm[trmn_size.ws_col - 73] = '\0';

        tasks.ps[tasks.task_count - 1] = ps;
    }
    closedir(dir);

    return tasks;
}

void getCmdline(char *file, char *buf)
{
    FILE *fp;
    fp = fopen(file, "r"); //   /proc/{pid}/cmdline

    memset(buf, 0, sizeof(*buf));
    fgets(buf, 256, fp);
    fclose(fp);
}

MEM_SWAP getMemSwap()
{
    FILE *fp;
    char tmp[32];
    long buffer, cached, SReclaim;
    int i;
    MEM_SWAP mem_swap;
    memset(&mem_swap, 0, sizeof(MEM_SWAP));

    fp = fopen("/proc/meminfo", "r");

    fscanf(fp, "%s %ld %s", tmp, &mem_swap.mem_total, tmp);
    fscanf(fp, "%s %ld %s", tmp, &mem_swap.mem_free, tmp);
    fscanf(fp, "%s %ld %s", tmp, &mem_swap.mem_avail, tmp);
    fscanf(fp, "%s %ld %s", tmp, &buffer, tmp);
    fscanf(fp, "%s %ld %s", tmp, &cached, tmp);

    for (i = 0; i < 27; i++)
        fscanf(fp, "%s", tmp);

    fscanf(fp, "%s %ld %s", tmp, &mem_swap.swap_total, tmp);
    fscanf(fp, "%s %ld %s", tmp, &mem_swap.swap_free, tmp);

    for (i = 0; i < 21; i++)
        fscanf(fp, "%s", tmp);
    fscanf(fp, "%s %ld %s", tmp, &SReclaim, tmp);

    mem_swap.mem_bufca = buffer + cached + SReclaim;
    mem_swap.mem_used = mem_swap.mem_total - mem_swap.mem_free - mem_swap.mem_bufca;
    mem_swap.swap_used = mem_swap.swap_total - mem_swap.swap_free;

    fclose(fp);

    return mem_swap;
}

void init_key()
{
    tcgetattr(0, &init_set);
    new_set = init_set;
    new_set.c_lflag &= ~ICANON;
    new_set.c_lflag &= ~ECHO;
    new_set.c_cc[VMIN] = 1;
    new_set.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_set);
}

void close_key()
{
    tcsetattr(0, TCSANOW, &init_set);
}

int _kbhit()
{
    unsigned char ch;
    int read_n;

    if (pk_ch != -1)
        return 1;
    new_set.c_cc[VMIN] = 0;
    tcsetattr(0, TCSANOW, &new_set);
    read_n = read(0, &ch, 1);
    new_set.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &new_set);
    if (read_n == 1)
    {
        pk_ch = ch;
        return 1;
    }
    return 0;
}

int _getch()
{
    char ch;

    if (pk_ch != -1)
    {
        ch = pk_ch;
        pk_ch = -1;
        return ch;
    }
    read(0, &ch, 1);
    return ch;
}