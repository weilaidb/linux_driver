
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


/**
 * argc:
 * argv:
 * ./chrdevbaseApp <filename> <1:2> 1表示读，2表示写 <写的数据>
 * ./chrdevbaseApp <filename> 1 表示从驱动里面读数据
 * ./chrdevbaseApp <filename> 2 writestring 表示向驱动里面写数据
 **/
int main(int argc, char const *argv[])
{
    int ret = 0;
    int fd = 0;
    const char *filename;
    char *pWritebuf = "Hello, world!";
    char readbuf[100];

    if(argc != 4) {
        printf("Invalid parameter\r\n");
        return -1;
    }


    filename = argv[1];
    pWritebuf = argv[3];


    fd = open(filename, O_RDWR);
    if(fd < 0) {
        printf("Can't open file %s\r\n", filename);
        return -1;
    }

    if(atoi(argv[2]) == 1) /*读*/
    {
        ret = read(fd, readbuf, sizeof(readbuf));
        if(ret < 0) {
            printf("Read file %s failed\r\n", filename);
            return -1;
        }
        printf("Read file %s success, data is %s\r\n", filename, readbuf);
    }
    else if(atoi(argv[2]) == 2) /*写*/
    {
        //ret = write(fd, writebuf, sizeof(writebuf));
        ret = write(fd, pWritebuf, strlen(pWritebuf));
        if(ret < 0) {
            printf("Write file %s failed\r\n", filename);
            return -1;
        }
        printf ("Write file %s success\r\n", filename);
    }
    else
    {
        printf("Invalid parameter\r\n");
    }

    ret = close(fd);
    if(ret < 0) {
        printf("Close file %s failed\r\n", filename);
    }
    return 0;
}
