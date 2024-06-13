#include "stm32f10x.h"
#include "debounceAvoid.h"
#include "port.h"
#include "PID.h"

struct Key_TypeDef key1;
struct Key_TypeDef key2;
struct Key_TypeDef key3;
struct Key_TypeDef key4;
struct Keys_Properties keysProperties;




void SysTick_Configuration(void)
{
   SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

   /* 72000000 / (72000000 / 1000) = 1000 */
   SysTick_Config(SystemCoreClock / 1000);
}

void RCC_Configuration(void)
{
   /* Enable clock of used periferals */
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO | RCC_APB2Periph_ADC1, ENABLE);
}

void GPIO_init(void)
{
  //  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure DAC channe1 output pin */
  //  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
  //  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  //  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* 2: Out 2MHz, Push-Pull general
   * 3: Out 50 MHz, Push-Pull general
   * 4: Input, floating
   * B: Out 50 MHz, Alt. func. Push-Pull
   */

  GPIOA->CRH = 0x2222222B; // B;
  GPIOA->CRL = 0x22202222;
  GPIOA->ODR = 0xFFFF;

  GPIOB->ODR = 0;
  GPIOB->CRH = 0x22222222; // #v8
  GPIOB->CRL = 0x44442222; // #v8
}

void TIM1_init(void)
{
   /* See PWM_FREQUENCY */
   TIM1->ARR = 62535;

   /* 110: PWM mode 1 - In upcounting, channel 1 is active as long as TIMx_CNT<TIMx_CCR1
    * else inactive. In downcounting, channel 1 is inactive (OC1REF=‘0’) as long as
    * TIMx_CNT>TIMx_CCR1 else active (OC1REF=’1’).
    * */
   TIM1->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;

   /* On - OC1-3 signal is output on the corresponding output pin */
   TIM1->CCER |= TIM_CCER_CC1E;


   /* Setting up deadtime (1980 ns) */
   TIM1->BDTR = (TIM_BDTR_DTG & 0x48); // DTG=0x10

   /* Complementary output enable */
   // TIM1->CCER |= TIM_CCER_CC1NE | TIM_CCER_CC2NE | TIM_CCER_CC3NE;

   /* Update interrupt enable */
   TIM1->DIER |= TIM_DIER_UIE;

   /* Reinitialize the counter and generates an update of the registers */
   TIM1->EGR |= TIM_EGR_UG;

   /* Enable counter */
   TIM1->CR1 |= TIM_CR1_CEN;

}

void DMA_ADC1_Configuration(void)
{
   ADC_InitTypeDef ADC_InitStructure;
   DMA_InitTypeDef DMA_InitStructure;

   DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_BASEADDRESS;
   DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adcToDma;
   DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
   DMA_InitStructure.DMA_BufferSize = ADC_TO_DMA_BUFF_SIZE;
   DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
   DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
   DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
   DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
   DMA_InitStructure.DMA_Priority = DMA_Priority_High;
   DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

   DMA_Init(DMA1_Channel1, &DMA_InitStructure);
   DMA_Cmd(DMA1_Channel1, ENABLE);

   ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
   ADC_InitStructure.ADC_ScanConvMode = ENABLE;
   ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
   ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
   ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
   ADC_InitStructure.ADC_NbrOfChannel = ADC_TO_DMA_BUFF_SIZE;
   ADC_Init(ADC1, &ADC_InitStructure);

   ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_239Cycles5);

   ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
   ADC_DMACmd(ADC1, ENABLE);
   ADC_Cmd(ADC1, ENABLE);

   ADC_ResetCalibration(ADC1);
   while (ADC_GetResetCalibrationStatus(ADC1))
      ;

   ADC_StartCalibration(ADC1);
   while (ADC_GetCalibrationStatus(ADC1))
      ;
}

/* Initialization of channels 6 and 7 of ADC1 in injected mode
 * First of all we powerup the ADC and start calibration,
 * after that we
 * */
void ADC1_init(void)
{
   /* Wake up from powerdown state */
   ADC1->CR2 |= ADC_CR2_ADON;

   /* Start calibration */
   ADC1->CR2 |= ADC_CR2_CAL;

   /* Wait until calibration is done */
   while ((ADC1->CR2 & ADC_CR2_CAL) > 0)
   {
   }

   /* Injected sequence length - 2 conversion */
   ADC1->JSQR = ADC_JSQR_JL_0;

   /*------ Overcurrent controll setup for channel 6 ------*/

   /* Analog watchdog high threshold - max */
   ADC1->HTR = 1600;

   /* Analog watchdog low threshold - min */
   ADC1->LTR = 0;

   /* Select input Channel 6 for watchdog*/
   ADC1->CR1 |= 0x07;

   /* Configuration for one injected channel */

   /* Enable analog watchdog interrupt */
   ADC1->CR1 |= ADC_CR1_AWDIE; // for overcurrent control

   /* Enable the watchdog on a single channel in scan mode */
   ADC1->CR1 |= ADC_CR1_AWDSGL;

   /* Analog watchdog enable on injected channels */
   ADC1->CR1 |= ADC_CR1_JAWDEN;

   /* -------------------------------------- */

   /* Interrupt enable for injected channels */
   ADC1->CR1 |= ADC_CR1_JEOCIE;

   /* Conversion on external event enabled */
   ADC1->CR2 |= ADC_CR2_JEXTTRIG;

   /* External event select for injected group - 111: JSWSTART */
   ADC1->CR2 |= ADC_CR2_JEXTSEL;

   /* Channel x Sample time selection - 111: 239.5 cycles*/
   ADC1->SMPR2 |= ADC_SMPR2_SMP6;
   ADC1->SMPR2 |= ADC_SMPR2_SMP7;


   /* sequence - 7 6 */
   ADC1->JSQR |= (ADC_JSQR_JSQ3_0 | ADC_JSQR_JSQ3_1 | ADC_JSQR_JSQ3_2 |
   				   ADC_JSQR_JSQ4_2 | ADC_JSQR_JSQ4_1);

   /* Scan mode enabled */
   ADC1->CR1 |= ADC_CR1_SCAN;

   /* Start conversion */
   ADC1->CR2 |= ADC_CR2_ADON;
}

void TIM4_init(void)
{
   TIM4->ARR = 2880;
   TIM4->PSC = 1000;
   TIM4->EGR |= TIM_EGR_UG;
   TIM4->SR &= ~TIM_SR_UIF;
   TIM4->DIER |= TIM_DIER_UIE;
   TIM4->DIER |= TIM_DIER_TIE;
   TIM4->CR1 |= TIM_CR1_CEN;
}

void DAC_init(void)
{
	/* Enable both channels */
	DAC->CR |= (1 << 2);
	DAC->CR |= (1 << 18);
	DAC->CR |= (7 << 3);
	DAC->CR |= (7 << 19);
	DAC->CR |= (1 << 0);
   DAC->CR |= (1 << 16);
}

void NVIC_init(void)
{
   NVIC_SetPriority(ADC1_2_IRQn, 0);
   NVIC_SetPriority(TIM1_UP_IRQn, 1);
   //   NVIC_SetPriority(TIM4_IRQn, 2);

   // if (working_mode)
   //    for (uint8_t c = 0; c < AJDR2_LENGTH; c++)
   //       ajdr2[c] = 410;

   NVIC_EnableIRQ(ADC1_2_IRQn);
   NVIC_EnableIRQ(TIM4_IRQn);
}

void KeyInit(void)
{
	keysProperties.autorepeatSpeed = 50;
	keysProperties.shortPressDelay = 50;
	keysProperties.longPressDelay = 1000;

	key1.GPIOx = GPIOB;
	key2.GPIOx = GPIOB;
	key3.GPIOx = GPIOB;
	key4.GPIOx = GPIOB;

	key1.pin = PIN_KEY1;
	key2.pin = PIN_KEY2;
	key3.pin = PIN_KEY3;
	key4.pin = PIN_KEY4;

	key1.flags.autorepeat = 1;
	key2.flags.autorepeat = 1;
	key3.flags.autorepeat = 1;
	key4.flags.autorepeat = 1;

	key1.shortPressID = 0;
	key2.shortPressID = 1;
	key3.shortPressID = 2;
	key4.shortPressID = 3;

	key1.longPressID = 4;
	key2.longPressID = 5;
	key3.longPressID = 6;
	key4.longPressID = 7;
}

void init(void)
{
   RCC_Configuration();
   SysTick_Configuration();
   GPIO_init();
   TIM4_init();
   DMA_ADC1_Configuration();
   NVIC_init();
   //NVIC_init();
   KeyInit();
}
