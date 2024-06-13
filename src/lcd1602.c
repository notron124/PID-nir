
#include "stm32f10x.h"
#include "char_table.h"
#include "lcd1602.h"

const u16 TableDelay[] = {800, 200, 100, 1200};
const u8 TableInitLCD[] = {0x28, 0x0C, 0x01, 0x06};

const u8 alt_sym[] =
{
   /* Код 0 - Пробел  */
   0b00000000, // 3 clock
   0b00001110,
   0b00010101,
   0b00010111,
   0b00010001,
   0b00001110,
   0b00000000,
   0b00000000,

   /* Код 1 - Нагреватель */
   /*
   0x0A, //0b00001010,
   0x15, //0b00010101,
   0x15, //0b00010101,
   0x15, //0b00010101,
   0x15, //0b00010101,
   0x11, //0b00010001,
   0x11, //0b00010001,
   0x1F, //0b00011111,
   */

   /*
   0x04,	// 1 bell - колокольчик
   0x0E,
   0x0E,
   0x0E,
   0x1F,
   0x04,
   0x00,
   0x00,
   */

   0x00, // 0b00000000,
   0x1F, // 0b00011111, // 1 Wi-Fi
   0x00, // 0b00000000,
   0x0E, // 0b00001110,
   0x00, // 0b00000000,
   0x04, // 0b00000100,
   0x00, // 0b00000000,
   0x00, // 0b00000000,

   /* Код 2 - Значок градуса  */
   0x06, // 0b00000110,
   0x09, // 0b00001001,
   0x09, // 0b00001001,
   0x06, // 0b00000110,
   0x00, // 0b00000000,
   0x00, // 0b00000000,
   0x00, // 0b00000000,
   0x00, // 0b00000000

   /* Код 3 - Процесс идет :)  */
   0x08, // 0b00001000,
   0x0C, // 0b00001100,
   0x0E, // 0b00001110,
   0x0F, // 0b00001111,
   0x0E, // 0b00001110,
   0x0C, // 0b00001100,
   0x08, // 0b00001000,
   0x00, // 0b00000000

   0x00, // 0b00000000,// 4 - "Знак "Меню". #newstick
   0x1F, // 0b00011111,
   0x00, // 0b00000000,
   0x1F, // 0b00011111,
   0x00, // 0b00000000,
   0x1F, // 0b00011111,
   0x00, // 0b00000000,
   0x00, // 0b00000000

   0x00, // 0b00000000, // 5 - "v".
   0x00, // 0b00000000,
   0x00, // 0b00000000,
   0x1F, // 0b00011111,
   0x0E, // 0b00001110,
   0x04, // 0b00000100,
   0x00, // 0b00000000,
   0x00, // 0b00000000,

   0x00, // 0b00000000,// 6 - "^"
   0x00, // 0b00000000,
   0x00, // 0b00000000,
   0x04, // 0b00000100,
   0x0E, // 0b00001110,
   0x1F, // 0b00011111,
   0x00, // 0b00000000,
   0x00, // 0b00000000,

   0x00, // 0b00000000, // 7 - Знак "галочка"
   0x00, // 0b00000000,
   0x01, // 0b00000001,
   0x02, // 0b00000010,
   0x14, // 0b00010100,
   0x08, // 0b00001000,
   0x00, // 0b00000000,
   0x00  // 0b00000000,
};

void delay_LCD(u32 time)
{
   while (time--)
      __NOP();
}

void PutChr_LCD(u8 status, u16 data)
{
   if (status)
      LCD_COMMAND;
   else
      LCD_DATA;

   PORT_LCD &= 0x0FFF; // #v8
   LCD_ENABLE;
   PORT_LCD |= (data & 0x00F0) << 8; // #v8
   __NOP();
   __NOP();
   LCD_DISABLE;
   PORT_LCD &= 0x0FFF; // #v8
   LCD_ENABLE;
   PORT_LCD |= ((data << 4) & 0x00F0) << 8;
   __NOP();
   __NOP();
   LCD_DISABLE;
   delay_LCD(200 * 16);
}

void PutClear_LCD(void)
{
   PutChr_LCD(_comm, 0x01);
   delay_LCD(2000 * 10);
}

void InitLCD(void)
{
   u8 cntr;
   for (cntr = 0; cntr < sizeof(TableInitLCD); cntr++)
   {
      delay_LCD(TableDelay[cntr] * 16);
      PutChr_LCD(_comm, TableInitLCD[cntr]);
   }

   delay_LCD(1000 * 16);
   PutChr_LCD(_comm, 0x40);

   for (cntr = 0; cntr < sizeof(alt_sym); cntr++)
      PutChr_LCD(_data, alt_sym[cntr]);

   PutChr_LCD(_comm, 0x80);
}

void PutStr_LCD(u8 x, u8 y, const u8 *data, u8 attrib)
{
   u8 k;

   k = 0x80 | y;
   // if(y) k = 0xC0;
   // else k = 0x80;

   if (x < 16)
      PutChr_LCD(_comm, k + x);

   while (*data)
   {
      if ((attrib & 0x80) && (BLINK_PARAM))
         PutChr_LCD(_data, 32);
      else
         PutChr_LCD(_data, SymbolToLCD(*data));

      data++;
   }
}

void PutDgt_LCD(u8 x, u8 y, u8 numdigits, u16 digit, u8 attrib, u8 lead)
{
   u8 k, n, no_lead, lcd;
   u16 dgt, mul;

   dgt = digit;
   no_lead = 0;
   k = 0x80 | y;
   // if(y) k = 0xC0;
   // else k = 0x80;

   if (x < 16)
      PutChr_LCD(_comm, k + x);

   for (k = numdigits; k; k--)
   {
      lcd = 0;
      mul = 1;

      for (n = 0; n < (k - 1); n++)
         mul *= 10;

      while (dgt >= mul)
      {
         dgt -= mul;
         lcd++;
      }

      if (lcd)
      {
         no_lead = 1;
         lcd += 48;
      }
      else
      {
         if (no_lead)
            lcd = 48;
         else
         {
            if (lead == '0')
               lcd = 48;
            else
            {
               if (k == 1)
                  lcd = 48;
               else
                  lcd = 32;
            }
         }
      }

      if ((attrib & 0x80) && (BLINK_PARAM))
         PutChr_LCD(_data, 32);
      else
         PutChr_LCD(_data, lcd);
   }
}
