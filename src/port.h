#ifndef PORT_H_
#define PORT_H_

#define NULL 0

volatile uint8_t flags;
#define F_1kHz          0x01
#define F_SECOND        0x02
#define F_25Hz          0x04

#define SET_1kHz    flags |= F_1kHz
#define RESET_1kHz  flags &= ~F_1kHz
#define _1kHz       (flags & F_1kHz)

#define SET_SECOND    flags |= F_SECOND
#define RESET_SECOND  flags &= ~F_SECOND
#define TGL_SECOND    flags ^= F_SECOND
#define SECOND       (flags & F_SECOND)

#define SET_25Hz    flags |= F_25Hz
#define RESET_25Hz  flags &= ~F_25Hz
#define _25Hz       (flags & F_25Hz)

#define SET_IO4      GPIOA->ODR |= 0x80
#define RESET_IO4    GPIOA->ODR &= ~0x80
#define TGL_IO4      GPIOA->ODR ^= 0x80

#define PORT_SND		GPIOA->ODR
#define PORT_KEYS		GPIOB->ODR

#define PIN_SND 		0x0001

uint16_t sndDelay;

#define SND_ON			{PORT_SND &= ~PIN_SND; sndDelay = 20;}
#define SND_LONG_ON	{PORT_SND &= ~PIN_SND; sndDelay = 100;}
#define SND_OFF		PORT_SND |= PIN_SND

#define PIN_KEY1		0x0010
#define PIN_KEY2		0x0020
#define PIN_KEY3		0x0040
#define PIN_KEY4		0x0080

#endif
