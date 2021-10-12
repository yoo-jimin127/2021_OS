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
#include <ncurses.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include <linux/kdev_t.h>
#include "myps.h"

int main(int argc, char *argv[])
{
    int opt_a = 0, opt_x = 0, opt_u = 0;
    int i;

    FILE *fp;
    DIR *dir;
    struct dirent *dir_file;
    struct stat filestat;
    struct passwd *pwd, *my_pwd;
    struct winsize trmn_size;

    int ps_pid;
    char ps_stat[32];
    int ps_stat_session, ps_ppid, ps_ttynr, ps_stat_fg, ps_stat_thread, ps_stat_prior;
    char ps_cmdline[256], ps_temp[256], ps_comm[256], ps_runningtime[6];
    long ps_vsize_tmp, ps_rss_tmp;
    char tempPath[256];

    time_t t;
    struct tm tm;

    double ps_mem, ps_cpu;
    char ps_tty[300] = "?";
    double os_starttime;
    int os_runningtime_sec;

    char ps_starttime[256];
    int ps_starttime_sec;
    int ps_runningtime_hour, ps_runningtime_min;
    long ps_vsize, ps_rss, ps_utime, ps_stime, ps_time;

    if (argc == 2)
    {
        for (i = 0; i < strlen(argv[1]); i++)
        {
            if (argv[1][i] == 'a')
                opt_a = 1;
            if (argv[1][i] == 'x')
                opt_x = 1;
            if (argv[1][i] == 'u')
                opt_u = 1;
        }
    }

    if (ioctl(0, TIOCGWINSZ, (char *)&trmn_size) < 0)
        printf("TIOCGWINSZ error");

    if (opt_u == 1)
        printf("USER         PID  %%CPU  %%MEM     VSZ     RSS    TTY    STAT    START  TIME  COMMAND\n");
    else
        printf("  PID  TTY    STAT   TIME CMD\n");

    fp = fopen("/proc/uptime", "r");
    fscanf(fp, "%lf", &os_starttime);
    os_runningtime_sec = (int)os_starttime;
    fclose(fp);

    DEV *dev = (DEV *)malloc(sizeof(DEV) * 1000);
    int dev_count = 0;
    struct stat st;
    dir = opendir("/dev");
    char dev_filepath[300];
    while ((dir_file = readdir(dir)) != NULL)
    {
        sprintf(dev_filepath, "/dev/%s", dir_file->d_name);
        lstat(dev_filepath, &st);

        if (!S_ISCHR(st.st_mode))
            continue;
        sprintf(dev[dev_count].filename, "%s ", dir_file->d_name);
        dev[dev_count].major = major(st.st_rdev);
        dev[dev_count++].minor = minor(st.st_rdev);
    }
    closedir(dir);

    dir = opendir("/dev/pts");
    while ((dir_file = readdir(dir)) != NULL)
    {
        sprintf(dev_filepath, "/dev/pts/%s", dir_file->d_name);
        lstat(dev_filepath, &st);

        if (!S_ISCHR(st.st_mode))
            continue;
        sprintf(dev[dev_count].filename, "pts/%s", dir_file->d_name);
        dev[dev_count].major = major(st.st_rdev);
        dev[dev_count++].minor = minor(st.st_rdev);
    }
    closedir(dir);

    CPU_USING cpu_using;
    cpu_using = getCpuUsing();

    MEM_SWAP mem_swap;
    mem_swap = getMemSwap();

    dir = opendir("/proc");
    while ((dir_file = readdir(dir)) != NULL)
    {
        lstat(dir_file->d_name, &filestat);

        if (!S_ISDIR(filestat.st_mode))
            continue;

        ps_pid = atoi(dir_file->d_name);
        if (ps_pid <= 0)
            continue;

        sprintf(tempPath, "/proc/%d/stat", ps_pid);
        fp = fopen(tempPath, "r");

        fscanf(fp, "%d", &ps_pid);
        fscanf(fp, "%s", ps_temp);

        fscanf(fp, "%s", ps_stat);

        fscanf(fp, "%d", &ps_ppid);
        fscanf(fp, "%s", ps_temp);

        if ((opt_a == 0) && (opt_x == 0) && (opt_u == 0) && ((ps_ppid != getppid()) && ps_pid != getppid()))
            continue;
        fscanf(fp, "%d", &ps_stat_session);

        fscanf(fp, "%d", &ps_ttynr);
        if (ps_ttynr != 0)
        {
            for (i = 0; i < dev_count; i++)
            {
                if (dev[i].major == MAJOR(ps_ttynr) && dev[i].minor == MINOR(ps_ttynr))
                {
                    sprintf(ps_tty, "%s", dev[i].filename);
                    break;
                }
            }
        }
        if (((opt_a == 1 && opt_x == 0)|| (opt_a == 0 && opt_x == 0 && opt_u == 1)) && !strcmp(ps_tty, "?"))
            continue;

        fscanf(fp, "%d", &ps_stat_fg);

        for (i = 0; i < 5; i++)
            fscanf(fp, "%s", ps_temp);

        fscanf(fp, "%ld", &ps_utime);
        fscanf(fp, "%ld", &ps_stime);
        ps_time = ps_utime + ps_stime;
        ps_time = ps_time / sysconf(_SC_CLK_TCK);
        ps_runningtime_hour = ps_time / 60;
        ps_runningtime_min = ps_time % 60;
        sprintf(ps_runningtime, "%02d:%02d", ps_runningtime_hour, ps_runningtime_min);

        ps_cpu = (ps_utime + ps_stime) * 100.0 / cpu_using.cpu_total;

        for (i = 0; i < 2; i++)
            fscanf(fp, "%s", ps_temp);

        fscanf(fp, "%d", &ps_stat_prior);
        if (ps_stat_prior < 20 && ps_stat_prior >= 0)
            sprintf(ps_stat, "%s<", ps_stat);
        else if (ps_stat_prior > 20)
            sprintf(ps_stat, "%sN", ps_stat);

        if (ps_pid == ps_stat_session)
            sprintf(ps_stat, "%ss", ps_stat);

        fscanf(fp, "%s", ps_temp);

        fscanf(fp, "%d", &ps_stat_thread);
        if (ps_stat_thread > 1)
            sprintf(ps_stat, "%sl", ps_stat);

        if (ps_stat_fg >= 0)
            sprintf(ps_stat, "%s+", ps_stat);

        fscanf(fp, "%s", ps_temp);

        fscanf(fp, "%s", ps_starttime);
        ps_starttime_sec = atoi(ps_starttime) / sysconf(_SC_CLK_TCK);
        time(&t);
        t = t - os_runningtime_sec;
        t = t + ps_starttime_sec;
        tm = *localtime(&t);
        sprintf(ps_starttime, "%02d:%02d", tm.tm_hour, tm.tm_min);

        fscanf(fp, "%ld", &ps_vsize_tmp);
        fscanf(fp, "%ld", &ps_rss_tmp);
        ps_vsize = ps_vsize_tmp / 1024;
        ps_rss = ps_rss_tmp * 4;
        fclose(fp);

        ps_mem = ps_rss * 100.0 / mem_swap.mem_total;

        sprintf(tempPath, "/proc/%d/status", ps_pid);
        fp = fopen(tempPath, "r");
        for (i = 0; i < 18; i++)
            fscanf(fp, "%s", ps_temp);

        int pwd_tmp;
        fscanf(fp, "%d", &pwd_tmp);

        fclose(fp);
        my_pwd = getpwuid(getuid());
        char my_name[300];
        sprintf(my_name, "%s", my_pwd->pw_name);
        pwd = getpwuid(pwd_tmp);
        if (((opt_a == 0 && opt_x == 1) || (opt_a == 0 && opt_x == 0 && opt_u == 1)) && strcmp(pwd->pw_name, my_name))
            continue;

        sprintf(tempPath, "/proc/%d/cmdline", ps_pid);
        getCmdline(tempPath, ps_cmdline);

        if (strlen(ps_cmdline) == 0)
        {
            sprintf(tempPath, "/proc/%d/comm", ps_pid);

            getCmdline(tempPath, ps_comm);
            ps_comm[strlen(ps_comm) - 1] = '\0';

            if (opt_u == 1)
            {
                if (trmn_size.ws_col > 75 && strlen(ps_comm) > (trmn_size.ws_col - 75))
                    ps_comm[trmn_size.ws_col - 74] = '\0';
                printf("%-10s %5d   %.1f   %.1f %7ld %7ld    %s%6s %8s %6s [%s]\n", pwd->pw_name, ps_pid, ps_cpu, ps_mem, ps_vsize, ps_rss, ps_tty, ps_stat, ps_starttime, ps_runningtime, ps_comm);
            }
            else
            {
                ps_comm[trmn_size.ws_col - 26] = '\0';
                printf("%5d  %s%6s %6s [%s]\n", ps_pid, ps_tty, ps_stat, ps_runningtime, ps_comm);
            }
        }
        else
        {
            if (opt_u == 1)
            {
                if (trmn_size.ws_col > 78 && strlen(ps_cmdline) > (trmn_size.ws_col - 78))
                    ps_cmdline[trmn_size.ws_col - 76] = '\0';
                printf("%-10s %5d   %.1f   %.1f %7ld %7ld    %s%6s %8s %6s %s\n", pwd->pw_name, ps_pid, ps_cpu, ps_mem, ps_vsize, ps_rss, ps_tty, ps_stat, ps_starttime, ps_runningtime, ps_cmdline);
            }
            else
            {
                ps_cmdline[trmn_size.ws_col - 26] = '\0';
                printf("%5d  %s%6s %6s %s\n", ps_pid, ps_tty, ps_stat, ps_runningtime, ps_cmdline);
            }
        }
        sprintf(ps_tty, "%s", "?");
    }
    closedir(dir);

    exit(0);
}

void getCmdline(char *file, char *buf)
{
    FILE *fp;
    fp = fopen(file, "r");

    memset(buf, 0, sizeof(*buf));
    fgets(buf, 256, fp);
    fclose(fp);
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

    return cpu_using;
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
    fclose(fp);

    return mem_swap;
}