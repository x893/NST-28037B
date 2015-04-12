//*** <<< Use Configuration Wizard in Context Menu >>> ***
#ifndef __X893_GPIO_H__
#define __X893_GPIO_H__

#include "stm32f10x.h"

#define PA0		0
#define PA1		1
#define PA2		2
#define PA3		3
#define PA4		4
#define PA5		5
#define PA6		6
#define PA7		7
#define PA8		8
#define PA9		9
#define PA10	10
#define PA11	11
#define PA12	12
#define PA13	13
#define PA14	14
#define PA15	15

#define PB0		16
#define PB1		17
#define PB2		18
#define PB3		19
#define PB4		20
#define PB5		21
#define PB6		22
#define PB7		23
#define PB8		24
#define PB9		25
#define PB10	26
#define PB11	27
#define PB12	28
#define PB13	29
#define PB14	30
#define PB15	31

typedef unsigned char PinMode_Type;

#if defined( STM32F2XX ) || defined( STM32F4XX )

	#define PinMode_AIn			(PinMode_Type)(GPIO_Mode_AIN)
	#define PinMode_In			(PinMode_Type)(GPIO_Mode_IN  | (GPIO_PuPd_NOPULL << 2))
	#define PinMode_In_PD		(PinMode_Type)(GPIO_Mode_IN  | (GPIO_PuPd_DOWN   << 2))
	#define PinMode_In_PU		(PinMode_Type)(GPIO_Mode_IN  | (GPIO_PuPd_UP     << 2))

	#define PinMode_Out_OD		PinMode_Out_OD_50M
	#define PinMode_Out_OD_2M	(PinMode_Type)(GPIO_Mode_OUT | (GPIO_OType_OD << 6) | (GPIO_Speed_2MHz   << 4))
	#define PinMode_Out_OD_10M	(PinMode_Type)(GPIO_Mode_OUT | (GPIO_OType_OD << 6) | (GPIO_Speed_10MHz  << 4))
	#define PinMode_Out_OD_50M	(PinMode_Type)(GPIO_Mode_OUT | (GPIO_OType_OD << 6) | (GPIO_Speed_50MHz  << 4))
	#define PinMode_Out_OD_100M	(PinMode_Type)(GPIO_Mode_OUT | (GPIO_OType_OD << 6) | (GPIO_Speed_100MHz << 4))

	#define PinMode_Out_PP		PinMode_Out_PP_50M
	#define PinMode_Out_PP_2M	(PinMode_Type)(GPIO_Mode_OUT | (GPIO_OType_PP << 6) | (GPIO_Speed_2MHz   << 4))
	#define PinMode_Out_PP_10M	(PinMode_Type)(GPIO_Mode_OUT | (GPIO_OType_PP << 6) | (GPIO_Speed_10MHz  << 4))
	#define PinMode_Out_PP_50M	(PinMode_Type)(GPIO_Mode_OUT | (GPIO_OType_PP << 6) | (GPIO_Speed_50MHz  << 4))
	#define PinMode_Out_PP_100M	(PinMode_Type)(GPIO_Mode_OUT | (GPIO_OType_PP << 6) | (GPIO_Speed_100MHz << 4))
	
	#define PinMode_AF_OD		(PinMode_Type)(GPIO_Mode_AF  | (GPIO_OType_OD << 6) | (GPIO_Speed_50MHz  << 4))

	#define PinMode_AF_PP		PinMode_AF_PP_50M
	#define PinMode_AF_PP_2M	(PinMode_Type)(GPIO_Mode_AF  | (GPIO_OType_PP << 6) | (GPIO_Speed_2MHz   << 4))
	#define PinMode_AF_PP_10M	(PinMode_Type)(GPIO_Mode_AF  | (GPIO_OType_PP << 6) | (GPIO_Speed_10MHz  << 4))
	#define PinMode_AF_PP_50M	(PinMode_Type)(GPIO_Mode_AF  | (GPIO_OType_PP << 6) | (GPIO_Speed_50MHz  << 4))
	#define PinMode_AF_PP_100M	(PinMode_Type)(GPIO_Mode_AF  | (GPIO_OType_PP << 6) | (GPIO_Speed_100MHz << 4))
	
	#define PinMode_AF_PP_PU	(PinMode_Type)(GPIO_Mode_AF  | (GPIO_OType_PP << 6) | (GPIO_PuPd_UP << 2) | (GPIO_Speed_50MHz  << 4))

#elif defined( STM32F10X_HD ) || defined( STM32F10X_LD_VL )

	#define PinMode_AIn			(PinMode_Type)(GPIO_Mode_AIN)
	#define PinMode_In			(PinMode_Type)(GPIO_Mode_IN_FLOATING)
	#define PinMode_In_PD		(PinMode_Type)(GPIO_Mode_IPD)
	#define PinMode_In_PU		(PinMode_Type)(GPIO_Mode_IPU)

	#define PinMode_Out_OD		PinMode_Out_OD_50M
	#define PinMode_Out_OD_2M	(PinMode_Type)(GPIO_Mode_Out_OD | GPIO_Speed_2MHz)
	#define PinMode_Out_OD_10M	(PinMode_Type)(GPIO_Mode_Out_OD | GPIO_Speed_10MHz)
	#define PinMode_Out_OD_50M	(PinMode_Type)(GPIO_Mode_Out_OD | GPIO_Speed_50MHz)

	#define PinMode_Out_PP		PinMode_Out_PP_50M
	#define PinMode_Out_PP_2M	(PinMode_Type)(GPIO_Mode_Out_PP | GPIO_Speed_2MHz)
	#define PinMode_Out_PP_10M	(PinMode_Type)(GPIO_Mode_Out_PP | GPIO_Speed_10MHz)
	#define PinMode_Out_PP_50M	(PinMode_Type)(GPIO_Mode_Out_PP | GPIO_Speed_50MHz)

	#define PinMode_AF_OD		(PinMode_Type)(GPIO_Mode_AF_OD  | GPIO_Speed_50MHz)
	#define PinMode_AF_PP		(PinMode_Type)(GPIO_Mode_AF_PP  | GPIO_Speed_50MHz)

#else
	#error "CPU type not define"
#endif

void	pinMode		(uint8_t pin, PinMode_Type mode);
void	pinLow		(uint8_t pin);
void	pinHigh		(uint8_t pin);
void	pinToggle	(uint8_t pin);
uint8_t	pinRead		(uint8_t pin);
void	pinRemap	(uint8_t pin, uint8_t GPIO_AF);

#endif
