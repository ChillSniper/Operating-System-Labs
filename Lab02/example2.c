// file: example32.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>

// —— 任务结构体 —— 
typedef struct {
    char        task_id;
    int         ci, ti;           // 最坏执行时间 Ci，周期 Ti
    int         ci_left, ti_left; // 剩余执行时间／剩余到下周期时间
    int         flag;             // 活跃标志：2=可执行，0=本周期已完成
    int         call_num;         // 周期调用计数
    int         arg;              // 线程参数索引
    pthread_t   th;               // 对应线程
} task_t;

// —— 全局变量 —— 
task_t         *tasks;
int             task_num   = 3;    // 任务数改为 3
int             curr_proc  = -1;
int             demo_time  = 120;  // 演示时间设为 120 单位（对应 ms）
int             alg;               // 1=EDF, 2=RMS
int             idle_num   = 0;
float           sum_util   = 0;
pthread_mutex_t proc_wait[100];
pthread_mutex_t main_wait, idle_wait;
pthread_t       idle_proc;

// —— 空闲线程：打印 “->” 表示一个 time slot 空闲 —— 
void* idle(void* _) {
    while (1) {
        pthread_mutex_lock(&idle_wait);
        printf("->");
        idle_num++;
        pthread_mutex_unlock(&main_wait);
    }
    return NULL;
}

// —— 任务线程：打印 “<id><周期号>” 并消耗一个 time slot —— 
void* proc(void* arg_void) {
    int idx = *(int*)arg_void;
    while (tasks[idx].ci_left > 0) {
        pthread_mutex_lock(&proc_wait[idx]);
        if (idle_num > 0) {
            printf("idle(%d)", idle_num);
            idle_num = 0;
        }
        printf("%c%d", tasks[idx].task_id, tasks[idx].call_num);
        tasks[idx].ci_left--;
        if (tasks[idx].ci_left == 0) {
            // 本周期执行完，打印 "(Ci)" 并换行
            printf("(%d)\n", tasks[idx].ci);
            tasks[idx].flag = 0;
            tasks[idx].call_num++;
        }
        pthread_mutex_unlock(&main_wait);
    }
    return NULL;
}

// —— 调度决策：EDF 选 ci_left 最小，RMS 选 ti 最小 —— 
int select_proc(int alg_sel) {
    int best = -1, tmp = 100000;
    // RMS 不抢占已在本周期运行的任务
    if (alg_sel == 2 && curr_proc!=-1 && tasks[curr_proc].flag==2)
        return curr_proc;
    for (int j = 0; j < task_num; j++) {
        if (tasks[j].flag == 2) {
            if (alg_sel == 1) {                     // EDF
                if (tasks[j].ci_left < tmp) {
                    tmp = tasks[j].ci_left; best = j;
                }
            } else {                                // RMS
                if (tasks[j].ti < tmp) {
                    tmp = tasks[j].ti;    best = j;
                }
            }
        }
    }
    return best;
}

// —— 模拟函数 —— 
void simulate(int alg_sel) {
    alg = alg_sel;
    double r = 1.0;
    if (alg == 2) {
        // RMS 理论可调度上限 r = n*(2^(1/n)-1)
        r = task_num * (pow(2.0, 1.0/task_num) - 1.0);
        printf("\nRMS 上界 r = %.6f, 总利用率 = %.6f\n", r, sum_util);
    } else {
        printf("\nEDF 总利用率 = %.6f\n", sum_util);
    }
    if (sum_util > r) {
        printf("(sum=%.6f > r=%.6f)，不可调度！\n", sum_util, r);
        return;
    }

    // 关闭 stdout 缓冲，保证每次 printf 都立刻输出
    setvbuf(stdout, NULL, _IONBF, 0);

    // 初始化主线程/空闲线程锁
    pthread_mutex_init(&main_wait, NULL);
    pthread_mutex_lock(&main_wait);
    pthread_mutex_init(&idle_wait, NULL);
    pthread_mutex_lock(&idle_wait);

    // 启动空闲线程
    pthread_create(&idle_proc, NULL, idle, NULL);

    // 为每个任务创建第一个周期线程
    for (int i = 0; i < task_num; i++) {
        pthread_mutex_init(&proc_wait[i], NULL);
        pthread_mutex_lock(&proc_wait[i]);
        pthread_create(&tasks[i].th, NULL, proc, &tasks[i].arg);
    }

    // 模拟 demo_time 个 time slot
    for (int t = 0; t < demo_time; t++) {
        if ((curr_proc = select_proc(alg)) != -1) {
            pthread_mutex_unlock(&proc_wait[curr_proc]);
            pthread_mutex_lock(&main_wait);
        } else {
            pthread_mutex_unlock(&idle_wait);
            pthread_mutex_lock(&main_wait);
        }
        // 周期到期：重置 ci_left/ti_left，标记活跃，并 spawn 新线程
        for (int j = 0; j < task_num; j++) {
            if (--tasks[j].ti_left == 0) {
                tasks[j].ti_left = tasks[j].ti;
                tasks[j].ci_left = tasks[j].ci;
                tasks[j].flag    = 2;
                pthread_mutex_init(&proc_wait[j], NULL);
                pthread_mutex_lock(&proc_wait[j]);
                pthread_create(&tasks[j].th, NULL, proc, &tasks[j].arg);
            }
        }
    }
    printf("\n");
}

int main() {
    // 禁用 stdout 缓冲
    setvbuf(stdout, NULL, _IONBF, 0);

    // 分配并初始化任务数组：A(10,30)、B(15,40)、C(5,50)
    tasks = malloc(sizeof(task_t) * task_num);
    if (!tasks) { perror("malloc"); exit(EXIT_FAILURE); }
    char ids[] = {'A','B','C'};
    int  cis[] = {10, 15, 5};
    int  tis[] = {30, 40, 50};
    for (int i = 0; i < task_num; i++) {
        tasks[i].task_id  = ids[i];
        tasks[i].ci       = cis[i];
        tasks[i].ti       = tis[i];
        tasks[i].ci_left  = cis[i];
        tasks[i].ti_left  = tis[i];
        tasks[i].flag     = 2;
        tasks[i].call_num = 1;
        tasks[i].arg      = i;
        sum_util += (float)cis[i] / tis[i];
    }

    printf("=== 习题32: {A(10,30), B(15,40), C(5,50)}, 演示时间 = %d ===\n", demo_time);
    printf(">>> 调用 EDF:\n");
    simulate(1);
    printf(">>> 调用 RMS:\n");
    simulate(2);

    return 0;
}
