#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>

using namespace std;

//block size = 1KB
#define block_size 1024

//i节点
struct iNode{
  char name[8];         //文件名
  int size;             // 文件大小(用文件块表示)
  int blockPointers[8]; // 数据块指针
  int used=0;           // 节点是否空闲(初始化为0)
};

class myFileSystem
{
private:
  fstream disk;
public:
  myFileSystem(char diskName[16]);
  int create_file(char name[8], int size);
  int delete_file(char name[8]);
  int ls();
  int read(char name[8], int blockNum, char buf[1024]);
  int write(char name[8], int blockNum, char buf[1024]);
  int close_disk();
};