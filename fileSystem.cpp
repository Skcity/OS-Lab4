#include "fileSystem.h"
#include <math.h> 
#include <vector>
#include <iostream>
#include <signal.h>

using namespace std;
typedef struct iNode iNode;

/*构造函数*/
myFileSystem::myFileSystem(char diskName[16]) {
  this->disk.open(diskName, ios::out | ios::in);// 打开之前创建的磁盘
}

/**********************创建文件************************/
int myFileSystem::create_file(char name[8], int size) {
    if (!this->disk.is_open()){
        cout<<"error disk not open";
        return -1;}

    if (size > 8 || size < 1){
        cout<<"size too large";
        return -1;}

    if (strlen(name) > 8 || strlen(name) == 0){
        cout<<"invalid name";
        return -1; }

    cout << "Attempting to create file: " << name << endl;

    /*首先检查磁盘上是否有足够的空间*/
    this->disk.sync();              //清空缓冲区
    this->disk.seekg(0, disk.beg);  //移动文件指针到磁盘头部
    char buf_free[128];
    this->disk.read(buf_free, 128); //读取磁盘头部的空闲块表

    vector<int> blocks_free;        // 收集空闲块的容器
    //扫描空闲块表
    for (int i = 1; i < 128; i ++){
        if (buf_free[i] == 0)
        blocks_free.push_back(i);
        if (blocks_free.size() >= (size_t)size)
        break;
    }
    //如果没有合适空闲块
    if (blocks_free.size() == 0 || blocks_free.size() < (size_t)size) {
        cout << "Error: Insufficient free blocks available!\n";
        return -1; 
    }

    /*查找空闲i节点*/
    iNode found_inode;
    int inode_pos = -1;
    //由题目假设可知第i个i节点位置在128+48*i
    for(int i = 0; i < 16; i ++){ 
    char tmp_buf[48];
    //磁盘继续向前读取inode
    this->disk.read(tmp_buf, 48);
    //拷贝磁盘内容到found_node,便于查看信息
    memcpy(&found_inode, tmp_buf, sizeof(found_inode));
    //检查该inode是否被使用
    if (found_inode.used == 0){
        found_inode.used = 1;
        inode_pos = i;
        break;
    }
    //检查是否重名
    else if(strncmp(name, found_inode.name, 8) == 0){
        cout << "Error: file with the same name exists!\n";
        return -1;
    } 
    else if (i == 15){
        cout << "Error: no free inodes!\n";
        return -1; 
    }
    }
    //拷贝文件名到i节点中
    strncpy(found_inode.name, name, 8); 
    //拷贝文件大小到i节点中
    found_inode.size = size; 

    /*给文件分配数据块(inode数据块指针)*/

    for (int i = 0; i < size; i ++){
        found_inode.blockPointers[i] = blocks_free[i];
        buf_free[blocks_free[i]] = 1;
    }

    /*将i节点与空闲块表写回磁盘*/
    this->disk.seekp(0, disk.beg); //移动文件指针至磁盘首部
    this->disk.write(buf_free, 128); // 将空闲列表写回
    this->disk.seekp(48 * inode_pos, disk.cur); // 移动文件指针至inode首部
    //类型转换
    char tmp_buf[48];
    memcpy(tmp_buf, &found_inode, sizeof(found_inode)); 

    this->disk.write(tmp_buf, 48); // 将inode写回磁盘
    this->disk.flush();
    return 1;
} 

/**********************删除文件************************/
int myFileSystem::delete_file(char name[8]){
    if (!this->disk.is_open()){
        cout<<"Error:Disk open failed!\n";
        return -1;
    }
    cout << "Deleting file : " << name << endl; //error disk not open

    /*定位对应的inode*/
    this->disk.sync();                  //清除缓冲区
    this->disk.seekg(128, disk.beg);    //文件指针回到inode首部
    iNode found_inode;
    int inode_pos = -1;
    //读取磁盘inode部分
    for(int i = 0; i < 16; i ++){
        char tmp_buf[48];
        this->disk.read(tmp_buf, 48);
        memcpy(&found_inode, tmp_buf, sizeof(found_inode));//"类型转换"
        //扫描inode
        if (found_inode.used == 1 && strncmp(found_inode.name, name, 8) == 0 && i != 15){ 
            inode_pos = i; 
            break;
        } 
        else if (i == 15){
            if (found_inode.used && strncmp(found_inode.name, name, 8) == 0){
                inode_pos = i; 
                break;
            } 
            else {
                cout << "Error: no matching inode, all nodes scanned!\n";
                return -1;
            }
        }
    }

    /*释放inode指针指向的数据块空间*/
    this->disk.seekg(0, disk.beg); 
    char buf_free[128];
    this->disk.read(buf_free, 128); 
    for (int i = 0; i < found_inode.size; i ++){
        buf_free[found_inode.blockPointers[i]] = 0; 
    }

    /*释放inode空间*/
    found_inode.used = 0;
    this->disk.seekp(0, disk.beg); 
    this->disk.write(buf_free, 128);
    this->disk.seekp(48 * inode_pos, disk.cur); 
    char tmp_buf[48];
    memcpy(tmp_buf, &found_inode, sizeof(found_inode)); 
    this->disk.write(tmp_buf, 48); 
    this->disk.flush();
    return 1;
} 

/**********************遍历文件信息************************/
int myFileSystem::ls(){
    if (!this->disk.is_open()){
        cout<<"Error:Disk open failed!\n";
        return -1;
    }
    this->disk.sync();
    this->disk.seekg(128, disk.beg);
    cout << "Listing INodes..." << endl;

    //查找非空inode,输出信息
    for(int i = 0; i < 16; i ++){
        iNode found_inode;
        char tmp_buf[48];
        this->disk.read(tmp_buf, 48);
        memcpy(&found_inode, tmp_buf, sizeof(found_inode));
        if (found_inode.used == 1){ 
        cout << "INode: " << found_inode.name << " size: " << found_inode.size << endl;
        }     
    }
    return 1;
}

/**********************读取文件************************/
int myFileSystem::read(char name[8], int blockNum, char buf[1024]){
    if (!this->disk.is_open()){
        cout<<"Error:Disk open failed!\n";
        return -1;
    }
    cout << "Reading file : " << name << " blockNum: " << blockNum << endl;

    this->disk.sync();
    this->disk.seekg(128, disk.beg);
    iNode found_inode;
    int inode_pos = -1;
    //查找inode
    for(int i = 0; i < 16; i ++){
        char tmp_buf[48];
        this->disk.read(tmp_buf, 48);
        memcpy(&found_inode, tmp_buf, sizeof(found_inode));
        if (found_inode.used == 1 && strncmp(found_inode.name, name, 8) == 0 && i != 15){ 
            inode_pos = i; 
            break;
        } 
        else if (i == 15){
            if (found_inode.used && strncmp(found_inode.name, name, 8) == 0){
                inode_pos = i; 
                break;
            } 
            else {
                cout << "Error in read_file, no matching inode, all nodes scanned\n";
                return -1;
            } 
        }
    }
    if (blockNum >= found_inode.size){
        cout << "Error: blocknum greater than size of inode!\n";
        return -1;
    }
    //计算offset
    int offset = found_inode.blockPointers[blockNum] * block_size;
    cout << "Read pointer: " << found_inode.blockPointers[blockNum] << " offset: " << offset << endl;
    this->disk.seekg(offset, disk.beg);
    this->disk.read(buf, block_size);
    return 1;
}

/**********************写入文件************************/
int myFileSystem::write(char name[8], int blockNum, char buf[1024]){
    if (!this->disk.is_open()){
        cout<<"Error:Disk open failed!\n";
        return -1;
    }
    cout << "Reading file : " << name << " blockNum: " << blockNum << endl;
    this->disk.sync();
    this->disk.seekg(128, disk.beg);

    iNode found_inode;
    int inode_pos = -1;
    //查找inode
    for(int i = 0; i < 16; i ++){   
        char tmp_buf[48];
        this->disk.read(tmp_buf, 48);
        memcpy(&found_inode, tmp_buf, sizeof(found_inode));
        if (found_inode.used == 1 && strncmp(found_inode.name, name, 8) == 0 && i != 15){ 
            inode_pos = i; 
            break;
        } 
        else if (i == 15){
            if (found_inode.used && strncmp(found_inode.name, name, 8) == 0){
                inode_pos = i; 
                break;
            } 
            else {
                cout << "Error in read_file, no matching inode, all nodes scanned\n";
                return -1;
            } 
        }
    }
    if (blockNum >= found_inode.size){
        cout << "Error requested blocknum greater than size of inode.\n";
        return -1;
    }
    int offset = found_inode.blockPointers[blockNum] * block_size;
    cout << "Write pointer: " << found_inode.blockPointers[blockNum] << " offset: " << offset << endl;
    this->disk.seekp(offset, disk.beg);
    this->disk.write(buf, block_size);
    this->disk.flush();
    return 1;
} 

int myFileSystem::close_disk(){
    disk.close();
    return 1;
}