/*
 * Timer0.c
 *
 * Created: 16-01-2018 05:41:36
 *  Author: ltpe
 */ 

 #include <avr/interrupt.h>
 #include "ProjectDefines.h"
 #include "Timer.h"
 #include "Timer0.h"
 

 static uint16_t Timer0OverflowValue;
 static uint16_t Timer0OverflowCounter = 0;

 void Setup_Timer0_Overflow_Interrupt(uint16_t ValueToVariable)
 {
	 TCNT0 = 0x00;                     // S�t 0 som Timer Counter start v�rdi.

	 TCCR0A = 0x00;                    // Brug Timer 0 som Standard op-T�ller.

	 TIFR0 = 0x00;					   // Clear TOV0 => Clear pending interrupts

	 TCCR0B |= GetBitValuesForClockSelect(Timer8BitPrescalerValue);

	 Timer0OverflowValue = ValueToVariable;
 }

 void Enable_Timer0_Overflow_Interrupt()
 {
	 TIMSK0 |= (1 << TOIE0);   // Enable timer0 overflow interrupt(TOIE0)
 }

 void Disable_Timer0_Overflow_Interrupt()
 {
	 TIMSK0 &= ~(1 << TOIE0);   // Disable timer0 overflow interrupt(TOIE0)
 }

 ISR(TIMER0_OVF_vect)
 {
	Timer0OverflowCounter++;
	if (Timer0OverflowCounter >= Timer0OverflowValue)
	{
		PORTB ^= (1 << RedLedBit);
		Timer0OverflowCounter = 0;
	}
	sei();
 }