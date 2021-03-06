#include "lcd.h"

#define GPECON  (*((volatile unsigned long *)0x7F008080))
#define GPEDAT  (*((volatile unsigned long *)0x7F008084))
#define GPEPUD  (*((volatile unsigned long *)0x7F008088))
#define GPFCON  (*((volatile unsigned long *)0x7F0080A0))
#define GPFDAT  (*((volatile unsigned long *)0x7F0080A4))
#define GPFPUD  (*((volatile unsigned long *)0x7F0080A8))
#define GPICON  (*((volatile unsigned long *)0x7F008100))
#define GPIPUD  (*((volatile unsigned long *)0x7F008108))
#define GPJCON  (*((volatile unsigned long *)0x7F008120))
#define GPJPUD  (*((volatile unsigned long *)0x7F008128))

/* display controller */
#define MIFPCON  	    (*((volatile unsigned long *)0x7410800C))
#define SPCON        	(*((volatile unsigned long *)0x7F0081A0))
#define VIDCON0      	(*((volatile unsigned long *)0x77100000))
#define VIDCON1      	(*((volatile unsigned long *)0x77100004))
#define VIDTCON0     	(*((volatile unsigned long *)0x77100010))
#define VIDTCON1     	(*((volatile unsigned long *)0x77100014))
#define VIDTCON2     	(*((volatile unsigned long *)0x77100018))
#define WINCON0      	(*((volatile unsigned long *)0x77100020))
#define VIDOSD0A      	(*((volatile unsigned long *)0x77100040))
#define VIDOSD0B      	(*((volatile unsigned long *)0x77100044))
#define VIDOSD0C      	(*((volatile unsigned long *)0x77100048))
#define VIDW00ADD0B0    (*((volatile unsigned long *)0x771000A0))
#define VIDW00ADD1B0    (*((volatile unsigned long *)0x771000D0))
#define VIDW00ADD2      (*((volatile unsigned long *)0x77100100))
#define DITHMODE        (*((volatile unsigned long *)0x77100170))

#define FRAME_BUFFER   0x54000000
#define ROW				272
#define COL				480
#define HSPW 				(2)
#define HBPD			 	(40- 1)
#define HFPD 				(5 - 1)
#define VSPW				(2)
#define VBPD 				(8 - 1)
#define VFPD 				(9 - 1)
#define LINEVAL 			(271)
#define HOZVAL				(479)

// 初始化LCD
void lcd_init(void)
{
	// 配置GPIO用于LCD相关的功能
	GPECON = 0x00011111;
	GPEDAT = 0x00000001;
	GPFCON = 0x96AAAAAA;
	GPFDAT = 0x00002000;
	GPICON = 0xAAAAAAAA;
	GPJCON = 0x00AAAAAA;

	// normal mode
	MIFPCON &= ~(1<<3);

	// RGB I/F
	SPCON =  (SPCON & ~(0x3)) | 1;

	// 配置VIDCONx，设置接口类型、时钟、极性和使能LCD控制器等
	VIDCON0 = (0<<26)|(0<<17)|(0<<16)|(10<<6)|(0<<5)|(1<<4)|(0<<2)|(3<<0);
	VIDCON1 |= 1<<5 | 1<<6;

	// 配置VIDTCONx，设置时序和长宽等
	// 设置时序
	VIDTCON0 = VBPD<<16 | VFPD<<8 | VSPW<<0;
	VIDTCON1 = HBPD<<16 | HFPD<<8 | HSPW<<0;
	// 设置长宽
	VIDTCON2 = (LINEVAL << 11) | (HOZVAL << 0);


	// 配置WINCON0，设置window0的数据格式
	WINCON0 |= 1<<0;
	WINCON0 &= ~(0xf << 2);
	WINCON0 |= 0xB<<2;

	// 配置VIDOSD0A/B/C,设置window0的坐标系
#define LeftTopX     0
#define LeftTopY     0
#define RightBotX   479
#define RightBotY   271
	VIDOSD0A = (LeftTopX<<11) | (LeftTopY << 0);
	VIDOSD0B = (RightBotX<<11) | (RightBotY << 0);
	VIDOSD0C = (LINEVAL + 1) * (HOZVAL + 1);

	// 置VIDW00ADD0B0和VIDW00ADD1B0，设置framebuffer的地址
	VIDW00ADD0B0 = FRAME_BUFFER;
	VIDW00ADD1B0 = (((HOZVAL + 1)*4 + 0) * (LINEVAL + 1)) & (0xffffff);
}

// 描点
void lcd_draw_pixel(int row, int col, int color)
{
	unsigned long * pixel = (unsigned long  *)FRAME_BUFFER;

	*(pixel + row * COL + col) = color;

}

// 清屏
void lcd_clear_screen(int color)
{
	int i, j;

	for (i = 0; i < ROW; i++)
		for (j = 0; j < COL; j++)
			lcd_draw_pixel(i, j, color);

}

// 划横线
void lcd_draw_hline(int row, int col1, int col2, int color)
{
	int j;

	// 描第row行，第j列
	for (j = col1; j <= col2; j++)
		lcd_draw_pixel(row, j, color);

}

// 划竖线
void lcd_draw_vline(int col, int row1, int row2, int color)
{
	int i;
	// 描第i行，第col列
	for (i = row1; i <= row2; i++)
		lcd_draw_pixel(i, col, color);

}

// 划十字
void lcd_draw_cross(int row, int col, int halflen, int color)
{
	lcd_draw_hline(row, col-halflen, col+halflen, color);
	lcd_draw_vline(col, row-halflen, row+halflen, color);
}

// 绘制同心圆
void lcd_draw_circle(void)
{
	int x,y;
	int color;
	unsigned char red,green,blue,alpha;
	int xsize = ROW;
	int ysize = COL;

	for (y = 0; y < ysize; y++)
		for (x = 0; x < xsize; x++)
		{
			color = ((x-xsize/2)*(x-xsize/2) + (y-ysize/2)*(y-ysize/2))/64;
			red   = (color/8) % 256;
			green = (color/4) % 256;
			blue  = (color/2) % 256;
			alpha = (color*2) % 256;

			color |= ((int)alpha << 24);
			color |= ((int)red   << 16);
			color |= ((int)green << 8 );
			color |= ((int)blue       );

			lcd_draw_pixel(x,y,color);
		}
}

// 打印字符
#define FONTDATAMAX 2048
extern const unsigned char fontdata_8x8[FONTDATAMAX];
void lcd_draw_char(unsigned char c)
{
	// 必须是静态变量
	static int x = 0;	// 第几列
	static int y = 0;	// 第几行

	int i,j;
	unsigned char line_dots;

	// 获得字模
	unsigned char *char_dots = fontdata_8x8 + c * 8;


	// 是否需要回车换行
	if (c == '\n')
	{
		y += 16;
		if (y > ROW)
			y = 0;
		return ;
	}
	else if (c == '\r')
	{
		x = 0;
		return;
	}

	for (i = 0; i < 8; i++)
	{
		line_dots = char_dots[i];
		for (j = 0; j < 8; j++)
		{
			// 为1,则描白点
			if (line_dots & (0x80 >> j))
			{
				lcd_draw_pixel(y+i, x+j, 0xffffff);
			}
			// 为0,则描黑点
			else
			{
				lcd_draw_pixel(y+i, x+j, 0);
			}
		}
	}

	// 光标移动到下一个16*16的位置，首先坐标+16
	x += 8;
	if (x > COL)
		x = 0;
	// 如果x坐标=0,则y坐标+16
	if (x == 0)
	{
		y += 8;
		if (y > ROW)
			y = 0;
	}
}
