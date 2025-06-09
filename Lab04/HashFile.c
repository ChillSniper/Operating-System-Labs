#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "HashFile.h"

// mode_t类型用来表示文件的权限和模式位
int hashfile_creat(const char *filename, mode_t mode, int reclen, int total_rec_num){
    struct HashFileHeader cur;
    int fd, WrittenBytesNumber = 0;
    char *buffer;
    cur.sig = HASH_FILE_MAGIC_NUM;
    cur.reclen = reclen;
    cur.current_rec_num = 0;
    cur.total_rec_num = total_rec_num;

    fd = creat(filename, mode);
    if(fd != -1){
        WrittenBytesNumber = write(fd, &cur, sizeof(struct HashFileHeader));
        if(WrittenBytesNumber != -1){
            int TargetLen = total_rec_num * (reclen + sizeof(struct SlotMeta));
            buffer = (char*)malloc(TargetLen);
            memset(buffer, 0, TargetLen);
            WrittenBytesNumber = write(fd, buffer, TargetLen);
            free(buffer);
        }
        close(fd);
        return WrittenBytesNumber;
    }
    else{
        close(fd); // remember to close it!
        return -1;
    }
}

int hashfile_open(const char *filename, int flags, mode_t mode){
    int fd = open(filename, flags, mode);
    struct HashFileHeader cur;
    int val = read(fd, &cur, sizeof(struct HashFileHeader));
    if(val != -1){
        lseek(fd, 0, SEEK_SET);
        if(cur.sig == HASH_FILE_MAGIC_NUM){
            return fd;
        }
        else{
            return -1;
        }
    }
    else{
        return -1;
    }
}

int hashfile_close(int fd)
{
    return close(fd);
}

// the effect: based on the offed key and reply the info to the buffer.
int hashfile_read(int fd, int key_offset, int key_len, void *buffer){
    struct HashFileHeader cur;
    readHashFileHeader(fd, &cur);
    int offset = hashfile_findrec(fd, key_offset, key_len, buffer);
    if(offset != -1){
        lseek(fd, offset + sizeof(struct SlotMeta), SEEK_SET);
        return read(fd, buffer, cur.reclen);
    }
    else{
        return -1;
    }
}

int hashfile_write(int fd,int keyoffset,int keylen,void *buf)
{
    return hashfile_saverec(fd,keyoffset,keylen,buf);
    //return -1;
}

int hashfile_delrec(int fd, int key_offset, int key_len, void* buffer){
    int offset = hashfile_findrec(fd, key_offset, key_len, buffer); // 找到键值记录所在槽在文件中的字节偏移

    if(offset != -1){
        struct SlotMeta tag;
        read(fd, &tag, sizeof(struct SlotMeta));
        tag.free = 0;
        lseek(fd, offset, SEEK_SET);
        write(fd, &tag, sizeof(struct SlotMeta));
        
        struct HashFileHeader cur;
        readHashFileHeader(fd, &cur);
        int address = hash(key_offset, key_len, buffer, cur.total_rec_num);
        offset = sizeof(struct HashFileHeader) + address * (cur.reclen + sizeof(struct SlotMeta));
        if(lseek(fd, offset, SEEK_SET) == -1){
            return -1;
        }
        read(fd, &tag, sizeof(struct SlotMeta));
        tag.collision -= 1;
        lseek(fd, offset, SEEK_SET);
        write(fd, &tag, sizeof(struct SlotMeta));
        cur.current_rec_num -= 1;
        lseek(fd, 0, SEEK_SET);
        return write(fd, &cur, sizeof(struct HashFileHeader));
    }
    else {
        return -1;
    }
}

int hashfile_findrec(int fd, int key_offset, int key_len, void *buffer){
    struct HashFileHeader cur;
    readHashFileHeader(fd, &cur);
    int address = hash(key_offset, key_len, buffer, cur.total_rec_num);
    int offset = sizeof(struct HashFileHeader) + address * (cur.reclen + sizeof(struct SlotMeta));
    if(lseek(fd, offset, SEEK_SET) == -1){
        return -1;
    }

    struct SlotMeta tag;
    read(fd, &tag, sizeof(struct SlotMeta));
    char collision_count = tag.collision;
    if(collision_count == 0){
        return -1;
    }
    
    while(1){

        if(tag.free == 0){
            offset += cur.reclen + sizeof(struct SlotMeta);
            if(lseek(fd, offset, SEEK_SET) == -1){
                return -1;
            }
            read(fd, &tag, sizeof(struct SlotMeta));

        }
        else{
            char *p = (char*)malloc(cur.reclen * sizeof(char));
            read(fd, p, cur.reclen);
            char *p1, *p2;
            p1 = (char*)buffer + key_offset;
            p2 = p             + key_offset;
            int cnt = 0;
            while ((*p1 == *p2) && cnt < key_len)
            {
                ++ cnt;
                ++ p1;
                ++ p2;
            }
            if(cnt == key_len){
                free(p);
                p = NULL;
                return offset;
            }
            else{
                // 此处实际上是在对哈希碰撞进行讨论
                int cur_hash_val = hash(key_offset, key_len, p, cur.total_rec_num);
                if(address == cur_hash_val){
                    -- collision_count;
                    if(collision_count == 0){
                        free(p);
                        p = NULL;
                        return -1;
                    }
                }
                free(p);
                p = NULL;
                offset += cur.reclen + sizeof(struct SlotMeta);
                if(lseek(fd, offset, SEEK_SET) == -1){
                    return -1;
                }
                read(fd, &tag, sizeof(struct SlotMeta));
            }   
        }
    }

}

int hashfile_saverec(int fd, int key_offset, int key_len, void *buffer){
    if(checkHashFileFull(fd)){
        return -1;
    }
    struct HashFileHeader cur;
    readHashFileHeader(fd, &cur);
    int address = hash(key_offset, key_len, buffer, cur.total_rec_num);
    int offset = sizeof(struct HashFileHeader) + address * (cur.reclen + sizeof(struct SlotMeta));
    if(lseek(fd, offset, SEEK_SET) == -1){
        return -1;
    }
    struct SlotMeta tag;
    read(fd, &tag, sizeof(struct SlotMeta));
    tag.collision ++;
    // 对当前文件偏移量进行回退，因为刚刚读取tag向前偏移了一个Tag的文件量
    lseek(fd, sizeof(struct SlotMeta) * (-1), SEEK_CUR);
    write(fd, &tag, sizeof(struct SlotMeta));

    while(tag.free != 0){ // 进行线性探查
        offset += cur.reclen + sizeof(struct SlotMeta);
        if(offset >= lseek(fd, 0, SEEK_END)){
            offset = sizeof(struct HashFileHeader);
        }
        if(lseek(fd, offset, SEEK_SET) == -1){
            return -1;
        }
        read(fd, &tag, sizeof(struct SlotMeta));
    }
    tag.free = 1;
    lseek(fd, sizeof(struct SlotMeta) * -1, SEEK_CUR);
    write(fd, &tag, sizeof(struct SlotMeta));
    write(fd, buffer, cur.reclen);
    cur.current_rec_num += 1;
    lseek(fd, 0, SEEK_SET);
    return write(fd, &cur, sizeof(struct HashFileHeader));
}

int hash(int key_offset, int key_len, void *buffer, int total_rec_num){
    char *p = (char*)buffer + key_offset;
    int sum_of_address = 0;
    for(int i = 0;i < key_len;i ++){
        sum_of_address += (int)(*p);
        ++ p;
    }
    int mod_num = (int)(total_rec_num * COLLISIONFACTOR);
    if(mod_num <= 0){
        mod_num = -(mod_num - total_rec_num);
    }
    return sum_of_address % mod_num;
}

int readHashFileHeader(int fd, struct HashFileHeader *cur){
    lseek(fd, 0, SEEK_SET);
    return read(fd, cur, sizeof(struct HashFileHeader));
}

// pay attention to judge "IsFull" !
int checkHashFileFull(int fd){
    struct HashFileHeader cur;
    readHashFileHeader(fd, &cur);
    if(cur.current_rec_num < cur.total_rec_num){
        return 0;
    }
    else{
        return 1;
    }
}