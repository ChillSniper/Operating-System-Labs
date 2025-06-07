#include <stdio.h>
#include <stdlib.h>

#define MAPSIZE 100

struct map {
    int m_addr;   /* ��������ʼ��ַ */
    int m_size;   /* ��������С */
};

static struct map map_table[MAPSIZE];

/* �����Ӧ��Best�\Fit������ */
int BF_malloc(struct map *mp, int size) {
    struct map *best = NULL, *p;
    int best_size = 0;

    for (p = mp; p->m_size != 0; p++) {
        if (p->m_size >= size) {
            if (best == NULL || p->m_size < best_size) {
                best = p;
                best_size = p->m_size;
            }
        }
    }
    if (best == NULL) {
        return -1;
    }

    int addr = best->m_addr;
    best->m_addr += size;
    best->m_size -= size;

    if (best->m_size == 0) {
        /* ɾ������ */
        struct map *q = best;
        do {
            q[0] = q[1];
            q++;
        } while (q->m_size != 0);
        /* ���ҲҪ������β�� m_size=0 */
        q[0] = q[1];
    }

    return addr;
}

/* ���Ӧ��Worst�\Fit������ */
int WF_malloc(struct map *mp, int size) {
    struct map *worst = NULL, *p;
    int worst_size = 0;

    for (p = mp; p->m_size != 0; p++) {
        if (p->m_size >= size) {
            if (worst == NULL || p->m_size > worst_size) {
                worst = p;
                worst_size = p->m_size;
            }
        }
    }
    if (worst == NULL) {
        return -1;
    }

    int addr = worst->m_addr;
    worst->m_addr += size;
    worst->m_size -= size;

    if (worst->m_size == 0) {
        /* ɾ������ */
        struct map *q = worst;
        do {
            q[0] = q[1];
            q++;
        } while (q->m_size != 0);
        q[0] = q[1];
    }

    return addr;
}

/* �ͷŲ��ϲ������� */
void mfree(struct map *mp, int aa, int size) {
    struct map *p = mp;
    /* �ҵ���һ�� m_addr > aa */
    while (p->m_size != 0 && p->m_addr <= aa) {
        p++;
    }

    /* ������ǰһ��ϲ� */
    if (p > mp && (p - 1)->m_addr + (p - 1)->m_size == aa) {
        (p - 1)->m_size += size;
        /* �ٴγ������һ��ϲ� */
        if (p->m_size != 0 && aa + size == p->m_addr) {
            (p - 1)->m_size += p->m_size;
            /* ɾ�� p ���� */
            struct map *q = p;
            do {
                q[0] = q[1];
                q++;
            } while (q->m_size != 0);
            q[0] = q[1];
        }
    }
    /* ������ǰһ��ϲ� */
    else if (p->m_size != 0 && aa + size == p->m_addr) {
        /* ���һ��ϲ� */
        p->m_addr = aa;
        p->m_size += size;
    }
    else {
        /* ����һ���±��� */
        int new_addr = aa, new_size = size;
        while (p->m_size != 0) {
            /* ���� p ���¿� */
            int tmp_addr = p->m_addr;
            int tmp_size = p->m_size;
            p->m_addr = new_addr;
            p->m_size = new_size;
            new_addr = tmp_addr;
            new_size = tmp_size;
            p++;
        }
        /* �����β��־ */
        p->m_addr = new_addr;
        p->m_size = new_size;
        p++;
        p->m_size = 0;
    }
}

/* ��ʼ�� */
void init_map(void) {
    int addr, size;
    printf("Please input starting addr and total size (��ʽ����ʼ,��С): ");
    if (scanf("%d,%d", &addr, &size) != 2) {
        fprintf(stderr, "�����ʽ�������� ����ʼ��ַ,��С�� ����������\n");
        exit(EXIT_FAILURE);
    }
    map_table[0].m_addr = addr;
    map_table[0].m_size = size;
    map_table[1].m_size = 0;  /* ��β */
}

/* ��ʾ��ǰ�������� */
void show_map(void) {
    printf("\nCurrent memory map:\n");
    printf(" Address\tSize\n");
    for (struct map *p = map_table; p->m_size != 0; p++) {
        printf(" <%5d\t%5d>\n", p->m_addr, p->m_size);
    }
    printf("\n");
}

int main(void) {
    init_map();

    char strategy;
    printf("Please input strategy: b for BF, w for WF: ");
    if (scanf(" %c", &strategy) != 1 ||
        (strategy != 'b' && strategy != 'w')) {
        fprintf(stderr, "Error: expected 'b' or 'w'.\n");
        return EXIT_FAILURE;
    }

    while (1) {
        show_map();

        printf("Please input command: 1=request, 2=release, 0=exit: ");
        int cmd;
        if (scanf("%d", &cmd) != 1) {
            fprintf(stderr, "Invalid input.\n");
            break;
        }

        if (cmd == 1) {
            int sz;
            printf("Please input size to allocate: ");
            if (scanf("%d", &sz) != 1) {
                fprintf(stderr, "Invalid size input.\n");
                break;
            }
            int addr = (strategy == 'b')
                           ? BF_malloc(map_table, sz)
                           : WF_malloc(map_table, sz);
            if (addr < 0) {
                printf("Request cannot be satisfied.\n");
            } else {
                printf("Allocated at address %d, size %d.\n", addr, sz);
            }
        }
        else if (cmd == 2) {
            int addr, sz;
            printf("Please input addr and size to release (��ʽ����ַ,��С): ");
            if (scanf("%d,%d", &addr, &sz) != 2) {
                fprintf(stderr, "Invalid input format.\n");
                break;
            }
            mfree(map_table, addr, sz);
        }
        else if (cmd == 0) {
            printf("Exiting.\n");
            break;
        }
        else {
            printf("Unknown command: %d\n", cmd);
        }
    }

    return EXIT_SUCCESS;
}
