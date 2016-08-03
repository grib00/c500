#include <avr/io.h>
#include <util/delay.h>

#define NOTE_OFF 0x80
#define NOTE_ON 0x90

#define CHANNEL_OUT 0

inline void serial_init()  {
	// Sets baud rate
#define USART_BAUDRATE 31250
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
	UBRR1H = (unsigned char) (BAUD_PRESCALE >> 8);
	UBRR1L = (unsigned char) BAUD_PRESCALE;
	// Turns on the transmission and reception circuitry
	UCSR1B = /*(1 << RXEN1) |*/ (1 << TXEN1);
	// UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);
}

inline void serial_put(uint8_t byte)  {
	while ((UCSR1A & (1 << UDRE1)) == 0); // Waits until UDR is ready
	UDR1 = byte; // Sends out the byte value
}

#define midi_out serial_put

void play_note(uint8_t note) {
	midi_out(NOTE_ON | CHANNEL_OUT);
	midi_out(note);
	midi_out(64);
	_delay_ms(100);
	midi_out(note);
	midi_out(0);
}

#define tune_size 18
static uint8_t tune[tune_size] = {
	65, 67, 56, 32, 70, 71, 48, 66, 62, 35,
	70, 66, 48, 65, 70, 64, 70, 47
};

void play_tune() {
	for (;;) {
		PORTD ^= (1 << PD5);
		for (int i = 0; i < tune_size; i++) {
			PORTD ^= (1 << PD6);
			play_note(tune[i]);
		}
	}
}

int main(void) {

	// ========== Set-up ==========

	serial_init();

	DDRD = (1 << PD6) | (1 << PD5); // LED Ports are in output mode
	PORTD = (1 << PD7); // Enables pull-up on PD7
	PORTD ^= (1 << PD5);

	// ========== Loop ==========

	while (1) {

		// serial_put(0);
		// serial_put(0xFF);
		// serial_put(0b10101010);
		// _delay_ms(0.5);
		// if (PIND & (1 << PD7)) _delay_ms(1000);
		// else _delay_ms(250); // blink faster when button pressed

		play_tune();
	}

	return 0;
}
