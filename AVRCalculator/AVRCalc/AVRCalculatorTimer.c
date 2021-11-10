//AVRCalculatorTimer.c : source file for the AVRCalculator Timers
//

#include "AVRCalculator.h"
#include "AVRCalculatorTimer.h"

/////////////////////////////////////////////////////////////////////////////
//AVRCalculatorTimer

void timers_init(void)
{
	//{{WIZARD_MAP(Timers)
	// Timer/Counter0 Clock source: System Clock
	// Timer/Counter0 Clock value: Stopped
	// Timer/Counter0 Mode: CTC, TOP=OCR0
	// Timer/Counter0 Output: Disconnected
	OCR0 = 0x00;
	TCNT0 = 0x00;
	TCCR0 = 0x08;

	// Timer/Counter1 Clock source: System Clock
	// Timer/Counter1 Clock value: 125.000kHz
	// Timer/Counter1 Mode: CTC, TOP=OCR1A
	// Timer/Counter1 Output: A: Disconnected, B: Disconnected
	OCR1A = 1200;//1562;  //~200 ms COMP1A interrupt
	OCR1B = 0;
	TCNT1 = 0;
	TCCR1A = 0b00000000;
	TCCR1B = 0b00001101;

	// Timer/Counter2 Clock source: System Clock
	// Timer/Counter2 Clock value: Stopped
	// Timer/Counter2 Mode: Normal
	// Timer/Counter2 Output: Disconnected
	ASSR = 0x00;
	OCR2 = 0x00;
	TCNT2 = 0x00;
	TCCR2 = 0x00;

	TIMSK = 0b00010000;
	//}}WIZARD_MAP(Timers)
}

