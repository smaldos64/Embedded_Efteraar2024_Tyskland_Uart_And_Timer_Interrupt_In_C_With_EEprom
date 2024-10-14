/*
 * Uart_And_Timer_Interrupt.c
 *
 * Created: 08-10-2024 22:14:43
 * Author : ltpe
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "Main.h"
#include "Timer.h"
#include "ProjectDefines.h"
#include "Uart.h"
#include "Timer0.h"
#include "EEprom.h"

// This declaration gives us a default value to put in the generated *.eep file.
// This *.eep file can be downloaded seperately independent of downloading SW 
// in the FlashProm.  
// The value stored in EEprom correspond directly to the timeout period measured
// in milliseconds. This value needs to be converted to give the correct value
// to program in the Timer register.
// If one need a timer interrupt time on 1000 milliseconds, the value 980 should 
// be programmed in the Timer register. A small function should take care of this
// conversion between time in milliseconds and the corresponding setting in the Timer
// register to achieve the Timer interrupt time in milliseconds.
EEMEM uint16_t TimeoutBaseValueInMillisecondsInEEprom = 250;

uint16_t TimeoutBaseValueInMiliseconds;
uint16_t TimeoutBaseValueInTimerRegister;

char RecievedDigitsFromPC[MaxDigitsInTimeoutArray];
uint8_t RecievedDigitsFromPCCounter = 0;
bool ErrorOccured = false;
char ErrorCharacter;
bool NewTimeOutValueShouldBeSet = false;

void ConvertReceivedChar(char *ReceivedChar)
{
	// Den smarte m�de at f� konverteret sm� bogstaver om til store bogstaver og
	// modsat er ved brug af Xor, som vi tidligere har set. S� kan vi klare det i
	// �n linje kode.
	//*ReceivedChar ^= Upper_Lower_Bit_Value;
	if ( ((*ReceivedChar >= 0x41) && (*ReceivedChar <= 0x5D)) ||
	     ((*ReceivedChar >= 0x61) && (*ReceivedChar <= 0x7D)))
	{
		*ReceivedChar = *ReceivedChar ^ Upper_Lower_Bit_Value;
	}
}

void ReceiveNewTimeoutValue(char *ReceivedChar)
{
	char SubStr[MaxDigitsInTimeoutArray + 1];
	
	if (isdigit(*ReceivedChar) && RecievedDigitsFromPCCounter < SizeOFArray(RecievedDigitsFromPC))
	{
		RecievedDigitsFromPC[RecievedDigitsFromPCCounter++] = *ReceivedChar;
		//RecievedDigitsFromPCCounter = ++RecievedDigitsFromPCCounter % 
	}
	else if (TimeoutValueEndCharacter == *ReceivedChar)
	{
		strncpy(SubStr, (const char *)RecievedDigitsFromPC, RecievedDigitsFromPCCounter);
		SubStr[RecievedDigitsFromPCCounter] = CharArrayEndCharacter;
		
		TimeoutBaseValueInMiliseconds = (uint16_t)strtoul(SubStr, 
		                                                  NULL, 
											              NumberBase);
											 
		RecievedDigitsFromPCCounter = 0;
		for (uint8_t Counter = 0; Counter < SizeOFArray(RecievedDigitsFromPC); Counter++)
		{
			RecievedDigitsFromPC[Counter] = CharArrayEndCharacter;
		}
		
		if (0 != TimeoutBaseValueInMiliseconds)
		{
			NewTimeOutValueShouldBeSet = true;
		}
	}
	else
	{
		ErrorOccured = true;
		ErrorCharacter = *ReceivedChar;
	}
}

uint16_t ConvertFromTimeOutValueInMilliseconds_ToTimerRegisterValue(uint16_t TimeoutBaseValueInMiliseconds)
{
	uint32_t Nominator = (uint32_t)((uint32_t)TimeoutBaseValueInMiliseconds * (uint32_t)VariableValue1SecValue8Bit);
	return (uint16_t)(Nominator/1000);
	//return (uint16_t)((uint32_t)(TimeoutBaseValueInMiliseconds * VariableValue1SecValue8Bit)/1000);
}

int main(void)
{
	DDRB |= (1 << GreenLedBit) | (1 <<YellowLedBit) | (1 <<RedLedBit);
	
	TimeoutBaseValueInMiliseconds = EEprom_Read_Word((uint16_t)&TimeoutBaseValueInMillisecondsInEEprom);
	if ( (0 == TimeoutBaseValueInMiliseconds) || (0xFFFF == TimeoutBaseValueInMiliseconds) )
	{
		TimeoutBaseValueInMiliseconds = TimeoutBaseValueDefaultInMilliseconds;
	}
	TimeoutBaseValueInTimerRegister = ConvertFromTimeOutValueInMilliseconds_ToTimerRegisterValue(TimeoutBaseValueInMiliseconds);
	
	RS232Init();
	//Setup_Timer0_Overflow_Interrupt(VariableValue2SecValue8Bit);
	Setup_Timer0_Overflow_Interrupt(TimeoutBaseValueInTimerRegister);
	
	Enable_Timer0_Overflow_Interrupt();
	Enable_UART_Receive_Interrupt();
	
	printf("The Timer Interrupt Value is : %dms\n", TimeoutBaseValueInMiliseconds);
	
	sei();
	
    /* Replace with your application code */
    while (1) 
    {
		if (ErrorOccured)
		{
			ErrorOccured = false;
			printf("%c is not a valid character. Type from start again !!!", ErrorCharacter);
		}
		
		if (NewTimeOutValueShouldBeSet)
		{
			NewTimeOutValueShouldBeSet = false;
			printf("The new Timer Interrupt Value is : %dms\n", TimeoutBaseValueInMiliseconds);
			TimeoutBaseValueInTimerRegister = ConvertFromTimeOutValueInMilliseconds_ToTimerRegisterValue(TimeoutBaseValueInMiliseconds);
			EEprom_Write_Word((uint16_t)&TimeoutBaseValueInMillisecondsInEEprom, TimeoutBaseValueInMiliseconds);
			
			Disable_Timer0_Overflow_Interrupt();
			Setup_Timer0_Overflow_Interrupt(TimeoutBaseValueInTimerRegister);
			Enable_Timer0_Overflow_Interrupt();
		}
    }
}

