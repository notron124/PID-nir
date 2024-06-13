#include "stm32f10x.h"
#include "PID.h"
#include "dwt_delay.h"
#include "glob.h"

/* Limits the value between min and max */
int32_t Constrain(int32_t value, int32_t minValue, int32_t maxValue)
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
void SetLimits(PID_TypeDef* PID, int32_t minOut, int32_t maxOut)
{
   PID->parameters.minOut = minOut;
   PID->parameters.maxOut = maxOut;
}

/* Sets the mode of PID. Refer to PID_modes enum in PID.h for more info. */
void SetMode(PID_TypeDef *PID, uint8_t mode)
{ 
   PID->parameters.flags.bits.mode = mode;
}

/* Sets the direction of PID. Refer to PID_direction enum in PID.h for more info. */
void SetDirection(PID_TypeDef *PID, uint8_t direction)
{
   PID->parameters.flags.bits.direction = direction;
}

/* This function initialises the PID regulator with your coefficients and default parameters.
 * To change parameters use dedicated functions.
 */
void InitPID(PID_TypeDef *PID, int32_t Kp, int32_t Ki, int32_t Kd, int32_t Dt)
{
	if (Dt > 0)
		SetDt(PID, Dt);
	else
		SetDt(PID, 100);

   PID->coefficients.Kp = Kp;
   PID->coefficients.Ki = Ki;
   PID->coefficients.Kd = Kd;

   PID->parameters.minOut = -100;
   PID->parameters.maxOut = 100;
}

/* Cleares data that left from previous process:
   - output
   - integral
   - prevInput
   - errorAccumulator
   - dCntr
   - dBuffer
*/
void ResetAccumulatedData(PID_TypeDef *PID)
{
   PID->output = 0;
   PID->integral = 0;
   PID->prevInput = 0;
   PID->errorAccumulator = 0;
   PID->dCntr = 0;

   for (uint8_t i = 0; i < D_BUFFER_SIZE; i++)
      PID->dBuffer[i] = 0;
}

/* Main fucntion of PID regilator, calcilates output according to parameters and defines.
 * Note: All coeffs multiplied by 100 to avoid using float.
   It means Kp/Ki/Kd = 1 equals Kp/Ki/Kd = 0.01
*/
int32_t CalculateOutput(PID_TypeDef *PID)
{
   int32_t error = PID->setPoint - PID->input;    // An error of regulation
   int32_t dInput = PID->prevInput - PID->input;  // Change of input signal over dt time
   PID->prevInput = PID->input;                    // Save prevInput

   if (PID->parameters.flags.bits.direction)
   {
      error = -error;
      dInput = -dInput;
   }

   PID->output = PID->parameters.flags.bits.mode ? 0 : (error * PID->coefficients.Kp) / (int32_t)COEFFS_MULTIPLICATION; // Proportional part
   proportional = PID->output;

   if (PID->dCntr++ >= D_BUFFER_SIZE)
      PID->dCntr = 0;

   /* Differetial part. Multiplying by 1000 to "convert" dt to seconds without float. */
   PID->dBuffer[PID->dCntr] = (dInput * PID->coefficients.Kd * (int32_t)1000 / PID->parameters.dt) / (int32_t)COEFFS_MULTIPLICATION;

   int32_t averageDPart = 0;

   for (uint8_t i = 0; i < D_BUFFER_SIZE; i++)
      averageDPart += PID->dBuffer[i];

   PID->output += averageDPart / (int32_t)D_BUFFER_SIZE;
   differetial = averageDPart / (int32_t)D_BUFFER_SIZE;

/* Integral window mode */
#if (PID_INTEGRAL_WINDOW > 0)
   if (++t >= PID_INTEGRAL_WINDOW)
      t = 0;
   
   PID->integral -= errors[t];
   errors[t] = (error * PID->coefficients.Ki * PID->parameters.dt / (int32_t)1000) / COEFFS_MULTIPLICATION;
   PID->integral += errors[t];
#else
   /* This is made so integral part would accumulate on small errors. 
    * It is needed because we don't use float and when division by COEFFS_MULTIPLICATION on low kI happens
    * it just floors down the value, and not accumulating it.
    */   
   PID->errorAccumulator += error * PID->coefficients.Ki * PID->parameters.dt / (int32_t)1000;
#ifdef PID_OPTIMIZED_I
   PID->output = Constrain(PID->output, PID->parameters.minOut, PID->parameters.maxOut);

   if (PID->output == PID->parameters.maxOut)
      PID->errorAccumulator = 0;

   // if (PID->output == PID->parameters.minOut && PID->errorAccumulator < 0)
   //    PID->errorAccumulator = 0;
      
   PID->errorAccumulator = Constrain(PID->errorAccumulator, ((PID->parameters.minOut * COEFFS_MULTIPLICATION - PID->output) * (int32_t)COEFFS_MULTIPLICATION * (int32_t)1000) / (PID->coefficients.Ki * PID->parameters.dt),
                                     ((PID->parameters.maxOut * COEFFS_MULTIPLICATION - PID->output) * (int32_t)COEFFS_MULTIPLICATION * (int32_t)1000) / (PID->coefficients.Ki * PID->parameters.dt));
#endif

   PID->errorAccumulator = Constrain(PID->errorAccumulator, PID->parameters.minOut * COEFFS_MULTIPLICATION, PID->parameters.maxOut * COEFFS_MULTIPLICATION);

   PID->integral = PID->errorAccumulator / (int32_t)COEFFS_MULTIPLICATION;
#endif

/* Experimental mode of limitation of integrall summ*/
#ifdef PID_OPTIMIZED_I
   if (PID->coefficients.Ki != 0)
   {
      // PID->output = Constrain(PID->output, PID->parameters.minOut, PID->parameters.maxOut);
      PID->integral = Constrain(PID->integral, ((PID->parameters.minOut - PID->output) * (int32_t)COEFFS_MULTIPLICATION * (int32_t)1000) / (PID->coefficients.Ki * PID->parameters.dt), \
                                               ((PID->parameters.maxOut - PID->output) * (int32_t)COEFFS_MULTIPLICATION * (int32_t)1000) / (PID->coefficients.Ki * PID->parameters.dt));

      if (PID->input >= PID->setPoint && PID->integral < 0)
      {
         PID->errorAccumulator = 0;
         PID->integral = 0;
      }
   }
     
#endif

   if (PID->parameters.flags.bits.mode)
      PID->integral += (dInput * PID->coefficients.Kp) / (int32_t)COEFFS_MULTIPLICATION;

   PID->integral = Constrain(PID->integral, PID->parameters.minOut, PID->parameters.maxOut);
   PID->output += PID->integral;
   PID->output = Constrain(PID->output, PID->parameters.minOut, PID->parameters.maxOut);
   
   return PID->output;
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
