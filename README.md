# disk_scrub
A disk data erasure program based on Linux system

基于大佬的scrub项目编写，指路：https://github.com/chaos/scrub

## 1、目录结构

```
disk_scrub(Linux)
|_ src
  |_	aes.c
  |_	aes.h
  |_	filldentry.c
  |_	filldentry.h
  |_	fillfile.c
  |_	fillfile.h
  |_	genrand.c
  |_	genrand.h
  |_	getsize.c
  |_	getsize.h
  |_	hwrand.c
  |_	hwrand.h
  |_	pattern.c
  |_	pattern.h
  |_	progress.c
  |_	progress.h
  |_	scrub.c
  |_	scrub.h
  |_	disk_scrub.c
  |_	sig.c
  |_	sig.h
  |_	util.c
  |_	util.h
|_ 说明
  |_ picture
  |_Readme.md
|_ Makefile.am
|_ NEWS
|_ AUTHORS
|_ README
|_ ChangeLog
```

## 2、安装说明

使用automake工具进行编译，生成可执行程序。过程如下：
1、进入源文件目录使用`autoscan`命令，生成configure.scan
2、进入configure.scan文件，在AC_INIT宏之后加入AM_INIT_AUTOMAKE(disk_scrub, 1.0)，这里disk_scrub是你要编译成的软件的名称，1.0是版本号，即你的这些源程序编译将生成一个软件Test-1.0版。然后退出，把configure.scan文件改名为configure.in。
3、运行`aclocal`，生成aclocal.m4文件。
4、运行`autoconf`命令，生成Configure文件。
5、运行`autoheader`。
6、运行`automake --add-missing`命令。其中重点关注是否有`Makefile.am:error: required file './XXX' not found`提示，其中XXX为某些文件名。如果出现这样的提示，请自行创建该文件（亲测使用vim创建对应文件名的空文件即可）。否则在后面使用automake时会提示无法生成Makefile.in文件，导致后续过程无法继续。
7、运行`./Configure`命令，生成Makefile。
8、运行`make`命令，编译链接工程，生成可执行文件。

**Makefile.am已经编写完成，可以直接使用**

**如果想了解关于automake等工具的进一步使用，可以上网查阅资料，或者查看我的博客：https://hackerhome.top/index.php/archives/12/**

## 3、使用说明

编译完成后会生成disk_scrub可执行文件（具体编译完成的文件名由Makefile.am中指定，可以自行修改）。
使用命令`sudo ./disk_scrub`即可执行。**注意，程序执行需要root权限！**
运行后界面如下：

 [![运行图片](https://s3.ax1x.com/2021/02/22/yHXtMT.png)](https://imgchr.com/i/yHXtMT)

共有三种功能：
```
1、物理盘或逻辑盘数据擦除
2、逻辑盘剩余空间数据擦除
3、目录及文件数据擦除
```
输入序号即可选择功能。
第一种功能：

[![功能一](https://s3.ax1x.com/2021/02/22/yHXNsU.png)](https://imgchr.com/i/yHXNsU)

第二种功能：

[![功能二](https://s3.ax1x.com/2021/02/22/yHXJzV.png)](https://imgchr.com/i/yHXJzV)

第三种功能：

[![功能三](https://s3.ax1x.com/2021/02/22/yHXGR0.png)](https://imgchr.com/i/yHXGR0)

输入要擦除的磁盘或目录后，可以输入序号选择擦除算法。如输入1使用`nnsa`算法，该算法先对磁盘进行两次随机数写入，后对磁盘进行0x00写入，最后验证。或输入12使用`fillzero`算法，该算法对磁盘进行一次快速0x00写入等，如下：

[![算法选择](https://s3.ax1x.com/2021/02/22/yHXULF.png)](https://imgchr.com/i/yHXULF)

最后等待程序完成即可。
