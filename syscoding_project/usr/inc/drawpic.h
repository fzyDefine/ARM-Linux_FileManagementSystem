/*
 * @Author: Fu Zhuoyue
 * @Date: 2023-07-20 19:44:59
 * @LastEditors: Fu Zhuoyue
 * @LastEditTime: 2023-07-22 09:36:09
 * @Description: 
 * @FilePath: /share_file/syscoding_project/usr/inc/drawpic.h
 */
#ifndef __DRAWPIC_H__
#define __DRAWPIC_H__

#define  color uint32_t

//基本图像信息
typedef struct BasicPIC{

    int pos_x;
    int pos_y;
    int width;
    int height;
    struct png_info_data* png;  //所指向的png图片信息指针

}BasicPIC;

typedef struct FileStrc{

    BasicPIC basic_pic; //基本图像信息
    bool is_choosen;    //是否被选中
    int dir_type;       //文件类型 
    char file_path[256]; //文件路径
    char name[50];      //文件名称

}FileStrc;

typedef struct TextStrc{

    uint8_t size;       //文字大小
    int pos_x;          //文字坐标
    int pos_y;    
    char text[100];
    color text_color;   //文字颜色 
    color back_color;   //背景颜色 
    
}TextStrc;

void Show_BasicPng(BasicPIC* photo, int area);
bool Is_TouchAmongPic(BasicPIC *base_pic);

#endif // !__DRAWPIC_H__