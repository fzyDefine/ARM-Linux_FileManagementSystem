/*
 * @Author: Fu Zhuoyue
 * @Date: 2023-07-24 09:10:20
 * @LastEditors: Fu Zhuoyue
 * @LastEditTime: 2023-07-24 09:14:44
 * @Description:;
 * @FilePath: /share_file/syscoding_project/usr/inc/copyfile.h
 */
#ifndef __COPYFILE_H__
#define __COPYFILE_H__

#include "list.h"
#include "thread_pool.h"
#define MAX_PATH_LEN 256
typedef struct {
    char src_path[MAX_PATH_LEN];
    char dst_path[MAX_PATH_LEN];
} CopyTask;

#if 0

void CopyFileTo_OtherFile(char *srcFilePath, char *tarFilePath);
void Printf_Dirent( char *path, int deep );
void Copy_Dirent( char *path, char *target_rootpath );
void *Copy_Dirent_Thread(void *arg);
void CopyFile_WithPthread( int pthread_num, char *src_path, char *tar_path);
int Count_FileNums(int pthread_num, char *path);
int Get_Cur_Dirrent(char *path, NodeList *head);

#endif

#if 1

void CopyFile_WithPthread(int pthread_num, char *src_path, char *dst_path);
void Recursive_Copy_Dirent(thread_pool *pool, char *src_path, char *dst_path) ;
void *CopyFileTo_Other_Pthread(void *arg);
void CopyFileTo_OtherFile(char *srcFilePath, char *dstFilePath);
bool is_pool_idle(thread_pool *pool);
int Get_Cur_Dirrent(char *path, NodeList *head);

#endif

extern unsigned int total_file_num;
extern unsigned int already_file_num;

#endif // !__COPYFILE_H__
