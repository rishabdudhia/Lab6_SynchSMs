/*	Author: Rishab Dudhia
 *  Partner(s) Name: 
 *	Lab Section: 022
 *	Assignment: Lab #6  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *	b0 then b1 then b2 each for one second
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

enum SM_States {SM_Start, zero, one, two} state;

void TickSM(){
	switch(state) {
		case SM_Start:
			state = zero;
			break;
		case zero:
			state = one;
			break;
		case one:
			state = two;
			break;
		case two:
			state = zero;
			break;
		default:
			state = SM_Start;
			break;
	}

	switch (state) {
		case SM_Start:
		case zero:
			PORTB = 0x01;
			break;
		case one:
			PORTB = 0x02;
			break;
		case two:
			PORTB = 0x04;
			break;
		default:
			break;
	}
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0xFF; // set portB to output
    PORTB = 0x00; // init port B to 0s
    TimerSet (1000);
    TimerOn();
    state = SM_Start;
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
