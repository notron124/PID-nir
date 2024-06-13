/*
 * lcd1602.h
 *
 *  Created on: 2 февр. 2018 г.
 *      Author: badrtdinov
 *
 *      #v8
 */

#ifndef LCD1602_H_
#define LCD1602_H_

#define _comm 1
#define _data 0

#define LCD_ENABLE   GPIOB->ODR |= 0x01   // #v8
#define LCD_DISABLE  GPIOB->ODR &= ~0x01 // #v8
#define LCD_DATA     GPIOB->ODR |= 0x02     // #v8
#define LCD_COMMAND  GPIOB->ODR &= ~0x02 // #v8
/* Порт E */
#define PORT_LCD GPIOB->ODR

uint8_t lcd_flags;
#define F_REFRESH_LCD		0x01
#define F_CLEAR_LCD			0x02
#define F_BLINK_PARAM		0x04
#define F_REINIT_LCD		   0x08

#define SET_REFRESH_LCD		lcd_flags |= F_REFRESH_LCD
#define RESET_REFRESH_LCD	lcd_flags &= ~F_REFRESH_LCD
#define REFRESH_LCD			(lcd_flags & F_REFRESH_LCD)

#define SET_CLEAR_LCD		lcd_flags |= F_CLEAR_LCD
#define RESET_CLEAR_LCD		lcd_flags &= ~F_CLEAR_LCD
#define CLEAR_LCD			   (lcd_flags & F_CLEAR_LCD)

#define SET_BLINK_PARAM		lcd_flags |= F_BLINK_PARAM
#define RESET_BLINK_PARAM	lcd_flags &= ~F_BLINK_PARAM
#define TGL_BLINK_PARAM		lcd_flags ^= F_BLINK_PARAM
#define BLINK_PARAM			(lcd_flags & F_BLINK_PARAM)

#define SET_REINIT_LCD		lcd_flags |= F_REINIT_LCD
#define RESET_REINIT_LCD	lcd_flags &= ~F_REINIT_LCD
#define REINIT_LCD			(lcd_flags & F_REINIT_LCD)

void delay_LCD(u32 time);
void PutChr_LCD(u8 status, u16 data);
void PutClear_LCD(void);
void InitLCD(void);
void PutStr_LCD(u8 x, u8 y, const u8 *data, u8 attrib);
void PutDgt_LCD(u8 x, u8 y, u8 numdigits, u16 digit, u8 attrib, u8 lead);

#endif /* LCD1602_H_ */
