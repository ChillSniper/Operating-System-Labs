// file: example1.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>

// ���� ����ṹ�� ���� 
typedef struct {
    char        task_id;
    int         ci, ti;           // �ִ��ʱ�� Ci������ Ti
    int         ci_left, ti_left; // ʣ��ִ��ʱ�䣯ʣ�ൽ������ʱ��
    int         flag;             // ��Ծ��־��2=��ִ�У�0=�����������
    int         call_num;         // ���ڵ��ü���
    int         arg;              // �̲߳�������
    pthread_t   th;               // ��Ӧ�߳�
} task_t;

// ȫ�ֱ��� 
task_t         *tasks;
int             task_num = 2;
int             curr_proc = -1;
int             demo_time = 200;
int             alg;            // 1=EDF, 2=RMS
int             idle_num = 0;
float           sum_util = 0;
pthread_mutex_t proc_wait[100];
pthread_mutex_t main_wait, idle_wait;
pthread_t       idle_proc;

// �����̣߳���ӡ ��->�� ��ʾһ�� time slot ���� ���� 
void* idle(void* _) {
    while (1) {
        pthread_mutex_lock(&idle_wait);
        printf("->");
        idle_num++;
        pthread_mutex_unlock(&main_wait);
    }
    return NULL;
}

// �����̣߳���ӡ ��<id><���ں�>�� ������һ�� time slot ���� 
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
            printf("(%d)\n", tasks[idx].ci);
            tasks[idx].flag = 0;
            tasks[idx].call_num++;
        }
        pthread_mutex_unlock(&main_wait);
    }
    return NULL;
}

// ���Ⱦ��ߣ�EDF ѡ ci_left ��С��RMS ѡ ti ��С ���� 
int select_proc(int alg_sel) {
    int best = -1, tmp = 100000;
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

// ģ�⺯�� 
void simulate(int alg_sel) {
    alg = alg_sel;
    double r = 1.0;
    if (alg == 2) {
        // RMS ���ۿɵ�������
        r = task_num * (pow(2.0, 1.0/task_num) - 1.0);
        printf("\nRMS �Ͻ� r = %.6f, �������� = %.6f\n", r, sum_util);
    } else {
        printf("\nEDF �������� = %.6f\n", sum_util);
    }
    if (sum_util > r) {
        printf("(sum=%.6f > r=%.6f)�����ɵ��ȣ�\n", sum_util, r);
        return;
    }

    // �ر� stdout ���壬��֤ÿ�� printf ���������
    setvbuf(stdout, NULL, _IONBF, 0);

    // ��ʼ�����߳�/�����߳���
    pthread_mutex_init(&main_wait, NULL);
    pthread_mutex_lock(&main_wait);
    pthread_mutex_init(&idle_wait, NULL);
    pthread_mutex_lock(&idle_wait);

    // ���������߳�
    pthread_create(&idle_proc, NULL, idle, NULL);

    // Ϊÿ�����񴴽���һ�����ڵ��߳�
    for (int i = 0; i < task_num; i++) {
        pthread_mutex_init(&proc_wait[i], NULL);
        pthread_mutex_lock(&proc_wait[i]);
        pthread_create(&tasks[i].th, NULL, proc, &tasks[i].arg);
    }

    // ģ�� demo_time �� time slot
    for (int t = 0; t < demo_time; t++) {
        if ((curr_proc = select_proc(alg)) != -1) {
            pthread_mutex_unlock(&proc_wait[curr_proc]);
            pthread_mutex_lock(&main_wait);
        } else {
            pthread_mutex_unlock(&idle_wait);
            pthread_mutex_lock(&main_wait);
        }
        // ���ڵ��ڣ����� ci_left/ti_left����ǻ�Ծ���� spawn ���߳�
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
    // ���û��壬ȷ���������ʵʱ�ɼ�
    setvbuf(stdout, NULL, _IONBF, 0);

    // ���䲢��ʼ����������
    tasks = malloc(sizeof(task_t) * task_num);
    if (!tasks) { perror("malloc"); exit(EXIT_FAILURE); }

    char ids[] = {'a','b'};
    int  cis[] = {10, 25};
    int  tis[] = {20, 50};
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

    printf("=== �� 1: {a(10,20), b(25,50)}, ��ʾʱ�� = %d ===\n", demo_time);
    printf(">>> ���� EDF:\n");
    simulate(1);
    printf(">>> ���� RMS:\n");
    simulate(2);

    return 0;
}
