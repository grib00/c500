#include <avr/io.h>
#include <util/delay.h>
 
int main(void) {
	DDRD = 0b01100000; // LED Ports are in output mode
	PORTD = 0b10000000; // Enable pull-up on PD7
	
	PORTD ^= (1 << PD5);
	while (1) {
		PORTD ^= (1 << PD5);
		PORTD ^= (1 << PD6);
		if (PIND & (1 << PD7)) _delay_ms(1000);
		else _delay_ms(250); // blink faster when button pressed
	}
	
	return 0;
}
