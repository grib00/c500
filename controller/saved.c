#include <avr/io.h>
#include <util/delay.h>

#define PD_LED_BLUE PD5
#define PD_LED_RED PD6
#define PD_SWITCH PD7
#define PD_ENCODER_0 PD0
#define PD_ENCODER_1 PD1

#define NOTE_OFF 0x80
#define NOTE_ON 0x9
#define CHANNEL_OUT 0

inline void ports_init()  {
	DDRD = (1 << PD_LED_RED) | (1 << PD_LED_BLUE); // LED Ports are in output mode
	PORTD = (1 << PD_SWITCH) | (1 << PD_ENCODER_1) |  (1 << PD_ENCODER_0); // Enables pull-up on inputs
	PORTD ^= (1 << PD_LED_BLUE);
}

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

static uint8_t state_up = 3;
static uint8_t state_down = 3;
static uint8_t next_state_up[4] = {2, 0, 3, 1}; // sequence 3102
static uint8_t next_state_down[4] = {1, 3, 0, 2}; // sequence 3201

void do_encoder() {
	uint8_t in = PIND & ((1 << PD_ENCODER_1) | (1 << PD_ENCODER_0));
	if (in != state_up) {
		uint8_t expected = next_state_up[state_up];
		if (in == expected) {
			if (state_up == 0) {
				// action here
			}
			state_up = in;
		} else if (in == 3) {
			state_up = in;
		}
	}
}

#define midi_out serial_put

void wait() {
	for (uint16_t i = 0; i < 20000; i++) {
		do_encoder();
	}
}

void play_note(uint8_t note) {
	midi_out(NOTE_ON | CHANNEL_OUT);
	midi_out(note);
	midi_out(64);
	// wait();
	_delay_ms(100);
	midi_out(NOTE_OFF | CHANNEL_OUT);
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
		PORTD ^= (1 << PD_LED_BLUE);
		for (int i = 0; i < tune_size; i++) {
			PORTD ^= (1 << PD_LED_RED);
			play_note(tune[i]);
		}
	}
}


inline void loop() {
	// serial_put(0);
	// serial_put(0xFF);
	// serial_put(0b10101010);
	// _delay_ms(0.5);
	// if (PIND & (1 << PD_SWITCH)) _delay_ms(1000);
	// else _delay_ms(250); // blink faster when button pressed

	play_tune();
}

int main() {
	ports_init();
	serial_init();
	while (1) {
		loop();
	}
	return 0;
}
