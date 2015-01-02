#include <avr/io.h>
#include "m_general.h"
int main(void)
{
	clear(DDRD, 4); //set port D4 to input
	clear(DDRD, 5); //set port D5 to input
	clear(DDRD, 6); //set port D6 to input
	clear(DDRD, 7); //set port D7 to input
	
	//STEP 1: Set Voltage Reference
	//Vcc
	clear(ADMUX, REFS1); //0
	set(ADMUX, REFS0); //1

	//STEP 2: Set ADC Prescaler
	//set ADC prescaler to /32
	set(ADCSRA, ADPS2); // 1
	clear(ADCSRA, ADPS1); // 0
	clear(ADCSRA, ADPS0); // 0
	
	//STEP 3: Disabling Digital Inputs
	set(DIDR0, ADC0D);
	
	//STEP 4: Interrupts and Triggering
	set(ADCSRA, ADATE);
	//Setting F0 to input
	clear(DDRF, 0);
	
	//STEP 5: Single-Ended Channel Selection
	//PORT ADC0
	//PIN F0
	clear(ADCSRB, MUX5); // 0
	clear(ADMUX, MUX2); // 0
	clear(ADMUX, MUX1); // 0
	clear(ADMUX, MUX0); // 0
	
	//STEP 6: Starting Conversion Process
	set(ADCSRA, ADEN); //enables the ADC subsystem
	set(ADCSRA, ADSC); //begin conversion
	//resetting the ADCSRA flag
	set(ADCSRA, ADIF);
	
	//Clock Source
	//set prescaler to /64
	clear(TCCR1B, CS12); // 0
	set(TCCR1B, CS11); // 1
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
	//Output B6 for LED m
	set(DDRB, 6);
	
	while(1){
		//initializing int variables
		int switch1; //x8 bit
		int switch2; //x4 bit
		int switch3; //x2 bit
		int switch4; //x1 bit
		int total; //total binary value

		//OCR1A is determined by ADC
		//0V corresponds to 30MHz
		//5V corresponds to 0.5MHz
		OCR1A = 1000 + (60 * ADC);
		//DIP Switch 1 Reading
		if(check(PIND, 7)){
			switch1 = 8;
		}
		else{
			switch1 = 0;
		}
		//DIP Switch 2 Reading
		if(check(PIND, 6)){
			switch2 = 4;
		}
		else{
			switch2 = 0;
		}
		//DIP Switch 3 Reading
		if(check(PIND, 5)){
			switch3 = 2;
		}
		else{
			switch3 = 0;
		}
		//DIP Switch 4 Reading
		if(check(PIND, 4)){
			switch4 = 1;
		}
		else{
			switch4 = 0;
		}
		//converting binary to int
		total = switch1 + switch2 + switch3 + switch4;
		//OCR1B is determined by binary value of DIP switches
		//0000 corresponds to low duty cycle
		//1111 corresponds to high duty cycle
		OCR1B = total * (OCR1A/15);
	}
}