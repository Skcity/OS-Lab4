### 利用命令行进行输入

###### 首先编译 `creatFile.cpp` 文件,得到磁盘文件

```
$ creatFile.exe <磁盘名称>
```

###### 其次编译main.cpp,指令格式如下

```
$ mian.exe <磁盘名称> <操作方式> <文件名称> <文件大小/文件块号>

操作方式:
C 创建文件 
D 删除文件
L 遍历文件
R 访问文件
W 写入文件
```

