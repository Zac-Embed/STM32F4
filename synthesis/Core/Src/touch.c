#include "touch.h"
#include "ltdc.h"
#include "usart.h"

#define	FT_ID_G_LIB_VERSION		0xA1

#define FT_CMD_WR 				0X70
#define FT_CMD_RD 				0X71

#define FT_DEVIDE_MODE 			0x00  //FT5206 mode control addr
#define FT_REG_NUM_FINGER       0x02//touch status regitster 

#define FT_TP1_REG 				0X03
#define FT_TP2_REG 				0X09		
#define FT_TP3_REG 				0X0F		
#define FT_TP4_REG 				0X15		
#define FT_TP5_REG 				0X1B		

#define GT_CMD_WR 		0X28 //write command
#define GT_CMD_RD 		0X29 //read command
//GT9147 Register
#define GT_CTRL_REG 	0X8040	//control register
#define GT_CFGS_REG 	0X8047	//Configure the actual address register
#define GT_CHECK_REG 	0X80FF	//Checksum register
#define GT_PID_REG 		0X8140	//Product ID register

#define GT_GSTID_REG 	0X814E	//The touch status that is currently detected
#define GT_TP1_REG 		0X8150	//1th touch point addr
#define GT_TP2_REG 		0X8158	//2th touch point addr
#define GT_TP3_REG 		0X8160	//3th touch point addr
#define GT_TP4_REG 		0X8168	//4th touch point addr
#define GT_TP5_REG 		0X8170	//5th touch point addr

#define TP_PRES_DOWN 0x80 //touch screen touched
#define TP_CATH_PRES 0x40	//button pressed

uint8_t CIP[5];

//bit 0: 0 Portrait screen; 1 Landscape screen
//bit 0: 0 Resistive screens; 1 Capacitive screen
uint8_t touchtype;
uint8_t direction=0; // 0 Portrait screen; 1 Landscape screen
uint16_t sta;//touch state; bit 15:1 press,0 unpress; bit 14 :0 no button press; 1 button press
//bit13-bit10 :reserved; bit9-bit0:number of pressed point ,0 unpress 1pressed
uint16_t x[10];
uint16_t y[10];//Up to 10 sets of coordinates

uint16_t width=600; //lcd width
uint16_t height=1024;

//============================================ delay ============================================
/*
	note:do not use ucos
		delay nus us
	The system clock is 180M, that is, 1 second counts 180 000 000 times, 1us counts 180 times, that is, 1us needs 180 system beats
*/
void delay_us(uint32_t nus)
{		
	uint32_t ticks;
	uint32_t told,tnow,tcnt=0;
	uint32_t reload=SysTick->LOAD;	//SysTick Reload Value Register	    	 
	//ticks=nus*fac_us;
	ticks=nus*180; 									//*** The number of beats that need to be delayed ***
	told=SysTick->VAL;        			//SysTick Current Value Register
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)
				tcnt+=told-tnow;	//SysTick is a decreasing counter
			else 
				tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//over the beats that need
		}  
	};
}

void delay_ms(uint16_t nms)
{
	uint32_t i;
	for(i=0;i<nms;i++) delay_us(1000);
}
//============================================ delay end ============================================


//============================================ simulated IIC ============================================

#define IIC_SDA_PIN GPIO_PIN_3
#define IIC_SDA_PORT GPIOI

#define IIC_SCL_H()     HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET)
#define IIC_SCL_L()     HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET)
#define IIC_SDA_H()     HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_SET)
#define IIC_SDA_L()     HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_RESET)

void CT_IIC_Init(void)
{       
	IIC_SDA_H();
	IIC_SCL_H();
}

//IIC start signal
void CT_IIC_Start(void)
{
	IIC_SDA_H();
	IIC_SCL_H();
	delay_us(2);
	IIC_SDA_L();
	delay_us(2);
	IIC_SCL_L();
	delay_us(2);
}

void CT_IIC_Stop(void)
{
	IIC_SDA_L();
	delay_us(2);
	IIC_SCL_H();
	delay_us(2);
	IIC_SDA_H();
	delay_us(2);
}

void CT_IIC_Send_Byte(uint8_t txd)
{
	for (uint8_t i = 0x80; i != 0; i >>= 1)
	{
		if(txd & i)
			IIC_SDA_H();
		else
			IIC_SDA_L();
		delay_us(2);
		IIC_SCL_H();
		delay_us(2);
		IIC_SCL_L();
	}
	IIC_SDA_H();
}

uint8_t CT_IIC_Wait_Ack(void)
{
	uint8_t ucErrTime = 0;
	uint8_t rack = 0;
	
	IIC_SDA_H();
	delay_us(2);
	IIC_SCL_H();
	delay_us(2);

	while (HAL_GPIO_ReadPin(IIC_SDA_PORT, IIC_SDA_PIN))
	{
		ucErrTime++;
		if (ucErrTime > 250)
		{
			CT_IIC_Stop();
			rack = 1;
			break;
		}
		delay_us(2);
	}
	IIC_SCL_L();
	delay_us(2);
	return rack;
}

void CT_IIC_NAck(void)
{
	IIC_SDA_H();
	delay_us(2);
	IIC_SCL_H();
	delay_us(2);
	IIC_SCL_L();
	delay_us(2);
}

void CT_IIC_Ack(void)
{
	IIC_SDA_L();
	delay_us(2);
	IIC_SCL_H();
	delay_us(2);
	IIC_SCL_L();
	delay_us(2);
	IIC_SDA_H();
	delay_us(2);
}

//read 1 byte ,when ack=1 send ACK; when ack=0 send nACK
uint8_t CT_IIC_Read_Byte(unsigned char ack)
{
  uint8_t i, receive = 0;

	for (i = 0x80; i != 0; i>>=1 )
	{
		IIC_SCL_H();
		delay_us(2);
		if (HAL_GPIO_ReadPin(IIC_SDA_PORT, IIC_SDA_PIN))
			receive|=i;
		IIC_SCL_L();
		delay_us(2);
	}
	if (!ack)
		CT_IIC_NAck();//发送nACK
	else 
		CT_IIC_Ack(); //发送ACK

	return receive;
}
//============================================ simulated IIC end ============================================
		  
void FT5206_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t i; 
 	CT_IIC_Start();	
 	CT_IIC_Send_Byte(FT_CMD_WR);   	//发送写命令 	 
	CT_IIC_Wait_Ack(); 	 										  		   
 	CT_IIC_Send_Byte(reg&0XFF);   	//发送低8位地址
	CT_IIC_Wait_Ack();  
 	CT_IIC_Start();  	 	   
	CT_IIC_Send_Byte(FT_CMD_RD);   	//发送读命令		   
	CT_IIC_Wait_Ack();	   
	for(i=0;i<len;i++)
	{	   
    	buf[i]=CT_IIC_Read_Byte(i==(len-1)?0:1); //发数据	  
	} 
    CT_IIC_Stop();//产生一个停止条件     
} 

void GT9147_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t i; 
	uint8_t ret;
 	CT_IIC_Start();	
 	CT_IIC_Send_Byte(GT_CMD_WR);   //发送写命令 	 
	ret=CT_IIC_Wait_Ack();
 	CT_IIC_Send_Byte(reg>>8);   	//发送高8位地址
	ret=CT_IIC_Wait_Ack(); 
 	CT_IIC_Send_Byte(reg&0XFF);   	//发送低8位地址
	ret=CT_IIC_Wait_Ack();  
 	CT_IIC_Start();  	 	   
	CT_IIC_Send_Byte(GT_CMD_RD);   //发送读命令		   
	ret=CT_IIC_Wait_Ack();	  
	for(i=0;i<len;i++)
	{	   
    	buf[i]=CT_IIC_Read_Byte(i==(len-1)?0:1); //发数据	  	
	} 
    CT_IIC_Stop();//产生一个停止条件    
} 

uint8_t GT9147_WR_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t i;
	uint8_t ret=0;
	CT_IIC_Start();	
 	CT_IIC_Send_Byte(GT_CMD_WR);   	//发送写命令 	 
	CT_IIC_Wait_Ack();
	CT_IIC_Send_Byte(reg>>8);   	//发送高8位地址
	CT_IIC_Wait_Ack(); 	 										  		   
	CT_IIC_Send_Byte(reg&0XFF);   	//发送低8位地址
	CT_IIC_Wait_Ack();  
	for(i=0;i<len;i++)
	{	   
    	CT_IIC_Send_Byte(buf[i]);  	//发数据
		ret=CT_IIC_Wait_Ack();
		if(ret)break;  
	}
    CT_IIC_Stop();					//产生一个停止条件	    
	return ret; 
}

uint8_t FT5206_Init(void)
{
    uint8_t temp[5];
    uint8_t res=1;
    GPIO_InitTypeDef GPIO_Initure;
 
    __HAL_RCC_GPIOH_CLK_ENABLE();			
    __HAL_RCC_GPIOI_CLK_ENABLE();			
                
    //PH7
    GPIO_Initure.Pin=GPIO_PIN_7;            //PH7
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      
    GPIO_Initure.Pull=GPIO_PULLUP;          
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     
    HAL_GPIO_Init(GPIOH,&GPIO_Initure);     
            
    //PI8
    GPIO_Initure.Pin=GPIO_PIN_8;            //PI8
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  
    HAL_GPIO_Init(GPIOI,&GPIO_Initure);     
	
	CT_IIC_Init();
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_8, GPIO_PIN_RESET);//FT_RST=0;
	delay_ms(20);
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_8, GPIO_PIN_SET);//FT_RST=1;
	delay_ms(50);
	temp[0]=0;
	
	FT5206_RD_Reg(FT_ID_G_LIB_VERSION,&temp[0],2);
	
	GT9147_RD_Reg(GT_PID_REG,temp,4);//read id
	temp[4]=0;
	printf("CTP ID:%s\r\n", temp);
	memcpy(CIP,temp, sizeof(uint8_t)*4);
	temp[0]=0X02;
	GT9147_WR_Reg(GT_CTRL_REG,temp,1); //software reset
	GT9147_RD_Reg(GT_CFGS_REG,temp,1);
	delay_ms(10);
	temp[0]=0X00;
	GT9147_WR_Reg(GT_CTRL_REG,temp,1);
	res = 0;
	return res;
}

const uint16_t FT5206_TPX_TBL[5]={FT_TP1_REG,FT_TP2_REG,FT_TP3_REG,FT_TP4_REG,FT_TP5_REG};
const uint16_t GT911_TPX_TBL[5]={GT_TP1_REG,GT_TP2_REG,GT_TP3_REG,GT_TP4_REG,GT_TP5_REG};
//Scan the touchscreen in a query mode
uint8_t FT5206_Scan(uint8_t mode)
{
	uint8_t g_gt_tnum=5;//support touch point num
	uint8_t buf[4];
	uint8_t i=0;
	uint8_t res=0;
	uint8_t temp;
  uint16_t tempsta;
	static uint8_t t=0;//Control query time and reduce CPU usage
	t++;
	if((t%10)==0||t<10)//When idle, enter ten times and scan once
	{
        if(strcmp((char *)CIP,"911")==0) //touch IC 911
        {
            GT9147_RD_Reg(GT_GSTID_REG,&mode,1); //read touch point state
            if((mode&0X80)&&((mode&0XF)<=g_gt_tnum))
            {
                i=0;
                GT9147_WR_Reg(GT_GSTID_REG,&i,1); //clear flag
            }
        }
        else
        {
            FT5206_RD_Reg(FT_REG_NUM_FINGER,&mode,1);//read touch point state 
        }
        
		if((mode&0XF)&&((mode&0XF)<=g_gt_tnum))
		{
			temp=0XFF<<(mode&0XF);//The number of points is converted into the number of digits of one
            tempsta=sta;    //save sta's value
            sta=(~temp)|TP_PRES_DOWN|TP_CATH_PRES;
            x[g_gt_tnum-1]=x[0];//保存触点0的数据,保存在最后一个上
            y[g_gt_tnum-1]=y[0];
            
            delay_ms(4); //Necessary time delays
            
			for(i=0;i<g_gt_tnum;i++)
			{
				if(sta&(1<<i))		//touchtype valid
				{
                    if(strcmp((char *)CIP,"911")==0) //触摸IC 911
                    {
                        GT9147_RD_Reg(GT911_TPX_TBL[i],buf,4);   //read x y value
                        if(touchtype&0X01) //横屏
                        {
                            x[i]=(((uint16_t)buf[1]<<8)+buf[0]);
                            y[i]=(((uint16_t)buf[3]<<8)+buf[2]);
                        }
                        else
                        {
                            y[i]=((uint16_t)buf[1]<<8)+buf[0];
                            x[i]=width-(((uint16_t)buf[3]<<8)+buf[2]);
                        }
                    }
                    else
                    {
                        FT5206_RD_Reg(FT5206_TPX_TBL[i],buf,4);	//读取XY坐标值 
                        if(touchtype&0X01)//横屏
                        {
                            y[i]=((uint16_t)(buf[0]&0X0F)<<8)+buf[1];
                            x[i]=((uint16_t)(buf[2]&0X0F)<<8)+buf[3];
                        }else
                        {
                            x[i]=width-(((uint16_t)(buf[0]&0X0F)<<8)+buf[1]);
                            y[i]=((uint16_t)(buf[2]&0X0F)<<8)+buf[3];
                        }  
                    }
//					printf("x[%d]:%d,y[%d]:%d\r\n",i,tp_dev.x[i],i,tp_dev.y[i]);
				}			
			} 
			res=1;
            if(x[0]>width||y[0]>height)  //非法数据(坐标超出了)
            {
                if((mode&0XF)>1)   // 有其他点有数据,则复第二个触点的数据到第一个触点.
                {
                    x[0]=x[1];
                    y[0]=y[1];
                    t=0;  // 触发一次,则会最少连续监测10次,从而提高命中率 
                }
                else        // 非法数据,则忽略此次数据(还原原来的) 
                {
                    x[0]=x[g_gt_tnum-1];
                    y[0]=y[g_gt_tnum-1];
                    mode=0X80;
                    sta=tempsta;   // 恢复tp_dev.sta 
                }
            }
            else t=0;      // 触发一次,则会最少连续监测10次,从而提高命中率 
		}
	}
    if(strcmp((char *)CIP,"911")==0) //触摸IC 911
    {
        if((mode&0X8F)==0X80)//无触摸点按下
        { 
            if(sta&TP_PRES_DOWN)	//之前是被按下的
            {
                sta&=~TP_PRES_DOWN;	//标记按键松开
            }else						//之前就没有被按下
            { 
                x[0]=0xffff;
                y[0]=0xffff;
                sta&=0XE0;	//清除点有效标记	
            }	 
        } 	
    }
    else
    {
        if((mode&0X1F)==0)//无触摸点按下
        { 
            if(sta&TP_PRES_DOWN)	//之前是被按下的
            {
                sta&=~TP_PRES_DOWN;	//标记按键松开
            }else						//之前就没有被按下
            { 
                x[0]=0xffff;
                y[0]=0xffff;
                sta&=0XE0;	//清除点有效标记	
            }
        }
    }
	if(t>240)t=10;//重新从10开始计数
	return res;
}

//touch screen init
uint8_t TP_Init(void)
{
	//lcddev.id == 0X7016
	
	//FT5206_Init
	FT5206_Init();
	//tp_dev.scan = FT5206_Scan;
	touchtype |= 0X80; //1000 0000
	touchtype |= direction & 0X01;  //direction
	return 0;
}

void gui_draw_hline(uint16_t x0,uint16_t y0,uint16_t len,uint16_t color)
{
	if(len==0)return;
    if((x0+len-1)>=width)x0=width-len-1;	//限制坐标范围
    if(y0>=height)y0=height-1;			//限制坐标范围
	//LCD_Fill(x0,y0,x0+len-1,y0,color);
	LTDC_Fill(x0,y0,x0+len-1,y0,color);	
}

void gui_fill_circle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t color)
{											  
	uint32_t i;
	uint32_t imax = ((uint32_t)r*707)/1000+1;
	uint32_t sqmax = (uint32_t)r*(uint32_t)r+(uint32_t)r/2;
	uint32_t x=r;
	gui_draw_hline(x0-r,y0,2*r,color);
	for (i=1;i<=imax;i++) 
	{
		if ((i*i+x*x)>sqmax)// draw lines from outside  
		{
 			if (x>imax) 
			{
				gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
				gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
			}
			x--;
		}
		// draw lines from inside (center)  
		gui_draw_hline(x0-x,y0+i,2*x,color);
		gui_draw_hline(x0-x,y0-i,2*x,color);
	}
}  

void lcd_draw_bline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint8_t size,uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		gui_fill_circle(uRow,uCol,size,color);//draw point
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
} 

const uint16_t POINT_COLOR_TBL[10]={RED,GREEN,BLUE,BROWN,GRED,BRED,GBLUE,LIGHTBLUE,BRRED,GRAY};

//
void ctp_test(void)
{
	uint8_t t=0;
	uint8_t i=0;	  	    
 	uint16_t lastpos[10][2];		//最后一次的数据 
	uint8_t maxp=5;

	while(1)
	{
		FT5206_Scan(0);
		for(t=0;t<maxp;t++)
		{
			if((sta)&(1<<t))
			{
				if(x[t]<width&&y[t]<height)
				{
					if(lastpos[t][0]==0XFFFF)
					{
						lastpos[t][0] = x[t];
						lastpos[t][1] = y[t];
					}
					lcd_draw_bline(lastpos[t][0],lastpos[t][1],x[t],y[t],2,POINT_COLOR_TBL[t]);//画线
					lastpos[t][0]=x[t];
					lastpos[t][1]=y[t];
					if(x[t]>(width-24)&&y[t]<20)
					{
//						Load_Drow_Dialog();//清除
					}
				}
			}else lastpos[t][0]=0XFFFF;
		} 
		delay_ms(5);i++;
		if(i%20==0)
			HAL_GPIO_TogglePin(GPIOB, LED1_Pin|LED0_Pin);
	}	
}