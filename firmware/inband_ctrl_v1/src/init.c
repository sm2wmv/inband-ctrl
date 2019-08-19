#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

//! Used for timer compare to match 1 ms
#define OCR0_1MS 230

/*! Initialize timer0 to use the main crystal clock and the output
  * compare interrupt feature to generate an interrupt approximately
  * once per millisecond to use as a general purpose time base. */
void init_timer_0(void) {
   TCCR0 = 0;
   TIMSK |= (1<<OCIE0);         /* enable output compare interrupt */
   TCCR0  = (1<<WGM01)|(1<<CS02)|(0<<CS01)|(0<<CS00); /* CTC, prescale = 1024 */
   TCNT0  = 0;
   OCR0   = OCR0_1MS;                     /* match in aprox 1 ms,  */
}

void init_timer_2(void) {
	TCCR2 = 0;
	TCNT2 = 0;
	TCCR2 = (1<<WGM21) | (0<<WGM20) | (0<<CS22) | (1<<CS21) | (1<<CS20); //Normal operation, toggle on compare, prescale clk/64
	TIFR |= (1<<OCF2);
	OCR2 = 30;	//Will trigger an interrupt each with an interval of 130us
	TIMSK |= (1<<OCIE2);
}

/*!Set the direction of the ports
*/
void init_ports(void) {
	DDRA = 0xFF;
	DDRB = 0xF7;
	DDRC = 0xFF;
	DDRD = 0xFB;
	DDRE = 0x00;
	DDRF = 0x00;
	DDRG = 0x00;

	PORTD |= (1<<2);
	PORTD |= (1<<3);

  PORTB &= ~(1<<4) | ~(1<<5) | ~(1<<6) | ~(1<<7);
  PORTC = 0x00;
  PORTD &= ~(1<<4) | ~(1<<5) | ~(1<<6) | ~(1<<7);
  
  PORTG = (1<<0) | (1<<1);
}
