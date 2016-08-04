#include <avr/io.h>
#include <util/delay.h>

#define PD_ENCODER_0 PD0
#define PD_ENCODER_1 PD1
#define PD_LED_BLUE PD5
#define PD_LED_RED PD6
#define PD_SWITCH PD7

// ========== Serial I/O ==========

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

// ========== Encoders ==========

static uint8_t state_up = 3;
static uint8_t state_down = 3;
static uint8_t next_state_up[4] = {2, 0, 3, 1}; // sequence 3102
static uint8_t next_state_down[4] = {1, 3, 0, 2}; // sequence 3201

static int8_t encoder_value = 0;

void do_encoder() {
	uint8_t in = PIND & ((1 << PD_ENCODER_0) | (1 << PD_ENCODER_1));
	if (in != state_up) {
		uint8_t next = next_state_up[state_up];
		if (in == next) {
			if (state_up == 0) {
				encoder_value++;
			}
			state_up = in;
		} else if (in == 3) {
			state_up = in;
		}
	}
	if (in != state_down) {
		uint8_t next = next_state_down[state_down];
		if (in == next) {
			if (state_down == 0) {
				encoder_value--;
			}
			state_down = in;
		} else if (in == 3) {
			state_down = in;
		}
	}
}

// ========== MIDI functions ==========

#define NOTE_OFF 0x80
#define NOTE_ON 0x90
#define CHANNEL_OUT 0

#define midi_out serial_put

void play_note(uint8_t note) {
	midi_out(NOTE_ON | CHANNEL_OUT);
	midi_out(note);
	midi_out(64);
	// _delay_ms(100);
	for (uint16_t i = 0; i < 10000; i++) {
		do_encoder();
	}
	midi_out(note);
	midi_out(0);
}

#define tune_size 4
static uint8_t tune[tune_size] = { 65, 67, 69, 70 };

void play_tune() {
	uint8_t i = 0;
	while(1) {
		PORTD ^= (1 << PD_LED_RED);
		play_note(tune[i] + encoder_value);
		i++;
		if (i >= tune_size) {
			i = 0;
			PORTD ^= (1 << PD_LED_BLUE);
		}
	}
}

// ========== Main ==========

int main(void) {

	serial_init();

	DDRD = (1 << PD_LED_BLUE) | (1 << PD_LED_RED); // LED Ports are in output mode
	PORTD = (1 << PD_SWITCH) | (1 << PD_ENCODER_1) |  (1 << PD_ENCODER_0); // Enables pull-up on inputs

	PORTD ^= (1 << PD_LED_RED); // switch off both LEDs
	PORTD ^= (1 << PD_LED_BLUE);

	PORTD ^= (1 << PD_LED_BLUE);
	_delay_ms(1000);
	PORTD ^= (1 << PD_LED_BLUE);

	play_tune();

	return 0;
}
