/* 创建一个文件作为磁盘，并将其格式化 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  if (argc == 1 ){
    fprintf(stderr,"usage: %s <磁盘文件名称> \n",argv[0]);
    exit(0);
  }

  int i, fd;
  char *buf;

  printf( "创建了一个128KB的文件: %s\n",argv[1]);
  printf("这个文件将作为文件系统的模拟磁盘空间。\n");

  /* 打开文件 */
  fd = open(argv[1],O_WRONLY | O_CREAT | O_TRUNC,  S_IRUSR | S_IWUSR);
  printf("正在格式化磁盘......\n");
  buf = (char *) calloc(1024,sizeof(char));//大小KB
  
  /* 写入1KB的超级块 */
  buf[0]=1;  /* 超级块所在的块已占用，因此需要将空闲块链表的第一个字节置为1 */

  /* 将超级块写入到文件 */
  if(write(fd,buf, 1024)<0) printf("error: write failed \n");  
  buf[0]=0;

  /* 写入剩下的127个块，并置零 */
  for(i=0;i<127;i++){
    if(write(fd,buf,1024)<0)
      printf("error: write failed \n");  
  }
  close(fd);
  exit(1);
}