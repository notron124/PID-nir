#ifndef _DWT_DELAY_H
#define _DWT_DELAY_H

// typedef struct
// {
//    volatile uint32_t DHCSR; /*!< Offset: 0x000 (R/W)  Debug Halting Control and Status Register */
//    volatile uint32_t DCRSR;  /*!< Offset: 0x004 ( /W)  Debug Core Register Selector Register */
//    volatile uint32_t DCRDR; /*!< Offset: 0x008 (R/W)  Debug Core Register Data Register */
//    volatile uint32_t DEMCR; /*!< Offset: 0x00C (R/W)  Debug Exception and Monitor Control Register */
// } CoreDebug_Type;

// /* Debug Exception and Monitor Control Register Definitions */
// #define CoreDebug_DEMCR_TRCENA_Pos         24U                                            /*!< CoreDebug DEMCR: TRCENA Position */
// #define CoreDebug_DEMCR_TRCENA_Msk         (1UL << CoreDebug_DEMCR_TRCENA_Pos)            /*!< CoreDebug DEMCR: TRCENA Mask */

#define DWT_CTRL_CYCCNTENA_Pos              0U                                         /*!< DWT CTRL: CYCCNTENA Position */
#define DWT_CTRL_CYCCNTENA_Msk             (0x1UL /*<< DWT_CTRL_CYCCNTENA_Pos*/)       /*!< DWT CTRL: CYCCNTENA Mask */

// #define CORE_DEBUG_BASE_ADDRESS            0xE000EDF0

// #define CORE_DEBUG   ((CoreDebug_Type *) CORE_DEBUG_BASE_ADDRESS)

typedef struct
{
   volatile uint32_t CTRL;
   volatile uint32_t CYCCNT;
   volatile uint32_t CPICNT;
   volatile uint32_t EXCCNT;
   volatile uint32_t SLEEPCNT;
   volatile uint32_t LSUCNT;
   volatile uint32_t FOLDCNT;
   volatile const uint32_t PCSR;
   volatile uint32_t COMP0;
   volatile uint32_t MASK0;
   volatile uint32_t FUNCTION0;
   volatile uint32_t COMP1;
   volatile uint32_t MASK1;
   volatile uint32_t FUNCTION1;
   volatile uint32_t COMP2;
   volatile uint32_t MASK2;
   volatile uint32_t FUNCTION2;
   volatile uint32_t COMP3;
   volatile uint32_t MASK3;
   volatile uint32_t FUNCTION3;
} DWT_TypeDef;

#define DWT_BASE_ADDRESS   0xE0001000

#define DWT       ((DWT_TypeDef *) DWT_BASE_ADDRESS)

void DWT_init(void);
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);
uint32_t getMillis(void);

#endif
