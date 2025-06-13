#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#define COLLISIONFACTOR 0.5  //Hash函数冲突因子
#define HASH_FILE_MAGIC_NUM 21230912

struct HashFileHeader
{
    int sig;  //Hash文件印鉴
    int reclen;  //记录长度
    int total_rec_num;  //总记录数
    int current_rec_num;  //当前记录数
};

struct SlotMeta
{
    char collision;  //冲突计数
    char free;  //空闲标志
};
int hashfile_creat(const char *filename, mode_t mode, int reclen, int recnum);
//int hashfile_open(const char *filename, int flags);
int hashfile_open(const char *filename, int flags, mode_t mode);
int hashfile_close(int fd);
int hashfile_read(int fd, int keyoffset, int keylen, void *buf);
int hashfile_write(int fd, int keyoffset, int keylen, void *buf);
int hashfile_delrec(int fd, int keyoffset, int keylen, void *buf);
int hashfile_findrec(int fd, int keyoffset, int keylen, void *buf);
int hashfile_saverec(int fd, int keyoffset, int keylen, void *buf);
int hash(int keyoffset, int keylen, void *buf, int recnum);
int checkHashFileFull(int fd);
int readHashFileHeader(int fd, struct HashFileHeader *hfh);
