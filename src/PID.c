#include "stm32f10x.h"
#include "PID.h"
#include "dwt_delay.h"

/* NOT PID RELATED */
void CalculateAverageADCValue(uint16_t *values, uint8_t arraySize,  uint16_t *averageValueHolder)
{
   *averageValueHolder = 0;
   for (uint8_t c = 0; c < arraySize; c++)
      *averageValueHolder += values[c];

   *averageValueHolder /= arraySize;
}

/* PID RELATED */

/* Limits the value between min and max */
int16_t Constrain(int32_t value, int16_t minValue, int16_t maxValue)
{
   if (value > maxValue)
      value = maxValue;
   else if (value < minValue)
      value = minValue;
   
   return value;
}

/* Sets time of sampling */
void SetDt(PID_TypeDef* PID, int32_t Dt)
{
   PID->parameters.dt = Dt;
}

/* Sets the limits of output parameter */
void SetLimits(PID_TypeDef* PID, int16_t minOut, int16_t maxOut)
{
   PID->parameters.minOut = minOut;
   PID->parameters.maxOut = maxOut;
}

/* Sets the mode of PID. Refer to PID_modes enum in PID.h for more info. */
void SetMode(PID_TypeDef *PID, uint8_t mode)
{ 
   PID->parameters.flags.mode = mode;
}

/* Sets the direction of PID. Refer to PID_direction enum in PID.h for more info. */
void SetDirection(PID_TypeDef *PID, uint8_t direction)
{
   PID->parameters.flags.direction = direction;
}

/* This function initialises the PID regulator with your coefficients and default parameters.
 * To change parameters use dedicated functions.
 */
void InitPID(PID_TypeDef *PID, int16_t Kp, int16_t Ki, int16_t Kd, int32_t Dt)
{
	if (Dt > 0)
		SetDt(PID, Dt);
	else
		SetDt(PID, 100);

   PID->coefficients.Kp = Kp;
   PID->coefficients.Ki = Ki;
   PID->coefficients.Kd = Kd;

   PID->setPoint = 0;
   PID->input = 0;
   PID->output = 0;
   PID->integral = 0;
   PID->prevInput = 0;
   PID->pidTimer = 0;
   PID->parameters.flags.mode = ON_ERROR;
   PID->parameters.flags.direction = NORMAL;
   PID->parameters.minOut = 0;
   PID->parameters.maxOut = 32765;
}

/* Main fucntion of PID regilator, calcilates output according to parameters and defines.
 * Note: All coeffs multiplied by 100 to avoid using float.
   It means Kp/Ki/Kd = 1 equals Kp/Ki/Kd = 0.01
*/
int16_t CalculateOutput(PID_TypeDef *PID)
{
   int16_t error = PID->setPoint - PID->input;    // An error of regulation
   int16_t dInput = PID->prevInput - PID->input;  // Change of input signal over dt time
   PID->prevInput = PID->input;                    // Save prevInput

   if (PID->parameters.flags.direction)
   {
      error = -error;
      dInput = -dInput;
   }

   PID->output = PID->parameters.flags.mode ? 0 : (error * PID->coefficients.Kp) / COEFFS_MULTIPLICATION; // Proportional part

   /* Differetial part. Multiplying by 1000 to "convert" dt to seconds without float. */
   PID->output += (dInput * PID->coefficients.Kd * 1000 / PID->parameters.dt) / COEFFS_MULTIPLICATION;

/* Integral window mode */
#if (PID_INTEGRAL_WINDOW > 0)
   if (++t >= PID_INTEGRAL_WINDOW)
      t = 0;
   
   PID->integral -= errors[t];
   errors[t] = (error * PID->coefficients.Ki * PID->parameters.dt / 1000) / COEFFS_MULTIPLICATION;
   PID->integral += errors[t];
#else
   PID->integral += (error * PID->coefficients.Ki * PID->parameters.dt / 1000) / COEFFS_MULTIPLICATION; // Regular summ of integral summ
#endif

/* Experimental mode of limitation of integrall summ*/
#ifdef PID_OPTIMIZED_I
   PID->output = Constrain(PID->output, PID->parameters.minOut, PID->parameters.maxOut);
   if (PID->coefficients.Ki != 0)
      PID->integral = Constrain(PID->integral, ((PID->parameters.minOut - PID->output) * COEFFS_MULTIPLICATION * 1000) / (PID.coefficients.Ki * PID. parameters.dt), \
                                               ((PID->parameters.maxOut - PID->output) * COEFFS_MULTIPLICATION * 1000) / (PID.coefficients.Ki * PID. parameters.dt));
#endif
   
   if (PID->parameters.flags.mode)
      PID->integral += (dInput * PID->coefficients.Kp) / COEFFS_MULTIPLICATION;

   PID->integral = Constrain(PID->integral, PID->parameters.minOut, PID->parameters.maxOut);
   PID->output += PID->integral;
   PID->output = Constrain(PID->output, PID->parameters.minOut, PID->parameters.maxOut);
   
   return (int16_t)PID->output;
}

/* Calculates output using DWT timer */
int16_t CalculateOutputTimed(PID_TypeDef *PID)
{
   if (getMillis() - PID->pidTimer >= PID->parameters.dt)
   {
      PID->pidTimer = getMillis();
      CalculateOutput(PID);
   }

   return (int16_t)PID->output;
}

/* Calculates the output with time between calls of this function. 
 * It means it sets dt by itself, measuring time between fucntion calls.
 * Time measuring not accurate yet, so use it if you don't need high accuracy.
 */
int16_t CalculateOutputInstant(PID_TypeDef *PID)
{
   SetDt(PID, getMillis() - PID->pidTimer);
   PID->pidTimer = getMillis();
   return CalculateOutput(PID);
}
