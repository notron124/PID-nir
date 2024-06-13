#ifndef _PID_H
#define _PID_H
/* Not PID related */
#define ADC1_DR_BASEADDRESS     ((uint32_t)0x4001244C)
#define ADC_TO_DMA_BUFF_SIZE    1

uint32_t adcToDma[ADC_TO_DMA_BUFF_SIZE];

#define ADC_DATA_ARR_LENGTH   8

uint8_t cntrADCData;

uint16_t adcData[ADC_DATA_ARR_LENGTH];

uint16_t averageAdcData;

void CalculateAverageADCValue(uint16_t *values, uint8_t arraySize,  uint16_t *averageValueHolder);

/* PID RELATED */

#define COEFFS_MULTIPLICATION    100

enum PID_modes
{
   /* Work on error of input signal.
    * Used most of the time, because most of the pocesses are self settled
    */
   ON_ERROR = 0,
   /* Work on change of input signal.
    * Used rarly for processes in wich output value affects changing speed of input value.
    */
   ON_RATE,
};

enum PID_direction
{
   /* Default.
    * Increasing output will increase input.
    */
   NORMAL = 0,
   /* Reverse of normal */
   REVERSE,
};
typedef struct
{
   struct
   {
      int16_t Kp;         // Proportional gain
      int16_t Ki;         // Integral gain
      int16_t Kd;         // Differetial gain
   } coefficients;

   struct
   {
      int32_t dt; // Time of iteration in milliseconds
      union
      {
         struct 
         {
            uint8_t mode : 1;       // Refer to PID_modes enum
            uint8_t direction : 1;  // Refer to PID_diretion enum
            uint8_t dummy : 6;
         } flags;

         uint8_t wflags;
      };

      int16_t minOut;   // Minimal output value
      int16_t maxOut;   // Maxmimum output value
#if (PID_INTEGRAL_WINDOW > 0)
      int16_t errors[PID_INTEGRAL_WINDOW];
      uint16_t t;
#endif
   } parameters;

   int16_t setPoint; // Value that PID regulator trying to go to
   int16_t input;    // Value from sensor (Temperature, speed, rpm etc.)
   int32_t output;   // Value from PID regulator that
   int32_t integral; // Integral summ
   int16_t prevInput; // Variable to hold previous input
   uint32_t pidTimer; // Variable for time related thins. Used with CalculateOutputTimed and CalculateOutputInstant.
} PID_TypeDef;

int16_t Constrain(int32_t value, int16_t minValue, int16_t maxValue);
void SetDt(PID_TypeDef* PID, int32_t Dt);
void SetLimits(PID_TypeDef *PID, int16_t minOut, int16_t maxOut);
void SetMode(PID_TypeDef *PID, uint8_t mode);
void SetDirection(PID_TypeDef *PID, uint8_t direction);
void InitPID(PID_TypeDef *PID, int16_t Kp, int16_t Ki, int16_t Kd, int32_t Dt);
int16_t CalculateOutput(PID_TypeDef *PID);
int16_t CalculateOutputTimed(PID_TypeDef *PID);
int16_t CalculateOutputInstant(PID_TypeDef *PID);

PID_TypeDef PIDRegulator;

#endif
