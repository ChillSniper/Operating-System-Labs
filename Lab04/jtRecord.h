#define RECORDLEN 32
// ҵ���¼
struct jtRecord
{
    int key;
    char other[RECORDLEN-sizeof(int)];
};
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
