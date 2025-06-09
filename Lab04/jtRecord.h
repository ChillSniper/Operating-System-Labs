#define RECORDLEN 32
// ÒµÎñ¼ÇÂ¼
struct jtRecord
{
    int key;
    char other[RECORDLEN-sizeof(int)];
};
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
