#include <stdio.h>
#include "scrub.h"
#include <fcntl.h>
#include <mntent.h> 
#include <linux/fs.h>  
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <math.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdbool.h>
static const unsigned long long G = 1024*1024*1024ull;
static const unsigned long long M = 1024*1024;
static const unsigned long long K = 1024;
char str[10]="\0";  //磁盘大小字符串
char path[512]="\0";  //磁盘挂载点目录
char *patterns[]={"nnsa","dod","bsi","usarmy","random","random2","schneier","pfitzner7","pfitzner33"
              ,"gutmann","dirent","fillzero","fillff","verify"}; //擦写算法

int choose_pattern()
{
    int num=12;
    printf("可用擦写算法:\n");
    printf("1\tnnsa\tNNSA NAP-14.1-C\n");
    printf("2\tdod\tDoD 5220.22-M\n");
    printf("3\tbsi\tBSI\n");
    printf("4\tusarmy\tUS Army AR380-19\n");
    printf("5\trandom\tOne Random Pass\n");
    printf("6\trandom2\tTwo Random Passes\n");
    printf("7\tschneier\tBruce Schneier Algorithm\n");
    printf("8\tpfitzner7\tRoy Pfitzner 7-random-pass method\n");
    printf("9\tpfitzner33\tRoy Pfitzner 33-random-pass method\n");
    printf("10\tgutmann\tGutmann\n");
    printf("11\tdirent\tdirent\n");
    printf("12\tfillzero\tQuick Fill with 0x00\n");
    printf("13\tfillff\tQuick Fill with 0xff\n");
    printf("14\tverify\tQuick Fill with 0x00 and verify\n");
    printf("输入您要使用的擦写算法编号:");
    scanf("%d",&num);
    return num;
}

char* kscale(unsigned long long size)
{
    if (size > G)
    {
        sprintf(str, "%0.2fG", size/(G*1.0));
        return str;
    }
    else if (size > M)
    {
        sprintf(str, "%0.2fM", size/(1.0*M));
        return str;
    }
    else if (size > K)
    {
        sprintf(str, "%0.2fK", size/(1.0*K));
        return str;
    }
    else
    {
        sprintf(str, "%0.2fB", size*1.0);
        return str;
    }
}

int isDisque(const char * dir) {
    if (strlen(dir) != 3) return 0;
    if ((dir[0] == 'h' || dir[0] == 's') && dir[1] == 'd') return 1;
    return 0;
}

int list_partitions(const char * dir) {
    struct dirent * lecture;
    DIR * rep;
    rep = opendir("/dev");
    int fd,r;
    unsigned long long size;  
    int len;
    char path[20];
    while ((lecture = readdir(rep))) {
        if (strstr(lecture -> d_name, dir) != NULL && strcmp(lecture -> d_name, dir)) {
            sprintf(path,"/dev/%s",lecture->d_name);
	    if ((fd = open(path, O_RDONLY)) < 0)  
            {  
                printf("open error %d\n");  
                return -1;  
    	    }  
	    if ((r = ioctl(fd, BLKGETSIZE64, &size)) < 0)  
	    {  
		printf("ioctl error \n");  
		return -1;   
	    }  
    	    //len = (size>>20);
	    printf("  |__ %s\t总容量：%s\n", path, kscale(size));
        }
    }
    closedir(rep);
}

int list_disques1() {
    struct dirent * lecture;
    DIR * rep;
    rep = opendir("/dev");
    int ind = 0;
    int fd,r;
    unsigned long long size;  
    int len;
    char path[20];
    while ((lecture = readdir(rep))) {
        if (isDisque(lecture -> d_name)) {
            sprintf(path,"/dev/%s",lecture->d_name);
	    if ((fd = open(path, O_RDONLY)) < 0)  
            {  
                printf("open error %d\n");  
                return -1;  
    	    }  
	    if ((r = ioctl(fd, BLKGETSIZE64, &size)) < 0)  
	    {  
		printf("ioctl error \n");  
		return -1;   
	    }  
    	    //len = (size>>20);
            printf("%d : %s\t总容量：%s\n", ind++, path, kscale(size));
            list_partitions(lecture -> d_name);
	    printf("\n");
        }

    }
    closedir(rep);
    return ind;
}

int list_disques2() {
    struct dirent * lecture;
    DIR * rep;
    rep = opendir("/dev");
    int ind = 0;
    int fd,r;
    unsigned long long size;  //磁盘大小（字节）
    int len;
    char path[20];
    while ((lecture = readdir(rep))) {
        if (isDisque(lecture -> d_name)) {
            list_partitions(lecture -> d_name);
        }

    }
    closedir(rep);
    return ind;
}

char *find_mount(char *device)
{
    FILE* mount_table=setmntent("/etc/mtab", "r");
    struct mntent *mount_entry;
    if (!mount_table)
    {
        fprintf(stderr, "set mount entry error/n");
        return -1;
    }
    while (1) {  //循环查找/etc/mtab文件中的数据项
        if (mount_table) {
            mount_entry = getmntent(mount_table);  //获取一个数据项
            if (!mount_entry) {
                endmntent(mount_table);  //末尾退出
                printf("未找到挂载目录\n");
                exit(-1);
            }
        }
        if(!strcmp(device,mount_entry->mnt_fsname))
	{
	    strcpy(path,mount_entry->mnt_dir);
	    break;
	}
	else
	    continue;
    }
    return path;
}

//判断是否为目录
bool is_dir(const char *path)
{
    struct stat statbuf;
    if(lstat(path, &statbuf) ==0)//lstat返回文件的信息，文件信息存放在stat结构中
    {
        return S_ISDIR(statbuf.st_mode) != 0;//S_ISDIR宏，判断文件类型是否为目录
    }
    return false;
}

//判断是否为可删除文件
bool is_file(const char *path)
{
    struct stat statbuf;
    if(lstat(path, &statbuf) ==0)
    {
	if(S_ISLNK(statbuf.st_mode)!=0)
	    return true;
	if(S_ISREG(statbuf.st_mode)!=0)
	    return true;
	if(S_ISSOCK(statbuf.st_mode)!=0)
	    return true;
    }
    return false;
}


//判断是否是特殊目录
bool is_special_dir(const char *path)
{
    return strcmp(path, ".") == 0 || strcmp(path, "..") == 0;
}

//生成完整的文件路径
void get_file_path(const char *path, const char *file_name,  char *file_path)
{
    strcpy(file_path, path);
    if(file_path[strlen(path) - 1] != '/')
        strcat(file_path, "/");
    strcat(file_path, file_name);
}

int delete_file(const char *path)
{
    DIR *dir;
    struct dirent *dir_info;
    char file_path[PATH_MAX];
    if(is_file(path))
    {
        remove(path);
        return 0;
    }
    if(is_dir(path))
    {
        if((dir = opendir(path)) == NULL)
	{
	    printf("目录%s不存在!\n",path);
            return -1;
	}
        while((dir_info = readdir(dir)) != NULL)
        {
            get_file_path(path, dir_info->d_name, file_path);
            if(is_special_dir(dir_info->d_name))
                continue;
            delete_file(file_path);
            rmdir(file_path);
        }
	rmdir(path);
    }
    return 0;
}

void cutdir(char *path,char *newpath)
{
    char *filename;
    strcpy(newpath,path);
    filename = strrchr(newpath,'/');//函数查找字符在指定字符串中最后一次出现的位置如果成功，则返回指定字符最后一次出现位置的地址，如果失败，则返回 false
    if(filename == NULL)
	return -1;
    *filename = '\0';
    return;
}

int func1()
{
    int ac=6;
    int fd;  
    //off_t size  
    unsigned long long size;  
    int len;  
    int r;  
    char s[20]="\0";
    int pattern;
    printf("警告：谨慎选择当前系统盘，否则会导致系统崩溃!\n");
    printf("当前系统中存在的物理盘和逻辑盘:\n");
    list_disques1();
    //getchar();
    printf("输入要擦除的磁盘:");
    gets(s);

    if ((fd = open(s, O_RDONLY)) < 0)  
    {  
        printf("open error %d\n");  
        return -1;  
    }  
  
    if ((r = ioctl(fd, BLKGETSIZE64, &size)) < 0)  
    {  
        printf("ioctl error \n");  
        return -1;   
    }  
  
    len = (size>>20);
    //printf("%d\n",len);
    char s_len[10];
    sprintf(s_len,"%dMB",len);
    pattern=choose_pattern();
    char *av[]={"scrub","-s",s_len,"-p",patterns[pattern-1],s};
    printf("确认要使用%s算法擦除磁盘:%s吗(默认为否)?[Y/N]: ",patterns[pattern-1],s);
    char c='N';
    getchar();
    c=getchar();
    if(c=='N'||c=='n')
	return 0;
    else if(c=='Y'||c=='y')
	ma(ac,av);
    else
	printf("输入错误!\n");
    //printf("%s %s %s %s %s\n",av[0],av[1],av[2],av[3],av[4]);
    //getchar();
    return 0;
}

int func2()
{
    int ac=5;
    int fd;  
    //off_t size  
    unsigned long long size;  
    int len;  
    int r;  
    char s[20]="\0";
    int pattern;
    printf("当前系统中存在的逻辑盘:\n");
    list_disques2();  //列出当前磁盘
    //getchar();
    printf("输入要擦除的磁盘:");
    gets(s);  //接收输入的磁盘

    if ((fd = open(s, O_RDONLY)) < 0)  
    {  
        printf("open error %d\n");  
        return -1;  
    }  
  
    if ((r = ioctl(fd, BLKGETSIZE64, &size)) < 0)  
    {  
        printf("ioctl error \n");  
        return -1;   
    }  
  
    len = (size>>20);  //磁盘大小
    char s_len[10];
    sprintf(s_len,"%dMB",len);  //转换成字符串
    char mount[512];
    strcpy(mount,find_mount(s));
    pattern=choose_pattern();
    char *av[]={"scrub","-X","-p",patterns[pattern-1],mount};
    printf("确认要使用%s算法擦除磁盘:%s吗(默认为否)?[Y/N]: ",patterns[pattern-1],s);
    char c='N';
    getchar();
    c=getchar();
    if(c=='N'||c=='n')
	return 0;
    else if(c=='Y'||c=='y')
	ma(ac,av);
    else
	printf("输入错误!\n");
    //printf("%s %s %s %s %s\n",av[0],av[1],av[2],av[3],av[4]);
    //getchar();
    return 0;
}

int func3()
{
    int ac1=7;
    int ac2=5;
    int pattern;
    char c;
    char dirpath[512]="\0";
    char parent_dir[512]="\0";
    char new_dir[512]="\0";
    printf("输入要擦除的文件或目录:");
    gets(dirpath);
    if(access(dirpath,F_OK)==-1)
    {
	printf("%s不存在!\n",dirpath);
        return -1;
    }
    pattern=choose_pattern();
    cutdir(dirpath,parent_dir);
    if(is_file(dirpath))
    {
	sprintf(new_dir,"%s/scrubfile",parent_dir);	
    }
    char *av1[]={"scrub","-r","-D",new_dir,"-p",patterns[pattern-1],dirpath};
    char *av2[]={"scrub","-X","-p",patterns[pattern-1],parent_dir};
    printf("确认要使用%s算法擦除:%s 吗(默认为否)?[Y/N]: ",patterns[pattern-1],dirpath);
    getchar();
    c=getchar();
    if(c=='N'||c=='n')
	return 0;
    else if(c=='Y'||c=='y')
    {
	if(is_file(dirpath))
	    //printf("%s %s %s %s %s %s %s\n",av1[0],av1[1],av1[2],av1[3],av1[4],av1[5],av1[6]);
	    ma(ac1,av1);
	else
	{
	    delete_file(dirpath);
            //printf("%s %s %s %s %s\n",av2[0],av2[1],av2[2],av2[3],av2[4]);
	    ma(ac2,av2);
	}
    }
    else
	printf("输入错误!\n");
    return 0;
}

int main()
{
    int num;
    while(1)
    {
	printf("==============================\n");        
	printf("磁盘擦除软件\n目录\n");
	printf("1、物理盘或逻辑盘数据擦除\n");
	printf("2、逻辑盘剩余空间数据擦除\n");
	printf("3、目录及文件数据擦除\n");
	printf("0、退出\n");
	printf("输入序号:");
	scanf("%d",&num);
	getchar();
        switch(num){
	    case 1:func1();break;
            case 2:func2();break;
	    case 3:func3();break;
	    case 0:return 0;
            default:printf("输入错误!\n");break;
	}
    }
    return 0;
}
