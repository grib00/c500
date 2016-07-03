#include <avr/io.h>
#include <util/delay.h>

int main(void) {
	uint16_t baud = (F_CPU / (16UL * 31250)) - 1;
	UBRR1H = (unsigned char) (baud >> 8);
	UBRR1L = (unsigned char) baud;
	UCSR1B = (1 << RXEN1) | (1 << TXEN1);

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

