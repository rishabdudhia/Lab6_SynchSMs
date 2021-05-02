/*	Author: Rishab Dudhia
 *  Partner(s) Name: 
 *	Lab Section: 022
 *	Assignment: Lab #6  Exercise #3
 *	Exercise Description: [optional - include for your own benefit]
 *	Lab 4 exercise 2 checking for press every 100ms
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0; // TimerISR() sets to 1

unsigned long _avr_timer_M = 1; //start count from here, down to 0. default 1 ms
unsigned long _avr_timer_cntcurr = 0; //current internal count of 1ms ticks

void TimerOn () {
	//AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B; //bit3 = 0: CTC mode (clear timer on compare)
		       //bit2bit1bit0 = 011: pre-scaler / 64
		       //00001011: 0x0B
		       //so, 8 MHz clock or 8,000,000 / 64 = 125,000 ticks/s
		       //thus, TCNT1 register will count at 125,000 ticks/s
	//AVR output compare register OCR1A
	OCR1A = 125; //timer interrupt will be generated when TCNT1 == OCR1A
	             //we want a 1 ms tick. 0.001s * 125,000 ticks/s = 125
		     //so when TCNT1 register equals 125,
		     //1 ms has passed. thus, we compare 125.
		     
	//AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1 = 0;

	_avr_timer_cntcurr = _avr_timer_M;
	//TimerISR will be called every _avr_timer_cntcurr milliseconds
	
	//Enable global interrupts
	SREG |= 0x80; // 0x80: 10000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0 = 000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

//In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect){
	//CPU automatically calls when TCNT1 == OCR1 (every 1ms per TimerOn settings)
	_avr_timer_cntcurr--; // count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

//Set TimerISR() to tick every M ms
void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

enum States {smstart, wait, inc, dec, inc_wait, dec_wait, reset_wait, reset } state;
unsigned char cntA0;
unsigned char cntA1;
void TickSM()
{
    static unsigned char waitA0 = 0;
    static unsigned char waitA1 = 0;
    unsigned char actualB = PINB ;
    //unsigned char tempC = PORTC & 0x03;
    switch(state)
    {
	case smstart:
	    state = wait;
	    PORTB = 0x07;
	    break;
	case wait:
	    if (((~PINA & 0x03) == 0x01) && (PORTB < 0x09))
	    {
		state = inc;
	    	cntA0 = cntA0 + 1;
	    }
	    else if (((~PINA & 0x03) == 0x02) && (PORTB > 0x00))
	    {
		state = dec;
	        cntA1 = cntA1 + 1;
	    }
	    /*else if ((PINA & 0x03) == 0x03)
	    {
		    state = reset;
	    	    cntA0 = cntA0 + 1;
		    cntA1 = cntA1 + 1;
	    }*/
	    else
	    {
		state = wait;
	    }
	    break;
	case inc_wait:
	    if((~PINA & 0x03) == 0x01)
	    {
		waitA0 = waitA0 + 1;
		if (waitA0 > 10)
			state = inc;
		state = inc_wait;
	    }
	    else if ((~PINA & 0x03) == 0x02)
	    {
		state = dec;
		cntA1 = cntA1 + 1;
	    }
	    else if ((~PINA & 0x03) == 0x03)
	    {
		cntA1 = cntA1 + 1;
		state = reset;
	    }
	    else
	    {
		state = wait;
	    }
	    break;
	case dec_wait:
	    if ((~PINA & 0x03) == 0x02)
	    {
		waitA1 = waitA1 + 1;
		if (waitA1 > 10)
			state = dec;
		state = dec_wait; 
	    }
	    else if ((~PINA & 0x03) == 0x01)
	    {
		state = inc;
		cntA0 = cntA0 + 1;
	    }
	    else if ((~PINA & 0x03) == 0x03)
	    {
		cntA0 = cntA0 + 1;
		state = reset;
	    }
	    else
	    {
		state = wait;
	    }
	    break;
	case reset_wait:
	    if ((~PINA & 0x03) == 0x00)
		    state = wait;
	    else
		    state = reset_wait;
	    break;
	case inc:
	    state = inc_wait;
	    break;
	case dec:
	    state = dec_wait;
	    break;
	case reset:
	    state = reset_wait;
	    break;

        default:
	    state = smstart;
	    break;
    }

    switch(state)
    {
        case smstart:
	    PORTB = 0x07;
	    break;
        case wait:
	    waitA0 = 0;
	    waitA1 = 0;
	    break;
	case reset_wait:
        case inc_wait:
	    //++waitA0;
	    //if (waitA0 > 10)
		    //++actualB;
	    //waitA0 = 0;
	    //PORTB = actualB;
	    break;
        case dec_wait:
	    //++waitA1;
	    //if (waitA1 > 10)
		    //--actualB;
	    //waitA1 = 0;
	    //PORTB = actualB;
	    break;
        case inc:
	    waitA0 = 1;
            actualB = actualB + 1;
	    PORTB = actualB;
            break;
	case dec:
	    waitA1 = 1;
	    actualB = actualB - 1;
	    PORTB = actualB;
	    break;
	case reset:
	    PORTB = 0x00;
	    break;
        default:
            break;
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; // set portA to input
    PORTA = 0xFF; // init port A to 1s
    DDRB = 0xFF; // set portB to output
    PORTB = 0x00; // init port B to 0s
    TimerSet (100);
    TimerOn();
    state = smstart;
    
    //unsigned char tmpB = 0x00;
    /* Insert your solution below */
    while (1) {
	//tmpB = ~tmpB; // toggle PORTB; Temporary, bad programming style
	//PORTB = tmpB;
	TickSM();
	while (!TimerFlag); // wait 1 sec
	TimerFlag = 0;
	//Note: for the above a better style would use a synchSM with TickSM()
	//This example just illustrates the use of the ISR and Flag
    }
    return 1;
}
