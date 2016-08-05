#include <avr/io.h>
#include <util/delay.h>

#define PD_ENCODER_0 PD0
#define PD_ENCODER_1 PD1
#define PD_LED_BLUE PD5
#define PD_LED_RED PD6
#define PD_SWITCH PD7

// ========== Serial I/O ==========

#define SERIAL_IN_READY (UCSR1A & (1 << RXC1))
#define SERIAL_OUT_READY (UCSR1A & (1 << UDRE1))
#define SERIAL_IN UDR1
#define SERIAL_OUT UDR1
#define SERIAL_BAUDRATE 31250
#define SERIAL_BAUD_PRESCALE (((F_CPU / (SERIAL_BAUDRATE * 16UL))) - 1)

inline void serial_init()  {
	// Sets baud rate
	UBRR1H = (unsigned char) (SERIAL_BAUD_PRESCALE >> 8);
	UBRR1L = (unsigned char) SERIAL_BAUD_PRESCALE;
	// Turns on the transmission and reception circuitry
	UCSR1B = (1 << RXEN1) | (1 << TXEN1);
}

inline void serial_put(uint8_t byte)  {
	while (!SERIAL_OUT_READY);
	SERIAL_OUT = byte;
}

// ========== Encoders ==========

static uint8_t state_up = 3;
static uint8_t state_down = 3;
static uint8_t next_state_up[4] = {2, 0, 3, 1}; // sequence 3102
static uint8_t next_state_down[4] = {1, 3, 0, 2}; // sequence 3201

int8_t scan_encoder() {
	int8_t r = 0;
	uint8_t in = PIND & ((1 << PD_ENCODER_0) | (1 << PD_ENCODER_1));
	if (in != state_up) {
		uint8_t next = next_state_up[state_up];
		if (in == next) {
			if (state_up == 0) {
				r = 1;
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
				r = -1;
			}
			state_down = in;
		} else if (in == 3) {
			state_down = in;
		}
	}
	return r;
}

// ========== MIDI functions ==========

#define NOTE_OFF 0x80
#define NOTE_ON 0x90
#define CHANNEL_OUT 0

#define midi_out serial_put

static int8_t transpose = 0;

void play_note(uint8_t note) {
	midi_out(NOTE_ON | CHANNEL_OUT);
	midi_out(note);
	midi_out(64);
	// _delay_ms(100);
	for (uint16_t i = 0; i < 10000; i++) {
		transpose += scan_encoder();
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
		play_note(tune[i] + transpose);
		i++;
		if (i >= tune_size) {
			i = 0;
			PORTD ^= (1 << PD_LED_BLUE);
		}
	}
}

void change_parameter(uint8_t p, uint8_t v) {
	midi_out(0xF0);
	midi_out(0x43);
	midi_out(0x10);
	midi_out(0x29);
	midi_out(0x08);
	midi_out(0x00);

	midi_out(0x00);
	midi_out(p);

	midi_out(0x00);
	midi_out(v);

	midi_out(0xF7);
}

// ========== Main ==========

int main(void) {
	DDRD = (1 << PD_LED_BLUE) | (1 << PD_LED_RED); // LED Ports are in output mode
	PORTD = (1 << PD_SWITCH) | (1 << PD_ENCODER_1) |  (1 << PD_ENCODER_0); // Enables pull-up on inputs
	serial_init();
	PORTD ^= (1 << PD_LED_RED); // switch off both LEDs
	PORTD ^= (1 << PD_LED_BLUE);

	while (1) {
		if (SERIAL_IN_READY) {
			PORTD ^= (1 << PD_LED_RED);
			uint8_t byte = SERIAL_IN;
			while (!SERIAL_OUT_READY);
			SERIAL_OUT = byte;
			PORTD ^= (1 << PD_LED_RED);
		}
		int8_t enc_delta = scan_encoder();
		if (enc_delta != 0) {
			if (enc_delta > 0) {
				PORTD ^= (1 << PD_LED_BLUE);
				change_parameter(0x09, 0x7F);
				change_parameter(0x09, 0x00);
			} else {
				PORTD ^= (1 << PD_LED_RED);
				change_parameter(0x08, 0x7F);
				change_parameter(0x08, 0x00);
			}
		}
	};
	return 0;
}
