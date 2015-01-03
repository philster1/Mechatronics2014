#include <avr/io.h>
#include "m_general.h"
#include "m_bus.h"
#include "m_rf.h"
#define CHANNEL 1
#define RXADDRESS 0x2B
#define PACKET_LENGTH 3

long durationCompare = 0;
char buffer[PACKET_LENGTH] = {0,0,0};
int sinetable[100] = {50,53,56,59,62,65,68,71,
73,76,79,81,83,86,88,90,
91,93,94,96,97,97,98,99,
99,99,99,99,98,97,97,96,
94,93,91,90,88,86,83,81,
79,76,73,71,68,65,62,59,
56,53,50,46,43,40,37,34,
31,28,26,23,20,18,16,13,
11,9,8,6,5,3,2,2,
1,0,0,0,0,0,1,2,
2,3,5,6,8,9,11,13,
16,18,20,23,26,28,31,34,
37,40,43,46,};

void init(void);

int main(void){
	//use full clockspeed of 16 MHz
	m_clockdivide(0);
	
	//call subroutine(s)
	init(); // enable timers 1 and timer 3
	m_bus_init(); //enable mBUS
	m_rf_open(CHANNEL, RXADDRESS, PACKET_LENGTH);
	
	//set timer variables
	OCR1A = 100;
	//set interrupt for timer
	set(TIMSK1, OCIE0B);
	sei();

	while(1){
		//reference first and second byte for frequency
		int frequencyx10 = *(int*)&buffer[0];
		float frequency = frequencyx10/10;
		
		//reference the last byte for duration
		long duration = buffer[2] * 15000;
		
		//equation for frequency
		ICR3 = 27000.0 - (85 * frequency);
		
		//controlling the time that pin B6 outputs the frequency
		if(durationCompare < duration){
			//set B6 for output
			set(DDRB, 6);
		}
		if (durationCompare >= duration){
			clear(DDRB, 6);
		}
		//see the position of the timer relative to the set period
		float timerPosition = TCNT3*1.0;
		float period = ICR3*1.0;
		int sineTableVal = timerPosition/period*100;
		//find the place in the sinetable to determine duty cycle
		OCR1B = sinetable[sineTableVal];
	}
}

void init(void){
	//#######TIMER 1###########
	//Clock Source
	//set prescaler to /1
	clear(TCCR1B, CS12); // 0
	clear(TCCR1B, CS11); // 0
	set(TCCR1B, CS10); // 1
	//TIMER MODE 15
	//UP to OCR1A, PWM Mode
	//Relation is clock/OCR1A
	set(TCCR1B, WGM13); // 1
	set(TCCR1B, WGM12); // 1
	set(TCCR1A, WGM11); // 1
	set(TCCR1A, WGM10); // 1
	//Channel B Compare Output (OC1B) Option
	//Works for MODE 5, 7, 14, or 15
	//clear at OCR1B, set at rollover
	set(TCCR1A, COM1B1); // 1
	clear(TCCR1A, COM1B0); // 0
	//#######TIMER 3###########
	//Clock Source
	//set prescaler to /1
	clear(TCCR3B, CS32); // 0
	clear(TCCR3B, CS31); // 1
	set(TCCR3B, CS30); // 1
	//TIMER MODE 12
	//UP to ICR3
	set(TCCR3B, WGM33); // 0
	set(TCCR3B, WGM32); // 0
	set(TCCR3B, WGM31); // 0
	clear(TCCR3B, WGM30); // 0
}

//Interrupt if mBUS receives signal
ISR(INT2_vect){
	m_rf_read(buffer,PACKET_LENGTH);
	m_green(TOGGLE); //toggle if packet received
	durationCompare = 0; //setting to zero every time packet received
}
ISR(TIMER1_COMPB_vect){
	durationCompare++;
}
