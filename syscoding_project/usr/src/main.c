/*
 * @Author: Fu Zhuoyue
 * @Date: 2023-07-20 21:39:45
 * @LastEditors: Fu Zhuoyue
 * @LastEditTime: 2023-07-24 13:50:30
 * @Description: 
 * @FilePath: /share_file/syscoding_project/usr/src/main.c
 */

#include "syshead.h"
#include "show_bmp.h"
#include "lcd.h"
#include "thread_pool.h"
#include "copyfile.h"
#include "font.h"
#include "show_mypng.h"
#include "list.h"
#include "kernel_list.h"
#include "drawpic.h"

#define FONTPATH "/usr/share/fonts/simfang.ttf"
#define PNGPATH  "/data/copyfile/resource"
#define LENGTHMX 256

//枚举文件类型
enum FILE_TAIL{
    FILETYPE = 0,
    PNG,
    C,
    H,
    TXT,
    OTHER
};

extern unsigned int total_file_num;

typedef struct FileNode{
    FileStrc file;
    struct list_head list;
}FileNode;

struct png_info_data file[6];
struct png_info_data icon_png[4];
BasicPIC Icon[4];

font *font_point;

char last_path[80];
char cur_path[80];

#if 1

//存储拷贝文件路径的链表头结点
NodeList *copy_dir_head = NULL;

NodeList *ret_dir_head = NULL;

void Destroy_FileNodelist(struct list_head* my_list) {
    FileNode* current_file_node;
    FileNode* tmp;

    list_for_each_entry_safe(current_file_node, tmp, my_list, list) {
        list_del(&current_file_node->list); // 从链表中删除当前节点
        free(current_file_node); // 释放当前节点的内存
    }
}

void Init_Icon()
{
    Init_Png(&file[0],"/data/copyfile/resource/file.png");
    Init_Png(&file[1],"/data/copyfile/resource/png.png");
    Init_Png(&file[2],"/data/copyfile/resource/c.png");
    Init_Png(&file[3],"/data/copyfile/resource/h.png");
    Init_Png(&file[4],"/data/copyfile/resource/txt.png");
    Init_Png(&file[5],"/data/copyfile/resource/else.png");

    Init_Png(&icon_png[0],"/data/copyfile/resource/close.png");
    Init_Png(&icon_png[1],"/data/copyfile/resource/back.png");
    Init_Png(&icon_png[2],"/data/copyfile/resource/up.png");
    Init_Png(&icon_png[3],"/data/copyfile/resource/down.png");

    for(int i = 0;i<4;i++)
        Icon[i].png = &icon_png[i]; 

    Icon[0].pos_x = 750;  Icon[0].pos_y = 5;
    Icon[0].width =30;    Icon[0].height = 30;

    Icon[1].pos_x = 120;  Icon[1].pos_y= 5;
    Icon[1].width =30;    Icon[1].height = 30;

    Icon[2].pos_x = 740;  Icon[2].pos_y = 240;
    Icon[2].width = 40;  Icon[2].height = 40;
    
    Icon[3].pos_x = 740;  Icon[3].pos_y = 320;
    Icon[3].width = 40;  Icon[3].height = 40;

}


void Show_Background(int cur_page,int all_page,int area)
{
    FillColor_Rectangle(C_WHITE,0,0,800,480,area);
    FillColor_Rectangle(0x413328,0,0,800,40,area);//画顶层框
    FillColor_Rectangle(0x605D53,0,40,120,440,area);//画左侧框
    
    char text[4][10] = {"进入","删除","复制","粘贴"};
    for(int i = 0;i<4; i++)
    {
        int x = 160+i*(160);
        int y = 400;
        FillColor_Rectangle(C_VIOLET,x,y,120,60,area);
        ShowFont_WithoutBackground(font_point,lcdpoint,text[i],
            x+26,y+12,C_BLACK,36,area );
    }
    
    for(int i = 0;i<4; i++)
    {
        Show_BasicPng(Icon+i,0);
    }
    
    //展示当前页
    char tempbuf[100] = "当前路径: ";
    strcat(tempbuf,cur_path);
    ShowFont_WithoutBackground(font_point,lcdpoint,tempbuf,
    160,8,C_WHITE,24,area );

    bzero(tempbuf,100);
    sprintf(tempbuf,"%d/%d",cur_page,all_page);
    
    ShowFont_WithoutBackground(font_point,lcdpoint,tempbuf,
    Icon[3].pos_x + 6,Icon[2].pos_y + 40 + 16,C_RED,18,area );
}


/**
 * @description: 更新当前实际显示的文件名到FILE链表,在这个过程中顺带完成类型赋值
 * @param {NodeList} *dir_head    
 * @param {FileNode} *filehead
 * @param {int} page    第几页
 * @return {*}
 */
void Updat_CurDirName_To_FileNode(NodeList *dir_head,FileNode *filehead,int page)
{
    NodeList *p_dir = dir_head->next;

    // FileNode *p_file = list_entry(filehead->list.next,FileNode, list);
    
    for(int i= 1 ;i< page; i++)  //有几页,移动p_dir成为新的首位节点
    {
        for(int j = 0;j<10;j++)
            p_dir = p_dir->next;
    }

    for( int i = 0  ; p_dir!= dir_head && i<10 ; p_dir = p_dir->next , i++ )
    {
        // 创建用户节点（大结构体）
        FileNode *new = malloc(sizeof(FileNode));
        new->file.is_choosen = false;
        //定义好图像显示位置
        int p_x = 160 + (i%5)*120;
        int p_y = 80 +(i/5)*160;

        new->file.basic_pic.pos_x = p_x;
        new->file.basic_pic.pos_y = p_y;
        new->file.basic_pic.width = 80;
        new->file.basic_pic.height = 80;

        //拷贝文件名字
        memcpy(new->file.name,p_dir->data, sizeof(new->file.name));    
            
        //拷贝文件路径
        char temp_buf[100] = "";
        sprintf(temp_buf,"%s/%s",cur_path,p_dir->data);
        memcpy(new->file.file_path, temp_buf,sizeof(new->file.file_path));

        //赋予文件信息
        struct stat info;           //  设备信息结构体
        stat(temp_buf, &info);      
        
        if( S_ISDIR(info.st_mode) ) //  如果是目录
        {
            new->file.dir_type = FILETYPE;
            new->file.basic_pic.png = &file[FILETYPE];
        }
        else{
            printf("tempbuf = %s\n",temp_buf);
            if(strstr( temp_buf,".png" ) != NULL)
            {
                printf("%s is png\n",temp_buf);
                new->file.dir_type = PNG;
                new->file.basic_pic.png = &file[PNG];
            }
            else if(strstr( temp_buf,".c" ) != NULL)
            {
                printf("%s is c\n",temp_buf);
                new->file.dir_type = C;
                new->file.basic_pic.png = &file[C];
            }
            else if(strstr( temp_buf,".h" ) != NULL)
            {
                printf("%s is h\n",temp_buf);
                new->file.dir_type = H;
                new->file.basic_pic.png = &file[H];
            }
            else if(strstr( temp_buf,".txt" ) != NULL)
            {
                printf("%s is txt\n",temp_buf);
                new->file.dir_type = TXT;
                new->file.basic_pic.png = &file[TXT];
            }
            else
            {   
                printf("%s is other\n",temp_buf);
                new->file.dir_type = OTHER;
                new->file.basic_pic.png = &file[OTHER];
            }
        }

        //赋值操作做好后插入头结点(顺序插入)
        list_add(&new->list, &filehead->list);
    }
}


//显示主要文件部分
void Show_MainFile(FileNode *file_head,int area)
{
    struct list_head *pos;
    struct list_head *n;
    list_for_each_safe(pos, n, &file_head->list)
    {
        
        FileNode *tmp = list_entry(pos,FileNode,list);
        int x = tmp->file.basic_pic.pos_x;
        int y = tmp->file.basic_pic.pos_y;
        Show_BasicPng(&(tmp->file.basic_pic),0);
        
        ShowFont_WithoutBackground(font_point,lcdpoint,tmp->file.name,
            x+6,y +2 +tmp->file.basic_pic.height,C_BLACK,18,area);
    }
}

bool Is_Dirrent(char *all_path)
{
    struct stat info;
    bzero(&info, sizeof(info));
    stat(all_path, &info);

    return S_ISDIR(info.st_mode);
}

bool Is_Png(char *all_path)
{
    return ( strstr(all_path,".png")!= NULL );
}

void Enter_Dirrent()
{
    enum TOUCHTYPE{     //按键类型
        NONE,
        ENTER,
        DEL,
        CPY,
        PASTE
    };

    enum TOUCHTYPE touch_type = NONE;
    bool ret_flag = false;
    bool png_show = false; //退出图片显示界面
    
    struct png_info_data * temp_png = NULL;


    chdir(cur_path);  //切换到当前路径
    NodeList *cur_dir_head = Init_HeadList();       //读取所有文件的链表
    
    /***1、创建首页展示链表***/
    FileNode *file_head = malloc(sizeof(FileNode)); //当前显示在首页的链表
    INIT_LIST_HEAD( &(file_head->list) ); //初始化首页的链表头结点

    /***2、创建选中链表,存放选中了的相对路径 ***/
    NodeList *chosen_dir_head = Init_HeadList();  //为已经选中的路径创建链表

    //获取当前文件夹下所有文件名
    int file_num = Get_Cur_Dirrent( cur_path ,cur_dir_head );
    printf("文件总数为%d\n",file_num);
    
    //算总页数
    int total_page = 0;
    int cur_page = 1;
    if(file_num == 0)
        total_page = 1;
    else
        total_page = (file_num - 1)/10 + 1;

    // //把前10个文件交给file_head链表
    Updat_CurDirName_To_FileNode(cur_dir_head,file_head,1);
    Show_Background(cur_page,total_page,0);
    Show_MainFile( file_head , 0);

    while (1)
    {
    /**************************算法结构、数据处理***************************/
        bool is_choosen_state = false;

        struct list_head *pos;
        struct list_head *n;
        list_for_each_safe(pos, n, &file_head->list)
        {
            FileNode *tmp_file = list_entry(pos,FileNode,list);
            if( Is_TouchAmongPic( &tmp_file->file.basic_pic ) )
            {
                tmp_file->file.is_choosen = !tmp_file->file.is_choosen;

                //选中
                if( tmp_file->file.is_choosen )
                {
                    int x = tmp_file->file.basic_pic.pos_x;
                    int y = tmp_file->file.basic_pic.pos_y;
                    FillColor_Rectangle(C_PINK,x,y,80,80,0);
                    Show_BasicPng(&tmp_file->file.basic_pic,0);

                    //添加到选中链表当中
                    NodeList *newnode = Create_Node( tmp_file->file.file_path );
                    Insert_Head(chosen_dir_head,newnode);
                }
                else    //反选中
                {
                    int x = tmp_file->file.basic_pic.pos_x;
                    int y = tmp_file->file.basic_pic.pos_y;
                    FillColor_Rectangle(C_WHITE,x,y,80,80,0);    
                    Show_BasicPng(&tmp_file->file.basic_pic,0);  
                 
                    NodeList *find_node = Find_FromList(chosen_dir_head,tmp_file->file.file_path);
                    Remove_Node(find_node);
                    free(find_node);
                                
                }
            }
        }

        if( Is_TouchAmongPic(&Icon[2]) && cur_page>1)//上键
        {
            printf("按了上键\n");
            cur_page --;
            Destory_NodeList(chosen_dir_head);

            Destroy_FileNodelist(&file_head->list);
            free(file_head);
            file_head = malloc(sizeof(FileNode)); //当前显示在首页的链表
            INIT_LIST_HEAD( &(file_head->list) ); //初始化首页的链表头结点

            Updat_CurDirName_To_FileNode(cur_dir_head,file_head,cur_page);

            Show_Background(cur_page,total_page,0);
            Show_MainFile(file_head,0);
        }
        if( Is_TouchAmongPic(&Icon[3]) && cur_page < total_page)
        {
            printf("按了下键\n");
            cur_page ++;
            Destory_NodeList(chosen_dir_head);

            Destroy_FileNodelist(&file_head->list);
            free(file_head);
            file_head = malloc(sizeof(FileNode)); //当前显示在首页的链表
            INIT_LIST_HEAD( &(file_head->list) ); //初始化首页的链表头结点
            
            Updat_CurDirName_To_FileNode(cur_dir_head,file_head,cur_page);

            Show_Background(cur_page,total_page,0);
            Show_MainFile(file_head,0);
        }
        
        if(IsTouch_AmongAnypx(160 + 0*160,400,120,60))//进入
        {
            //遍历选中链表,找到第一个文件夹进入
            for(NodeList *tmp = chosen_dir_head->next;tmp != chosen_dir_head ; tmp = tmp->next)
            {
                if(Is_Dirrent(tmp->data))
                {
                    memcpy(cur_path,tmp->data,strlen(tmp->data));

                    //插入到路径链表中
                    NodeList *newnode = Create_Node( tmp->data );
                    Insert_Head( ret_dir_head,newnode );

                    ret_flag = true;
                    break;
                }
                else if(Is_Png(tmp->data))
                {
                    png_show = true;
                    temp_png = malloc(sizeof(struct png_info_data));
                    Init_Png(temp_png , tmp->data);     
                    show_png_pos(120,40,17*40,11*40,temp_png,0);
                }
            }
        }

        if(IsTouch_AmongAnypx(160 + 1*160,400,120,60))//删除
        {
            for(NodeList *tmp = chosen_dir_head->next;tmp != chosen_dir_head ; tmp = tmp->next)
            {
                char temp[256] = "";
                bzero(temp,256);
                if(Is_Dirrent(tmp->data))
                    sprintf(temp,"rm %s -rf",tmp->data );
                else
                    sprintf(temp,"rm %s ",tmp->data );
                
                system(temp);
            }          
            break; 
        }

        if(IsTouch_AmongAnypx(160 + 2*160,400,120,60))//复制
        {
            copy_dir_head = Init_HeadList(); //创建拷贝链表,只有粘贴完成了才允许销毁
            for(NodeList *tmp = chosen_dir_head->next;tmp != chosen_dir_head ; tmp = tmp->next)
            {
                NodeList *newnode = Create_Node( tmp->data );
                Insert_Head( copy_dir_head,newnode );    
                printf("已复制%s\n",tmp->data );      
            }
            
        }

        if(IsTouch_AmongAnypx(160 + 3*160,400,120,60))//粘贴
        {   
            if(  copy_dir_head != NULL && copy_dir_head->next != copy_dir_head )
            {
                int i = 1;
                for(NodeList *tmp = copy_dir_head->next;tmp != copy_dir_head ; tmp = tmp->next , i++)
                {
                    if(Is_Dirrent(tmp->data))  //如果是目录拷贝则递归+线程
                    {
                        printf("是文件夹\n");
                        printf("正在拷贝第%d组文件:%s\n",i,tmp->data );
                        CopyFile_WithPthread( 20, tmp->data , cur_path);
                    }
                    else        //如果是普通文件普通拷贝法
                    {
                        printf("是普通文件\n");
                        char tar_pbuf[256] = "";
                        bzero(tar_pbuf,256);
                        char *ret;
                        ret = strrchr(tmp->data, '/');
                        sprintf(tar_pbuf,"%s%s",cur_path,ret);
                        CopyFileTo_OtherFile(tmp->data , tar_pbuf);
                    }

                }
                
                printf("123\n");

                Destory_NodeList(copy_dir_head);
                free(copy_dir_head);
                copy_dir_head = NULL;
                break;  //拷贝完刷新

            }
        }
        if( Is_TouchAmongPic( &Icon[0] ) ) //刷新/关闭
        {
            bzero(cur_path,sizeof(cur_path));
            memcpy(cur_path,"/",2);

            Destory_NodeList(ret_dir_head);
            free(ret_dir_head);

            ret_dir_head = Init_HeadList();     
            NodeList *newnode = Create_Node( cur_path );
            Insert_Head( ret_dir_head,newnode );  

            break;  
        }
        if( Is_TouchAmongPic( &Icon[1] ) ) //返回上一级
        {   
            if(!png_show)
            {
                if(ret_dir_head->next->next != ret_dir_head)  //在首目录就不用返回了
                {
                    NodeList *p = ret_dir_head->prev;
                    memcpy(cur_path,p->prev->data,strlen(p->data));
                    Remove_Node(p);
                    free(p);

                    break;
                }
            }
            else
            {
                free(temp_png);
                break;
            }
        }

        if(ret_flag)
            break;
    }

    Destory_NodeList(cur_dir_head);
    free(cur_dir_head);

    Destroy_FileNodelist(&file_head->list);
    free(file_head);

    Destory_NodeList(chosen_dir_head);
    free(chosen_dir_head);
}

void *touch(void *arg)
{
    while(1)
    {
        Get_TouchValue();
    }
}

#endif

int main()
{

    //CopyFile_WithPthread( 200, "//data/tempdir","//data/emptydir");

    // CopyFile_WithPthread(20,"//data/PushBoxGame","//data/bigdir");
    // CopyFile_WithPthread(20,"//dir_2","//dir_3");

#if 1

    Lcd_Init(C_WHITE);
    font_point = fontLoad(FONTPATH);

    pthread_t tid;
    pthread_create(&tid,NULL,touch,NULL);

    Init_Icon();

    //初始化当前位置为根目录
    memcpy(cur_path,"/",2);
    memcpy(last_path,"/",2);
        
    //存储走过的路
    ret_dir_head = Init_HeadList();     
    NodeList *newnode = Create_Node( cur_path );
    Insert_Head( ret_dir_head,newnode );  

    while (1)
    {
        Enter_Dirrent();
    }
    
    // show_png_pos(160,80,80,80,file,0);

#endif


    return 0;
}