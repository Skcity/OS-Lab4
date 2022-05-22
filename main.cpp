#include "fileSystem.h"

int main(int argc, char *argv[]){

    if (argc == 1 ){
        fprintf(stderr,"usage: %s <文件名> \n",argv[0]);
        exit(0);
    }
    myFileSystem f ((char *)argv[1]); // 创建磁盘对象
    char* cmd[3]; 

    //构造满的块，便于后续访问非空数据块
    char buff[1024];
    for (int i = 0; i < 1024; i++){
        buff[i] = '1';
    };

    //处理输入命令行
    cmd[0]=argv[2];//操作指令
    cmd[1]=argv[3];//文件名
    cmd[2]=argv[4];//大小或者块号

    switch (*cmd[0]){
    case 'C':{
        f.create_file((char*)cmd[1],atoi(cmd[2]));
        break;
    }
    case 'D':{
        f.delete_file((char*)cmd[1]);
        break;
    }
    case 'L':{
        f.ls();
        break;
    }
    case 'R':{
        f.read((char*)cmd[1],atoi(cmd[2]),buff);
        break;
    }
    case 'W':{
        f.write((char*)cmd[1],atoi(cmd[2]),buff);
        break;
    }
    default:{
        printf("%s\n",cmd[0]);
        break;
    }
    }
    f.close_disk();
    return 0;
}
