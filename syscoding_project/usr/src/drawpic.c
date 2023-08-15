/*
 * @Author: Fu Zhuoyue
 * @Date: 2023-07-20 19:45:44
 * @LastEditors: Fu Zhuoyue
 * @LastEditTime: 2023-07-22 09:08:48
 * @Description: 
 * @FilePath: /share_file/syscoding_project/usr/src/drawpic.c
 */
#include "syshead.h"
#include "show_mypng.h"
#include "drawpic.h"
#include "lcd.h"


// 是否点击了某张图片
bool Is_TouchAmongPic(BasicPIC *base_pic)
{
    return IsTouch_AmongAnypx( base_pic->pos_x , base_pic ->pos_y ,base_pic->width, base_pic->height );
}

void Show_BasicPng(BasicPIC* photo, int area)
{   
    int x = photo->pos_x;
    int y = photo->pos_y;
    struct png_info_data* png = photo->png;
    int w = photo->width;
    int h = photo->height;
    
    show_png_pos(x,y,w,h,png,area); 

}

void Draw_ChoosenRctg()
{





    
}