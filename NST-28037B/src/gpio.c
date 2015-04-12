#include "stm32f10x.h"
#include "gpio.h"

#define GPIO_PIN2PORT(pin)	(GPIOA_BASE + ((pin & 0xF0) << 6))

//	Pins:
//	0..15	PA0..PA15
//	16..31	PB0..PB15
//	32..47	PC0..PC15
//	48..63	PD0..PD15
//	64..79	PE0..PE15

#if defined( STM32F2XX ) || defined( STM32F4XX )

void pinMode(u8 pin, PinMode_Type mode)
{
	GPIO_TypeDef * GPIOx = (GPIO_TypeDef *)GPIO_PIN2PORT(pin);
	u8 pin2 = (pin & 0x0F) * 2;
	u32 mask = ~(0x03 << pin2);
	pin &= 0x0F;
	
	GPIOx->MODER		= (GPIOx->MODER   & mask) | ((mode & 0x3) << pin2);
	mode >>= 2;
	GPIOx->PUPDR		= (GPIOx->PUPDR   & mask) | ((mode & 0x3) << pin2);
	mode >>= 2;
	GPIOx->OSPEEDR	= (GPIOx->OSPEEDR & mask) | ((mode & 0x3) << pin2);
	mode >>= 2;
	GPIOx->OTYPER		= (GPIOx->OTYPER  & ~(1 << pin)) | ((mode & 0x1)  << pin);
}

void pinLow(u8 pin)
{
	GPIO_TypeDef * GPIOx = (GPIO_TypeDef *)GPIO_PIN2PORT(pin);
	GPIOx->BSRRH = (1 << (pin & 0xF));
}

void pinHigh(u8 pin)
{
	GPIO_TypeDef * GPIOx = (GPIO_TypeDef *)GPIO_PIN2PORT(pin);
	GPIOx->BSRRL = (1 << (pin & 0xF));
}

void pinRemap(uint8_t pin, uint8_t GPIO_AF)
{
	GPIO_TypeDef * GPIOx = (GPIO_TypeDef *)GPIO_PIN2PORT(pin);
	uint8_t pin2 = (pin & 0x7) * 4;
	pin = (pin & 0x0F) >> 3;
	GPIOx->AFR[pin] = (GPIOx->AFR[pin] & ~((uint32_t)0xF << pin2)) | ((uint32_t)GPIO_AF << pin2);
}

void pinToggle(u8 pin)
{
	GPIO_TypeDef * GPIOx = (GPIO_TypeDef *)GPIO_PIN2PORT(pin);
	uint16_t mask = (1 << (pin & 0xF));
	if (GPIOx->ODR & mask)
		GPIOx->BSRRH = mask;
	else
		GPIOx->BSRRL = mask;
}

#elif defined( STM32F10X_HD ) || defined( STM32F10X_LD_VL )

void pinMode(uint8_t pin, PinMode_Type mode)
{
	__IO uint32_t * crx;
	uint32_t mask;
	uint8_t pin2;
	GPIO_TypeDef * port = (GPIO_TypeDef *)GPIO_PIN2PORT(pin);
	
	pin &= 0x0F;
	crx = (pin < 8) ? &(port->CRL) : &(port->CRH);
	pin2 = (pin & 0x07) << 2;
	mask = ~(0x0F << pin2);
	*crx = (*crx & mask) | ((mode & 0x0F) << pin2);
	if (mode == GPIO_Mode_IPD)
		port->BRR  = (1 << pin);
	else if (mode == GPIO_Mode_IPU)
		port->BSRR = (1 << pin);
}

void pinLow(u8 pin)
{
	GPIO_TypeDef * port = (GPIO_TypeDef *)GPIO_PIN2PORT(pin);
	port->BRR = (1 << (pin & 0xF));
}

void pinHigh(u8 pin)
{
	GPIO_TypeDef * port = (GPIO_TypeDef *)GPIO_PIN2PORT(pin);
	port->BSRR = (1 << (pin & 0xF));
}

#else
	#error "CPU type not define"
#endif

u8 pinRead(u8 pin)
{
	GPIO_TypeDef * port = (GPIO_TypeDef *)GPIO_PIN2PORT(pin);
	return (port->IDR & (1 << (pin & 0xF))) ? SET : RESET;
}
