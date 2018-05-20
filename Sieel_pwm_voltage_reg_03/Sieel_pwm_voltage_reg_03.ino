/***********************************************************************
 * PWM DAC controlled Power supply using LM317
 * - based on Arduino Uno/Nano
 * - LM317 regulator
 ***********************************************************************
 * Connections
 * - A0 -> voltage divider at regulator output
 * - D6 -> square wave to negative voltage supply
 * - D9 -> PWM signal - Timer1 - 10 bit
 ***********************************************************************
 * Exemplo desenvolvido para SIEEL 2018 - Workshop
 * "Do mundo real ao digital: condicionamento de sinais", 08/05/2018
 *
 * Configuração PWM(Timer1): http://forum.arduino.cc/index.php?topic=327479.0
 *
 * author: Prof. Haroldo Amaral			agaelema@gmail.com
 * https://github.com/agaelema/
 * https://www.embarcados.com.br/author/agaelema/
 *
 * v0.3 - 2018/08/19
 ***********************************************************************/
#include "Arduino.h"
#include	<stdint.h>

/***********************************************************************
 * 						debug definitions
 ***********************************************************************/
//#define		DEBUG_ENABLED									// habilita debug via serial
//#define		DEBUG_PLOT_ENABLED								// habilita debug via serial

#define		PLOT_SEPARATOR		(',')						// SerialPlot
//#define		PLOT_SEPARATOR		(' ')						// Arduino plotter

#define		DAC_PWM				5							// pino relacionado ao DAC PWM
#define		inverter_osc		6							// pwm usado para tensao negativa

#define		PWM_LIMIT			1023

uint8_t	const average = 32;

int16_t		PWM = 0;

float pwm_bit_voltage = 5.0f/1023;							// tensão por level pwm
float adc_bit_voltage = (5.0f/1023) * 2.0f;					// tensão por bit (tensão máxima de 10V)

float analogVoltage = 0;
uint16_t analog = 0;

float setPoint_float = 0;

/***********************************************************************
 * Set 'TOP' for PWM resolution.  Assumes 16 MHz clock.
 * un-comment one of the choices
 ***********************************************************************/
//const unsigned int TOP = 0xFFFF; // 16-bit resolution.   244 Hz PWM
//const unsigned int TOP = 0x7FFF; // 15-bit resolution.   488 Hz PWM
//const unsigned int TOP = 0x3FFF; // 14-bit resolution.   976 Hz PWM
//const unsigned int TOP = 0x1FFF; // 13-bit resolution.  1953 Hz PWM
//const unsigned int TOP = 0x0FFF; // 12-bit resolution.  3906 Hz PWM
//const unsigned int TOP = 0x07FF; // 11-bit resolution.  7812 Hz PWM
const unsigned int TOP = 0x03FF; // 10-bit resolution. 15624 Hz PWM


/***********************************************************************
 * prototype of functions
 ***********************************************************************/
void PWM16Begin();
void PWM16EnableA();
void PWM16DisableA();
void PWM16EnableB();
void PWM16DisableB();
inline void PWM16A(unsigned int PWMValue);
inline void PWM16B(unsigned int PWMValue);


void setup()
{
	Serial.begin(9600);

	PWM16Begin();											// Initialize TIMER1 in PWM mode

	PWM16A(0);												// Set initial PWM value for Pin 9
	PWM16DisableA();										// disable PWM - force "0"

	analogWrite(inverter_osc, 127);							// activate PWM on pin 6 - used to negative voltage
}


void loop()
{
	/***********************************************************************
	 * check for float value on serial (ASCII format)
	 * - used as setpoint to modify PWM value
	 ***********************************************************************/
	if (Serial.available() > 0)								// if there is data
	{
		setPoint_float = Serial.parseFloat();				// search for float value
		Serial.print("SetPoint: ");	Serial.println(setPoint_float);

		float PWM_voltage;
		// convert voltage to the range of PWM
		PWM_voltage = ((setPoint_float - 1.26f) / pwm_bit_voltage) / 2;

		if (PWM_voltage < 0)								// if less than zero
		{
			PWM_voltage = 0;								// force zero
		}

		PWM = PWM_voltage;									// typecast - float to int
		Serial.print("PWM: "); Serial.println(PWM);

		if (PWM == 0)
		{
			PWM16DisableA();								// force zero
		}
		else
		{
			PWM16EnableA();									// enable PWM HW
			PWM16A(PWM);									// set PWM value
		}
	}

	/***********************************************************************
	 * Average of n conversions - filter noise
	 ***********************************************************************/
	unsigned char i;
	for (i = average; i > 0; i--)
	{
		analog += analogRead(A0);							// sum two values
		analog >>= 1;										// divide by two
	}
	analogVoltage = (float)analog*adc_bit_voltage;			// convert to voltage

	/***********************************************************************
	 * convert float values to integer - faster verification
	 ***********************************************************************/
	uint16_t analogVoltage_int = (unsigned int)(analogVoltage*1000.0);
	uint16_t setPoint_int = (unsigned int)(setPoint_float*1000.0);

	/***********************************************************************
	 * simple closed loop - just proportional action
	 * - use the error to update PWM value
	 ***********************************************************************/
	int16_t error_int = 0;
	// calculate the error
	error_int = (int16_t)(setPoint_int) - (int16_t)(analogVoltage_int);
	error_int >>= 7;										//divide by 128

	if (analogVoltage_int < setPoint_int)					// if output is less than setpoint
	{
		if (PWM < PWM_LIMIT)								// check pwm limit
		{
//			PWM++;											// incrementa o pwm;
			PWM += error_int;								// ajust pwm value
		}
	}
	if (analogVoltage_int > setPoint_int)					// if output is more than setpoint
	{
		if (PWM != 0)										// check if PWM value is greater then zero
		{
//			PWM--;											// decrementa pwm
			PWM += error_int;								// ajust pwm value

			if(PWM < 0)										// if negative ajust value
			{
				PWM = 0;									// force zero
			}
		}
	}
	else;													// do nothing

#if defined (DEBUG_ENABLED)
	Serial.print("PWM: "); Serial.println(PWM);
	Serial.print("Volt: "); Serial.println(analogVoltage);
#endif
#if defined (DEBUG_PLOT_ENABLED)
	Serial.print(PWM); Serial.print(PLOT_SEPARATOR);
	Serial.print(analogVoltage);
	Serial.print('\r'); Serial.print('\n');
#endif

	if (PWM == 0)
	{
		PWM16DisableA();
	}
	else
	{
		PWM16EnableA();
		PWM16A(PWM);
	}

	delay(3);												// small delay - compensate filter settling time
}




void PWM16Begin()
{
  // Stop Timer/Counter1
  TCCR1A = 0;												// Timer/Counter1 Control Register A
  TCCR1B = 0; 												// Timer/Counter1 Control Register B
  TIMSK1 = 0;												// Timer/Counter1 Interrupt Mask Register
  TIFR1 = 0; 												// Timer/Counter1 Interrupt Flag Register
  ICR1 = TOP;
  OCR1A = 0;												// Default to 0% PWM
  OCR1B = 0;												// Default to 0% PWM

  // Set clock prescale to 1 for maximum PWM frequency
  TCCR1B |= (1 << CS10);

  // Set to Timer/Counter1 to Waveform Generation Mode 14: Fast PWM with TOP set by ICR1
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << WGM12) ;
}

void PWM16EnableA()
{
  // Enable Fast PWM on Pin 9: Set OC1A at BOTTOM and clear OC1A on OCR1A compare
  TCCR1A |= (1 << COM1A1);
  pinMode(9, OUTPUT);
}

void PWM16DisableA()
{
  // Enable Fast PWM on Pin 9: Set OC1A at BOTTOM and clear OC1A on OCR1A compare
  TCCR1A &= ~(1 << COM1A1);
  pinMode(9, OUTPUT);
  digitalWrite(9, 0);
}

void PWM16EnableB()
{
  // Enable Fast PWM on Pin 10: Set OC1B at BOTTOM and clear OC1B on OCR1B compare
  TCCR1A |= (1 << COM1B1);
  pinMode(10, OUTPUT);
}

void PWM16DisableB()
{
  // Enable Fast PWM on Pin 10: Set OC1B at BOTTOM and clear OC1B on OCR1B compare
  TCCR1A &= ~(1 << COM1B1);
  pinMode(10, OUTPUT);
  digitalWrite(10, 0);
}

inline void PWM16A(unsigned int PWMValue)
{
  OCR1A = constrain(PWMValue, 0, TOP);
}

inline void PWM16B(unsigned int PWMValue)
{
  OCR1B = constrain(PWMValue, 0, TOP);
}
