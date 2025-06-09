#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUF_LEN 50

int main(void)
{
    int fd;
    char buf[BUF_LEN] = "pear to dev!";

    printf("program test is running!\n");

    fd = open("/dev/mydev", O_RDWR);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    write(fd, buf, strlen(buf));
    printf("write \"%s\"\n", buf);

    strcpy(buf, "apple to dev!");
    printf("buffer is changed to \"%s\"\n", buf);

    memset(buf, 0, BUF_LEN);
    read(fd, buf, BUF_LEN);
    printf("read from dev is \"%s\"\n", buf);

    close(fd);
    return 0;
}
