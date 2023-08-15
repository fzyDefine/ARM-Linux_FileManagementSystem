
/*
 * @Author: Fu Zhuoyue
 * @Date: 2023-07-19 20:39:48
 * @LastEditors: Fu Zhuoyue
 * @LastEditTime: 2023-07-24 15:54:26
 * @Description: 
 * @FilePath: /share_file/syscoding_project/usr/src/copyfile.c
 */

#include "syshead.h"
#include "pthread.h"
#include "thread_pool.h"
#include "copyfile.h"
#include "show_mypng.h"
#include "show_bmp.h"

unsigned int total_file_num = 0;
unsigned int already_file_num = 0;

pthread_mutex_t counter_lock;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;//定义条件量

typedef struct _CopyDirTask {
    char src_path[MAX_PATH_LEN];
    char dst_path[MAX_PATH_LEN];
    thread_pool *pool; // Add a pointer to the thread pool
} CopyDirTask;

void *CopyDirTo_Other_Pthread(void *arg) {
    CopyDirTask *task = (CopyDirTask*)arg;

    mkdir(task->dst_path, 0777);
    Recursive_Copy_Dirent(task->pool, task->src_path, task->dst_path);

    free(task);
    return NULL;
}

void CopyFileTo_OtherFile(char *srcFilePath, char *dstFilePath) {
    int src_fd = open(srcFilePath,O_RDWR,0777);
    if(src_fd == -1) {
        printf("打开源文件失败,path = %s\n",srcFilePath);
        perror("open");
        return;
    }

    int dst_fd = open(dstFilePath,O_RDWR|O_CREAT|O_TRUNC, 0777 );
    if(dst_fd == -1) {
        printf("创建目标文件%s失败\n",dstFilePath);
        perror("open");
        close(src_fd);
        return;
    }

    char buf[4096];  // Change the buffer size to 4KB
    ssize_t bytes;

    while ((bytes = read(src_fd, buf, sizeof(buf))) > 0) {
        write(dst_fd, buf, bytes);
    }

    close(src_fd);
    close(dst_fd);
}

void *CopyFileTo_Other_Pthread(void *arg) {
    CopyTask *task = (CopyTask*)arg;
    
    CopyFileTo_OtherFile(task->src_path, task->dst_path);

    pthread_mutex_lock(&counter_lock);

    already_file_num++;
    printf("already_file = %d\n",already_file_num);

    pthread_cond_signal(&cond);  // 唤醒进度条线程

    pthread_mutex_unlock(&counter_lock);

    free(task);
    return NULL;
}

void Recursive_Copy_Dirent(thread_pool *pool, char *src_path, char *dst_path) {
    DIR *dp = opendir(src_path);
    if(dp == NULL) {
        printf("打开错误\n");
        perror("opendir");
        return;
    }

    struct dirent *ep;
    while ((ep = readdir(dp)) != NULL) {
        if (!strncmp(ep->d_name, ".", 1) || !strncmp(ep->d_name, "..", 2)) {
            continue;
        }
 
        char old_path[MAX_PATH_LEN] = "";
        snprintf(old_path, MAX_PATH_LEN, "%s/%s", src_path, ep->d_name);

        struct stat info;
        stat(old_path, &info);  

        char dst_new_path[MAX_PATH_LEN] = "";
        snprintf(dst_new_path, MAX_PATH_LEN, "%s/%s", dst_path, ep->d_name);

        if(S_ISDIR(info.st_mode)) {
            CopyDirTask *task = (CopyDirTask *)malloc(sizeof(CopyDirTask));
            strncpy(task->src_path, old_path, MAX_PATH_LEN);
            strncpy(task->dst_path, dst_new_path, MAX_PATH_LEN);
            task->pool = pool;  // set the pool pointer
            add_task(pool, CopyDirTo_Other_Pthread, (void *)task);
            
        } else if (S_ISREG(info.st_mode)) {
            CopyTask *task = (CopyTask *)malloc(sizeof(CopyTask));
            strncpy(task->src_path, old_path, MAX_PATH_LEN);
            strncpy(task->dst_path, dst_new_path, MAX_PATH_LEN);
            add_task(pool, CopyFileTo_Other_Pthread, (void *)task);
        }
    }
    closedir(dp);
}

void *Draw_DownloadBar(void *arg)
{
    int length = 0;
    while (length <= 99 )
    {
        pthread_mutex_lock(&counter_lock);

        while(already_file_num  *100 / total_file_num == length) {  // 如果进度没有变化，就等待
            pthread_cond_wait(&cond, &counter_lock);
        }
        length = already_file_num  *100 / total_file_num;


        pthread_mutex_unlock(&counter_lock);   

        printf("画图already_file_num = %d\n",already_file_num);
        printf("画图length = %d\n",length);
        FillColor_Rectangle(C_BLUE,240+27,120+96, (int)(3.55*length),14,0);

        usleep(2000);  // 睡眠一段时间，让文件处理的线程有时间运行
    }
    
}

void List_Dirent(char *path)
{
    DIR *dp = opendir(path);
    while (1)
    {    
        struct dirent *ep;      // 获取目录项名字
        ep = readdir(dp);

        if(ep == NULL)
            break ;

        if (!strncmp(ep->d_name, ".", 1) || !strncmp(ep->d_name, "..", 2)) {
            continue;
        }

        char old_path[512] = "";
        bzero(old_path,512);
        sprintf(old_path,"%s/%s",path,ep->d_name);
        struct stat info;           //设备信息结构体
        stat(old_path, &info);    

        if( S_ISDIR(info.st_mode) ) //如果是目录
        {
            List_Dirent( old_path );
        }
        else
        {
            total_file_num ++;
            printf("path = %s,num = %u\n",old_path,total_file_num);
            
        }
    }
    closedir(dp);
}

void CopyFile_WithPthread(int pthread_num, char *src_path, char *dst_path) {
    
    struct png_info_data *bk_png = malloc(sizeof(struct png_info_data));
    already_file_num = 0;
    total_file_num = 0;
    pthread_cond_init(&cond, NULL);

    pthread_t draw_bar_pid;
    pthread_attr_t draw_attr;
    // 初始化线程属性对象
    pthread_attr_init(&draw_attr);


    Init_Png(bk_png,"/data/copyfile/resource/load.png");
    show_png_pos(240,120,400,200,bk_png,0);

    //计算文件个数
    List_Dirent(src_path);
    printf("total = %d\n",total_file_num);

    
    pthread_mutex_init(&counter_lock, NULL);    //创建互斥锁

    if(access(dst_path, F_OK) != 0) {
        printf("目标路径不存在,为您创建路径%s\n",dst_path);
        mkdir(dst_path,0777);
    }

    thread_pool *pool = malloc(sizeof(thread_pool));   
    init_pool(pool, pthread_num);

    printf("线程任务开始\n");

    //绘画功能设置成分离属性,画完图自己回收
    pthread_create(&draw_bar_pid, &draw_attr, Draw_DownloadBar, NULL);

    Recursive_Copy_Dirent(pool, src_path, dst_path);

    // 等待所有任务完成
    while (!is_pool_idle(pool)) {
        sleep(1);  // 
    }

    printf("线程池任务完成\n");
    // 所有任务完成后销毁线程池
    destroy_pool(pool);

    pthread_mutex_destroy(&counter_lock);
    printf("线程池已销毁\n");

    // 销毁线程属性对象
    pthread_attr_destroy(&draw_attr);

    free(bk_png);
    
}

//获取当前文件夹下所有文件的名字放在head链表里(不递归),返回文件总数
int Get_Cur_Dirrent(char *path, NodeList *head)
{   
    int num = 0;
    DIR *dp = opendir(path);

    while (1)
    {    
        struct dirent *ep;      // 获取目录项名字
        ep = readdir(dp);

        if(ep == NULL)
            break;

        if (!strncmp(ep->d_name, ".", 1) || !strncmp(ep->d_name, "..", 2)) {
            continue;
        }

        char old_path[512] = "";
        bzero(old_path,512);
        sprintf(old_path,"%s/%s",path,ep->d_name);
        struct stat info;           //设备信息结构体
        stat(old_path, &info);    

        if( S_ISDIR(info.st_mode) ) //如果是目录
        {
            printf("%s是目录\n",ep->d_name);
        }
        else{
            printf("%s是普通文件\n",ep->d_name);
        }
        
        //把当前位置下所有文件名拷贝进链表
        NodeList *newnode = Create_Node(ep->d_name);
        Insert_Head(head,newnode);

        num ++;
    }
    closedir(dp);

    return num;
}