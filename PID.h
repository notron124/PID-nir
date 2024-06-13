#ifndef _PID_H
#define _PID_H

#define PID_OPTIMIZED_I

#define COEFFS_MULTIPLICATION    100

#define D_BUFFER_SIZE   8

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

enum PID_calculateSetpoint 
{
   _false = 0,
   _true,
};

typedef struct
{
   int32_t Kp; // Proportional gain
   int32_t Ki; // Integral gain
   int32_t Kd; // Differetial gain
} PID_Coefficients;

typedef struct
{
   uint8_t mode : 1;              // Refer to PID_modes enum
   uint8_t direction : 1;         // Refer to PID_diretion enum
   uint8_t calculateSetpoint : 1; // can be _true or _flase, optional flag for when you want heating with certain speed
   uint8_t dummy : 5;
} PID_Bits;

typedef union
{
   PID_Bits bits;
   uint8_t flagsHolder;
} PID_Flags;

typedef struct
{
   int32_t dt; // Time of iteration in milliseconds

   PID_Flags flags;

   int32_t minOut; // Minimal output value
   int32_t maxOut; // Maxmimum output value
#if (PID_INTEGRAL_WINDOW > 0)
   int16_t errors[PID_INTEGRAL_WINDOW];
   uint16_t t;
#endif
} PID_Parameters;


typedef struct
{
   PID_Coefficients coefficients;
   PID_Parameters parameters;

   int32_t setPoint; // Value that PID regulator trying to go to
   int32_t input;    // Value from sensor (Temperature, speed, rpm etc.)
   int32_t output;   // Value from PID regulator that
   int32_t integral; // Integral summ
   int32_t prevInput; // Variable to hold previous input
   uint32_t pidTimer; // Variable for time related thins. Used with CalculateOutputTimed and CalculateOutputInstant.
   int32_t errorAccumulator;
   int32_t dCntr;
   int32_t dBuffer[D_BUFFER_SIZE];

} PID_TypeDef;

int32_t Constrain(int32_t value, int32_t minValue, int32_t maxValue);
void SetDt(PID_TypeDef* PID, int32_t Dt);
void SetLimits(PID_TypeDef *PID, int32_t minOut, int32_t maxOut);
void SetMode(PID_TypeDef *PID, uint8_t mode);
void SetDirection(PID_TypeDef *PID, uint8_t direction);
void InitPID(PID_TypeDef *PID, int32_t Kp, int32_t Ki, int32_t Kd, int32_t Dt);
int32_t CalculateOutput(PID_TypeDef *PID);
int16_t CalculateOutputTimed(PID_TypeDef *PID);
int16_t CalculateOutputInstant(PID_TypeDef *PID);

PID_TypeDef PIDRegulator;

#endif
