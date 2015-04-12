#include "stm32f10x.h"
#include <stdbool.h>

#define WAIT_TIMEOUT		4
#define USER_APP_OFFSET		0x800
#define USER_APP_BASE		(FLASH_BASE + USER_APP_OFFSET)

#define SIG1	0x1F

#if defined ( STM32F10X_LD_VL ) || defined ( STM32F10X_LD )	|| defined ( STM32F10X_MD )

	#define FLASH_PAGE_SIZE		0x400
	#define SIG2				0x90

#elif defined ( STM32F10X_HD )

	#define FLASH_PAGE_SIZE		0x800
	#define SIG2				0x91

#else
	#error "CPU type not define"
#endif


#if   defined ( STM32F10XX4 )
	#define FLASH_TOP	((uint32_t)(FLASH_BASE + (16 * 1024)))
	#define SIG3		0x01
#elif defined ( STM32F10XX6 )
	#define FLASH_TOP	((uint32_t)(FLASH_BASE + (32 * 1024)))
	#define SIG3		0x02
#elif defined ( STM32F10XX8 )
	#define FLASH_TOP	((uint32_t)(FLASH_BASE + (64 * 1024)))
	#define SIG3		0x03
#elif defined ( STM32F10XXB )
	#define FLASH_TOP	((uint32_t)(FLASH_BASE + (128 * 1024)))
	#define SIG3		0x04
#elif defined ( STM32F10XXC )
	#define FLASH_TOP	((uint32_t)(FLASH_BASE + (256 * 1024)))
	#define SIG3		0x05
#elif defined ( STM32F10XXD )
	#define FLASH_TOP	((uint32_t)(FLASH_BASE + (384 * 1024)))
	#define SIG3		0x06
#elif defined ( STM32F10XXE )
	#define FLASH_TOP	((uint32_t)(FLASH_BASE + (512 * 1024)))
	#define SIG3		0x07
#elif defined ( STM32F10XXF )
	#define FLASH_TOP	((uint32_t)(FLASH_BASE + (784 * 1024)))
	#define SIG3		0x08
#elif defined ( STM32F10XXG )
	#define FLASH_TOP	((uint32_t)(FLASH_BASE + (1024 * 1024)))
	#define SIG3		0x09
#else
	#error "CPU model not define"
#endif


#define HW_VER		0x01
#define SW_MAJOR	0x01
#define SW_MINOR	0x00

#define FLASH_KEY1	((uint32_t)0x45670123)
#define FLASH_KEY2	((uint32_t)0xCDEF89AB)

typedef void (*pFuncVoid)(void);

typedef	union uint32_ui
{
	uint32_t U32;
	uint16_t U16[2];
	uint8_t  U8[4];
} uint32_u;

typedef	union uint16_ui
{
	uint16_t U16;
	uint8_t  U8[2];
} uint16_u;

typedef struct Context_s
{
	uint32_u	address;
	uint16_u	length;
	uint8_t		boot_delay;
	union {
		uint16_t	buff16[FLASH_PAGE_SIZE / 2];
		uint8_t		buff8[FLASH_PAGE_SIZE];
	} buff;
} Context_t;

Context_t Context;

const char Signature[] = { SIG1, SIG2, SIG3, 0x10 };

/**
  * @brief
  * @param
  * @retval
  */
void SystemInit(void)
{
	RCC_TypeDef * rcc = RCC;
	__IO uint32_t StartUpCounter = 0;

	rcc->CR |= (uint32_t)0x00000001;	// Set HSION bit
	rcc->CFGR &= (uint32_t)0xF8FF0000;	// Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits
	rcc->CR &= (uint32_t)0xFEF6FFFF;	// Reset HSEON, CSSON and PLLON bits
	rcc->CR &= (uint32_t)0xFFFBFFFF;	// Reset HSEBYP bit
	rcc->CFGR &= (uint32_t)0xFF80FFFF;	// Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits
	rcc->CIR = 0x009F0000;				// Disable all interrupts and clear pending bits
	rcc->CFGR2 = 0x00000000;      		// Reset CFGR2 register

	SCB->VTOR = FLASH_BASE;				// Vector Table Relocation in Internal FLASH.

	rcc->CR |= ((uint32_t)RCC_CR_HSEON);	// Enable HSE
	while((rcc->CR & RCC_CR_HSERDY) == 0 && StartUpCounter != HSE_STARTUP_TIMEOUT)
	{	// Wait till HSE is ready and if Time out is reached exit
		StartUpCounter++;
	}

	if ((rcc->CR & RCC_CR_HSERDY) != RESET)
	{
#if defined ( STM32F10X_LD_VL ) || defined ( STM32F10X_LD ) || defined ( STM32F10X_MD )
		rcc->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;	// HCLK = SYSCLK
		rcc->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV1;	// PCLK2 = HCLK
		rcc->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV1;	// PCLK1 = HCLK
		//  PLL configuration:  = (HSE / 2) * 6 = 24 MHz
		rcc->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
		rcc->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLXTPRE_PREDIV1_Div2 | RCC_CFGR_PLLMULL6);
		rcc->CR |= RCC_CR_PLLON;					// Enable PLL
		while((rcc->CR & RCC_CR_PLLRDY) == 0)
		{ }	// Wait till PLL is ready

		// Select PLL as system clock source
		rcc->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
		rcc->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;

		// Wait till PLL is used as system clock source
		while ((rcc->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x08)
		{ }
	}
	else
	{	//	If HSE fails to start-up, the application will have wrong clock
		//	configuration. User can add here some code to deal with this error
		rcc->CFGR |= (RCC_CFGR_PLLSRC_HSI_Div2 | RCC_CFGR_PLLMULL6);
		rcc->CR |= RCC_CR_PLLON;
		while (!(rcc->CR & RCC_CR_PLLRDY))
		{ }
		// Select PLL as system clock source
		rcc->CFGR = ((rcc->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL);
		// Wait till PLL is used as system clock source
		while (!(rcc->CFGR & RCC_CFGR_SWS_PLL))
		{ }
#endif
	}
}

/**
  * @brief
  * @param
  * @retval
  */
void HardwareInit(void)
{
	USART_TypeDef * uart = USART1;
	GPIO_TypeDef * gpioa = GPIOA;
	GPIO_TypeDef * gpiob = GPIOB;
	
	RCC->APB2ENR = (RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_USART1EN);
	AFIO->MAPR  = AFIO_MAPR_SWJ_CFG_1;

	gpioa->CRL = 0x33330000;
	gpioa->CRH = 0x288448B2;
	gpioa->ODR = 0x000025F0;

	gpiob->CRL = 0x22222222;
	gpiob->CRH = 0x22222222;

	uart->BRR  = 0x00000D0;
	uart->CR1  = (USART_CR1_RE | USART_CR1_TE);
	uart->CR1 |= USART_CR1_UE;
}

/**
  * @brief
  * @param
  * @retval
  */
uint8_t getch(void)
{
	uint8_t data;
	USART_TypeDef * uart = USART1;
	uint32_t delay = 1800000UL;

	while (delay != 0)
	{
		--delay;
		if (uart->SR & USART_FLAG_RXNE)
		{
			data = uart->DR;
			if (uart->SR & (USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE))
				data = uart->DR;
			else
				return data;
		}
	}
	return 0;
}

/**
  * @brief
  * @param
  * @retval
  */
void putch(char data)
{
	USART_TypeDef * uart = USART1;
	while (!(uart->SR & USART_FLAG_TXE));
	uart->DR = data;
}

/**
  * @brief
  * @param
  * @retval
  */
void putstr(const char * src)
{
	char ch;
	while ((ch = *src++) != 0)
		putch(ch);
}

/**
  * @brief
  * @param
  * @retval
  */
bool wait_space(void)
{
	if (getch() == ' ')
		return true;
	return false;
}
		
/**
  * @brief	Wait SPACE from host and send 0x14 on receive
  * @param	None
  * @retval
  */
bool wait_space_0x14(void)
{
	if (wait_space())
	{
		putch(0x14);
		return true;
	}
	return false;
}

/**
  * @brief
  * @param
  * @retval
  */
void put_response(uint8_t val)
{
	putch(val);
	putch(0x10);
}

/**
  * @brief	Skip <count> chars from host
  * @param	None
  * @retval	None
  */
void getNch(uint8_t count)
{
	while (count--)
		getch();
}

/**
  * @brief	Erase flash page by <address>
  * @param	page address
  * @retval	true on success
  */
bool erase_page(uint32_t address)
{
	FLASH_Status status;
	FLASH_Unlock();
	status = FLASH_ErasePage(address);
	FLASH_Lock();
	return (status == FLASH_COMPLETE);
}

/**
  * @brief
  * @param	mode	0 compare 
  *					1 write
  * @retval	true	write error or not equal
  */
bool write_pass(uint8_t mode)
{
	Context_t * gc = &Context;
	uint16_t w;
	uint16_t * p_flash;
	uint16_t * p_buff;
	uint16_t * p_addr = (uint16_t *)(FLASH_BASE + gc->address.U32);

	p_flash = p_addr;
	p_buff = &gc->buff.buff16[0];
	
	if (mode)
		FLASH_Unlock();

	for (w = 0; w < gc->length.U16; w += 2)
	{
		if (p_flash >= (uint16_t *)USER_APP_BASE && p_flash < (uint16_t *)FLASH_TOP)
		{
			if (mode)
			{
				if (*p_flash != *p_buff)
				{
					if (FLASH_ProgramHalfWord((uint32_t)p_flash, *p_buff) != FLASH_COMPLETE)
						return true;
				}
				p_flash++;
				p_buff++;
			}
			else if (*p_flash != *p_buff++)
				return true;
		}
		p_flash++;
	}
	if (mode)
		FLASH_Lock();
	return false;
}

/**
  * @brief
  * @param
  * @retval
  */
bool write_page(register Context_t * gc)
{
	if (write_pass(0))
	{	// Not equesl, erase flash page
		if (erase_page(FLASH_BASE + gc->address.U32)
		&&	!write_pass(1)
			)
			return true;
		return false;
	}
	return true;
}

/**
  * @brief
  * @param
  * @retval
  */
void checkUserApp(register Context_t * gc)
{
	if (*(uint8_t *)(USER_APP_BASE + 3) == 0x20
	&&	*(uint8_t *)(USER_APP_BASE + 7) == 0x08
		)
		gc->boot_delay = WAIT_TIMEOUT;
	else
		gc->boot_delay = 0;
}

/**
  * @brief
  * @param
  * @retval
  */
void jumpUserApp(void)
{
	pFuncVoid userMain = (pFuncVoid)(*(uint32_t *)(USER_APP_BASE + 4));
	__set_MSP(*(uint32_t *)(USER_APP_BASE + 0));

	GPIOA->CRH = 0x28844844;
	USART1->CR1 |= USART_CR1_UE;

	userMain();
}

/**
  * @brief
  * @param
  * @retval
  */
int main(void)
{
	uint8_t ch;
	uint16_t w;
	register Context_t * gc = &Context;

	SystemInit();
	HardwareInit();
	checkUserApp(gc);

	if (gc->boot_delay == 0)
		putstr("\nBL 1.0\n");

	while(1)
	{
		ch = getch();
		if (ch == 0)
		{	// Timeout
			// putch('U');
			if (gc->boot_delay != 0)
			{
				gc->boot_delay--;
				if (gc->boot_delay == 0)
					jumpUserApp();
			}
			continue;
		}

		switch (ch)
		{
			case '0':
				break;
			case 'P':	// Enter programming mode, disable autostart
				gc->boot_delay = 0;
				break;
			case 'Q':	// Leave programming mode, enable autostart (if user application vallid)
				checkUserApp(gc);
				break;
			case '1':	// Request programmer ID
				if (wait_space())
					putstr("\x14STM ISP\x10");
				continue;
			case 'A':	// Get bootloader info	A\x80	hardware info
						//						A\x81	Major
						//						A\x82	Minor
						//						A\x98	0x03
						//						An		0x00
				ch = getch();
				if (wait_space_0x14())
				{
					if      (ch == 0x80)	ch = HW_VER;	// Hardware version
					else if (ch == 0x81)	ch = SW_MAJOR;	// Software major version
					else if (ch == 0x82)	ch = SW_MINOR;	// Software minor version
					else if (ch == 0x98)	ch = 0x03;		// Unknown
					else					ch = 0x00;		// Covers various unnecessary responses we don't care about
					put_response(ch);
				}
				continue;
			case 'B':	// Device Parameters  DON'T CARE, DEVICE IS FIXED
				getNch(20);
				break;
			case 'E':	// Parallel programming stuff  DON'T CARE
				getNch(4);
				break;
			case 'U': 	// Set address, little endian.
				gc->address.U8[0] = getch();
				gc->address.U8[1] = getch();
				break;
			case 'u':	// Get device signature bytes
				if (wait_space_0x14())
					putstr(Signature);
				continue;
			case 'V':	// Erase user flash	"Vnnnn "
				getNch(4);
				if (wait_space())
				{
					uint32_t addr_u32 = USER_APP_BASE;
					while (addr_u32 < FLASH_TOP)
					{
						if (!erase_page(addr_u32))
							break;
						addr_u32 += FLASH_PAGE_SIZE;
					}
					putch(0x14);
					put_response(0x0);
				}
				continue;
			case 'd':	// Write memory, length is big endian and is in bytes
				gc->length.U8[1] = getch();
				gc->length.U8[0] = getch();
				if (getch() == 'F')	// Memory type (F - flash, E - eeprom)
				{
					for (w = 0; w < gc->length.U16; w++)
						gc->buff.buff8[w] = getch();	// Store data in buffer, can't keep up with serial data stream whilst programming pages
					if (wait_space())
					{
						for (; w < FLASH_PAGE_SIZE; w++)
							gc->buff.buff8[w] = 0xFF;	// Fill 0xFF up to page end

						if (write_page(gc))
						{
							putch(0x14);
							putch(0x10);
						}
					}
				}
				continue;
			case 't':	// Read memory block mode, length is big endian.
				gc->length.U8[1] = getch();
				gc->length.U8[0] = getch();
				if (getch() == 'F')	// Memory type (F - flash, E - eeprom)
				{
					if (wait_space_0x14())
					{
						uint8_t * addr = (uint8_t *)(FLASH_BASE + gc->address.U32);
						for (w = 0; w < gc->length.U16; w++)
						{
							if (addr >= (uint8_t *)USER_APP_BASE && addr < (uint8_t *)FLASH_TOP)
								putch(*addr);
							else
								putch(0xFF);
							addr++;
						}
						putch(0x10);
					}
				}
				continue;
			default:
				continue;
		}
		if (wait_space_0x14())
			putch(0x10);
	}
}

/**
  * @brief	Exceptions handler
  * @param
  * @retval
  */
void NMI_Handler(void)			__attribute__((alias("ExceptionHandler")));
void HardFault_Handler(void)	__attribute__((alias("ExceptionHandler")));
void MemManage_Handler(void)	__attribute__((alias("ExceptionHandler")));
void BusFault_Handler(void)		__attribute__((alias("ExceptionHandler")));
void UsageFault_Handler(void)	__attribute__((alias("ExceptionHandler")));
void ExceptionHandler(void)		__attribute((noreturn));

void ExceptionHandler(void)
{
	SystemInit();
	NVIC_SystemReset();
	while(1)
	{ }
}
