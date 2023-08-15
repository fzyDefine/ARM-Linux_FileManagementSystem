/*
 * @Author: Fu Zhuoyue
 * @Date: 2023-07-20 21:12:50
 * @LastEditors: Fu Zhuoyue
 * @LastEditTime: 2023-07-22 09:45:48
 * @Description: 
 * @FilePath: /share_file/syscoding_project/usr/inc/list.h
 */
#ifndef __LIST_H__
#define __LIST_H__

#define DATAMAX 256

typedef struct NodeList
{
    char data[DATAMAX];
    struct NodeList *next;
    struct NodeList *prev;

}NodeList;

NodeList *Init_HeadList();
NodeList *Create_Node(char *data);
void Insert_Head(NodeList *target_node,NodeList *src_node);
void Insert_Tail(NodeList *target_node,NodeList *src_node);
void Show_NodeList(NodeList *head);
void Remove_Node( NodeList *node );
void Destory_NodeList(NodeList *head);

NodeList *Find_FromList(NodeList *head ,char *data);

#endif // !__LIST_H__
