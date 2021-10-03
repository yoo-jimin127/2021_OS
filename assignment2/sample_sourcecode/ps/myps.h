typedef struct
{
    char filename[280];
    int major, minor;
} DEV;

typedef struct
{
    long cpu_total;
    double cpu_user, cpu_nice, cpu_system, cpu_idle;
    double cpu_wa, cpu_hi, cpu_si, cpu_st;
} CPU_USING;

typedef struct
{
    long mem_total, mem_free, mem_used, mem_bufca, mem_avail;
    long swap_total, swap_free, swap_used;
} MEM_SWAP;

CPU_USING getCpuUsing();
MEM_SWAP getMemSwap();
void getCmdline(char *file, char *buf);