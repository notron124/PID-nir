#include "stm32f10x.h"
#include "dwt_delay.h"

void DWT_init(void)
{
   CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
   DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* Blocking delay in milliseconds.
 * Be aware of variable overflow.
 */
void delay_ms(uint32_t ms)
{
   uint32_t us_tics = ms * (SystemCoreClock / 1000U);
   DWT->CYCCNT = 0U;
   while (DWT->CYCCNT < us_tics)
      ;
}

/* Blocking delay in microsecodns*/
void delay_us(uint32_t us)
{
   uint32_t us_tics = us * (SystemCoreClock / 1000000U);
   DWT->CYCCNT = 0U;
   while (DWT->CYCCNT < us_tics)
      ;
}

/* Returns time in milliseconds */
uint32_t getMillis(void)
{
   return (DWT->CYCCNT / (SystemCoreClock / 1000));
}