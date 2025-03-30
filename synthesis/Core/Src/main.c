/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma2d.h"
#include "ltdc.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
void delay_ms(uint16_t nms);
uint8_t TP_Init(void);
void ctp_test(void);
void lcd_draw_bline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint8_t size,uint16_t color);
void gui_fill_circle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t color);
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_FMC_Init();
  MX_DMA2D_Init();
  MX_LTDC_Init();
  /* USER CODE BEGIN 2 */
	SDRAM_Device_Init();
	LTDC_Init();
	TP_Init();

	ctp_test();
#if 0
	int retval;
	retval = SDRAM_Test();	
	if(retval != SUCCESS)
		printf("SDRAM test error\r\n");
	else
		printf("SDRAM test OK!!!!\r\n");
#endif
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		HAL_GPIO_TogglePin(GPIOB, LED1_Pin|LED0_Pin);
		printf("test..\r\n");
		delay_ms(500);
//		LTDC_Clear(YELLOW);
//		HAL_Delay(500);
//		LTDC_Clear(BLUE);
//		HAL_Delay(500);
//		LTDC_Clear(RED);
//		HAL_Delay(500);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
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

//init delay func
//presently unsupport os
static uint32_t fac_us=0;//us delay multiple
void delay_init(uint8_t SYSCLK)
{
	fac_us=SYSCLK;//180
}

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
	ticks=nus*180; 									//The number of beats that need to be delayed
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


void CT_IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOH_CLK_ENABLE();       
    __HAL_RCC_GPIOI_CLK_ENABLE();       

    GPIO_Initure.Pin = GPIO_PIN_6;              //PH6
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_OD;    
    GPIO_Initure.Pull = GPIO_PULLUP;            
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;       
    HAL_GPIO_Init(GPIOH, &GPIO_Initure);        

    GPIO_Initure.Pin = GPIO_PIN_3;              //PI3
    HAL_GPIO_Init(GPIOI, &GPIO_Initure);        
	
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_SET); //CT_IIC_SDA = 1;
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);//CT_IIC_SCL = 1;
}

//IIC start signal
void CT_IIC_Start(void)
{
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_SET);//CT_IIC_SDA = 1;
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);//CT_IIC_SCL = 1;
    delay_us(2);
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_RESET);//CT_IIC_SDA = 0; //START:when CLK is high,DATA change form high to low
    delay_us(2);
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET);//CT_IIC_SCL = 0; 
    delay_us(2);
}

void CT_IIC_Stop(void)
{
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_RESET);//CT_IIC_SDA = 0; //STOP:when CLK is high DATA change form low to high
    delay_us(2);
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_SET);//CT_IIC_SCL = 1;
    delay_us(2);
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_SET);//CT_IIC_SDA = 1; //·¢ËÍI2C×ÜÏß½áÊøÐÅºÅ
    delay_us(2);
}

void CT_IIC_Send_Byte(uint8_t txd)
{
    uint8_t t;

    for (t = 0; t < 8; t++)
    {
        HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, ((txd&0x80)>>7));//CT_IIC_SDA = (txd & 0x80) >> 7;
        delay_us(2);
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);//CT_IIC_SCL = 1;
        delay_us(2);
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET);//CT_IIC_SCL = 0;
        txd <<= 1;
    }

    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3,GPIO_PIN_SET);
}

uint8_t CT_IIC_Wait_Ack(void)
{
    uint8_t ucErrTime = 0;
    uint8_t rack = 0;
    
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_SET);//CT_IIC_SDA = 1;
    delay_us(2);
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);//CT_IIC_SCL = 1;
    delay_us(2);

    while (HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_3))
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
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET);//CT_IIC_SCL = 0; //Ê±ÖÓÊä³ö0
    delay_us(2);
    return rack;
}

void CT_IIC_NAck(void)
{
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_SET);//CT_IIC_SDA = 1;
    delay_us(2);
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);//CT_IIC_SCL = 1;
    delay_us(2);
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET);//CT_IIC_SCL = 0;
    delay_us(2);
}

void CT_IIC_Ack(void)
{
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_RESET);//CT_IIC_SDA = 0;
    delay_us(2);
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);//CT_IIC_SCL = 1;
    delay_us(2);
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET);//CT_IIC_SCL = 0;
    delay_us(2);
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_SET);//CT_IIC_SDA = 1;
    delay_us(2);
}

//read 1 byte ,when ack=1 send ACK; when ack=0 send nACK
uint8_t CT_IIC_Read_Byte(unsigned char ack)
{
    uint8_t i, receive = 0;

    for (i = 0; i < 8; i++ )
    {
        receive <<= 1;
        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);//CT_IIC_SCL = 1;
        delay_us(2);

        if (HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_3))
					receive++;

        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET);//CT_IIC_SCL = 0;
        delay_us(2);
    }

    if (!ack)
			CT_IIC_NAck();//·¢ËÍnACK
    else 
			CT_IIC_Ack(); //·¢ËÍACK

    return receive;
}
		  
void FT5206_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t i; 
 	CT_IIC_Start();	
 	CT_IIC_Send_Byte(FT_CMD_WR);   	//·¢ËÍÐ´ÃüÁî 	 
	CT_IIC_Wait_Ack(); 	 										  		   
 	CT_IIC_Send_Byte(reg&0XFF);   	//·¢ËÍµÍ8Î»µØÖ·
	CT_IIC_Wait_Ack();  
 	CT_IIC_Start();  	 	   
	CT_IIC_Send_Byte(FT_CMD_RD);   	//·¢ËÍ¶ÁÃüÁî		   
	CT_IIC_Wait_Ack();	   
	for(i=0;i<len;i++)
	{	   
    	buf[i]=CT_IIC_Read_Byte(i==(len-1)?0:1); //·¢Êý¾Ý	  
	} 
    CT_IIC_Stop();//²úÉúÒ»¸öÍ£Ö¹Ìõ¼þ     
} 

void GT9147_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t i; 
	uint8_t ret;
 	CT_IIC_Start();	
 	CT_IIC_Send_Byte(GT_CMD_WR);   //·¢ËÍÐ´ÃüÁî 	 
	ret=CT_IIC_Wait_Ack();
 	CT_IIC_Send_Byte(reg>>8);   	//·¢ËÍ¸ß8Î»µØÖ·
	ret=CT_IIC_Wait_Ack(); 
 	CT_IIC_Send_Byte(reg&0XFF);   	//·¢ËÍµÍ8Î»µØÖ·
	ret=CT_IIC_Wait_Ack();  
 	CT_IIC_Start();  	 	   
	CT_IIC_Send_Byte(GT_CMD_RD);   //·¢ËÍ¶ÁÃüÁî		   
	ret=CT_IIC_Wait_Ack();	  
	for(i=0;i<len;i++)
	{	   
    	buf[i]=CT_IIC_Read_Byte(i==(len-1)?0:1); //·¢Êý¾Ý	  	
	} 
    CT_IIC_Stop();//²úÉúÒ»¸öÍ£Ö¹Ìõ¼þ    
} 

uint8_t GT9147_WR_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t i;
	uint8_t ret=0;
	CT_IIC_Start();	
 	CT_IIC_Send_Byte(GT_CMD_WR);   	//·¢ËÍÐ´ÃüÁî 	 
	CT_IIC_Wait_Ack();
	CT_IIC_Send_Byte(reg>>8);   	//·¢ËÍ¸ß8Î»µØÖ·
	CT_IIC_Wait_Ack(); 	 										  		   
	CT_IIC_Send_Byte(reg&0XFF);   	//·¢ËÍµÍ8Î»µØÖ·
	CT_IIC_Wait_Ack();  
	for(i=0;i<len;i++)
	{	   
    	CT_IIC_Send_Byte(buf[i]);  	//·¢Êý¾Ý
		ret=CT_IIC_Wait_Ack();
		if(ret)break;  
	}
    CT_IIC_Stop();					//²úÉúÒ»¸öÍ£Ö¹Ìõ¼þ	    
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
            x[g_gt_tnum-1]=x[0];//±£´æ´¥µã0µÄÊý¾Ý,±£´æÔÚ×îºóÒ»¸öÉÏ
            y[g_gt_tnum-1]=y[0];
            
            delay_ms(4); //Necessary time delays
            
			for(i=0;i<g_gt_tnum;i++)
			{
				if(sta&(1<<i))		//touchtype valid
				{
                    if(strcmp((char *)CIP,"911")==0) //´¥ÃþIC 911
                    {
                        GT9147_RD_Reg(GT911_TPX_TBL[i],buf,4);   //read x y value
                        if(touchtype&0X01) //ºáÆÁ
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
                        FT5206_RD_Reg(FT5206_TPX_TBL[i],buf,4);	//¶ÁÈ¡XY×ø±êÖµ 
                        if(touchtype&0X01)//ºáÆÁ
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
            if(x[0]>width||y[0]>height)  //·Ç·¨Êý¾Ý(×ø±ê³¬³öÁË)
            {
                if((mode&0XF)>1)   // ÓÐÆäËûµãÓÐÊý¾Ý,Ôò¸´µÚ¶þ¸ö´¥µãµÄÊý¾Ýµ½µÚÒ»¸ö´¥µã.
                {
                    x[0]=x[1];
                    y[0]=y[1];
                    t=0;  // ´¥·¢Ò»´Î,Ôò»á×îÉÙÁ¬Ðø¼à²â10´Î,´Ó¶øÌá¸ßÃüÖÐÂÊ 
                }
                else        // ·Ç·¨Êý¾Ý,ÔòºöÂÔ´Ë´ÎÊý¾Ý(»¹Ô­Ô­À´µÄ) 
                {
                    x[0]=x[g_gt_tnum-1];
                    y[0]=y[g_gt_tnum-1];
                    mode=0X80;
                    sta=tempsta;   // »Ö¸´tp_dev.sta 
                }
            }
            else t=0;      // ´¥·¢Ò»´Î,Ôò»á×îÉÙÁ¬Ðø¼à²â10´Î,´Ó¶øÌá¸ßÃüÖÐÂÊ 
		}
	}
    if(strcmp((char *)CIP,"911")==0) //´¥ÃþIC 911
    {
        if((mode&0X8F)==0X80)//ÎÞ´¥Ãþµã°´ÏÂ
        { 
            if(sta&TP_PRES_DOWN)	//Ö®Ç°ÊÇ±»°´ÏÂµÄ
            {
                sta&=~TP_PRES_DOWN;	//±ê¼Ç°´¼üËÉ¿ª
            }else						//Ö®Ç°¾ÍÃ»ÓÐ±»°´ÏÂ
            { 
                x[0]=0xffff;
                y[0]=0xffff;
                sta&=0XE0;	//Çå³ýµãÓÐÐ§±ê¼Ç	
            }	 
        } 	
    }
    else
    {
        if((mode&0X1F)==0)//ÎÞ´¥Ãþµã°´ÏÂ
        { 
            if(sta&TP_PRES_DOWN)	//Ö®Ç°ÊÇ±»°´ÏÂµÄ
            {
                sta&=~TP_PRES_DOWN;	//±ê¼Ç°´¼üËÉ¿ª
            }else						//Ö®Ç°¾ÍÃ»ÓÐ±»°´ÏÂ
            { 
                x[0]=0xffff;
                y[0]=0xffff;
                sta&=0XE0;	//Çå³ýµãÓÐÐ§±ê¼Ç	
            }
        }
    }
	if(t>240)t=10;//ÖØÐÂ´Ó10¿ªÊ¼¼ÆÊý
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
    if((x0+len-1)>=width)x0=width-len-1;	//ÏÞÖÆ×ø±ê·¶Î§
    if(y0>=height)y0=height-1;			//ÏÞÖÆ×ø±ê·¶Î§
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
	delta_x=x2-x1; //¼ÆËã×ø±êÔöÁ¿ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //ÉèÖÃµ¥²½·½Ïò 
	else if(delta_x==0)incx=0;//´¹Ö±Ïß 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//Ë®Æ½Ïß 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //Ñ¡È¡»ù±¾ÔöÁ¿×ø±êÖá 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//»­ÏßÊä³ö 
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
 	uint16_t lastpos[10][2];		//×îºóÒ»´ÎµÄÊý¾Ý 
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
					lcd_draw_bline(lastpos[t][0],lastpos[t][1],x[t],y[t],2,POINT_COLOR_TBL[t]);//»­Ïß
					lastpos[t][0]=x[t];
					lastpos[t][1]=y[t];
					if(x[t]>(width-24)&&y[t]<20)
					{
//						Load_Drow_Dialog();//Çå³ý
					}
				}
			}else lastpos[t][0]=0XFFFF;
		} 
		delay_ms(5);i++;
		if(i%20==0)
			HAL_GPIO_TogglePin(GPIOB, LED1_Pin|LED0_Pin);
	}	
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
