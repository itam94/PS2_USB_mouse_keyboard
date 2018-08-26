/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */
#include <stdbool.h>
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim10;
TIM_HandleTypeDef htim11;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
//typedef int bool;
//enum { false, true };

int bitnumber;

bool Sending = false;
bool Reciving = false;
int LastRecived = 0;
int LastProcessed = 51;
bool Sended = false;
bool StreamMode = false;
bool first = true;
int bytesCount = 3;
int intellimouseState = 0;
uint8_t dataSendPacket;
uint8_t dataRecivedPacket;
bool isSendingWaiting = false;
bool preSendingState = false;
uint8_t Recived = 0;

uint8_t AllRecived[51];
uint8_t AllSended[51];
uint8_t isDataReady;
uint8_t ParityTable[51];
uint32_t start;
uint32_t diff;
uint32_t end;
uint16_t adc[2];
int channel = 0;
uint8_t OutBuffer[10];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM10_Init(void);
static void MX_TIM11_Init(void);
static void MX_ADC1_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void readBit();
void checkParity();
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void PS2_Handle_Reciving();
void PS2_Handle_Sending();
void setNextDataBit(uint32_t bitnumber);
void setParityBit();
int calculateCount(uint8_t data);
void SendPackage(uint8_t *data);
void Handle_Recived();
int8_t setBit(int i);
uint8_t setButtons();
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);
void saveKeySettings();
void readDataFromFlash();

struct mouseHID_t {
		  uint8_t reportId;
	      uint8_t buttons;
	      int8_t x;
	      int8_t y;
	      int8_t wheel;
	  };
struct keyboardHID_t{
	 uint8_t report_id;
	    uint8_t modifier;
	    uint8_t reserved;
	    uint8_t keycode[10];
};

struct keycodesHID_t{
	 uint8_t report_id;
	 uint8_t keycode[10];
};


struct keysMapHID_t{
	uint8_t report_id;
	uint8_t keys [10];
};
struct mouseHID_t mouseHID;
struct keyboardHID_t keyboardHID;
struct keysMapHID_t keyMapsHid;


/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */



	  bitnumber = 0;

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_USB_DEVICE_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  //HAL_GPIO_WritePin(Clock_device_GPIO_Port,Clock_device_Pin,1);
  readDataFromFlash();
  HAL_TIM_Base_Start_IT(&htim11);
  HAL_ADC_Start_IT(&hadc1);
  keyboardHID.report_id = 0x01;
  mouseHID.reportId = 0x02;
  keyMapsHid.report_id = 0x03;
  isDataReady = 0;

  //saveKeySettings();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  if(isDataReady == 1){
		  if(OutBuffer[0] == 0xff){
				USBD_HID_SendReport(&hUsbDeviceFS, &keyMapsHid, sizeof(struct keysMapHID_t));

		  }else{
			  for(int i = 0; i<10; i++){
				  keyMapsHid.keys[i] = OutBuffer[i];
			  }
			  //keyMapsHid.keys = OutBuffer;

			  saveKeySettings();
		  }
		  isDataReady = 0;

	  }

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

	     //USBD_HID_SendReport(&hUsbDeviceFS, &mouseHID, sizeof(struct mouseHID_t));

  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
  hadc1.Init.Resolution = ADC_RESOLUTION_8B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM10 init function */
static void MX_TIM10_Init(void)
{

  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 143;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 99;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM11 init function */
static void MX_TIM11_Init(void)
{

  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 143;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 9999;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, Clock_host_Pin|Data_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : Clock_host_Pin Data_Pin */
  GPIO_InitStruct.Pin = Clock_host_Pin|Data_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : Clock_device_Pin */
  GPIO_InitStruct.Pin = Clock_device_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Clock_device_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : User_Button_Pin */
  GPIO_InitStruct.Pin = User_Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(User_Button_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

}

/* USER CODE BEGIN 4 */
void saveKeySettings(){

	HAL_FLASH_Unlock();
	static FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.Sector = FLASH_SECTOR_2;
	EraseInitStruct.NbSectors = 1;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_2;
	static uint32_t SectorError;
	HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
	uint32_t startaddres = 0x08008000;
	uint32_t endaddres = 0x0800800A;
	int i = 0;
	while(startaddres<endaddres){
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,startaddres,OutBuffer[i])==HAL_OK){
			startaddres += 1;
			i +=1;

		}
	}
	HAL_FLASH_Lock();

}

void readDataFromFlash(){

	uint32_t startaddress = 0x08008000;
		uint32_t endaddress = 0x0800800A;
		int i = 0;
		while(startaddress<endaddress){
			keyMapsHid.keys[i] = *(__IO uint8_t *)startaddress;
				startaddress += 1;
				i +=1;


		}

}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	if(hadc == &hadc1){

		adc[channel] = HAL_ADC_GetValue(&hadc1);
		channel++;
		if(channel == 2) channel = 0;

	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == Clock_device_Pin && preSendingState == false) {
		if (HAL_GPIO_ReadPin(Clock_device_GPIO_Port, Clock_device_Pin)
				== GPIO_PIN_RESET && Sending == false) {

			PS2_Handle_Reciving();

		} else if(HAL_GPIO_ReadPin(Clock_device_GPIO_Port, Clock_device_Pin)
				== GPIO_PIN_RESET && Sending == true){
			PS2_Handle_Sending();


		}
}else if(GPIO_Pin == User_Button_Pin){
		if(HAL_GPIO_ReadPin(User_Button_GPIO_Port, User_Button_Pin) == GPIO_PIN_SET){
			keyboardHID.keycode[0] = 0x04;
			USBD_HID_SendReport(&hUsbDeviceFS, &keyboardHID, sizeof(struct keyboardHID_t));
		}else{
			keyboardHID.keycode[0] = 0x00;
			USBD_HID_SendReport(&hUsbDeviceFS, &keyboardHID, sizeof(struct keyboardHID_t));
		}


	}
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim == &htim10){

		HAL_TIM_Base_Stop_IT(&htim10);
		//end = HAL_GetTick();
		//diff = end - start;
		HAL_GPIO_WritePin(Data_GPIO_Port,Data_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(Clock_host_GPIO_Port,Clock_host_Pin,GPIO_PIN_SET);
		preSendingState = false;
		Sending = true;



	}else if(htim == &htim11) {
		if(first == true){
			uint8_t toSend =0xFF;
			SendPackage(&toSend);
			first = false;
		}else if(StreamMode == false && Sending == false && Reciving == false){
				  uint8_t toSend;
				  if(Recived){
					  Handle_Recived();
				  }
				  else if(intellimouseState >8){
					  toSend = 0xF4;
					  SendPackage(&toSend);

				  }else{
					  switch(intellimouseState++){
					  case 1: toSend = 0xC8;
						  break;
					  case 3: toSend = 0x64;
						  break;
					  case 5: toSend = 0x50;
						  break;
					  case 6: toSend = 0xE8;
					  		break;
					  case 7: toSend = 0x03;
					  		break;
					  case 8: toSend = 0xE7;
					  		break;
					  default: toSend = 0xF3;
						  break;
					  }
					  SendPackage(&toSend);
				  }
			  }
	}
}

void Handle_Recived(){
	int process;
	 if(LastProcessed >= 50){
		process = 0;
		LastProcessed = 0;
	}else{
		process = LastProcessed++;
	}
	switch(AllRecived[process-1]){
	case 0xFA:if(dataSendPacket == 0xF4){
			StreamMode = true;
			HAL_TIM_Base_Stop_IT(&htim11);
			}
		break;
	case 0xAA:
		break;
	case 0x00:
		break;
	case 0xFE: SendPackage(&dataSendPacket);
		break;
	case 0xFC: SendPackage(&dataSendPacket);
		break;
	case 0x03:bytesCount = 4;
		break;
	case 0x04:bytesCount = 4;
		break;

	}
	if(process == LastRecived){
		Recived = false;
	}

}

void PS2_Handle_Sending() {
	if (Reciving == true) {
		bitnumber = 0;
		Reciving = false;
	}else{
		++bitnumber;
		switch(bitnumber){
		case 9:
			setParityBit();
			break;
		case 10:
			HAL_GPIO_WritePin(Data_GPIO_Port,Data_Pin,GPIO_PIN_SET);
			break;
		case 11:
			Sending = false;
			bitnumber = 0;
			break;
		default:
			setNextDataBit(bitnumber);
			break;
		}

	}

}

void setNextDataBit(uint32_t bitnumber){
	uint8_t k = 1;
	if (dataSendPacket & (k << (bitnumber - 1))) {
		HAL_GPIO_WritePin(Data_GPIO_Port,Data_Pin,GPIO_PIN_SET);
	}else{
		HAL_GPIO_WritePin(Data_GPIO_Port,Data_Pin,GPIO_PIN_RESET);
	}

}

void setParityBit(){
	int count = calculateCount(dataSendPacket);
	if (count%2 == 0){
		HAL_GPIO_WritePin(Data_GPIO_Port,Data_Pin,GPIO_PIN_SET);
	}else{
		HAL_GPIO_WritePin(Data_GPIO_Port,Data_Pin,GPIO_PIN_RESET);
	}
}

void SendPackage(uint8_t *data){
	AllSended[LastRecived]= *data;
	dataSendPacket=*data;
	if(Reciving == true && bitnumber>9){
		isSendingWaiting = true;
	}else{
		preSendingState = true;
		HAL_GPIO_WritePin(Clock_host_GPIO_Port,Clock_host_Pin,GPIO_PIN_RESET);
		HAL_TIM_Base_Start_IT(&htim10);
		start = HAL_GetTick();
	}

}



void PS2_Handle_Reciving() {
	if (bitnumber == 0) {
		dataRecivedPacket = 0;
		Reciving = true;
		bitnumber++;

	} else if (bitnumber < 9 && Reciving == true) {
		readBit();
		bitnumber++;
	} else if (bitnumber == 9) {
		checkParity();
		bitnumber++;
	} else {
		Reciving = false;
		bitnumber = 0;

			if(LastRecived == 50 || (LastRecived >= 3 && StreamMode == true)){
						LastRecived = 0;
			}else{
				++LastRecived;
			}
			Recived = true;


		if(isSendingWaiting == true){
			SendPackage(&dataSendPacket);
		}
	}
}

void checkParity() {
	unsigned int x = HAL_GPIO_ReadPin(Data_GPIO_Port, Data_Pin);
	int count = calculateCount(dataRecivedPacket);


	if ((count + x)%2 == 1 && (StreamMode == true && LastRecived == 3)) {

			  mouseHID.buttons = setButtons();
			  mouseHID.x = setBit(1);
			  mouseHID.y = -setBit(2);
			  mouseHID.wheel = setBit(3);
		USBD_HID_SendReport(&hUsbDeviceFS, &mouseHID, sizeof(struct mouseHID_t));
	}

}

int8_t setBit(int i){
	return AllRecived[i];
}

uint8_t setButtons(){
	uint8_t toSend;
	toSend = AllRecived[0];
	toSend = toSend<<5;

	return  toSend>>5;

}

int calculateCount(uint8_t data){
	int count = 0, i, b = 1;

		for (i = 0; i < 8; i++) {
			if (data & (b << i)) {
				count++;
			}
		}
	return count;
}


void readBit() {
	unsigned int x = HAL_GPIO_ReadPin(Data_GPIO_Port, Data_Pin);
	dataRecivedPacket = dataRecivedPacket + ((x%2)<<(bitnumber-1));

	if(LastRecived == 50 || (LastRecived == 3 && StreamMode == true)){
		AllRecived[0]=dataRecivedPacket;
	}else{
		AllRecived[LastRecived+1]=dataRecivedPacket;
	}


}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
