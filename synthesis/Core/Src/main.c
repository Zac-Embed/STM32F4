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

//���ݴ���оƬIIC�ӿڳ�ʼ��
void CT_IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOH_CLK_ENABLE();       //����GPIOHʱ��
    __HAL_RCC_GPIOI_CLK_ENABLE();       //����GPIOIʱ��

    GPIO_Initure.Pin = GPIO_PIN_6;              //PH6
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_OD;    //�������
    GPIO_Initure.Pull = GPIO_PULLUP;            //����
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;       //����
    HAL_GPIO_Init(GPIOH, &GPIO_Initure);        //��ʼ��

    GPIO_Initure.Pin = GPIO_PIN_3;              //PI3
    HAL_GPIO_Init(GPIOI, &GPIO_Initure);        //��ʼ��
	
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_SET); //CT_IIC_SDA = 1;
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);//CT_IIC_SCL = 1;
}

uint8_t FT5206_Init(void)
{
    uint8_t temp[5];
    uint8_t res=1;
    GPIO_InitTypeDef GPIO_Initure;
 
    __HAL_RCC_GPIOH_CLK_ENABLE();			//����GPIOHʱ��
    __HAL_RCC_GPIOI_CLK_ENABLE();			//����GPIOIʱ��
                
    //PH7
    GPIO_Initure.Pin=GPIO_PIN_7;            //PH7
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      //����
    GPIO_Initure.Pull=GPIO_PULLUP;          //����
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //����
    HAL_GPIO_Init(GPIOH,&GPIO_Initure);     //��ʼ��
            
    //PI8
    GPIO_Initure.Pin=GPIO_PIN_8;            //PI8
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //�������
    HAL_GPIO_Init(GPIOI,&GPIO_Initure);     //��ʼ��
	
	CT_IIC_Init();
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_8, GPIO_PIN_SET);//FT_RST=0;
	
	
	
}

//touch screen init
uint8_t TP_Init(void)
{
	//lcddev.id == 0X7016
	
	//FT5206_Init
	FT5206_Init();

	

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
