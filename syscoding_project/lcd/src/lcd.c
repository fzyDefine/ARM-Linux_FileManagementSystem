#include "lcd.h"
#include "font.h"
#include <math.h>

unsigned char cur_lcd_area = 0; //当前屏幕区域 0是A区  1是B区

int touch_pos_x = 0;
int touch_pos_y = 0;

//屏幕文件
int lcdfd;  
int tp;

//获取lcd属性、可变参数
struct fb_var_screeninfo vinfo; 

//屏幕映射指针
unsigned char *lcdpoint;

int Lcd_Init(Color32_T color)
{
    //打开液晶屏的驱动
	lcdfd=open("/dev/fb0",O_RDWR);
	if(lcdfd==-1)
	{
		perror("打开lcd失败了!\n");
		return -1;
	}

    //打开触摸屏的驱动
    tp = open("/dev/input/event0", O_RDWR);
    if(tp == -1)
    {
        printf("打开触摸失败\r\n");
        return -1;
    }
    else
    {
        printf("打开触摸屏成功\r\n");
    }

    //映射得到lcd在内存中的首地址   (申请一个两倍的内存)
	lcdpoint=mmap(NULL,2*   800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcdfd,0);
	if(lcdpoint==NULL)
	{
		perror("获取lcd的首地址失败了!\n");
		return -1;
	}

    cur_lcd_area = 0;
    vinfo.xoffset = 0;
    vinfo.yoffset = 0 + 480*(cur_lcd_area);
    ioctl(lcdfd, FBIOPAN_DISPLAY, &vinfo);

    memset(lcdpoint,0,800*480*4 *2);
    FillColor_Rectangle(color,0,0,800,480,0);
    FillColor_Rectangle(color,0,0,800,480,1);
    return 0;
}

//锁定当前屏幕视角
void Lock_CurView()
{
    //根据当前视角确定offset偏移
    vinfo.xoffset = 0;
    vinfo.yoffset = 0 + 480*(cur_lcd_area);
    ioctl(lcdfd, FBIOPAN_DISPLAY, &vinfo);

}

//转移当前屏幕视角
void Change_CurView()
{
    cur_lcd_area = !cur_lcd_area;

    vinfo.xoffset = 0;
    vinfo.yoffset = 0 + 480*(cur_lcd_area);
    ioctl(lcdfd, FBIOPAN_DISPLAY, &vinfo);

}

//清除触摸信息
static void Clean_TouchMessage()
{
    touch_pos_x = -1;
    touch_pos_y = -1;
}

//是否点击了某个区域
bool IsTouch_AmongAnypx( int position_leftTop_x , int position_leftTop_y ,int px_width, int px_height )
{
    if(     (touch_pos_x> position_leftTop_x && touch_pos_x< position_leftTop_x + px_width)
            && (touch_pos_y > position_leftTop_y && touch_pos_y <  position_leftTop_y + px_height)   )
    {
            printf("成功点击指定区域:(%d,%d)(%d,%d)\r\n"
                ,position_leftTop_x,position_leftTop_y,position_leftTop_x+px_width,position_leftTop_y+px_height);
            printf("%d,%s\r\n",__LINE__,__FILE__);
            Clean_TouchMessage();       //如果成功响应触摸清空坐标信息
            return true;
    }
    else
        return false;
}

// //是否点击了某个png图片
// bool IsTouch_AtPngPic(PngStruct *png_member)
// {
//     int px_width  = png_member->width;
//     int px_height = png_member->height;

//     return IsTouch_AmongAnypx( png_member->pos_x, png_member->pos_y , px_width , px_height );

// }

//获取触摸值
int Get_TouchValue()
{
    int temp_x = 0;
    int temp_y = 0;
    int first_x = 0;
    int first_y = 0;
    int last_x = 0;
    int last_y = 0;

    int last_pos_type = 10;
    int now_pos_type = 10; 
    struct input_event buf;

    int is_first_press = 0;

    while(1)
    {
        // printf("进入触摸读取\r\n");
        bzero(&buf, sizeof(buf));
        read(tp, &buf, sizeof(buf));

        if(  buf.type == EV_ABS )
        {
            last_pos_type = now_pos_type;
            now_pos_type = buf.code;    

            if(buf.code == ABS_X)
            {
                temp_x = buf.value;        
            }

            //
            if( last_pos_type ==  ABS_X && now_pos_type == ABS_Y )    //正确获取到x,y的坐标
            {
                temp_y = buf.value;   
                touch_pos_x = temp_x;
                touch_pos_y = temp_y;

                is_first_press++;  //if is_first_press == 1,第一次有效读取

                if( is_first_press == 1 )
                {
                    first_x = temp_x;
                    first_y = temp_y;
                    printf("first_x = %d, first_y = %d \r\n",first_x,first_y);  //抬手的时候输出一下坐标
                }
            }
        }
        
        //printf("x = %d, y = %d \r\n",touch_pos_x,touch_pos_y);

		if (buf.type == EV_KEY)         //按键事件
        {
			if (buf.code == BTN_TOUCH)  //手指按压事件
            {
				if (buf.value == KEY_RESERVED)  //松手事件
                {
                    is_first_press = 0;
                    last_x = touch_pos_x;
                    last_y = touch_pos_y;
                    printf("last_x = %d, last_y = %d \r\n",touch_pos_x,touch_pos_y);  //抬手的时候输出一下坐标

                    break;
				}
			}
		}

    }

    if(abs(last_x - first_x) >= 60 || abs(last_y - first_y) >= 60 ) //触摸范围
    {
        if(abs(last_x - first_x) > abs(last_y - first_y) )  //左右
        {
            if(last_x > first_x) //右滑
            {
                printf("右滑\n");
                return RIGHT;
            }
            else        //左滑
            {
                printf("zuo滑\n");
                return LEFT;
            }

        }
        else        //上下
        {
            if(last_y > first_y) //下滑
            {
                printf("xia滑\n");
                return DOWN;
            }
            else        //上滑
            {
                printf("shang滑\n");
                return UP;
            }        
        }
    }

}

