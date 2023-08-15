/*
 * @Author: Fu Zhuoyue
 * @Date: 2023-07-12 08:49:51
 * @LastEditors: Fu Zhuoyue
 * @LastEditTime: 2023-07-22 09:59:27
 * @Description: 
 * @FilePath: /share_file/syscoding_project/usr/src/sx_list.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"

bool Is_EmptyList(NodeList *head)
{
    return ((head->next == head) || (head->prev == head));
}

//初始化头结点
NodeList *Init_HeadList()
{
    NodeList *head = malloc(sizeof(NodeList));
    head->prev = head;
    head->next = head;
}

//创建新节点
NodeList *Create_Node(char *data)
{
    NodeList *node = malloc(sizeof(NodeList));
    node->prev = node;
    node->next = node;

    memcpy(node->data,data,DATAMAX);

    return node;
}

/**
 * @description: 头插法
 * @param {NodeList} *target_node   需要被插入的节点
 * @param {NodeList} *src_node      传入的节点
 * @return {*}
 */
void Insert_Head(NodeList *target_node,NodeList *src_node)
{
    target_node->prev->next = src_node;
    src_node->prev = target_node->prev;
    src_node->next = target_node;
    target_node->prev = src_node;
}

/**
 * @description:    尾插法
 * @param {NodeList} *target_node   需要被插入的节点
 * @param {NodeList} *src_node      传入的节点
 * @return {*}
 */
void Insert_Tail(NodeList *target_node,NodeList *src_node)
{
    target_node->next->prev = src_node;
    src_node->next = target_node->next;
    src_node->prev = target_node;
    target_node->next = src_node;
}

void Show_NodeList(NodeList *head)
{
    if( Is_EmptyList(head) )
    {
        printf("链表为空\n");
        return;
    }
    for(NodeList *tmp_node = head->next;tmp_node != head ; tmp_node = tmp_node->next)
    {
        printf("%s \n",tmp_node->data);
    }
}

NodeList *Find_FromList(NodeList *head ,char *data)
{
    if( Is_EmptyList(head) )
    {
        printf("链表为空\n");
        return NULL;
    }
    for(NodeList *tmp_node = head->next;tmp_node != head ; tmp_node = tmp_node->next)
    {
        if( memcmp(data,tmp_node->data,strlen(data)) == 0 )
        {
            return tmp_node;
        }
    }
    return NULL;
}

void Remove_Node( NodeList *node )
{
    node->prev->next = node->next;
    node->next->prev = node->prev;

    node->prev = node;
    node->next = node;
}

void Destory_NodeList(NodeList *head)
{
    if(Is_EmptyList(head))
    {
        printf("链表为空\n");
        return;
    }

    NodeList *n;
    for(NodeList *tmp = head->next; tmp!=head; tmp=n)
    {
        n = tmp->next;
        Remove_Node(tmp);
        free(tmp);
    }

}


