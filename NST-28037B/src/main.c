#include "NST-28037B.h"

#define COMMAND_SET		0x14	// CTRL-T

#define COMMAND_MODE	0x01
uint8_t StatusFlags;

#define UART_RX_SIZE   2048
#define UART_TX_SIZE   16

char RxBuffer[UART_RX_SIZE];
char TxBuffer[UART_TX_SIZE];

uint16_t  RxGet, RxPut;
uint16_t  TxGet, TxPut;

__IO uint16_t	TimingDelay;
uint16_t	CursorTimer;

#ifdef USE_TOUCH
uint8_t	AdcStartDelay;
#endif

/**
  * @brief	SysTick interrupt handler
  * @param
  * @retval
  */
void SysTick_Handler(void)
{
#ifdef USE_TOUCH
	if (AdcStartDelay != 0)
	{
		--AdcStartDelay;
		if (AdcStartDelay == 0)
			ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	}
#endif
	if (TimingDelay != 0)
		--TimingDelay;
	if (CursorTimer != 0)
		--CursorTimer;
}

/**
  * @brief	Delay in milliseconds with interrupts
  * @param
  * @retval
  */
void Delay_ms(uint16_t ms)
{
	TimingDelay = ms;
	while (TimingDelay != 0) { }
}

/**
  * @brief	HOST_USART interrupt handler
  * @param
  * @retval
  */
void HOST_USART_IRQHandler()
{
	uint16_t next;
	char data;

	if (USART_GetITStatus(HOST_USART, USART_IT_RXNE) != RESET)
	{
		data = USART_ReceiveData(HOST_USART);
		if (USART_GetFlagStatus(HOST_USART, (USART_FLAG_FE | USART_FLAG_NE | USART_FLAG_ORE)) != RESET)
		{
			data = USART_ReceiveData(HOST_USART);
		}
		else
		{
			next = RxPut + 1;
			if (next == UART_RX_SIZE)
				next = 0;
			if (next != RxGet)
			{
				RxBuffer[RxPut] = data;
				RxPut = next;
			}
			else
			{
				// No space for data, ignore it
			}
		}
	}

	if (USART_GetITStatus(HOST_USART, USART_IT_TXE) != RESET)
	{
		if (TxGet != TxPut)
		{
			USART_SendData(HOST_USART, TxBuffer[TxGet]);
			TxGet++;
			if (TxGet == UART_TX_SIZE)
				TxGet = 0;
		}
		if (TxGet == TxPut)
			USART_ITConfig(HOST_USART, USART_IT_TXE, DISABLE);
	}
}

/**
  * @brief
  * @param
  * @retval
  */
void BSOD(const char *text)
{
	__IO uint32_t delay;

	SetForeground(WHITE);
	SetBackground(BRIGHTBLUE);
	ClearScreen();
	SetFontScale(2);
	DisplayTextAt(0, 0, "Fatal Error:");
	DisplayTextAt(0, 2, text);
	DisplayTextAt(0, 3, "Reboot...");

	for(delay = 8000000; delay != 0; --delay)
	{
		__NOP();	__NOP();
		__NOP();	__NOP();
		__NOP();	__NOP();
		__NOP();	__NOP();
	}

	NVIC_SystemReset();
}

void NMI_Handler()			{	BSOD("NMI");			}
void HardFault_Handler()	{	BSOD("Hard Fault");		}
void MemManage_Handler()	{	BSOD("Mem Manage");		}
void BusFault_Handler()		{	BSOD("Bus Fault");		}
void UsageFault_Handler()	{	BSOD("Usage Fault");	}
void SVC_Handler()			{	BSOD("SVC");			}
void DebugMon_Handler()		{	BSOD("Debug Mon");		}
void PendSV_Handler()		{	BSOD("PendSV");			}

/**
  * @brief
  * @param
  * @retval
  */
void SetupHardware()
{
	NVIC_InitTypeDef NVIC_InitStructure;

#ifdef USE_TOUCH
	ADC_InitTypeDef  ADC_InitStructure;
#endif

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA	|
							RCC_APB2Periph_GPIOB	|
							RCC_APB2Periph_ADC1		|
							RCC_APB2Periph_AFIO,
							ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	//	--------------
	//	Initialize ADC
	//
#ifdef USE_TOUCH
#if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL)
	/* ADCCLK = PCLK2/2 */
	RCC_ADCCLKConfig(RCC_PCLK2_Div2);
	NVIC_InitStructure.NVIC_IRQChannel = ADC1_IRQn;
#else
	/* ADCCLK = PCLK2/4 */
	RCC_ADCCLKConfig(RCC_PCLK2_Div4); 
	NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
#endif
#endif
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

#ifdef USE_TOUCH
	ADC_DeInit(ADC1);
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_Cmd(ADC1, ENABLE);

	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1))
		;
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1))
		;

	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
#endif

	NVIC_InitStructure.NVIC_IRQChannel = HOST_USART_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//	Initialize SysTick
	TimingDelay = 0;
	if (SysTick_Config(SystemCoreClock / 1000))
		BSOD("SysTick not init");
}

/**
  * @brief
  * @param
  * @retval
  */
int UsartAvailable()
{
	return (RxGet == RxPut ? 0 : 1);
}

/**
  * @brief
  * @param
  * @retval
  */
void UsartFlush()
{
	__disable_irq();
	TxGet = TxPut = RxGet = RxPut = 0;
	__enable_irq();
}

/**
  * @brief
  * @param
  * @retval
  */
char UsartRead()
{
	uint16_t next = RxGet;
	char c;
	if (next == RxPut)
		return 0;

	c = RxBuffer[next++];
	if (next == UART_RX_SIZE)
		next = 0;
	RxGet = next;
	return c;
}

/**
  * @brief
  * @param
  * @retval
  */
void UsartWrite(const char * src)
{
	uint16_t next;
	char c;
	while ((c = *src++) != 0)
	{
		next = TxPut + 1;
		if (next == UART_TX_SIZE)
			next = 0;
		while (next == TxGet)
		{	// Wait for buffer free
			USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		}

		TxBuffer[TxPut] = c;
		TxPut = next;
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	}
}

/**
  * @brief
  * @param
  * @retval
  */
void UsartInit(uint32_t speed)
{
	USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	USART_DeInit(USART1);
	UsartFlush();

	USART_InitStructure.USART_BaudRate = speed;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	pinMode(HOST_TXD, PinMode_AF_PP);
	pinMode(HOST_RXD, PinMode_In_PU);

	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

/**
  * @brief
  * @param
  * @retval
  */
int main(void)
{
	char c;

	StatusFlags = 0;

	SetupHardware();
	DisplayInit();

	DisplayTextLine("Speed: 115200");
	UsartInit(115200);

	while (1)
	{
		c = UsartRead();
		if (c != 0)
		{
			if (StatusFlags & COMMAND_MODE)
			{
				if (c == 'X')
					NVIC_SystemReset();
				if (c == 'E')
					ClearScreen();
				StatusFlags &= ~COMMAND_MODE;
			}
			else if (c == COMMAND_SET)
				StatusFlags |= COMMAND_MODE;
			else
				DisplayChar(c);
		}
		DisplayIdleApps();
	}
}
